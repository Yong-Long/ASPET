#! /bin/bash
# bash utils/function/build_sharelib.sh

rm -rf data/src/tmp/
rm -rf utils/lib/*.so

g++ -fPIC utils/function/extract_data.cpp -shared -o utils/lib/extract_data.so -Wall
g++ -fPIC utils/function/extract_entries_distribution.cpp -shared -o utils/lib/extract_entries_distribution.so -Wall
g++ -fPIC utils/function/extract_energy_information.cpp -shared -o utils/lib/extract_energy_info.so -Wall
g++ -fPIC utils/function/reconstruction.cpp -shared -o utils/lib/reconstruction.so -Wall
g++ -fPIC utils/function/sorting.cpp -shared -o utils/lib/sorting.so -Wall

gcc -fPIC utils/recon/SRM/*.c -shared -o utils/lib/libsrm.so
gcc -fPIC utils/recon/MLEM/*.c -shared -o utils/lib/libmlem.so
gcc -fPIC utils/recon/3angle/*.c -shared -o utils/lib/lib3angle.so