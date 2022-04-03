# Hilog子系统介绍<a name="ZH-CN_TOPIC_0000001054083094"></a>

## 简介<a name="zh-cn_topic_0000001051742157_section11660541593"></a>

该仓库用于存放DFX框架的代码。主要包含DFR（可靠性）和DFT（可测试性）特性。

由于芯片平台资源有限，且硬件平台多样，因此需要针对不同硬件架构和资源提供组件化且可定制的DFX框架。根据RISC-V、Cortex-M、Cortex-A不同硬件平台，提供两种不同的轻量级DFX框架，以下简称mini、featured。

-   mini框架：针对处理架构为Cortex-M或同等处理能力的硬件平台，系统内存一般低于512KB，无文件系统或者仅提供一个可有限使用的轻量级文件系统，遵循CMSIS接口规范。

-   featured框架：处理架构为Cortex-A或同等处理能力的硬件平台，内存资源大于512KB，文件系统完善，可存储大量数据，遵循POSIX接口规范。


## 目录<a name="zh-cn_topic_0000001051742157_section1464106163817"></a>

**表 1**  轻量级DFX子系统源代码目录结构

<a name="zh-cn_topic_0000001051742157_table2977131081412"></a>
<table><thead align="left"><tr id="zh-cn_topic_0000001051742157_row7977610131417"><th class="cellrowborder" valign="top" width="30.34%" id="mcps1.2.3.1.1"><p id="zh-cn_topic_0000001051742157_p18792459121314"><a name="zh-cn_topic_0000001051742157_p18792459121314"></a><a name="zh-cn_topic_0000001051742157_p18792459121314"></a>名称</p>
</th>
<th class="cellrowborder" valign="top" width="69.66%" id="mcps1.2.3.1.2"><p id="zh-cn_topic_0000001051742157_p77921459191317"><a name="zh-cn_topic_0000001051742157_p77921459191317"></a><a name="zh-cn_topic_0000001051742157_p77921459191317"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="zh-cn_topic_0000001051742157_row17977171010144"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p2793159171311"><a name="zh-cn_topic_0000001051742157_p2793159171311"></a><a name="zh-cn_topic_0000001051742157_p2793159171311"></a>interface</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p879375920132"><a name="zh-cn_topic_0000001051742157_p879375920132"></a><a name="zh-cn_topic_0000001051742157_p879375920132"></a>存放所有对外接口定义头文件</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row259142201312"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p5197366257"><a name="zh-cn_topic_0000001051742157_p5197366257"></a><a name="zh-cn_topic_0000001051742157_p5197366257"></a>interfaces/kits/hilog</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p10406450131319"><a name="zh-cn_topic_0000001051742157_p10406450131319"></a><a name="zh-cn_topic_0000001051742157_p10406450131319"></a>featured框架流水日志对外接口定义</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row580915918401"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p9809189144014"><a name="zh-cn_topic_0000001051742157_p9809189144014"></a><a name="zh-cn_topic_0000001051742157_p9809189144014"></a>interfaces/kits/hilog_lite</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p168101694401"><a name="zh-cn_topic_0000001051742157_p168101694401"></a><a name="zh-cn_topic_0000001051742157_p168101694401"></a>mini框架流水日志对外接口定义</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row1188919458130"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p14561174816401"><a name="zh-cn_topic_0000001051742157_p14561174816401"></a><a name="zh-cn_topic_0000001051742157_p14561174816401"></a>interfaces/innerkits/hievent_lite</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p1254413131146"><a name="zh-cn_topic_0000001051742157_p1254413131146"></a><a name="zh-cn_topic_0000001051742157_p1254413131146"></a>mini框架事件打点对外接口定义</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row6978161091412"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p37931659101311"><a name="zh-cn_topic_0000001051742157_p37931659101311"></a><a name="zh-cn_topic_0000001051742157_p37931659101311"></a>services/hilogcat_lite</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p6793059171318"><a name="zh-cn_topic_0000001051742157_p6793059171318"></a><a name="zh-cn_topic_0000001051742157_p6793059171318"></a>流水日志相关服务和命令</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row6978201031415"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p1738210441049"><a name="zh-cn_topic_0000001051742157_p1738210441049"></a><a name="zh-cn_topic_0000001051742157_p1738210441049"></a>services/hilogcat_lite/apphilogcat</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p1629020401941"><a name="zh-cn_topic_0000001051742157_p1629020401941"></a><a name="zh-cn_topic_0000001051742157_p1629020401941"></a>featured框架流水日志落盘服务</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row1596814581415"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p158313363613"><a name="zh-cn_topic_0000001051742157_p158313363613"></a><a name="zh-cn_topic_0000001051742157_p158313363613"></a>services/hilogcat_lite/command</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p12969358749"><a name="zh-cn_topic_0000001051742157_p12969358749"></a><a name="zh-cn_topic_0000001051742157_p12969358749"></a>mini框架dfx命令</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row175618551244"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p73791172718"><a name="zh-cn_topic_0000001051742157_p73791172718"></a><a name="zh-cn_topic_0000001051742157_p73791172718"></a>services/hilogcat_lite/hilogcat</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p107568558416"><a name="zh-cn_topic_0000001051742157_p107568558416"></a><a name="zh-cn_topic_0000001051742157_p107568558416"></a>featured框架流水日志输出命令</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row11587111583"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p4491153104614"><a name="zh-cn_topic_0000001051742157_p4491153104614"></a><a name="zh-cn_topic_0000001051742157_p4491153104614"></a>services/hiview_lite</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p159210361388"><a name="zh-cn_topic_0000001051742157_p159210361388"></a><a name="zh-cn_topic_0000001051742157_p159210361388"></a>DFX框架服务化注册</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row144311669479"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p443219624716"><a name="zh-cn_topic_0000001051742157_p443219624716"></a><a name="zh-cn_topic_0000001051742157_p443219624716"></a>frameworks/ddrdump_lite</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p343218604718"><a name="zh-cn_topic_0000001051742157_p343218604718"></a><a name="zh-cn_topic_0000001051742157_p343218604718"></a>mini框架ddrdump信息转储</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row1125881215472"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p52581712124718"><a name="zh-cn_topic_0000001051742157_p52581712124718"></a><a name="zh-cn_topic_0000001051742157_p52581712124718"></a>frameworks/hievent_lite</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p1825921284710"><a name="zh-cn_topic_0000001051742157_p1825921284710"></a><a name="zh-cn_topic_0000001051742157_p1825921284710"></a>mini框架事件打点记录实现</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row13101195476"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p1210117994714"><a name="zh-cn_topic_0000001051742157_p1210117994714"></a><a name="zh-cn_topic_0000001051742157_p1210117994714"></a>frameworks/hilog_lite</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p910159144719"><a name="zh-cn_topic_0000001051742157_p910159144719"></a><a name="zh-cn_topic_0000001051742157_p910159144719"></a>流水日志接口实现</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row2442416175011"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p14911117105011"><a name="zh-cn_topic_0000001051742157_p14911117105011"></a><a name="zh-cn_topic_0000001051742157_p14911117105011"></a>frameworks/hilog_lite/featured</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p18491171775019"><a name="zh-cn_topic_0000001051742157_p18491171775019"></a><a name="zh-cn_topic_0000001051742157_p18491171775019"></a>featured框架流水日志接口实现</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row481417116116"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p468592018492"><a name="zh-cn_topic_0000001051742157_p468592018492"></a><a name="zh-cn_topic_0000001051742157_p468592018492"></a>frameworks/hilog_lite/mini</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p2051115253494"><a name="zh-cn_topic_0000001051742157_p2051115253494"></a><a name="zh-cn_topic_0000001051742157_p2051115253494"></a>mini框架流水日志接口实现</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row13247163492"><td class="cellrowborder" valign="top" width="30.34%" headers="mcps1.2.3.1.1 "><p id="zh-cn_topic_0000001051742157_p710851611910"><a name="zh-cn_topic_0000001051742157_p710851611910"></a><a name="zh-cn_topic_0000001051742157_p710851611910"></a>utils/lite</p>
</td>
<td class="cellrowborder" valign="top" width="69.66%" headers="mcps1.2.3.1.2 "><p id="zh-cn_topic_0000001051742157_p112471431895"><a name="zh-cn_topic_0000001051742157_p112471431895"></a><a name="zh-cn_topic_0000001051742157_p112471431895"></a>公共基础操作定义实现。包含了miini框架的config配置</p>
</td>
</tr>
</tbody>
</table>

## 约束<a name="zh-cn_topic_0000001051742157_section1718733212019"></a>

mini框架整体代码使用标准C开发，对外的接口依赖统一通过util封装，如软硬件平台不同需要适配，需要在vendor下实现适配处理。

## 使用-mini框架<a name="zh-cn_topic_0000001051742157_section99168524220"></a>

DFX-mini是一套简单小巧的DFX设计，对外提供log功能：

-   **以下以A模块为例说明如何添加模块并进行日志打印。**
    1.  **第一步添加模块ID**

        在interfaces/kits/hilog\_lite/hiview\_log.h的HiLogModuleDef中添加HILOG\_MODULE\_A定义。

        ```
        typedef enum {
            /** DFX */    
            HILOG_MODULE_HIVIEW = 0,    
            /** System Module A */    
            HILOG_MODULE_A,    
            /** Maximum number of modules */
            HILOG_MODULE_MAX
        } HiLogModuleType;
        ```


    1.  **第二步模块注册**
    
        在A模块的初始化流程中添加如下代码，注册模块到日志框架中：
    
        ```
        HiLogRegisterModule(HILOG_MODULE_SAMGR, "A");
        ```


    1.  **第三步调整DFX框架静态配置**
    
        根据需要调整
    
        ```
        utils/lite/hiview_config.c
        ```
    
        的g\_hiviewConfig全局参数配置。默认情况下不用修改，日志默认输出到串口。
    
        <a name="zh-cn_topic_0000001051742157_table15664428164516"></a>
        <table><thead align="left"><tr id="zh-cn_topic_0000001051742157_row07061028154510"><th class="cellrowborder" valign="top" width="25.180000000000003%" id="mcps1.1.3.1.1"><p id="zh-cn_topic_0000001051742157_p10706128184514"><a name="zh-cn_topic_0000001051742157_p10706128184514"></a><a name="zh-cn_topic_0000001051742157_p10706128184514"></a>配置项</p>
        </th>
        <th class="cellrowborder" valign="top" width="74.82%" id="mcps1.1.3.1.2"><p id="zh-cn_topic_0000001051742157_p11706928194520"><a name="zh-cn_topic_0000001051742157_p11706928194520"></a><a name="zh-cn_topic_0000001051742157_p11706928194520"></a>取值及含义</p>
        </th>
        </tr>
        </thead>
        <tbody><tr id="zh-cn_topic_0000001051742157_row8706828154511"><td class="cellrowborder" valign="top" width="25.180000000000003%" headers="mcps1.1.3.1.1 "><p id="zh-cn_topic_0000001051742157_p19706112824514"><a name="zh-cn_topic_0000001051742157_p19706112824514"></a><a name="zh-cn_topic_0000001051742157_p19706112824514"></a>outputOption</p>
        </td>
        <td class="cellrowborder" valign="top" width="74.82%" headers="mcps1.1.3.1.2 "><p id="zh-cn_topic_0000001051742157_p9706528104512"><a name="zh-cn_topic_0000001051742157_p9706528104512"></a><a name="zh-cn_topic_0000001051742157_p9706528104512"></a>日志的输出方式。取值如下：</p>
        <p id="zh-cn_topic_0000001051742157_p107061428124515"><a name="zh-cn_topic_0000001051742157_p107061428124515"></a><a name="zh-cn_topic_0000001051742157_p107061428124515"></a>OUTPUT_OPTION_DEBUG 日志不进行跨任务调度直接输出到串口，仅适合临时调测使用。</p>
        <p id="zh-cn_topic_0000001051742157_p870682819450"><a name="zh-cn_topic_0000001051742157_p870682819450"></a><a name="zh-cn_topic_0000001051742157_p870682819450"></a>OUTPUT_OPTION_FLOW  日志流式输出到串口（默认设置）</p>
        <p id="zh-cn_topic_0000001051742157_p16707182819454"><a name="zh-cn_topic_0000001051742157_p16707182819454"></a><a name="zh-cn_topic_0000001051742157_p16707182819454"></a>OUTPUT_OPTION_TEXT_FILE 日志输出为文本文件</p>
        <p id="zh-cn_topic_0000001051742157_p117071528184514"><a name="zh-cn_topic_0000001051742157_p117071528184514"></a><a name="zh-cn_topic_0000001051742157_p117071528184514"></a>OUTPUT_OPTION_BIN_FILE 日志输出为二进制文件（后续支持）</p>
        </td>
        </tr>
        <tr id="zh-cn_topic_0000001051742157_row1270720281453"><td class="cellrowborder" valign="top" width="25.180000000000003%" headers="mcps1.1.3.1.1 "><p id="zh-cn_topic_0000001051742157_p137071528164516"><a name="zh-cn_topic_0000001051742157_p137071528164516"></a><a name="zh-cn_topic_0000001051742157_p137071528164516"></a>level</p>
        </td>
        <td class="cellrowborder" valign="top" width="74.82%" headers="mcps1.1.3.1.2 "><p id="zh-cn_topic_0000001051742157_p177071428154510"><a name="zh-cn_topic_0000001051742157_p177071428154510"></a><a name="zh-cn_topic_0000001051742157_p177071428154510"></a>日志的输出级别，仅输出大于等于该级别的日志。取值如下：</p>
        <p id="zh-cn_topic_0000001051742157_p1470712824515"><a name="zh-cn_topic_0000001051742157_p1470712824515"></a><a name="zh-cn_topic_0000001051742157_p1470712824515"></a>HILOG_LV_DEBUG/ HILOG_LV_INFO/ HILOG_LV_WARN/ HILOG_LV_ERROR/ HILOG_LV_FATAL</p>
        </td>
        </tr>
        <tr id="zh-cn_topic_0000001051742157_row17707142814513"><td class="cellrowborder" valign="top" width="25.180000000000003%" headers="mcps1.1.3.1.1 "><p id="zh-cn_topic_0000001051742157_p11707192814517"><a name="zh-cn_topic_0000001051742157_p11707192814517"></a><a name="zh-cn_topic_0000001051742157_p11707192814517"></a>logSwitch</p>
        </td>
        <td class="cellrowborder" valign="top" width="74.82%" headers="mcps1.1.3.1.2 "><p id="zh-cn_topic_0000001051742157_p870711281452"><a name="zh-cn_topic_0000001051742157_p870711281452"></a><a name="zh-cn_topic_0000001051742157_p870711281452"></a>流水日志功能开关。编译前关闭不影响log组件初始化。默认打开。取值如下：</p>
        <p id="zh-cn_topic_0000001051742157_p1570822810456"><a name="zh-cn_topic_0000001051742157_p1570822810456"></a><a name="zh-cn_topic_0000001051742157_p1570822810456"></a>HIVIEW_FEATURE_ON/ HIVIEW_FEATURE_OFF</p>
        </td>
        </tr>
        <tr id="zh-cn_topic_0000001051742157_row6708132813453"><td class="cellrowborder" valign="top" width="25.180000000000003%" headers="mcps1.1.3.1.1 "><p id="zh-cn_topic_0000001051742157_p1570813280453"><a name="zh-cn_topic_0000001051742157_p1570813280453"></a><a name="zh-cn_topic_0000001051742157_p1570813280453"></a>dumpSwitch</p>
        </td>
        <td class="cellrowborder" valign="top" width="74.82%" headers="mcps1.1.3.1.2 "><p id="zh-cn_topic_0000001051742157_p970852810459"><a name="zh-cn_topic_0000001051742157_p970852810459"></a><a name="zh-cn_topic_0000001051742157_p970852810459"></a>dump功能开关。编译前关闭则不再初始化DUMP组件。默认关闭。取值如下：</p>
        <p id="zh-cn_topic_0000001051742157_p147081328174519"><a name="zh-cn_topic_0000001051742157_p147081328174519"></a><a name="zh-cn_topic_0000001051742157_p147081328174519"></a>HIVIEW_FEATURE_ON/ HIVIEW_FEATURE_OFF</p>
        </td>
        </tr>
        <tr id="zh-cn_topic_0000001051742157_row87081282452"><td class="cellrowborder" valign="top" width="25.180000000000003%" headers="mcps1.1.3.1.1 "><p id="zh-cn_topic_0000001051742157_p11708128124511"><a name="zh-cn_topic_0000001051742157_p11708128124511"></a><a name="zh-cn_topic_0000001051742157_p11708128124511"></a>eventSwitch</p>
        </td>
        <td class="cellrowborder" valign="top" width="74.82%" headers="mcps1.1.3.1.2 "><p id="zh-cn_topic_0000001051742157_p17708202834519"><a name="zh-cn_topic_0000001051742157_p17708202834519"></a><a name="zh-cn_topic_0000001051742157_p17708202834519"></a>事件功能开关。编译前关闭则不再初始化DUMP组件。默认关闭。取值如下：</p>
        <p id="zh-cn_topic_0000001051742157_p1670852894513"><a name="zh-cn_topic_0000001051742157_p1670852894513"></a><a name="zh-cn_topic_0000001051742157_p1670852894513"></a>HIVIEW_FEATURE_ON/ HIVIEW_FEATURE_OFF</p>
        </td>
        </tr>
        </tbody>
        </table>
    
    2.  **第四步日志打印**
    
        在需要打印日志的.c文件中 \#include "log.h"，调用如下接口：
    
        HILOG\_INFO\(HILOG\_MODULE\_SAMGR, “log test: %d”, 88\);
    
        接口参数说明：
    
        <a name="zh-cn_topic_0000001051742157_table119971217175510"></a>
        <table><thead align="left"><tr id="zh-cn_topic_0000001051742157_row1350818135519"><th class="cellrowborder" valign="top" width="11.57%" id="mcps1.1.5.1.1"><p id="zh-cn_topic_0000001051742157_p175031855513"><a name="zh-cn_topic_0000001051742157_p175031855513"></a><a name="zh-cn_topic_0000001051742157_p175031855513"></a>参数名</p>
        </th>
        <th class="cellrowborder" valign="top" width="11.83%" id="mcps1.1.5.1.2"><p id="zh-cn_topic_0000001051742157_p250131825511"><a name="zh-cn_topic_0000001051742157_p250131825511"></a><a name="zh-cn_topic_0000001051742157_p250131825511"></a>是否必填</p>
        </th>
        <th class="cellrowborder" valign="top" width="15.21%" id="mcps1.1.5.1.3"><p id="zh-cn_topic_0000001051742157_p3501418175516"><a name="zh-cn_topic_0000001051742157_p3501418175516"></a><a name="zh-cn_topic_0000001051742157_p3501418175516"></a>参数类型</p>
        </th>
        <th class="cellrowborder" valign="top" width="61.39%" id="mcps1.1.5.1.4"><p id="zh-cn_topic_0000001051742157_p12501718125512"><a name="zh-cn_topic_0000001051742157_p12501718125512"></a><a name="zh-cn_topic_0000001051742157_p12501718125512"></a>参数说明</p>
        </th>
        </tr>
        </thead>
        <tbody><tr id="zh-cn_topic_0000001051742157_row195016185559"><td class="cellrowborder" valign="top" width="11.57%" headers="mcps1.1.5.1.1 "><p id="zh-cn_topic_0000001051742157_p6503180557"><a name="zh-cn_topic_0000001051742157_p6503180557"></a><a name="zh-cn_topic_0000001051742157_p6503180557"></a>mod</p>
        </td>
        <td class="cellrowborder" valign="top" width="11.83%" headers="mcps1.1.5.1.2 "><p id="zh-cn_topic_0000001051742157_p45061835518"><a name="zh-cn_topic_0000001051742157_p45061835518"></a><a name="zh-cn_topic_0000001051742157_p45061835518"></a>是</p>
        </td>
        <td class="cellrowborder" valign="top" width="15.21%" headers="mcps1.1.5.1.3 "><p id="zh-cn_topic_0000001051742157_p1050618155511"><a name="zh-cn_topic_0000001051742157_p1050618155511"></a><a name="zh-cn_topic_0000001051742157_p1050618155511"></a>uint8</p>
        </td>
        <td class="cellrowborder" valign="top" width="61.39%" headers="mcps1.1.5.1.4 "><p id="zh-cn_topic_0000001051742157_p1051618115519"><a name="zh-cn_topic_0000001051742157_p1051618115519"></a><a name="zh-cn_topic_0000001051742157_p1051618115519"></a>模块\服务的ID。</p>
        <p id="zh-cn_topic_0000001051742157_p2511918165511"><a name="zh-cn_topic_0000001051742157_p2511918165511"></a><a name="zh-cn_topic_0000001051742157_p2511918165511"></a>统一规划分配，最大支持64个，其中第三方APP统一使用HILOG_MODULE_APP作为模块ID。</p>
        </td>
        </tr>
        <tr id="zh-cn_topic_0000001051742157_row451818105516"><td class="cellrowborder" valign="top" width="11.57%" headers="mcps1.1.5.1.1 "><p id="zh-cn_topic_0000001051742157_p15151845520"><a name="zh-cn_topic_0000001051742157_p15151845520"></a><a name="zh-cn_topic_0000001051742157_p15151845520"></a>fmt</p>
        </td>
        <td class="cellrowborder" valign="top" width="11.83%" headers="mcps1.1.5.1.2 "><p id="zh-cn_topic_0000001051742157_p251111817557"><a name="zh-cn_topic_0000001051742157_p251111817557"></a><a name="zh-cn_topic_0000001051742157_p251111817557"></a>是</p>
        </td>
        <td class="cellrowborder" valign="top" width="15.21%" headers="mcps1.1.5.1.3 "><p id="zh-cn_topic_0000001051742157_p1351151814555"><a name="zh-cn_topic_0000001051742157_p1351151814555"></a><a name="zh-cn_topic_0000001051742157_p1351151814555"></a>char *</p>
        </td>
        <td class="cellrowborder" valign="top" width="61.39%" headers="mcps1.1.5.1.4 "><p id="zh-cn_topic_0000001051742157_p95117189556"><a name="zh-cn_topic_0000001051742157_p95117189556"></a><a name="zh-cn_topic_0000001051742157_p95117189556"></a>格式化输出字符串。</p>
        <p id="zh-cn_topic_0000001051742157_p25171895512"><a name="zh-cn_topic_0000001051742157_p25171895512"></a><a name="zh-cn_topic_0000001051742157_p25171895512"></a>1、  最大支持6个可变参数，不支持%s。</p>
        <p id="zh-cn_topic_0000001051742157_p351181875510"><a name="zh-cn_topic_0000001051742157_p351181875510"></a><a name="zh-cn_topic_0000001051742157_p351181875510"></a>2、  格式化后的单条日志最大长度128字节，超过将无法打印。</p>
        </td>
        </tr>
        <tr id="zh-cn_topic_0000001051742157_row95213183550"><td class="cellrowborder" valign="top" width="11.57%" headers="mcps1.1.5.1.1 "><p id="zh-cn_topic_0000001051742157_p15291815518"><a name="zh-cn_topic_0000001051742157_p15291815518"></a><a name="zh-cn_topic_0000001051742157_p15291815518"></a>可变参</p>
        </td>
        <td class="cellrowborder" valign="top" width="11.83%" headers="mcps1.1.5.1.2 "><p id="zh-cn_topic_0000001051742157_p1852151815514"><a name="zh-cn_topic_0000001051742157_p1852151815514"></a><a name="zh-cn_topic_0000001051742157_p1852151815514"></a>否</p>
        </td>
        <td class="cellrowborder" valign="top" width="15.21%" headers="mcps1.1.5.1.3 "><p id="zh-cn_topic_0000001051742157_p145221855520"><a name="zh-cn_topic_0000001051742157_p145221855520"></a><a name="zh-cn_topic_0000001051742157_p145221855520"></a>int32</p>
        </td>
        <td class="cellrowborder" valign="top" width="61.39%" headers="mcps1.1.5.1.4 "><p id="zh-cn_topic_0000001051742157_p15521018115510"><a name="zh-cn_topic_0000001051742157_p15521018115510"></a><a name="zh-cn_topic_0000001051742157_p15521018115510"></a>仅支持数字类型，最大支持6个变参。</p>
        </td>
        </tr>
        </tbody>
        </table>



## 使用-featured框架<a name="zh-cn_topic_0000001051742157_section166251104236"></a>

DFX featured框架提供完整的DFX特性，对外提供log接口：

**Native C/C++**接口

hilog  可用API

```
HILOGD(fmt,...) HILOGI(fmt,...) HILOGW(fmt,...) HILOGE(fmt,...)
```

使用介绍

首先需要定义TAG，DOMAIN需要找DFT申请，未经申请的DOMAIN，日志打印不出来。

本地调试，可以临时使用domain数值 0。

包含头文件：\#include <hilog/log.h\>

在BUILD.gn中添加依赖库 libhilog。

接口规则介绍：

1. 格式化字符串默认是非隐私 HILOGI\("Hello World\\n"\); \>\> Hello World

2. 格式化参数默认是隐私 HILOGI\("Age is %d\\n", 10\); \>\> Age is <private\>

3. %\{private\}标识的参数是隐私 HILOGI\("Age is %\{private\}d\\n", 10\); \>\> Age is <private\>

4. %\{public\}标识的参数是非隐私 HILOGI\("Age is %\{public\}d\\n", 10\); \>\>Age is 10

接口参数介绍

<a name="zh-cn_topic_0000001051742157_table2021218594324"></a>
<table><thead align="left"><tr id="zh-cn_topic_0000001051742157_row72672598321"><th class="cellrowborder" valign="top" width="25.94%" id="mcps1.1.3.1.1"><p id="zh-cn_topic_0000001051742157_p1226718595323"><a name="zh-cn_topic_0000001051742157_p1226718595323"></a><a name="zh-cn_topic_0000001051742157_p1226718595323"></a>参数名字</p>
</th>
<th class="cellrowborder" valign="top" width="74.06%" id="mcps1.1.3.1.2"><p id="zh-cn_topic_0000001051742157_p1026765918324"><a name="zh-cn_topic_0000001051742157_p1026765918324"></a><a name="zh-cn_topic_0000001051742157_p1026765918324"></a>参数含义</p>
</th>
</tr>
</thead>
<tbody><tr id="zh-cn_topic_0000001051742157_row152675592321"><td class="cellrowborder" valign="top" width="25.94%" headers="mcps1.1.3.1.1 "><p id="zh-cn_topic_0000001051742157_p12267135915328"><a name="zh-cn_topic_0000001051742157_p12267135915328"></a><a name="zh-cn_topic_0000001051742157_p12267135915328"></a>domain</p>
</td>
<td class="cellrowborder" valign="top" width="74.06%" headers="mcps1.1.3.1.2 "><p id="zh-cn_topic_0000001051742157_p1826745953219"><a name="zh-cn_topic_0000001051742157_p1826745953219"></a><a name="zh-cn_topic_0000001051742157_p1826745953219"></a>领域标识ID，需要找DFT申请，未经申请的domain会出现日志打印不出来的问题</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row20267135963212"><td class="cellrowborder" valign="top" width="25.94%" headers="mcps1.1.3.1.1 "><p id="zh-cn_topic_0000001051742157_p152675591328"><a name="zh-cn_topic_0000001051742157_p152675591328"></a><a name="zh-cn_topic_0000001051742157_p152675591328"></a>tag</p>
</td>
<td class="cellrowborder" valign="top" width="74.06%" headers="mcps1.1.3.1.2 "><p id="zh-cn_topic_0000001051742157_p4267145920323"><a name="zh-cn_topic_0000001051742157_p4267145920323"></a><a name="zh-cn_topic_0000001051742157_p4267145920323"></a>日志tag</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row02671159203214"><td class="cellrowborder" valign="top" width="25.94%" headers="mcps1.1.3.1.1 "><p id="zh-cn_topic_0000001051742157_p1926785953216"><a name="zh-cn_topic_0000001051742157_p1926785953216"></a><a name="zh-cn_topic_0000001051742157_p1926785953216"></a>isFmtPrivate</p>
</td>
<td class="cellrowborder" valign="top" width="74.06%" headers="mcps1.1.3.1.2 "><p id="zh-cn_topic_0000001051742157_p326785963220"><a name="zh-cn_topic_0000001051742157_p326785963220"></a><a name="zh-cn_topic_0000001051742157_p326785963220"></a>标识格式化字符串fmt是否是隐私,是yes时fmt会被认为是隐私</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row326795910325"><td class="cellrowborder" valign="top" width="25.94%" headers="mcps1.1.3.1.1 "><p id="zh-cn_topic_0000001051742157_p72678597327"><a name="zh-cn_topic_0000001051742157_p72678597327"></a><a name="zh-cn_topic_0000001051742157_p72678597327"></a>fmt</p>
</td>
<td class="cellrowborder" valign="top" width="74.06%" headers="mcps1.1.3.1.2 "><p id="zh-cn_topic_0000001051742157_p11267195914324"><a name="zh-cn_topic_0000001051742157_p11267195914324"></a><a name="zh-cn_topic_0000001051742157_p11267195914324"></a>格式化字符串</p>
</td>
</tr>
<tr id="zh-cn_topic_0000001051742157_row1226825913326"><td class="cellrowborder" valign="top" width="25.94%" headers="mcps1.1.3.1.1 "><p id="zh-cn_topic_0000001051742157_p142681259203215"><a name="zh-cn_topic_0000001051742157_p142681259203215"></a><a name="zh-cn_topic_0000001051742157_p142681259203215"></a>args</p>
</td>
<td class="cellrowborder" valign="top" width="74.06%" headers="mcps1.1.3.1.2 "><p id="zh-cn_topic_0000001051742157_p1326895911325"><a name="zh-cn_topic_0000001051742157_p1326895911325"></a><a name="zh-cn_topic_0000001051742157_p1326895911325"></a>格式化字符串参数</p>
</td>
</tr>
</tbody>
</table>

**日志查看**

1.  debug版本hilog日志会保存到/storage/data/log/hilogs 目录下面。

2.  可以执行hilogcat实时查看hilog日志。


**日志系统架构**

![](http://tools.harmonyos.com/mirrors/hpm-image/hilog_m_README/figures/zh-cn_image_0000001052080708.png)

1.  hilogtask流水日志的内核任务。
    -   此功能是一个linux内核的任务或者线程，在系统启动时初始化。
    -   当内核中一个模块调用它的日志接口，将格式化好的日志内容传输给改任务，并将其存储在一个环形缓冲区中 。
    -   当用户态调用日志接口，将格式化好的日志内容通过ioctl调用写入驱动节点，驱动节点再将日志内容发送到hilogtask，hilogtask将日志内容存储到环形缓冲区中。

2.  hilogcatd用户态日志存储服务。
    -   这是一个用户态的进程，负责定时将内核的ringbuffer读取出来，存储到日志文件中。
    -   日志文件输出支持gzip压缩，使用zlib
    -   每个类型的ringbuffer分开存储。
    -   存储文件的单个文件大小，文件个数可在编译时配置。

3.  hilogcat日志查看命令行工具。

    从内核驱动接口读取ringbuffer内容，输出到标准输出。

4.  支持日志缓冲区可配置。
    -   编译时可以配置日志缓冲区的大小。
    -   编译时可以指定日志缓冲区的类型，类型个数就是ringbuffer的个数。


## 涉及仓<a name="zh-cn_topic_0000001051742157_section6899131818455"></a>

hiviewdfx\_frameworks\_hievent\_lite

hiviewdfx\_frameworks\_ddrdump\_lite

hiviewdfx\_frameworks\_hilog\_lite

hiviewdfx\_interfaces\_innerkits\_hilog

hiviewdfx\_interfaces\_innerkits\_hievent\_lite

hiviewdfx\_interfaces\_kits\_hilog

hiviewdfx\_interfaces\_kits\_hilog\_lite

hiviewdfx\_services\_hiview\_lite

hiviewdfx\_services\_hilogcat\_lite

hiviewdfx\_utils\_lite

