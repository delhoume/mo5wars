#!/bin/bash
mkdir -p build/disk


cmoc 	-o build/MO5WARS.BIN --thommo -funsigned-char --org=2600 -Wno-assign-in-condition -D__CMOC__ -I src \
  src/main.c src/reader.c src/filereader.c src/zxdecoder.c src/dzx0.c 


if [ ! -f tools/fdfs ]; then
  gcc tools/src/fdfs.c -o tools/fdfs
fi

mkdir -p  output

pushd build/disk
cp ../../dist/ANIM64.ZXS .
cp ../../build/MO5WARS.BIN   .
cp ../../assets/BOOTMO.BIN .
../../tools/fdfs -addBL ../../output/mo5wars.fd  BOOTMO.BIN MO5WARS.BIN ANIM64.ZXS
popd

ls -la output/*.fd