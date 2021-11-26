#!/bin/sh

PORT=/dev/ttyACM1
echo 'Waiting for DFU...'
until [ -e $PORT ]; do sleep 1; done
mdloader -p $PORT M -D firmware.bin --restart
