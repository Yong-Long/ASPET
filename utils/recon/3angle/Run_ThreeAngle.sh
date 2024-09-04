#!/bin/bash
"""
# Usage: 
cd utils/recon/3angle/
bash Run_ThreeAngle.sh
"""

# 3-angle recon
rm -f ThreeAngle_exe
gcc -c *.c
gcc -o ThreeAngle_exe *.o -lm
rm -f *.o

# Run ThreeAngle exe with arguments (1. bin file, 2. iterations, 3. sapcing, 4. voxel size (x), 5. recon planes)
./ThreeAngle_exe 134341.600_NA22_B0F1S13-B1F1S13_HV1000_2S_POSX0mm 10 256 0.8 320