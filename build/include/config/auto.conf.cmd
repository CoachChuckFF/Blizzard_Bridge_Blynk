deps_config := \
	/home/penguin/Programio/esp-idf/components/app_trace/Kconfig \
	/home/penguin/Programio/blizzard-esp/components/arduino/Kconfig \
	/home/penguin/Programio/esp-idf/components/aws_iot/Kconfig \
	/home/penguin/Programio/esp-idf/components/bt/Kconfig \
	/home/penguin/Programio/esp-idf/components/esp32/Kconfig \
	/home/penguin/Programio/esp-idf/components/ethernet/Kconfig \
	/home/penguin/Programio/esp-idf/components/fatfs/Kconfig \
	/home/penguin/Programio/esp-idf/components/freertos/Kconfig \
	/home/penguin/Programio/esp-idf/components/log/Kconfig \
	/home/penguin/Programio/esp-idf/components/lwip/Kconfig \
	/home/penguin/Programio/esp-idf/components/mbedtls/Kconfig \
	/home/penguin/Programio/esp-idf/components/openssl/Kconfig \
	/home/penguin/Programio/esp-idf/components/spi_flash/Kconfig \
	/home/penguin/Programio/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/penguin/Programio/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/penguin/Programio/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/penguin/Programio/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
