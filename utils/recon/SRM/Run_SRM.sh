#!/bin/bash
# bash utils/SRM/Run_SRM.sh

rm  -f utils/lib/SRM_exe
gcc -c utils/SRM/*.c  # Compilation and assembly
gcc -o utils/lib/SRM_exe *.o -lm  # Linking. Flags: -l(linker), -m(preprocessor)
rm  -f *.o

./utils/lib/SRM_exe 60 0.8 75