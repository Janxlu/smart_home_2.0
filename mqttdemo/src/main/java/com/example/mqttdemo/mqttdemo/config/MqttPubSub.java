import org.springframework.integration.support.MessageBuilder;
import org.springframework.messaging.MessageChannel;
import org.springframework.beans.factory.annotation.Autowired;
// 其他导入...

public class MqttPubSub {

    @Autowired
    private MessageChannel mqttOutboundChannel; // 注入MQTT发送通道

    private final ObjectMapper objectMapper = new ObjectMapper();
    @Autowired
    private DataProcessingService dataProcessingService;

    // 其他代码...

    @Bean
    @ServiceActivator(inputChannel = "mqttInputChannel")
    public MessageHandler inboundHandler() {
        return message -> {
            String topic = (String) message.getHeaders().get("mqtt_receivedTopic");
            String payload = (String) message.getPayload();
            log.info("收到模拟硬件的MQTT数据，主题[{}]：{}", topic, payload);

            try {
                DeviceStatus deviceData = objectMapper.readValue(payload, DeviceStatus.class);
                String feedback = dataProcessingService.process(deviceData);
                String feedbackTopic = "feedback/device/" + deviceData.getDeviceId();

                // 构造MQTT消息并发送到通道
                mqttOutboundChannel.send(
                        MessageBuilder
                                .withPayload(feedback)
                                .setHeader("mqtt_topic", feedbackTopic)
                                .build()
                );
                log.info("已发送反馈到主题[{}]：{}", feedbackTopic, feedback);

            } catch (Exception e) {
                log.error("处理MQTT数据失败", e);
            }
        };
    }

    // 其他代码（mqttOutbound、mqttOutboundChannel 等配置保持不变）...
}