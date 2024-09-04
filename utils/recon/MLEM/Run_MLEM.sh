#!/bin/bash
# bash utils/MLEM/Run_MLEM.sh

rm  -f utils/lib/MLEM_exe
gcc -c utils/MLEM/*.c
gcc -o utils/lib/MLEM_exe *.o -lm
rm  -f *.o

./utils/lib/MLEM_exe 134341.600_NA22_B0F1S13-B1F1S13_HV1000_2S_POSX0mm 10 60 0.8 75s