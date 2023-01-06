# Hiview介绍<a name="ZH-CN_TOPIC_0000001054063172"></a>

## 简介<a name="section469617221261"></a>

提供DFX子系统整体的初始化功能，控制各组件按需启动

## 架构<a name="section15884114210197"></a>

**图 1**  Hivew架构流程图<a name="fig1524205533518"></a>  
![](http://tools.harmonyos.com/mirrors/hpm-image/hiview_README/figures/Hivew架构流程图.png "Hivew架构流程图")

DFX子系统初始化分为两个阶段：DEVICE\_INIT、CORE\_INIT。

DEVICE\_INIT阶段：

1\)   初始化config模块，初始化DFX子系统核心配置参数（配置开启关闭dump\\event功能）。

2\)   初始化log组件，不能涉及内存动态分配、文件操作能力。

3\)   记录当前DFX子系统的状态信息到config中。

CORE\_INIT阶段：

1\)   根据config的参数，按需初始化log、dump、event及对应的output组件。

2\)   该阶段内存管理、文件系统已经正常启动，可按需申请内存并创建文件。

静态预编译组件化：

编译子系统提供统一的编译参数控制入口，DFX子系统根据配置后的预编译参数控制各组件是否参与编译，实现各组件预编译控制。

