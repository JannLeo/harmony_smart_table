#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_errno.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"
#include "wifiiot_i2c_ex.h"
#include "nfc.h"

// 从 Intent 中获取 TagInfo，初始化 TagInfo 实例TagInfo tagInfo =
getIntent().getParcelableExtra(NfcController.EXTRA_TAG_INFO);

// 查询 Tag 设备支持的技术或协议，返回值为支持的技术或协议列表
int[] profiles = tagInfo.getTagSupportedProfiles();
// 查询是否支持 NfcA，若支持，构造一个 NfcATag
bool isSupportedNfcA = tagInfo.isProfileSupported(TagManager.NFC_A);
if (isSupportedNfcA) {
NfcATag tagNfcA = NfcATag.getInstance(tagInfo);
}
// 查询是否支持 NfcB，若支持，构造一个 NfcBTag
bool isSupportedNfcB = tagInfo.isProfileSupported(TagManager.NFC_B);
if (isSupportedNfcB) {
NfcBTag tagNfcB = NfcBTag.getInstance(tagInfo);
} /
/ 查询是否支持 IsoDep，若支持，构造一个 IsoDepTag
bool isSupportedIsoDep =
tagInfo.isProfileSupported(TagManager.ISO_DEP);
if (isSupportedIsoDep) {
IsoDepTag tagIsoDep = new IsoDepTag(mTagInfo);
}
// 查询是否支持 NDEF，若支持，构造一个 NdefTag
bool isSupportedNdefDep =
tagInfo.isProfileSupported(TagManager.NDEF);
if (isSupportedNdefDep) {
NdefTag tagNdef = new NdefTag(mTagInfo);
}
// 查询是否支持 MifareClassic，若支持，构造一个 MifareClassicTag
bool isSupportedMifareClassic = tagInfo.isProfileSupported(TagManager.MIFARE_CLASSIC);
if (isSupportedMifareClassic) {
MifareClassicTag mifareClassicTag =
MifareClassicTag.getInstance(tagInfo);
}
// 查询是否支持 MifareUltralight，若支持，构造一个 MifareUltralightTag
boolean isSupportedMifareUltralight =tagInfo.isProfileSupported(TagManager.MIFARE_ULTRALIGHT);
if (isSupportedMifareUltralight) {
MifareUltralightTag mifareUltralightTag =
MifareUltralightTag.getInstance(tagInfo);
}