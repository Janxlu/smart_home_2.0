package com.example.mqttdemo.mqttdemo;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.integration.annotation.IntegrationComponentScan;
import org.springframework.integration.config.EnableIntegration;
import org.springframework.scheduling.annotation.EnableScheduling;

@SpringBootApplication
@EnableIntegration       // 启用Spring Integration（支持MQTT集成）
@IntegrationComponentScan // 扫描Integration组件（如MQTT适配器）
@EnableScheduling        // 支持定时任务（预留扩展）
public class MqttdemoApplication {
    public static void main(String[] args) {
        SpringApplication.run(MqttdemoApplication.class, args);
        System.out.println("MQTT与TCP服务已启动，等待数据...");
    }
}