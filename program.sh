#! /bin/bash

sudo chmod 777 /dev/ttyUSB0
cp main/modified-drivers/* $IDF_PATH/components/driver/
make flash && make monitor
