loaderboot_srcs = startup common common/crc32 common/nvm common/partition_table drivers/lsadc drivers/flash drivers/efuse secure
loaderboot_common_srcs = ../commonboot/crc32 ../commonboot/efuse ../commonboot/flash
BOOT_CCFLAGS := -mabi=ilp32 -march=rv32imc -freorder-blocks-algorithm=simple -fno-schedule-insns -nostdinc -fno-aggressive-loop-optimizations -fno-builtin -fno-exceptions -fno-short-enums -mtune=size -msmall-data-limit=0 -Wall -Werror -Os -std=c99 -falign-functions=2 -fdata-sections -ffunction-sections -fno-common -fstack-protector-strong
BOOT_DEFINE := -DARCH_RISCV -DLOS_COMPILE_LDM -DHI_BOARD_ASIC
BOOT_INC := -I$(MAIN_TOPDIR)/boot/loaderboot/include \
    -I$(MAIN_TOPDIR)/boot/loaderboot/fixed/include \
    -I$(MAIN_TOPDIR)/boot/loaderboot/secure \
    -I$(MAIN_TOPDIR)/boot/commonboot

ifeq ($(CONFIG_COMPRESSION_OTA_SUPPORT), y)
    BOOT_DEFINE += -DCONFIG_COMPRESSION_OTA_SUPPORT
endif

ifeq ($(CONFIG_DUAL_PARTITION_OTA_SUPPORT), y)
    BOOT_DEFINE += -DCONFIG_DUAL_PARTITION_OTA_SUPPORT
endif

ifeq ($(CONFIG_TARGET_SIG_ECC), y)
    BOOT_DEFINE += -DCONFIG_TARGET_SIG_ECC
endif

ifeq ($(CONFIG_TARGET_SIG_RSA_V15), y)
    BOOT_DEFINE += -DCONFIG_TARGET_SIG_RSA_V15
endif

ifeq ($(CONFIG_TARGET_SIG_RSA_PSS), y)
    BOOT_DEFINE += -DCONFIG_TARGET_SIG_RSA_PSS
endif

ifeq ($(CONFIG_FLASH_ENCRYPT_SUPPORT), y)
    BOOT_DEFINE += -DCONFIG_FLASH_ENCRYPT_SUPPORT
endif

ifeq ($(CONFIG_CHIP_PKT_48K), y)
    BOOT_DEFINE += -DCONFIG_CHIP_PKT_48K
else
    BOOT_DEFINE += -DCONFIG_CHIP_PKT_32K
endif

BOOT_ASFLAGS = -mabi=ilp32 -march=rv32imc -x assembler-with-cpp -Os -Wall -Werror -nostdinc -fno-common
BOOT_LINK_FLAGS = -nostdlib -nostartfiles -static --gc-sections
