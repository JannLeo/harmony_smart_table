# 项目介绍<a name="ZH-CN_TOPIC_0000001054181923"></a>

## 简介<a name="section469617221261"></a>

IoT外设控制模块提供对外围设备的操作能力。

本模块提供如下外围设备操作接口：ADC, AT, FLASH, GPIO, I2C, I2S, PARTITION, PWM, SDIO, UART, WATCHDOG等。

## 目录<a name="section1290615452227"></a>

**表 1**  IoT外设控制模块源代码目录结构

<a name="table1090712457224"></a>
<table><thead align="left"><tr id="row490711456228"><th class="cellrowborder" valign="top" width="60.07%" id="mcps1.2.3.1.1"><p id="p17907145122212"><a name="p17907145122212"></a><a name="p17907145122212"></a>名称</p>
</th>
<th class="cellrowborder" valign="top" width="39.93%" id="mcps1.2.3.1.2"><p id="p20907184572213"><a name="p20907184572213"></a><a name="p20907184572213"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row109071345102214"><td class="cellrowborder" valign="top" width="60.07%" headers="mcps1.2.3.1.1 "><p id="p199072452223"><a name="p199072452223"></a><a name="p199072452223"></a>base/iot_hardware/interfaces/kits/wifiiot_lite</p>
</td>
<td class="cellrowborder" valign="top" width="39.93%" headers="mcps1.2.3.1.2 "><p id="p0907154512215"><a name="p0907154512215"></a><a name="p0907154512215"></a>IoT外设控制模块接口</p>
</td>
</tr>
<tr id="row190716454221"><td class="cellrowborder" valign="top" width="60.07%" headers="mcps1.2.3.1.1 "><p id="p15907245122216"><a name="p15907245122216"></a><a name="p15907245122216"></a>base/iot_hardware/frameworks/wifiiot_lite/src</p>
</td>
<td class="cellrowborder" valign="top" width="39.93%" headers="mcps1.2.3.1.2 "><p id="p0907545142217"><a name="p0907545142217"></a><a name="p0907545142217"></a>IoT外设控制模块实现</p>
</td>
</tr>
<tr id="row179071245122218"><td class="cellrowborder" valign="top" width="60.07%" headers="mcps1.2.3.1.1 "><p id="p7907845112212"><a name="p7907845112212"></a><a name="p7907845112212"></a>base/iot_hardware/hals/wifiiot_lite</p>
</td>
<td class="cellrowborder" valign="top" width="39.93%" headers="mcps1.2.3.1.2 "><p id="p189071445192218"><a name="p189071445192218"></a><a name="p189071445192218"></a>HAL适配层接口</p>
</td>
</tr>
<tr id="row159076459221"><td class="cellrowborder" valign="top" width="60.07%" headers="mcps1.2.3.1.1 "><p id="p29072045132219"><a name="p29072045132219"></a><a name="p29072045132219"></a>vendor/hisi/hi3861/hi3861_adapter/hals/iot_hardware/wifiiot_lite</p>
</td>
<td class="cellrowborder" valign="top" width="39.93%" headers="mcps1.2.3.1.2 "><p id="p190854562216"><a name="p190854562216"></a><a name="p190854562216"></a>HAL适配层接口实现</p>
</td>
</tr>
</tbody>
</table>

## 约束<a name="section1718733212019"></a>

IoT外设控制模块使用C语言编写，目前仅支持Hi3861开发板。

