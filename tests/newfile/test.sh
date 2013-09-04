#!/bin/bash
rm -f 1.c
rm -f 2.c
cp 1.c.initial 1.c
../../spank rebuild
sleep 1
cp 1.c.new 1.c
cp 2.c.new 2.c
../../spank --jobs 1 build
#gdb --args ../../spank --jobs 1 --verbosity 0 build
