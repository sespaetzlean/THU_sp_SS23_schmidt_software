@echo off
cd /D F:\Github\THU_student_project_SS23_schmidt\Software\aktor_relais\dk_build\zephyr\kconfig || (set FAIL_LINE=2& goto :ABORT)
D:\ncs\toolchains\v2.3.0\opt\bin\cmake.exe -E env ZEPHYR_BASE=D:/ncs/v2.3.0/zephyr PYTHON_EXECUTABLE=D:/ncs/toolchains/v2.3.0/opt/bin/python.exe srctree=D:/ncs/v2.3.0/zephyr KERNELVERSION=0x3026300 CONFIG_=CONFIG_ KCONFIG_CONFIG=F:/Github/THU_student_project_SS23_schmidt/Software/aktor_relais/dk_build/zephyr/.config ARCH=arm ARCH_DIR=D:/ncs/v2.3.0/zephyr/arch BOARD_DIR=D:/ncs/v2.3.0/zephyr/boards/arm/nrf52dk_nrf52832 BOARD_REVISION= KCONFIG_BINARY_DIR=F:/Github/THU_student_project_SS23_schmidt/Software/aktor_relais/dk_build/Kconfig ZEPHYR_TOOLCHAIN_VARIANT=zephyr TOOLCHAIN_KCONFIG_DIR=D:/ncs/toolchains/v2.3.0/opt/zephyr-sdk/cmake/zephyr TOOLCHAIN_HAS_NEWLIB=y EDT_PICKLE=F:/Github/THU_student_project_SS23_schmidt/Software/aktor_relais/dk_build/zephyr/edt.pickle NCS_MEMFAULT_FIRMWARE_SDK_KCONFIG=D:/ncs/v2.3.0/nrf/modules/memfault-firmware-sdk/Kconfig ZEPHYR_NRF_MODULE_DIR=D:/ncs/v2.3.0/nrf ZEPHYR_HOSTAP_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/hostap ZEPHYR_MCUBOOT_MODULE_DIR=D:/ncs/v2.3.0/bootloader/mcuboot ZEPHYR_MCUBOOT_KCONFIG=D:/ncs/v2.3.0/nrf/modules/mcuboot/Kconfig ZEPHYR_MBEDTLS_MODULE_DIR=D:/ncs/v2.3.0/modules/crypto/mbedtls ZEPHYR_MBEDTLS_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/mbedtls/Kconfig ZEPHYR_TRUSTED_FIRMWARE_M_MODULE_DIR=D:/ncs/v2.3.0/modules/tee/tf-m/trusted-firmware-m ZEPHYR_TRUSTED_FIRMWARE_M_KCONFIG=D:/ncs/v2.3.0/nrf/modules/trusted-firmware-m/Kconfig ZEPHYR_CJSON_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/cjson ZEPHYR_CJSON_KCONFIG=D:/ncs/v2.3.0/nrf/modules/cjson/Kconfig ZEPHYR_AZURE_SDK_FOR_C_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/azure-sdk-for-c ZEPHYR_AZURE_SDK_FOR_C_KCONFIG=D:/ncs/v2.3.0/nrf/modules/azure-sdk-for-c/Kconfig ZEPHYR_MEMFAULT_FIRMWARE_SDK_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/memfault-firmware-sdk ZEPHYR_CIRRUS_LOGIC_MODULE_DIR=D:/ncs/v2.3.0/modules/hal/cirrus-logic ZEPHYR_OPENTHREAD_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/openthread ZEPHYR_OPENTHREAD_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/openthread/Kconfig ZEPHYR_CANOPENNODE_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/canopennode ZEPHYR_CANOPENNODE_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/canopennode/Kconfig ZEPHYR_CHRE_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/chre ZEPHYR_CMSIS_MODULE_DIR=D:/ncs/v2.3.0/modules/hal/cmsis ZEPHYR_FATFS_MODULE_DIR=D:/ncs/v2.3.0/modules/fs/fatfs ZEPHYR_FATFS_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/fatfs/Kconfig ZEPHYR_HAL_NORDIC_MODULE_DIR=D:/ncs/v2.3.0/modules/hal/nordic ZEPHYR_HAL_NORDIC_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/hal_nordic/Kconfig ZEPHYR_ST_MODULE_DIR=D:/ncs/v2.3.0/modules/hal/st ZEPHYR_HAL_WURTHELEKTRONIK_MODULE_DIR=D:/ncs/v2.3.0/modules/hal/wurthelektronik ZEPHYR_LIBMETAL_MODULE_DIR=D:/ncs/v2.3.0/modules/hal/libmetal ZEPHYR_LIBLC3_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/liblc3 ZEPHYR_LIBLC3_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/liblc3/Kconfig ZEPHYR_LITTLEFS_MODULE_DIR=D:/ncs/v2.3.0/modules/fs/littlefs ZEPHYR_LITTLEFS_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/littlefs/Kconfig ZEPHYR_LORAMAC_NODE_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/loramac-node ZEPHYR_LORAMAC_NODE_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/loramac-node/Kconfig ZEPHYR_LVGL_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/gui/lvgl ZEPHYR_LZ4_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/lz4 ZEPHYR_LZ4_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/lz4/Kconfig ZEPHYR_MIPI_SYS_T_MODULE_DIR=D:/ncs/v2.3.0/modules/debug/mipi-sys-t ZEPHYR_NANOPB_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/nanopb ZEPHYR_NANOPB_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/nanopb/Kconfig ZEPHYR_NRF_HW_MODELS_MODULE_DIR=D:/ncs/v2.3.0/modules/bsim_hw_models/nrf_hw_models ZEPHYR_OPEN_AMP_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/open-amp ZEPHYR_PICOLIBC_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/picolibc ZEPHYR_SEGGER_MODULE_DIR=D:/ncs/v2.3.0/modules/debug/segger ZEPHYR_TINYCBOR_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/tinycbor ZEPHYR_TINYCRYPT_MODULE_DIR=D:/ncs/v2.3.0/modules/crypto/tinycrypt ZEPHYR_TRACERECORDER_MODULE_DIR=D:/ncs/v2.3.0/modules/debug/TraceRecorder ZEPHYR_TRACERECORDER_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/TraceRecorder/Kconfig ZEPHYR_TF_M_TESTS_MODULE_DIR=D:/ncs/v2.3.0/modules/tee/tf-m/tf-m-tests ZEPHYR_PSA_ARCH_TESTS_MODULE_DIR=D:/ncs/v2.3.0/modules/tee/tf-m/psa-arch-tests ZEPHYR_UOSCORE_UEDHOC_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/uoscore-uedhoc ZEPHYR_UOSCORE_UEDHOC_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/uoscore-uedhoc/Kconfig ZEPHYR_ZCBOR_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/zcbor ZEPHYR_ZCBOR_KCONFIG=D:/ncs/v2.3.0/zephyr/modules/zcbor/Kconfig ZEPHYR_ZSCILIB_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/zscilib ZEPHYR_NRFXLIB_MODULE_DIR=D:/ncs/v2.3.0/nrfxlib ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR=D:/ncs/v2.3.0/modules/lib/matter SHIELD_AS_LIST= DTS_POST_CPP=F:/Github/THU_student_project_SS23_schmidt/Software/aktor_relais/dk_build/zephyr/zephyr.dts.pre DTS_ROOT_BINDINGS=D:/ncs/v2.3.0/nrf/dts/bindings?D:/ncs/v2.3.0/zephyr/dts/bindings D:/ncs/toolchains/v2.3.0/opt/bin/python.exe D:/ncs/v2.3.0/zephyr/scripts/kconfig/guiconfig.py D:/ncs/v2.3.0/zephyr/Kconfig || (set FAIL_LINE=3& goto :ABORT)
goto :EOF

:ABORT
set ERROR_CODE=%ERRORLEVEL%
echo Batch file failed at line %FAIL_LINE% with errorcode %ERRORLEVEL%
exit /b %ERROR_CODE%