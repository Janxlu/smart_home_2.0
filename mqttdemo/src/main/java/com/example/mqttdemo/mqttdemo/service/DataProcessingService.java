package com.example.mqttdemo.mqttdemo.service;

import com.example.mqttdemo.mqttdemo.entity.DeviceStatus;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

@Slf4j
@Service
public class DataProcessingService {

    /**
     * 处理设备数据并生成反馈
     * @param deviceData 设备发送的数据
     * @return 处理结果反馈
     */
    public String process(DeviceStatus deviceData) {
        // 1. 模拟数据采集（实际场景可存储到数据库）
        log.info("采集到设备数据：{}", deviceData);

        // 2. 根据设备类型和状态生成反馈（示例逻辑）
        String feedback;
        switch (deviceData.getType()) {
            case "sensor":
                // 传感器数据（如温湿度）
                feedback = String.format(
                        "传感器[%s]数据已接收，状态：%s，处理时间：%s",
                        deviceData.getDeviceId(),
                        deviceData.getStatus(),
                        LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss"))
                );
                break;
            case "light":
                // 灯光设备
                feedback = String.format(
                        "灯光[%s]已%s，处理时间：%s",
                        deviceData.getDeviceId(),
                        deviceData.getStatus().equals("on") ? "开启" : "关闭",
                        LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss"))
                );
                break;
            default:
                feedback = "未知设备类型，数据已接收";
        }

        return feedback;
    }
}
