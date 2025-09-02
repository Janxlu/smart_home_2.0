package com.example.mqttdemo.mqttdemo.service;

import com.example.mqttdemo.mqttdemo.entity.DeviceStatus;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.HashMap;
import java.util.Map;

@Slf4j
@Service
public class DataProcessingService {

    // 设备处理策略映射（策略模式）
    private static final Map<String, DeviceHandler> HANDLERS = new HashMap<>();

    static {
        // 注册设备处理器（新增设备类型只需添加新实现）
        HANDLERS.put("sensor", new SensorHandler());
        HANDLERS.put("light", new LightHandler());
        HANDLERS.put("door", new DoorHandler());
    }

    // 数据处理主方法
    public String process(DeviceStatus deviceData) {
        if (!isValidData(deviceData)) {
            return "数据校验失败：无效的设备信息";
        }

        log.info("已采集设备数据：{}", deviceData);

        // 调用对应设备类型的处理器
        DeviceHandler handler = HANDLERS.get(deviceData.getType().toLowerCase());
        return handler != null ? handler.handle(deviceData) :
                String.format("未知设备类型[%s]，数据已接收（ID：%s）",
                        deviceData.getType(), deviceData.getDeviceId());
    }

    // 数据有效性校验（公开方法，供MQTT解析时复用）
    public boolean isValidData(DeviceStatus data) {
        if (data == null) {
            log.error("设备数据为空");
            return false;
        }
        if (data.getDeviceId() == null || data.getDeviceId().trim().isEmpty()) {
            log.error("设备ID为空");
            return false;
        }
        if (data.getType() == null || data.getType().trim().isEmpty()) {
            log.error("设备类型为空（ID：{}）", data.getDeviceId());
            return false;
        }
        if (data.getTimestamp() <= 0) {
            log.warn("设备数据时间戳无效（ID：{}）", data.getDeviceId());
        }
        return true;
    }

    // 设备处理策略接口
    private interface DeviceHandler {
        String handle(DeviceStatus data);
    }

    // 传感器处理器
    private static class SensorHandler implements DeviceHandler {
        @Override
        public String handle(DeviceStatus data) {
            return String.format(
                    "传感器[%s]数据正常：%s，处理时间：%s",
                    data.getDeviceId(), data.getStatus(),
                    LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss"))
            );
        }
    }

    // 灯光设备处理器
    private static class LightHandler implements DeviceHandler {
        @Override
        public String handle(DeviceStatus data) {
            String statusDesc = "on".equals(data.getStatus()) ? "开启" : "关闭";
            return String.format(
                    "灯光[%s]已%s，处理时间：%s",
                    data.getDeviceId(), statusDesc,
                    LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss"))
            );
        }
    }

    // 门禁设备处理器
    private static class DoorHandler implements DeviceHandler {
        @Override
        public String handle(DeviceStatus data) {
            String statusDesc = "open".equals(data.getStatus()) ? "打开" : "关闭";
            return String.format(
                    "门禁[%s]状态：%s，处理时间：%s",
                    data.getDeviceId(), statusDesc,
                    LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss"))
            );
        }
    }
}