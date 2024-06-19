#!/bin/bash
set -e
TOPDIR=`pwd`/../
WORKDIR=`pwd`

## rtthread.bin (cat_eye demo)
cd ${TOPDIR}/cat_eye_demo
make clean && make
cp rtthread.bin ${WORKDIR}/rt_thread/
make clean
