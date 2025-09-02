package com.example.mqttdemo.mqttdemo.tcp;

import com.example.mqttdemo.mqttdemo.service.MqttMessageSender;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Value;

import java.nio.charset.StandardCharsets;

@Slf4j
// 移除@Component注解（Netty处理器由代码手动创建，无需Spring管理）
public class TcpServerHandler extends ChannelInboundHandlerAdapter {

    // 声明为final，确保必须初始化
    private final MqttMessageSender mqttSender;

    @Value("${tcp.device-id.split-separator:,}")
    private String splitSeparator;

    // 构造器显式初始化mqttSender（关键修正）
    public TcpServerHandler(MqttMessageSender mqttSender) {
        this.mqttSender = mqttSender;
    }

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        try {
            ByteBuf buf = (ByteBuf) msg;
            String tcpMsg = buf.toString(StandardCharsets.UTF_8);
            log.info("收到TCP消息：{}", tcpMsg);

            // 提取设备ID并转发到MQTT
            String deviceId = extractDeviceId(tcpMsg);
            String mqttTopic = "tcp/device/" + deviceId;
            // 使用初始化后的mqttSender
            mqttSender.sendMessage(mqttTopic, tcpMsg);

            // 回复客户端
            String response = "TCP消息已转发至MQTT主题：" + mqttTopic;
            ctx.writeAndFlush(Unpooled.copiedBuffer(response, StandardCharsets.UTF_8));

        } catch (Exception e) {
            log.error("TCP消息处理失败", e);
            ctx.writeAndFlush(Unpooled.copiedBuffer("处理失败：" + e.getMessage(), StandardCharsets.UTF_8));
        }
    }

    // 从TCP消息中提取设备ID
    private String extractDeviceId(String tcpMsg) {
        try {
            if (tcpMsg == null || tcpMsg.isEmpty()) {
                return "unknown";
            }
            String[] parts = tcpMsg.split(splitSeparator);
            return parts.length > 0 ? parts[0].trim() : "unknown";
        } catch (Exception e) {
            log.warn("解析设备ID失败：{}", tcpMsg);
            return "unknown";
        }
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        log.error("TCP连接异常（客户端：{}）", ctx.channel().remoteAddress(), cause);
        ctx.close();
    }
}
