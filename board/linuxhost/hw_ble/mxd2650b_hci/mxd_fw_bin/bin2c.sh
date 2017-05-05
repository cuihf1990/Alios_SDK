#!/bin/sh

FILE_NAME="ble_iram_mdf"
ARR_NAME="g_ble_iram"

echo "#include <stdint.h>" > ${FILE_NAME}.c
echo "uint8_t ${ARR_NAME}[]={" >> ${FILE_NAME}.c
hexdump -e '16/1 "0x%02X," "\n"' ${FILE_NAME}.bin | sed 's/,0x  //g' >> ${FILE_NAME}.c
echo "};" >> ${FILE_NAME}.c
echo "uint32_t ${ARR_NAME}_size = sizeof(${ARR_NAME});" >> ${FILE_NAME}.c


FILE_NAME="rf_iram_mdf"
ARR_NAME="g_rf_iram"

echo "#include <stdint.h>" > ${FILE_NAME}.c
echo "uint8_t ${ARR_NAME}[]={" >> ${FILE_NAME}.c
hexdump -e '16/1 "0x%02X," "\n"' ${FILE_NAME}.bin | sed 's/,0x  //g' >> ${FILE_NAME}.c
echo "};" >> ${FILE_NAME}.c
echo "uint32_t ${ARR_NAME}_size = sizeof(${ARR_NAME});" >> ${FILE_NAME}.c
