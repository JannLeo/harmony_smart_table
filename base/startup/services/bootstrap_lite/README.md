# bootstrap\_readme

# 项目介绍<a name="ZH-CN_TOPIC_0000001054692064"></a>

## 简介<a name="section469617221261"></a>

bootstrap启动服务模块：

支持使用LiteOS-M内核的平台，当前包括：Hi3861平台。

提供了各服务和功能的启动入口标识。在SAMGR启动时，会调用boostrap标识的入口函数，并启动系统服务。

## 目录<a name="section1464106163817"></a>

**表 1**  bootstrap启动服务模块目录结构

<a name="table2977131081412"></a>
<table><thead align="left"><tr id="row7977610131417"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p18792459121314"><a name="p18792459121314"></a><a name="p18792459121314"></a>名称</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p77921459191317"><a name="p77921459191317"></a><a name="p77921459191317"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row17977171010144"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1823814275563"><a name="p1823814275563"></a><a name="p1823814275563"></a>base/startup/services/bootstrap_lite</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p15238152713562"><a name="p15238152713562"></a><a name="p15238152713562"></a>服务启动模块，启动系统核心服务外的其他服务。</p>
<p id="p101610019353"><a name="p101610019353"></a><a name="p101610019353"></a>Hi3861</p>
</td>
</tr>
</tbody>
</table>

## 使用<a name="section1483211215513"></a>

bootstrap模块无需单独配置，在SAMGR启动时会自动调用，用于启动系统服务。

## 涉及仓<a name="section134824191082"></a>

startup\_services\_bootstrap\_lite



