#!/bin/sh

cp -rf . ~/qmk_firmware/keyboards/massdrop/alt/
qmk compile -kb massdrop/alt -km mbednarek360
mv ~/qmk_firmware/massdrop_alt_mbednarek360.bin firmware.bin
