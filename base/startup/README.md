# 项目介绍<a name="ZH-CN_TOPIC_0000001054621940"></a>

## 简介<a name="section469617221261"></a>

系统属性模块，根据鸿蒙CDD文档提供获取设备信息的接口，如：产品名、品牌名、厂家名等，同时提供设置/读取系统属性的接口。

## 目录<a name="section692981610397"></a>

**表 1**  系统属性模块源代码目录结构

<a name="table11831450193015"></a>
<table><thead align="left"><tr id="row81831503309"><th class="cellrowborder" valign="top" width="21.02%" id="mcps1.2.3.1.1"><p id="p418325003018"><a name="p418325003018"></a><a name="p418325003018"></a>名称</p>
</th>
<th class="cellrowborder" valign="top" width="78.97999999999999%" id="mcps1.2.3.1.2"><p id="p218385053019"><a name="p218385053019"></a><a name="p218385053019"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row10183550123016"><td class="cellrowborder" valign="top" width="21.02%" headers="mcps1.2.3.1.1 "><p id="p71831850123012"><a name="p71831850123012"></a><a name="p71831850123012"></a>base/startup/interfaces/kits/syspara_lite</p>
</td>
<td class="cellrowborder" valign="top" width="78.97999999999999%" headers="mcps1.2.3.1.2 "><p id="p71820194428"><a name="p71820194428"></a><a name="p71820194428"></a>系统属性模块对外接口</p>
</td>
</tr>
<tr id="row201831550173017"><td class="cellrowborder" valign="top" width="21.02%" headers="mcps1.2.3.1.1 "><p id="p16655103010434"><a name="p16655103010434"></a><a name="p16655103010434"></a>base/startup/hals/syspara_lite</p>
</td>
<td class="cellrowborder" valign="top" width="78.97999999999999%" headers="mcps1.2.3.1.2 "><p id="p10423140174415"><a name="p10423140174415"></a><a name="p10423140174415"></a>系统属性模块硬件抽象层头文件</p>
</td>
</tr>
<tr id="row9184050203013"><td class="cellrowborder" valign="top" width="21.02%" headers="mcps1.2.3.1.1 "><p id="p18184550193013"><a name="p18184550193013"></a><a name="p18184550193013"></a>base/startup/interfaces</p>
</td>
<td class="cellrowborder" valign="top" width="78.97999999999999%" headers="mcps1.2.3.1.2 "><p id="p20413161014013"><a name="p20413161014013"></a><a name="p20413161014013"></a>服务启动模块对外接口，由main函数来调用，启动服务框架。</p>
</td>
</tr>
<tr id="row178841725886"><td class="cellrowborder" valign="top" width="21.02%" headers="mcps1.2.3.1.1 "><p id="p88841825887"><a name="p88841825887"></a><a name="p88841825887"></a>base/startup/frameworks/syspara_lite</p>
</td>
<td class="cellrowborder" valign="top" width="78.97999999999999%" headers="mcps1.2.3.1.2 "><p id="p98851625589"><a name="p98851625589"></a><a name="p98851625589"></a>系统属性模块源码文件</p>
</td>
</tr>
</tbody>
</table>

使用C语言开发。

同时支持LiteOS-M、LiteOS-A平台。系统属性各字段由OEM厂商负责定义，当前方案仅提供框架及默认值。具体值需产品方按需进行调整。

## 使用<a name="section1464106163817"></a>

-   获取系统属性

    ```
    char* value1 = GetProductType();
    printf("Product type =%s\n", value1);
    free(value1);
    char* value2 = GetManufacture();
    printf("Manufacture =%s\n", value2);
    free(value2);
    char* value3 = GetBrand();
    printf("GetBrand =%s\n", value3);
    free(value3);
    ```


