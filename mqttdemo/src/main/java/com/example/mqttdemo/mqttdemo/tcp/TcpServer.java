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
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;

@Component
@Slf4j
public class TcpServer {

    @Value("${tcp.port:8888}")
    private int tcpPort;

    private final MqttMessageSender mqttSender;
    private EventLoopGroup bossGroup;
    private EventLoopGroup workerGroup;

    // 注入MqttMessageSender
    @Autowired
    public TcpServer(MqttMessageSender mqttSender) {
        this.mqttSender = mqttSender;
    }

    // 异步启动TCP服务
    @PostConstruct
    public void start() {
        new Thread(() -> {
            try {
                doStart();
            } catch (InterruptedException e) {
                log.error("TCP服务器启动失败", e);
                Thread.currentThread().interrupt();
            }
        }, "tcp-server-start").start();
    }

    private void doStart() throws InterruptedException {
        bossGroup = new NioEventLoopGroup();
        workerGroup = new NioEventLoopGroup();

        try {
            ServerBootstrap bootstrap = new ServerBootstrap()
                    .group(bossGroup, workerGroup)
                    .channel(NioServerSocketChannel.class)
                    .option(ChannelOption.SO_BACKLOG, 128)
                    .childOption(ChannelOption.SO_KEEPALIVE, true)
                    .childHandler(new ChannelInitializer<SocketChannel>() {
                        @Override
                        protected void initChannel(SocketChannel ch) {
                            // 手动传递mqttSender到处理器（关键修正）
                            ch.pipeline().addLast(new TcpServerHandler(mqttSender));
                        }
                    });

            ChannelFuture future = bootstrap.bind(tcpPort).sync();
            log.info("TCP服务器启动成功，端口：{}", tcpPort);
            future.channel().closeFuture().sync();
        } finally {
            workerGroup.shutdownGracefully();
            bossGroup.shutdownGracefully();
        }
    }

    // 服务停止时释放资源
    @PreDestroy
    public void stop() {
        if (bossGroup != null) bossGroup.shutdownGracefully();
        if (workerGroup != null) workerGroup.shutdownGracefully();
    }
}
