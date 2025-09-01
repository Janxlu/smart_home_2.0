package com.example.mqttdemo.mqttdemo.mqtt;

import com.example.mqttdemo.mqttdemo.service.MqttMessageSender;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

@Component
public class MqttDeviceSimulator {

    @Autowired
    private MqttMessageSender mqttSender;

    // 定时发送模拟数据（每5秒一次，需要开启定时任务）
    @Scheduled(fixedRate = 5000)
    public void simulateDeviceData() {
        // 模拟温湿度数据（例如卧室传感器）
        String temperature = String.format("%.1f", 25 + Math.random() * 5); // 25-30℃
        String humidity = String.format("%.1f", 40 + Math.random() * 20); // 40-60%
        // 构造JSON格式数据
        String data = String.format(
                "{\"deviceId\":\"simulator_001\",\"type\":\"温湿度\",\"temp\":%s,\"humidity\":%s,\"time\":%d}",
                temperature, humidity, System.currentTimeMillis() / 1000
        );

        // 发送到MQTT主题（例如"bedroom"，与配置文件中的topic对应）
        mqttSender.sendMessage("bedroom", data);
        System.out.println("模拟硬件发送数据：" + data);
    }
}