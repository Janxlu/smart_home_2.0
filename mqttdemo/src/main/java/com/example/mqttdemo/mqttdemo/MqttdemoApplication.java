package com.example.mqttdemo.mqttdemo;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.integration.annotation.IntegrationComponentScan;
import org.springframework.integration.config.EnableIntegration;
import org.springframework.scheduling.annotation.EnableScheduling; // 新增

@SpringBootApplication
@EnableIntegration
@IntegrationComponentScan
@EnableScheduling // 开启定时任务支持
public class MqttdemoApplication {
    public static void main(String[] args) {
        SpringApplication.run(MqttdemoApplication.class, args);
        System.out.println("MQTT与TCP服务已启动，等待数据...");
    }
}