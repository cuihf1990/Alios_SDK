# MQTT 压测使用说明

## 原理
---
1. java客户端
- java客户端定时向`topic1`发布消息
- java客户端订阅`topic2`

2. 设备端
- 设备端订阅`topic3`
- 收到`topic3`后将payload发布的到`topic4`
- 设备端不主动发送消息

3. 规则引擎
- 将`topic1`转发到`topic3`
- 将`topic4`转发到`topic2`

4. 完整链路：
- java端`topic1`---->设备端`topic3`,设备端`topic4`--->java端`topic2`
- java端能够统计发送的`topic1`和订阅的`topic2`数量是否相等

## Usage
---
### 设备端
- linuxhost: `$ mqttest@linuxhost`
- 开发板: 烧录 `mqttest@mk3060.bin`

### java客户端
转到[这儿](http://gitlab.alibaba-inc.com/shaofa.lsf/iotx-sdk-c-test/blob/master/stress/readme.md)