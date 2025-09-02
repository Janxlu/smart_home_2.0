package com.example.mqttdemo.mqttdemo.service;

import lombok.extern.slf4j.Slf4j;
import org.springframework.integration.mqtt.outbound.MqttPahoMessageHandler;
import org.springframework.integration.support.MessageBuilder;
import org.springframework.messaging.Message;
import org.springframework.stereotype.Service;

import javax.annotation.Resource;

@Slf4j
@Service
public class MqttMessageSender {

    // 按名称注入MQTT消息处理器
    @Resource(name = "mqttOutboundHandler")
    private MqttPahoMessageHandler mqttHandler;

    /**
     * 发送MQTT消息
     * @param topic 主题
     * @param payload 消息内容
     */
    public void sendMessage(String topic, String payload) {
        // 主题校验
        if (topic == null || topic.trim().isEmpty()) {
            log.error("MQTT发送失败：主题不能为空");
            return;
        }

        // 处理空消息
        if (payload == null) {
            payload = "";
            log.warn("发送空消息到主题：{}", topic);
        }

        try {
            // 构造消息（包含 payload 和 header）
            Message<String> message = MessageBuilder
                    .withPayload(payload)
                    .setHeader("mqtt_topic", topic)  // 指定发送主题
                    .setHeader("mqtt_qos", 1)         // 设置消息质量
                    .build();

            // 发送消息
            mqttHandler.handleMessage(message);
            log.info("MQTT消息发送成功，主题[{}]：{}", topic, payload);
        } catch (Exception e) {
            log.error("MQTT消息发送失败，主题[{}]", topic, e);
        }
    }
}
