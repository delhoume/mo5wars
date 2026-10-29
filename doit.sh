#!/bin/bash

# uses standard unix tools as well as lax qa on the`` go compiled utilities



mkdir -p tmp


# requires gnu parallel (
# remove delays
cat assets/asciiwars.txt | awk '!(NR%14==1)'> tmp/asciiwars_no_delays.txt

# isolate delays
cat assets/asciiwars.txt | awk 'NR%14==1' > tmp/asciiwars_delays.txt
# and creates include 

if [ ! -f tools/gendelays ]; then
  gcc tools/src/gendelays.c -o tools/gendelays
fi
tools/gendelays tmp/asciiwars_delays.txt src/delays.h

#pads to width 67 width and crops to 64

if [ ! -f tools/build67 ]; then
  g++ -o tools/build67 tools/src/build67.cpp
fi
tools/build67 tmp/asciiwars_no_delays.txt tmp/asciiwars67.txt tmp/asciiwars64.txt

rm -f parts/*/


# extract all frames
# nmkdir -p parts/alframes64
# split -a 4 -d -b 832 asciiwars64 parts/allframes64/f_
	
mkdir -p parts/64
split -a 3 -b  8320 -d tmp/asciiwars64.txt parts/64/f_


if [ ! -f tools/zx0 ]; then
 echo "tools/zx0 not found, download and build from github or appropriate source"
fi
if [ ! -f tools/salvador ]; then
  echo "tools/salvador not found, download and build from github or appropriate source"
fi

# compress parts
ls -1 parts/64/f_??? | parallel --bar tools/salvador -classic {} {//}/s_{/.}.zx0
ls -1 parts/64/f_??? | parallel --bar tools/zx0 -c -f  {} {//}/d_{/.}.zx0

## create one final file
if [ ! -f tools/buildfred ]; then
  gcc tools/src/buildfred.c -o tools/buildfred
fi

tools/buildfred tmp/asciiwars64.txt tmp/asciiwars64_salvador.zxs 10 "parts/64/s_f_" 64 13 10
tools/buildfred tmp/asciiwars64.txt tmp/asciiwars64_dzx0.zxs 10 "parts/64/d_f_" 64 13 10

#list results
ls -la tmp/asciiwars64_*.zxs


mkdir -p dist
# usually the smallest
cp -f tmp/asciiwars64_dzx0.zxs dist/ANIM64.ZXS
