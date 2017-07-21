#! /bin/bash

cp main/modified-drivers/* ../esp-idf/components/driver/
make flash && make monitor
