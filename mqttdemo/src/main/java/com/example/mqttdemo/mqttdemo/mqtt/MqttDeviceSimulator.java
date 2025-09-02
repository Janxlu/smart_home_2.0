package com.example.mqttdemo.mqttdemo.mqtt;

import com.example.mqttdemo.mqttdemo.entity.DeviceStatus;
import com.example.mqttdemo.mqttdemo.service.MqttMessageSender;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.util.Random;

@Slf4j
@Component
public class MqttDeviceSimulator {

    @Autowired
    private MqttMessageSender mqttSender;

    @Autowired
    private ObjectMapper objectMapper;

    // 定义smart_home_2.0的6个分区
    private static final String[] SMART_HOME_ZONES = {
            "bedroom",
            "living_room",
            "kitchen",
            "toilet",
            "central",
            "access_control"
    };

    private final Random random = new Random();
    private int currentZoneIndex = 0; // 用于轮询切换分区

    // 每5秒发送一次模拟数据（可调整频率）
    @Scheduled(fixedRate = 5000)
    public void simulateDeviceData() {
        // 生成模拟设备数据
        DeviceStatus deviceData = new DeviceStatus();
        deviceData.setDeviceId("simulator_001");
        deviceData.setType("温湿度");
        deviceData.setTemp(20 + random.nextDouble() * 10); // 20-30℃
        deviceData.setHumidity(30 + random.nextDouble() * 50); // 30-80%
        deviceData.setTime(System.currentTimeMillis() / 1000);

        try {
            // 转换为JSON字符串
            String payload = objectMapper.writeValueAsString(deviceData);
            log.info("模拟硬件发送数据：{}", payload);

            // 方式1：轮询切换分区（依次发送到6个部分）
            String topic = SMART_HOME_ZONES[currentZoneIndex];
            currentZoneIndex = (currentZoneIndex + 1) % SMART_HOME_ZONES.length; // 循环切换

            // 方式2：随机选择分区（注释掉方式1，启用此方式）
            // int randomIndex = random.nextInt(SMART_HOME_ZONES.length);
            // String topic = SMART_HOME_ZONES[randomIndex];

            // 发送数据到选中的分区主题
            mqttSender.sendMessage(topic, payload);

        } catch (JsonProcessingException e) {
            log.error("模拟数据JSON序列化失败", e);
        }
    }
}
