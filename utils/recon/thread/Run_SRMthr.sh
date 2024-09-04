#!/bin/bash
# sh thread/Run_SRMthr.sh
cd thread/
rm -rf srmthr
gcc *.c -o srmthr -lpthread -lm
./srmthr