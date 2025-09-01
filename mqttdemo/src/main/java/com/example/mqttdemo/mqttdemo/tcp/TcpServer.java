package com.example.mqttdemo.mqttdemo.tcp;

import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelOption;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import com.example.mqttdemo.mqttdemo.service.MqttMessageSender;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;

@Component
public class TcpServer {

    // 从配置文件读取TCP端口（默认8888）
    @Value("${tcp.port:8888}")
    private int tcpPort;

    // 注入MQTT消息发送服务
    private final MqttMessageSender mqttSender;

    @Autowired
    public TcpServer(MqttMessageSender mqttSender) {
        this.mqttSender = mqttSender;
    }

    private EventLoopGroup bossGroup;
    private EventLoopGroup workerGroup;

    // 服务启动时自动初始化TCP服务器（Spring的PostConstruct注解）
    @PostConstruct
    public void start() throws InterruptedException {
        // 主线程组（接受连接）
        bossGroup = new NioEventLoopGroup();
        // 工作线程组（处理连接）
        workerGroup = new NioEventLoopGroup();

        try {
            ServerBootstrap bootstrap = new ServerBootstrap()
                    .group(bossGroup, workerGroup)
                    .channel(NioServerSocketChannel.class) // 使用NIO通道
                    .option(ChannelOption.SO_BACKLOG, 128) // 连接队列大小
                    .childOption(ChannelOption.SO_KEEPALIVE, true) // 保持连接
                    .childHandler(new ChannelInitializer<SocketChannel>() {
                        @Override
                        protected void initChannel(SocketChannel ch) {
                            // 添加TCP消息处理器，并传入mqttSender依赖
                            ch.pipeline().addLast(new TcpServerHandler(mqttSender));
                        }
                    });

            // 绑定端口并启动（同步阻塞等待启动完成）
            ChannelFuture future = bootstrap.bind(tcpPort).sync();
            System.out.println("TCP服务器启动成功，端口：" + tcpPort);
            // 阻塞等待通道关闭（保持服务运行）
            future.channel().closeFuture().sync();
        } finally {
            // 优雅关闭线程组
            workerGroup.shutdownGracefully();
            bossGroup.shutdownGracefully();
        }
    }

    // 服务停止时关闭资源（Spring的PreDestroy注解）
    @PreDestroy
    public void stop() {
        if (bossGroup != null) bossGroup.shutdownGracefully();
        if (workerGroup != null) workerGroup.shutdownGracefully();
    }
}