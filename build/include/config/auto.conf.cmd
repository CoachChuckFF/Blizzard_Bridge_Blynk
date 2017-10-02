deps_config := \
	/home/ece453/Documents/ece453/esp-idf/components/app_trace/Kconfig \
	/home/ece453/Documents/work/blizzard-esp/components/arduino/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/aws_iot/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/bt/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/esp32/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/ethernet/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/fatfs/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/freertos/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/heap/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/log/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/lwip/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/mbedtls/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/openssl/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/pthread/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/spi_flash/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/spiffs/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/tcpip_adapter/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/wear_levelling/Kconfig \
	/home/ece453/Documents/ece453/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/ece453/Documents/ece453/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/ece453/Documents/ece453/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/ece453/Documents/ece453/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
