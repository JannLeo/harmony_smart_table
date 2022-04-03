# IoT Subsystem<a name="EN-US_TOPIC_0000001085756528"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section1464106163817)
-   [Repositories Involved](#section1718733212019)

## Introduction<a name="section11660541593"></a>

The IoT subsystem provides samples for integrating third-party SDKs. For details, see  [Third-Party SDK Integration](https://device.harmonyos.com/en/docs/develop/demo/oem_device_wifi_sdk-0000001054412155).

## Directory Structure<a name="section1464106163817"></a>

```
domains/iot/                              # IoT subsystem
└── link
    ├── BUILD.gn                       # Build script
    ├── demolink                       # Code for adapting third-party APIs to OpenHarmonyOS
    │   ├── BUILD.gn
    │   ├── demosdk_adapter.c
    │   └── demosdk_adapter.h
    └── libbuild                       # Code for building third-party SDKs
        ├── BUILD.gn
        ├── demosdk.c
        └── demosdk.h
```

## Repositories Involved<a name="section1718733212019"></a>

iot\_link

