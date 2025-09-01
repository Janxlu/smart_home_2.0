package com.example.mqttdemo.mqttdemo.tcp;

import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import com.example.mqttdemo.mqttdemo.service.MqttMessageSender;

public class TcpServerHandler extends ChannelInboundHandlerAdapter {

    private final MqttMessageSender mqttSender;

    // 通过构造函数注入MqttMessageSender
    public TcpServerHandler(MqttMessageSender mqttSender) {
        this.mqttSender = mqttSender;
    }

    // 接收TCP客户端消息
    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        // 解析TCP消息（Netty默认用ByteBuf存储数据）
        ByteBuf buf = (ByteBuf) msg;
        byte[] bytes = new byte[buf.readableBytes()];
        buf.readBytes(bytes);
        String tcpMsg = new String(bytes);
        System.out.println("收到TCP消息：" + tcpMsg);

        // 转发TCP消息到MQTT（例如转发到"tcp_to_mqtt"主题）
        mqttSender.sendMessage("tcp_to_mqtt", tcpMsg);

        // 回复TCP客户端（原样返回消息）
        ctx.writeAndFlush(msg);
    }

    // 处理异常（关闭连接）
    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        cause.printStackTrace();
        ctx.close();
    }
}