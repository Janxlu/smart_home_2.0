package com.example.mqttdemo.mqttdemo.entity;

public class DeviceStatus {
    private String deviceId;
    private String type;
    private String status;
    private long timestamp;
    // 补充温度和湿度属性
    private double temp;
    private double humidity;

    // 默认构造函数
    public DeviceStatus() {
    }

    // 全参构造函数
    public DeviceStatus(String deviceId, String type, String status, long timestamp, double temp, double humidity) {
        this.deviceId = deviceId;
        this.type = type;
        this.status = status;
        this.timestamp = timestamp;
        this.temp = temp;
        this.humidity = humidity;
    }

    public String getDeviceId() {
        return deviceId;
    }

    public void setDeviceId(String deviceId) {
        this.deviceId = deviceId;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }

    public long getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(long timestamp) {
        this.timestamp = timestamp;
    }

    // 补充温度的getter和setter实现
    public double getTemp() {
        return temp;
    }

    public void setTemp(double temp) {
        this.temp = temp;
    }

    // 补充湿度的getter和setter实现
    public double getHumidity() {
        return humidity;
    }

    public void setHumidity(double humidity) {
        this.humidity = humidity;
    }

    // 完善时间设置方法（与timestamp关联）
    public void setTime(long time) {
        this.timestamp = time;
    }

    // 重写toString方法，方便日志打印
    @Override
    public String toString() {
        return "DeviceStatus{" +
                "deviceId='" + deviceId + '\'' +
                ", type='" + type + '\'' +
                ", status='" + status + '\'' +
                ", timestamp=" + timestamp +
                ", temp=" + temp +
                ", humidity=" + humidity +
                '}';
    }
}