package com.example.mqttdemo.mqttdemo.config;

import com.example.mqttdemo.mqttdemo.entity.DeviceStatus;
import com.example.mqttdemo.mqttdemo.service.DataProcessingService;
import com.fasterxml.jackson.databind.ObjectMapper;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.integration.channel.DirectChannel;
import org.springframework.integration.core.MessageProducer;
import org.springframework.integration.mqtt.core.DefaultMqttPahoClientFactory;
import org.springframework.integration.mqtt.core.MqttPahoClientFactory;
import org.springframework.integration.mqtt.inbound.MqttPahoMessageDrivenChannelAdapter;
import org.springframework.integration.mqtt.outbound.MqttPahoMessageHandler;
import org.springframework.integration.mqtt.support.DefaultPahoMessageConverter;
import org.springframework.messaging.Message;
import org.springframework.messaging.MessageChannel;
import org.springframework.messaging.MessageHandler;
import org.springframework.messaging.MessagingException;
import org.springframework.messaging.support.MessageBuilder;
import org.springframework.stereotype.Component;
import org.springframework.integration.annotation.ServiceActivator;

@Component
@Slf4j
public class MqttPubSub {

    @Value("${mqtt.broker-url:tcp://localhost:1883}")
    private String brokerUrl;

    @Value("${mqtt.client-id:smart_home_2.0}")
    private String clientId;

    @Value("${mqtt.topic.simulate-prefix:simulate/device/}")
    private String simulateTopicPrefix;

    @Value("${mqtt.topic.feedback-prefix:feedback/device/}")
    private String feedbackTopicPrefix;

    @Autowired
    private DataProcessingService dataProcessingService;

    @Autowired
    private ObjectMapper objectMapper;

    @Bean
    public MqttPahoClientFactory mqttClientFactory() {
        DefaultMqttPahoClientFactory factory = new DefaultMqttPahoClientFactory();
        return factory;
    }

    @Bean
    public MessageChannel mqttInputChannel() {
        return new DirectChannel();
    }

    @Bean
    public MessageProducer inbound() {
        MqttPahoMessageDrivenChannelAdapter adapter = new MqttPahoMessageDrivenChannelAdapter(
                brokerUrl, clientId + "_inbound", mqttClientFactory(), simulateTopicPrefix + "#");
        adapter.setConverter(new DefaultPahoMessageConverter());
        adapter.setQos(1);
        adapter.setOutputChannel(mqttInputChannel());
        return adapter;
    }

    @Bean
    @ServiceActivator(inputChannel = "mqttInputChannel")
    public MessageHandler inboundHandler() {
        return new MessageHandler() {
            @Override
            public void handleMessage(Message<?> message) throws MessagingException {
                String topic = (String) message.getHeaders().get("mqtt_receivedTopic");
                String payload = (String) message.getPayload();
                log.info("收到MQTT消息，主题[{}]：{}", topic, payload);

                try {
                    DeviceStatus deviceData = parseDeviceData(payload);
                    if (deviceData == null) return;

                    String feedback = dataProcessingService.process(deviceData);
                    sendFeedback(deviceData, feedback);

                } catch (Exception e) {
                    log.error("处理MQTT消息失败（主题：{}）", topic, e);
                }
            }
        };
    }

    @Bean
    public MessageChannel mqttOutboundChannel() {
        return new DirectChannel();
    }

    // 关键修复：将Bean名称指定为mqttOutboundHandler
    @Bean(name = "mqttOutboundHandler")
    @ServiceActivator(inputChannel = "mqttOutboundChannel")
    public MessageHandler outbound() {
        MqttPahoMessageHandler handler = new MqttPahoMessageHandler(
                brokerUrl, clientId + "_outbound", mqttClientFactory());
        handler.setAsync(true);
        handler.setDefaultQos(1);
        return handler;
    }

    private DeviceStatus parseDeviceData(String payload) {
        try {
            DeviceStatus deviceData = objectMapper.readValue(payload, DeviceStatus.class);
            if (!dataProcessingService.isValidData(deviceData)) {
                log.error("设备数据校验失败：{}", payload);
                return null;
            }
            return deviceData;
        } catch (Exception e) {
            log.error("JSON解析失败，原始数据：{}", payload, e);
            return null;
        }
    }

    private void sendFeedback(DeviceStatus deviceData, String feedback) {
        String feedbackTopic = feedbackTopicPrefix + deviceData.getDeviceId();
        Message<String> feedbackMessage = MessageBuilder
                .withPayload(feedback)
                .setHeader("mqtt_topic", feedbackTopic)
                .build();
        mqttOutboundChannel().send(feedbackMessage);
        log.info("反馈已发送，主题[{}]：{}", feedbackTopic, feedback);
    }
}