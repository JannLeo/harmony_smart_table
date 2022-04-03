OUTPUT_ARCH( "riscv" )
ENTRY(_start)
STACK_SIZE = 2k;
IRQ_STACK_SIZE = 3K;
EXC_STACK_SIZE = 2K;
NMI_STACK_SIZE = 2K;
DOWNLOAD_STACK_SIZE = 4K;
DOWNLOAD_IRQ_STACK_SIZE = 2K;
MEMORY
{
    BIN(rx) : ORIGIN = 4248512, LENGTH = 2M
    ROM_TEXT(rx) : ORIGIN = 0x003b8000, LENGTH = (278K - 32)
    ROM_DATA0(rwx) : ORIGIN = ((0x11DFFF - (32 + 2016 + 11776 + 0x40) + 1) + 11776), LENGTH = 32
    ROM_DATA1(rwx) : ORIGIN = (((0x11DFFF - (32 + 2016 + 11776 + 0x40) + 1) + 11776) + 32), LENGTH = 1768
    ROM_BSS(rwx) : ORIGIN = (0x11DFFF - (32 + 2016 + 11776 + 0x40) + 1), LENGTH = 11776
    STACK(rw) : ORIGIN = (0x11DFFF - (32 + 2016 + 11776 + 0x40) + 1) - (STACK_SIZE + IRQ_STACK_SIZE + EXC_STACK_SIZE + NMI_STACK_SIZE), LENGTH = (STACK_SIZE + IRQ_STACK_SIZE + EXC_STACK_SIZE + NMI_STACK_SIZE)
    CHECK_INFO(rw) : ORIGIN = ((((0x11DFFF - (32 + 2016 + 11776 + 0x40) + 1) + 11776) + 32) + 2016), LENGTH = 0x40
    FLASH(rwx) : ORIGIN = 4248512, LENGTH = 0x00400000 + 2M - 4248512
    PATCH_BSS(rwx) : ORIGIN = 0x000d8000, LENGTH = 1024
    RAM(rwx) : ORIGIN = (0x000d8000 + 1024 + 0), LENGTH = (280K - (32 + 2016 + 11776 + 0x40) - 1024 - (STACK_SIZE + IRQ_STACK_SIZE + EXC_STACK_SIZE + NMI_STACK_SIZE) - 0)
    EXTERN_ROM_DATA1_BSS(rwx) : ORIGIN = 0x11dec8, LENGTH = 2016 -1768
}
SECTIONS
{
    .text_rom : AT ((4248512 - (278K - 32) - 32))
    {
        . = ALIGN(0x20);
        __rom_text_begin__ = .;
        __text_cache_start1_ = .;
        _rom_size_download_start = .;
        KEEP(SORT(*)(.text.download_entry))
        . = ALIGN(0x4);
        KEEP(SORT(*)(.text.trap_download_vector))
        KEEP(SORT(*)(.text.download_assemble))
        . = ALIGN(0x4);
        _rom_size_download_end = .;
        _rom_size_download = _rom_size_download_end - _rom_size_download_start;
        KEEP(SORT(libwifi_base.o)(.text*))
        KEEP(SORT(*)(.wifi.rom.text*))
        . = ALIGN(0x20);
        _rom_wifi_rom_end = .;
        _rom_size_libwifi = _rom_wifi_rom_end - _rom_size_download;
        KEEP(SORT(libbsp_base.o)(.text*))
        _rom_bsp_rom_end = .;
        _rom_size_libbsp = _rom_bsp_rom_end - _rom_wifi_rom_end;
        KEEP(SORT(liblitekernel_base.o)(.text*))
        _rom_kernel_rom_end = .;
        _rom_size_kernel = _rom_kernel_rom_end - _rom_bsp_rom_end;
        KEEP(SORT(*libgcc.a:save-restore.o)(.text*)) KEEP(SORT(*libgcc.a:_clzsi2.o)(.text*)) KEEP(SORT(*libgcc.a:_umoddi3.o)(.text*)) KEEP(SORT(*libgcc.a:_clz.o)(.text*)) KEEP(SORT(*libgcc.a:_ashldi3.o)(.text*)) KEEP(SORT(*libgcc.a:_lshrdi3.o)(.text*)) KEEP(SORT(*libgcc.a:fixdfsi.o)(.text*)) KEEP(SORT(*libgcc.a:_udivdi3.o)(.text*))
        _rom_libgcc_end = .;
        _rom_size_libgcc = _rom_libgcc_end - _rom_kernel_rom_end;
        KEEP(SORT(libc_base.o)(.text*))
        _rom_libc_rom_end = .;
        _rom_size_libc = _rom_libc_rom_end - _rom_libgcc_end;
        KEEP(SORT(libsec_base.o)(.text*))
        _rom_sec_rom_end = .;
        _rom_size_libsec = _rom_sec_rom_end - _rom_libc_rom_end;
        __text_cache_end1_ = .;
        KEEP(SORT(libwifi_base.o)(.rodata*))
        KEEP(SORT(*)(.wifi.rom.rodata*))
        . = ALIGN(0x4);
        _rom_hi1131_wifi_rom_rodata_end = .;
        _rom_size_rodata_wifi = _rom_hi1131_wifi_rom_rodata_end - _rom_sec_rom_end;
        KEEP(SORT(libbsp_base.o)(.rodata*))
        KEEP(SORT(*)(.bsp.rom.rodata))
        _rom_libbsp_rom_rodata_end = .;
        _rom_size_rodata_bsp = _rom_libbsp_rom_rodata_end - _rom_hi1131_wifi_rom_rodata_end;
        KEEP(SORT(liblitekernel_base.o)(.rodata*))
        _rom_litekenel_rom_rodata_end = .;
        _rom_size_rodata_kernel = _rom_litekenel_rom_rodata_end - _rom_size_rodata_bsp;
        KEEP(SORT(*libgcc.a:save-restore.o)(.rodata*)) KEEP(SORT(*libgcc.a:_clzsi2.o)(.rodata*)) KEEP(SORT(*libgcc.a:_umoddi3.o)(.rodata*)) KEEP(SORT(*libgcc.a:_clz.o)(.rodata*)) KEEP(SORT(*libgcc.a:_ashldi3.o)(.rodata*)) KEEP(SORT(*libgcc.a:_lshrdi3.o)(.rodata*)) KEEP(SORT(*libgcc.a:fixdfsi.o)(.rodata*)) KEEP(SORT(*libgcc.a:_udivdi3.o)(.rodata*))
        _rom_libgcc_rodata_end = .;
        _rom_size_rodata_libgcc = _rom_libgcc_rodata_end - _rom_litekenel_rom_rodata_end;
        KEEP(SORT(libc_base.o)(.rodata*))
        _rom_libcrom_rodata_end = .;
        _rom_size_rodata_libc = _rom_libcrom_rodata_end - _rom_libgcc_rodata_end;
        KEEP(SORT(libsec_base.o)(.rodata*))
        _rom_libsec_rom_rodata_end = .;
        _rom_size_rodata_sec = _rom_libsec_rom_rodata_end - _rom_libcrom_rodata_end;
        __rom_text_end__ = .;
    } > ROM_TEXT
    __rom_text_size__ = __rom_text_end__ - __rom_text_begin__;
    .data_rom0 : AT (((4248512 - (278K - 32) - 32) + (278K - 32)))
    {
        . = ALIGN(0x20);
        __rom_data0_begin__ = .;
        KEEP(SORT(liblitekernel_base.o) (.data.kernel.rom*))
        KEEP(SORT(*)(.bsp.rom.data0))
        . = ALIGN(0x4);
        __rom_data0_end__ = .;
    } > ROM_DATA0
    __rom_data0_size__ = __rom_data0_end__ - __rom_data0_begin__;
    _rom_size_total = __rom_data0_size__ + __rom_text_size__;
    __rom_data0_load = 0x003b8000 + (278K - 32);
    .text_non_rom : AT ((((4248512 - (278K - 32) - 32) + (278K - 32)) + 32))
    {
        . = ALIGN(0x20);
        __text_cache_start2_ = .;
        KEEP(*(.entry.text))
        . = ALIGN(0x20);
        SORT(*)(.init*)
        SORT(*)(.rom.text.patch)
        SORT(*)(EXCLUDE_FILE(*libasm_flash.o) .text*)
        SORT(*)(EXCLUDE_FILE(*libasm_flash.o) .got*)
        . = ALIGN(0x20);
        __text_cache_end2_ = .;
        SORT(*)(.rodata*)
        . = ALIGN(0x20);
        __text_rodata_end_ = .;
        . = ALIGN(0x20);
    } > FLASH
    .zInit : AT (ADDR(.text_non_rom) + SIZEOF(.text_non_rom))
    {
       __zinitcall_bsp_start = .;
       KEEP (*(.zinitcall.bsp0.init))
       KEEP (*(.zinitcall.bsp1.init))
       KEEP (*(.zinitcall.bsp2.init))
       KEEP (*(.zinitcall.bsp3.init))
       KEEP (*(.zinitcall.bsp4.init))
       __zinitcall_bsp_end = .;
       __zinitcall_device_start = .;
       KEEP (*(.zinitcall.device0.init))
       KEEP (*(.zinitcall.device1.init))
       KEEP (*(.zinitcall.device2.init))
       KEEP (*(.zinitcall.device3.init))
       KEEP (*(.zinitcall.device4.init))
       __zinitcall_device_end = .;
       __zinitcall_core_start = .;
       KEEP (*(.zinitcall.core0.init))
       KEEP (*(.zinitcall.core1.init))
       KEEP (*(.zinitcall.core2.init))
       KEEP (*(.zinitcall.core3.init))
       KEEP (*(.zinitcall.core4.init))
       __zinitcall_core_end = .;
       __zinitcall_sys_service_start = .;
       KEEP (*(.zinitcall.sys.service0.init))
       KEEP (*(.zinitcall.sys.service1.init))
       KEEP (*(.zinitcall.sys.service2.init))
       KEEP (*(.zinitcall.sys.service3.init))
       KEEP (*(.zinitcall.sys.service4.init))
       __zinitcall_sys_service_end = .;
       __zinitcall_sys_feature_start = .;
       KEEP (*(.zinitcall.sys.feature0.init))
       KEEP (*(.zinitcall.sys.feature1.init))
       KEEP (*(.zinitcall.sys.feature2.init))
       KEEP (*(.zinitcall.sys.feature3.init))
       KEEP (*(.zinitcall.sys.feature4.init))
       __zinitcall_sys_feature_end = .;
       __zinitcall_run_start = .;
       KEEP (*(.zinitcall.run0.init))
       KEEP (*(.zinitcall.run1.init))
       KEEP (*(.zinitcall.run2.init))
       KEEP (*(.zinitcall.run3.init))
       KEEP (*(.zinitcall.run4.init))
       __zinitcall_run_end = .;
       __zinitcall_app_service_start = .;
       KEEP (*(.zinitcall.app.service0.init))
       KEEP (*(.zinitcall.app.service1.init))
       KEEP (*(.zinitcall.app.service2.init))
       KEEP (*(.zinitcall.app.service3.init))
       KEEP (*(.zinitcall.app.service4.init))
       __zinitcall_app_service_end = .;
       __zinitcall_app_feature_start = .;
       KEEP (*(.zinitcall.app.feature0.init))
       KEEP (*(.zinitcall.app.feature1.init))
       KEEP (*(.zinitcall.app.feature2.init))
       KEEP (*(.zinitcall.app.feature3.init))
       KEEP (*(.zinitcall.app.feature4.init))
       __zinitcall_app_feature_end = .;
       __zinitcall_test_start = .;
       KEEP (*(.zinitcall.test0.init))
       KEEP (*(.zinitcall.test1.init))
       KEEP (*(.zinitcall.test2.init))
       KEEP (*(.zinitcall.test3.init))
       KEEP (*(.zinitcall.test4.init))
       __zinitcall_test_end = .;
       __zinitcall_exit_start = .;
       KEEP (*(.zinitcall.exit0.init))
       KEEP (*(.zinitcall.exit1.init))
       KEEP (*(.zinitcall.exit2.init))
       KEEP (*(.zinitcall.exit3.init))
       KEEP (*(.zinitcall.exit4.init))
       __zinitcall_exit_end = .;
    } > FLASH
    .ram_text : AT ((((((((4248512 - (278K - 32) - 32) + (278K - 32)) + 32) + SIZEOF (.text_non_rom) + (0x20) - 1) & ~ ((0x20) - 1)) + SIZEOF (.zInit) + (0x20) - 1) & ~ ((0x20) - 1)))
    {
        __ram_text_load = LOADADDR(.ram_text);
        __ram_text_start = .;
        . = ALIGN(0x20);
        SORT(*)(.trap_vector.text)
        SORT(*)(.kernel.ram.text)
        SORT(*)(.ram.kernel)
        SORT(*)(.bsp.ram.text)
        SORT(libasm_flash.o)(.text*)
        . = ALIGN(0x20);
        __ram_text_end = .;
    } > RAM
    __ram_text_size = __ram_text_end - __ram_text_start;
    .data : AT ((((((((((4248512 - (278K - 32) - 32) + (278K - 32)) + 32) + SIZEOF (.text_non_rom) + (0x20) - 1) & ~ ((0x20) - 1)) + SIZEOF (.zInit) + (0x20) - 1) & ~ ((0x20) - 1)) + SIZEOF (.ram_text) + (0x20) -1) & ~ ((0x20) - 1)))
    {
        __data_load = LOADADDR(.data);
       . = ALIGN(0x20);
        __data_start = .;
        SORT(*)(EXCLUDE_FILE(*liblitekernel_base.o *libc_base.o *libsec_base.o *libwifi_base.o *libbsp_base.o) .data*)
        SORT(*)(EXCLUDE_FILE(*.o) .wifi.rom.data*)
        SORT(*)(EXCLUDE_FILE(*.o) .data.system.rom*)
        SORT(*)(.rom.data.patch)
        SORT(*)(.sdata*)
        . = ALIGN(4);
        INCLUDE system_config.ld
        . = ALIGN(0x20);
        __data_end = .;
    } > RAM
    __data_size = __data_end - __data_start;
    .data_rom1 : AT ((((((((((((4248512 - (278K - 32) - 32) + (278K - 32)) + 32) + SIZEOF (.text_non_rom) + (0x20) - 1) & ~ ((0x20) - 1)) + SIZEOF (.zInit) + (0x20) - 1) & ~ ((0x20) - 1)) + SIZEOF (.ram_text) + (0x20) -1) & ~ ((0x20) - 1)) + SIZEOF(.data) + (0x20) - 1) & ~ ((0x20) - 1)))
    {
        __rom_data1_load = LOADADDR(.data_rom1);
        . = ALIGN(0x20);
        __rom_data1_begin__ = .;
        KEEP(SORT(*libgcc.a:save-restore.o)(.data*)) KEEP(SORT(*libgcc.a:_clzsi2.o)(.data*)) KEEP(SORT(*libgcc.a:_umoddi3.o)(.data*)) KEEP(SORT(*libgcc.a:_clz.o)(.data*)) KEEP(SORT(*libgcc.a:_ashldi3.o)(.data*)) KEEP(SORT(*libgcc.a:_lshrdi3.o)(.data*)) KEEP(SORT(*libgcc.a:fixdfsi.o)(.data*)) KEEP(SORT(*libgcc.a:_udivdi3.o)(.data*))
        KEEP(SORT(liblitekernel_base.o) (.data*))
        KEEP(SORT(libbsp_base.o) (.data*))
        KEEP(SORT(*)(.data.system.rom*))
        KEEP(SORT(libc_base.o) (.data*))
        KEEP(SORT(libsec_base.o) (.data*))
        KEEP(SORT(libwifi_base.o) (.data*))
        KEEP(SORT(*)(.wifi.rom.data*))
        . = ALIGN(4);
        __rom_data1_end__ = .;
    } > ROM_DATA1
    __rom_data1_size__ = __rom_data1_end__ - __rom_data1_begin__;
    .patch_bss (NOLOAD) :
    {
        __patch_bss_start = .;
        KEEP(SORT(*)(.fpb.remap))
        . = ALIGN(4);
        __patch_bss_end__ = .;
    } > PATCH_BSS
    .bss_rom (NOLOAD) :
    {
        . = ALIGN(0x20);
        __rom_bss_begin__ = .;
        __global_pointer$ = .;
        KEEP(SORT(*libgcc.a:save-restore.o)(.bss*)) KEEP(SORT(*libgcc.a:_clzsi2.o)(.bss*)) KEEP(SORT(*libgcc.a:_umoddi3.o)(.bss*)) KEEP(SORT(*libgcc.a:_clz.o)(.bss*)) KEEP(SORT(*libgcc.a:_ashldi3.o)(.bss*)) KEEP(SORT(*libgcc.a:_lshrdi3.o)(.bss*)) KEEP(SORT(*libgcc.a:fixdfsi.o)(.bss*)) KEEP(SORT(*libgcc.a:_udivdi3.o)(.bss*))
        KEEP(SORT(liblitekernel_base.o )(.bss*))
        KEEP(SORT(libc_base.o)(.bss*))
        KEEP(SORT(libsec_base.o)(.bss*))
        KEEP(SORT(libbsp_base.o)(.bss*))
        KEEP(SORT(libwifi_base.o) (.bss*))
        KEEP(SORT(*)(.wifi.rom.bss*))
        . = ALIGN(4);
        __rom_bss_end__ = .;
        SORT(*)(.extern.rom.bss*)
        . = ALIGN(4);
        __extern_rom_bss_end__ = .;
    } > ROM_BSS
    __rom_bss_size__ = __extern_rom_bss_end__ - __rom_bss_begin__;
    .extern_rom_data1_bss (NOLOAD) :
    {
    . = ALIGN(0x20);
       __extern_rom_data1_bss_begin__ = .;
        SORT(*)(.extern.rom.data1.bss*)
        . = ALIGN(4);
        __extern_rom_data1_bss_end__ = .;
    } > EXTERN_ROM_DATA1_BSS
    __extern_rom_data1_bss_size__ = __extern_rom_data1_bss_end__ - __extern_rom_data1_bss_begin__;
    __RAM_BEGIN__ = ORIGIN(RAM);
    __RAM_SIZE__ = LENGTH(RAM);
    __DOWNLOAD_IRQ_STACK = ORIGIN(ROM_BSS) - DOWNLOAD_STACK_SIZE;
    __DOWNLOAD_STACK_END = ORIGIN(ROM_BSS);
    .bss (NOLOAD) : ALIGN(0x20)
    {
        . = ALIGN(0x20);
        __bss_begin = .;
        SORT(*)(.bss*)
        SORT(*)(.rom.bss.patch)
        SORT(*)(.sbss*)
        . = ALIGN(0x20);
        __bss_end = .;
    } > RAM
    __bss_size__ = __bss_end - __bss_begin;
    _end = .;
    .heap (NOLOAD) :
    {
        . = ALIGN(64);
        __HEAP_BEGIN__ = ABSOLUTE(.);
    } > RAM
    __HEAP_SIZE__ = (__RAM_BEGIN__ + __RAM_SIZE__) - __HEAP_BEGIN__;
    .stack (NOLOAD) : ALIGN(0x20)
    {
        __SYSTEM_STACK_BEGIN__ = .;
        . += STACK_SIZE;
        __SYSTEM_STACK_END__ = .;
        . = ALIGN(0x20);
        __irq_stack_bottom = .;
        . += IRQ_STACK_SIZE;
        __irq_stack_top = .;
 . = ALIGN(0x20);
        __nmi_stack_bottom = .;
        . += NMI_STACK_SIZE;
        __nmi_stack_top = .;
        . = ALIGN(0x20);
        __exc_stack_bottom = .;
        . += EXC_STACK_SIZE;
        __exc_stack_top = .;
    } > STACK
    __SYSTEM_STACK_SIZE__ = __SYSTEM_STACK_END__ - __SYSTEM_STACK_BEGIN__;
    .check_info (NOLOAD) :
    {
        __check_info_addr = .;
        KEEP(SORT(*)(.lowpower.ram.bss*))
    } > CHECK_INFO
    __ram_start = 0x000d8000;
    __ram_end = 0x000d8000 + 280K;
    . = ALIGN(4);
    end = . ;
}
