# WLAN 驱动开发<a name="ZH-CN_TOPIC_0000001054461988"></a>

## 简介<a name="section11660541593"></a>

WLAN 是目前比较主流的接入方式，WLAN本身业务较为复杂，驱动代码量大，技术门槛相对较高，同时对芯片厂家高度依赖。HDF基于WLAN的业务特点，设计组件化的驱动开发框架，建立不同框架体系下一致的WIFI抽象，并以相同的规则来约束和规范鸿蒙生态中不同设备形态下WLAN驱动的开发。

## 架构<a name="section17101269359"></a>

一个驱动分配两个大部分，一部分为基于HCS的驱动配置\(图中  **C**\)，一部分是WLAN驱动实体（图中A+B）。其中WLAN驱动实体包含2个逻辑模块

**A：公共核心驱动部分; B: 模组私有驱动部分**;

![](http://tools.harmonyos.com/mirrors/hpm-image/wlan_README/figures/zh-cn_image_0000001054807526.png)

组件A&组件A与周边的关系

![](http://tools.harmonyos.com/mirrors/hpm-image/wlan_README/figures/zh-cn_image_0000001054647537.png)

## 目录<a name="section11828228164412"></a>

表1：源码路径

<a name="table440718439534"></a>
<table><tbody><tr id="row1443024317536"><td class="cellrowborder" valign="top" width="50.83%"><p id="p0430134305310"><a name="p0430134305310"></a><a name="p0430134305310"></a>路径</p>
</td>
<td class="cellrowborder" valign="top" width="49.17%"><p id="p19430443135311"><a name="p19430443135311"></a><a name="p19430443135311"></a>描述</p>
</td>
</tr>
<tr id="row1443014345319"><td class="cellrowborder" valign="top" width="50.83%"><p id="p17430643125315"><a name="p17430643125315"></a><a name="p17430643125315"></a>drivers\hdf\frameworks\model\network\wifi</p>
</td>
<td class="cellrowborder" valign="top" width="49.17%"><p id="p164301843105312"><a name="p164301843105312"></a><a name="p164301843105312"></a>wlan 模型，包含模型入口以及简单组件，netDevice</p>
</td>
</tr>
<tr id="row743115439534"><td class="cellrowborder" valign="top" width="50.83%"><p id="p9431184375310"><a name="p9431184375310"></a><a name="p9431184375310"></a>vendor\huawei\hdf\huawei_proprietary\wifi</p>
</td>
<td class="cellrowborder" valign="top" width="49.17%"><p id="p343114335320"><a name="p343114335320"></a><a name="p343114335320"></a>wlan 驱动实体，CoreDriver部分，包含wlan 模型实例</p>
</td>
</tr>
<tr id="row12431154385318"><td class="cellrowborder" valign="top" width="50.83%"><p id="p1543114305311"><a name="p1543114305311"></a><a name="p1543114305311"></a>vendor\huawei\hdf\libs\hi35xxxx</p>
</td>
<td class="cellrowborder" valign="top" width="49.17%"><p id="p34312433536"><a name="p34312433536"></a><a name="p34312433536"></a>wlan 驱动实体，ChipDriver部分</p>
</td>
</tr>
<tr id="row843164317531"><td class="cellrowborder" valign="top" width="50.83%"><p id="p7431174311534"><a name="p7431174311534"></a><a name="p7431174311534"></a>vendor\hisi\hi35xx\hi35xxxx\config\wifi</p>
</td>
<td class="cellrowborder" valign="top" width="49.17%"><p id="p104311043105310"><a name="p104311043105310"></a><a name="p104311043105310"></a>HCS配置部分</p>
</td>
</tr>
</tbody>
</table>

## 使用<a name="section8160132723814"></a>

1. WLAN 模型可以根据源码进行开发，CoreDriver 可以根据HCS配置组装。

**表 1**  WLAN 组件管理接口

<a name="table1521573319472"></a>
<table><thead align="left"><tr id="row121519334474"><th class="cellrowborder" valign="top" width="15.079999999999998%" id="mcps1.2.4.1.1"><p id="p1221510339475"><a name="p1221510339475"></a><a name="p1221510339475"></a>头文件</p>
</th>
<th class="cellrowborder" valign="top" width="60.33%" id="mcps1.2.4.1.2"><p id="p0215153344716"><a name="p0215153344716"></a><a name="p0215153344716"></a>接口名称</p>
</th>
<th class="cellrowborder" valign="top" width="24.59%" id="mcps1.2.4.1.3"><p id="p1421503315478"><a name="p1421503315478"></a><a name="p1421503315478"></a>功能描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row112150333476"><td class="cellrowborder" rowspan="4" valign="top" width="15.079999999999998%" headers="mcps1.2.4.1.1 "><p id="p2155710125317"><a name="p2155710125317"></a><a name="p2155710125317"></a>wifi_module.h</p>
<p id="p189132019183"><a name="p189132019183"></a><a name="p189132019183"></a></p>
</td>
<td class="cellrowborder" valign="top" width="60.33%" headers="mcps1.2.4.1.2 "><p id="p363110387399"><a name="p363110387399"></a><a name="p363110387399"></a>struct WifiModule *WifiModuleCreate(const struct HdfConfigWifiModuleConfig *config);</p>
</td>
<td class="cellrowborder" valign="top" width="24.59%" headers="mcps1.2.4.1.3 "><p id="p1363012387393"><a name="p1363012387393"></a><a name="p1363012387393"></a>基于HDF开发WIFI驱动时，创建一个WIFI Module。</p>
</td>
</tr>
<tr id="row112151233194714"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p7629163817393"><a name="p7629163817393"></a><a name="p7629163817393"></a>void WifiModuleDelete(struct WifiModule *module);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p2627638173917"><a name="p2627638173917"></a><a name="p2627638173917"></a>基于HDF开发WIFI驱动时，删除并释放WIFI Module所有数据。</p>
</td>
</tr>
<tr id="row1121533316475"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p12626103814399"><a name="p12626103814399"></a><a name="p12626103814399"></a>int32_t DelFeature(struct WifiModule *module, uint16_t featureType);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p1162543816393"><a name="p1162543816393"></a><a name="p1162543816393"></a>基于HDF开发WIFI驱动时，从 WIFI Module 删除一个功能组件。</p>
</td>
</tr>
<tr id="row172153335473"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p162433816392"><a name="p162433816392"></a><a name="p162433816392"></a>int32_t AddFeature(struct WifiModule *module, uint16_t featureType, struct WifiFeature *featureData);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p186235383393"><a name="p186235383393"></a><a name="p186235383393"></a>基于HDF开发WIFI驱动时，注册一个功能组件到 WIFI Module。</p>
</td>
</tr>
<tr id="row451796205011"><td class="cellrowborder" rowspan="4" valign="top" width="15.079999999999998%" headers="mcps1.2.4.1.1 "><p id="p2659417135013"><a name="p2659417135013"></a><a name="p2659417135013"></a>wifi_mac80211_ops.h</p>
</td>
<td class="cellrowborder" valign="top" width="60.33%" headers="mcps1.2.4.1.2 "><p id="p175181615011"><a name="p175181615011"></a><a name="p175181615011"></a>int32_t (*startVap)(NetDevice *netDev);</p>
</td>
<td class="cellrowborder" valign="top" width="24.59%" headers="mcps1.2.4.1.3 "><p id="p195182610507"><a name="p195182610507"></a><a name="p195182610507"></a>启动AP。</p>
</td>
</tr>
<tr id="row5518663503"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p125181260501"><a name="p125181260501"></a><a name="p125181260501"></a>int32_t (*stopVap)(NetDevice *netDev);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p1151815635014"><a name="p1151815635014"></a><a name="p1151815635014"></a>停止AP。</p>
</td>
</tr>
<tr id="row851915617503"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p20519865500"><a name="p20519865500"></a><a name="p20519865500"></a>int32_t (*connect)(NetDevice *netDev, WifiConnectParams *param);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p14519469509"><a name="p14519469509"></a><a name="p14519469509"></a>开始连接。</p>
</td>
</tr>
<tr id="row18519136185016"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p145195620502"><a name="p145195620502"></a><a name="p145195620502"></a>int32_t (*disconnect)(NetDevice *netDev, uint16_t reasonCode);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p175191863503"><a name="p175191863503"></a><a name="p175191863503"></a>断开连接。</p>
</td>
</tr>
<tr id="row176421942125016"><td class="cellrowborder" rowspan="5" valign="top" width="15.079999999999998%" headers="mcps1.2.4.1.1 "><p id="p7937165012500"><a name="p7937165012500"></a><a name="p7937165012500"></a>hdf_netbuf.h</p>
</td>
<td class="cellrowborder" valign="top" width="60.33%" headers="mcps1.2.4.1.2 "><p id="p1964211423505"><a name="p1964211423505"></a><a name="p1964211423505"></a>void NetBufQueueInit(struct NetBufQueue *q);</p>
</td>
<td class="cellrowborder" valign="top" width="24.59%" headers="mcps1.2.4.1.3 "><p id="p364254211507"><a name="p364254211507"></a><a name="p364254211507"></a>初始化net buffer队列。</p>
</td>
</tr>
<tr id="row664264225020"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p166421942115017"><a name="p166421942115017"></a><a name="p166421942115017"></a>struct NetBuf *NetBufDevAlloc(uint32_t size);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p3642164215501"><a name="p3642164215501"></a><a name="p3642164215501"></a>申请net buffer。</p>
</td>
</tr>
<tr id="row19642134215018"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p964310425501"><a name="p964310425501"></a><a name="p964310425501"></a>void NetBufFree(struct NetBuf *nb);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p1464312427503"><a name="p1464312427503"></a><a name="p1464312427503"></a>释放net buffer。</p>
</td>
</tr>
<tr id="row7643194215013"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p20643164218508"><a name="p20643164218508"></a><a name="p20643164218508"></a>struct NetBuf *Pbuf2NetBuf(const struct NetDevice *netdev, struct pbuf *lwipBuf);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p186437429509"><a name="p186437429509"></a><a name="p186437429509"></a>lwip的pbuf转换为net buffer。</p>
</td>
</tr>
<tr id="row7657132317518"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p86576231557"><a name="p86576231557"></a><a name="p86576231557"></a>struct pbuf *NetBuf2Pbuf(const struct NetBuf *nb);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p1965702312510"><a name="p1965702312510"></a><a name="p1965702312510"></a>net buffer转换为lwip的pbuf。</p>
</td>
</tr>
</tbody>
</table>

**表 2**  需要开发人员实现的接口

<a name="table74613501475"></a>
<table><thead align="left"><tr id="row194625016476"><th class="cellrowborder" valign="top" width="20.75%" id="mcps1.2.4.1.1"><p id="p10468502479"><a name="p10468502479"></a><a name="p10468502479"></a>头文件</p>
</th>
<th class="cellrowborder" valign="top" width="52.75%" id="mcps1.2.4.1.2"><p id="p184615501477"><a name="p184615501477"></a><a name="p184615501477"></a>接口名称</p>
</th>
<th class="cellrowborder" valign="top" width="26.5%" id="mcps1.2.4.1.3"><p id="p1146135044719"><a name="p1146135044719"></a><a name="p1146135044719"></a>功能描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row04616509472"><td class="cellrowborder" rowspan="6" valign="top" width="20.75%" headers="mcps1.2.4.1.1 "><p id="p14615017477"><a name="p14615017477"></a><a name="p14615017477"></a>net_device.h</p>
</td>
<td class="cellrowborder" valign="top" width="52.75%" headers="mcps1.2.4.1.2 "><p id="p144943564611"><a name="p144943564611"></a><a name="p144943564611"></a>int32_t (*init)(struct NetDevice *netDev);</p>
</td>
<td class="cellrowborder" valign="top" width="26.5%" headers="mcps1.2.4.1.3 "><p id="p18822442135411"><a name="p18822442135411"></a><a name="p18822442135411"></a>初始化net device。</p>
</td>
</tr>
<tr id="row1546250114713"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1490010315564"><a name="p1490010315564"></a><a name="p1490010315564"></a>struct NetDevStats *(*getStats)(struct NetDevice *netDev);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p5900163115564"><a name="p5900163115564"></a><a name="p5900163115564"></a>获取net device的状态。</p>
</td>
</tr>
<tr id="row1646165010470"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p16909135319564"><a name="p16909135319564"></a><a name="p16909135319564"></a>int32_t (*setMacAddr)(struct NetDevice *netDev, void *addr);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p122001431115713"><a name="p122001431115713"></a><a name="p122001431115713"></a>设置mac地址。</p>
</td>
</tr>
<tr id="row12471250184711"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p154213655215"><a name="p154213655215"></a><a name="p154213655215"></a>void (*deInit)(struct NetDevice *netDev);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p14845675719"><a name="p14845675719"></a><a name="p14845675719"></a>注销net device。</p>
</td>
</tr>
<tr id="row13471050104719"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p16686131655218"><a name="p16686131655218"></a><a name="p16686131655218"></a>int32_t (*open)(struct NetDevice *netDev);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p164825613576"><a name="p164825613576"></a><a name="p164825613576"></a>打开net device。</p>
</td>
</tr>
<tr id="row1747125054714"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p2310615407"><a name="p2310615407"></a><a name="p2310615407"></a>int32_t (*stop)(struct NetDevice *netDev);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p1982212428542"><a name="p1982212428542"></a><a name="p1982212428542"></a>关闭net device。</p>
</td>
</tr>
</tbody>
</table>

2.上层业务接口

3.对上业务功能

Wi-Fi驱动模块对HAL层开发人员提供的接口功能有：建立/关闭Wi-Fi热点、扫描Wi-Fi、开始关联、断开连接等等

**表 3**  上层业务接口

<a name="table13821942165419"></a>
<table><thead align="left"><tr id="row6821194225417"><th class="cellrowborder" valign="top" width="11.16%" id="mcps1.2.4.1.1"><p id="p1115541095311"><a name="p1115541095311"></a><a name="p1115541095311"></a>头文件</p>
</th>
<th class="cellrowborder" valign="top" width="71.22%" id="mcps1.2.4.1.2"><p id="p3822542145420"><a name="p3822542145420"></a><a name="p3822542145420"></a>接口名称</p>
</th>
<th class="cellrowborder" valign="top" width="17.62%" id="mcps1.2.4.1.3"><p id="p0822942175411"><a name="p0822942175411"></a><a name="p0822942175411"></a>功能描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row1782284211544"><td class="cellrowborder" rowspan="6" valign="top" width="11.16%" headers="mcps1.2.4.1.1 "><p id="p15454165415187"><a name="p15454165415187"></a><a name="p15454165415187"></a>hdf_wifi_cmd.h</p>
</td>
<td class="cellrowborder" valign="top" width="71.22%" headers="mcps1.2.4.1.2 "><p id="p13453155441814"><a name="p13453155441814"></a><a name="p13453155441814"></a>ErrorCode WifiHalScan(const RequestContext *context, const DataBlock *reqData, DataBlock *rspData);</p>
</td>
<td class="cellrowborder" valign="top" width="17.62%" headers="mcps1.2.4.1.3 "><p id="p345225441813"><a name="p345225441813"></a><a name="p345225441813"></a>启动扫描。</p>
</td>
</tr>
<tr id="row1489993155613"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p134519546189"><a name="p134519546189"></a><a name="p134519546189"></a>ErrorCode WifiHalSetAp(const RequestContext *context, const DataBlock *reqData, DataBlock *rspData);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p9450454101811"><a name="p9450454101811"></a><a name="p9450454101811"></a>设置AP启动参数。</p>
</td>
</tr>
<tr id="row1909165317562"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p124491454121818"><a name="p124491454121818"></a><a name="p124491454121818"></a>ErrorCode WifiHalStopAp(const RequestContext *context, const DataBlock *reqData, DataBlock *rspData);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p14448155417185"><a name="p14448155417185"></a><a name="p14448155417185"></a>关闭AP模式。</p>
</td>
</tr>
<tr id="row39747473560"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p84489549187"><a name="p84489549187"></a><a name="p84489549187"></a>ErrorCode WifiHalSendEapol(const RequestContext *context, const DataBlock *reqData, DataBlock *rspData);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p4447145441819"><a name="p4447145441819"></a><a name="p4447145441819"></a>发送EAPOL帧。</p>
</td>
</tr>
<tr id="row12765124315610"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1444520545183"><a name="p1444520545183"></a><a name="p1444520545183"></a>ErrorCode WifiHalReceiveEapol(const RequestContext *context, const DataBlock *reqData, DataBlock *rspData);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p2444145412188"><a name="p2444145412188"></a><a name="p2444145412188"></a>接收EAPOL帧。</p>
</td>
</tr>
<tr id="row168221342195417"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p3443145418181"><a name="p3443145418181"></a><a name="p3443145418181"></a>ErrorCode WifiHalStaRemove(const RequestContext *context, const DataBlock *reqData, DataBlock *rspData);</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p4421554111812"><a name="p4421554111812"></a><a name="p4421554111812"></a>删除STA</p>
</td>
</tr>
</tbody>
</table>

Wi-Fi驱动模块提供了驱动开发人员可直接调用的能力接口，主要功能有：创建/释放Wi-Fi Module、关联/取消关联、申请/释放net buf、开始关联、lwip的pbuf和netbuf的相互转换等等。

