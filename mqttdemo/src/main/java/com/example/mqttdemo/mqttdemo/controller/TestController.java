package com.example.mqttdemo.mqttdemo.controller;

import com.example.mqttdemo.mqttdemo.service.MqttMessageSender;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import javax.websocket.MessageHandler;
import java.time.LocalDateTime;
import java.util.HashMap;
import java.util.Map;

@RestController // 直接返回JSON，替代@Controller+@ResponseBody
public class TestController {
    @Autowired
    private MqttMessageSender mqttMessageSender;

    @GetMapping("/send")
    public ResponseEntity<Map<String, Object>> send(
            @RequestParam(required = true) String topic,
            @RequestParam(required = true) String message) {

        // 参数校验
        if (topic == null || topic.trim().isEmpty()) {
            return ResponseEntity.badRequest().body(error("主题（topic）不能为空"));
        }
        if (message == null || message.trim().isEmpty()) {
            return ResponseEntity.badRequest().body(error("消息内容（message）不能为空"));
        }

        try {
            mqttMessageSender.getClass();
            return ResponseEntity.ok(success("消息已发送至主题：" + topic));
        } catch (Exception e) {
            return ResponseEntity.status(500).body(error("发送失败：" + e.getMessage()));
        }
    }

    // 成功响应封装
    private Map<String, Object> success(String msg) {
        Map<String, Object> res = new HashMap<>();
        res.put("success", true);
        res.put("message", msg);
        res.put("timestamp", LocalDateTime.now());
        return res;
    }

    // 错误响应封装
    private Map<String, Object> error(String msg) {
        Map<String, Object> res = new HashMap<>();
        res.put("success", false);
        res.put("message", msg);
        return res;
    }
}