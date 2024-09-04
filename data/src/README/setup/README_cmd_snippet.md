# Command snippet

## 1. Deploy SRM files

a. Extract pre-generated SRM files

```bash
tar -xzvf ~/Downloads/_srm_sp60_vx0.8_x75.zip -C ~/Desktop/ASPET_software/data/SRM/
tar -xzvf ~/Downloads/_srm_sp256_vx0.8_x320.zip -C ~/Desktop/ASPET_software/data/SRM/
unzip ~/Downloads/_srm_sp60_vx0.8_x75.zip

rm -rf SRM/_srm_sp60_vx0.8_x75/
mv ~/Downloads/_srm_sp60_vx0.8_x75/ SRM/_srm_sp60_vx0.8_x75/

rm -rf MLEM/*.c MLEM/*.h
cp *.c *.h ~/Desktop/ASPET_software/data/src/tmp/
```

b. Use 'SRM gen' function in software

## 2. Compile MLEM, SRM gen & 3-angle program

```bash
# ====== ====== MLEM recon ====== ======
rm -f utils/lib/MLEM_exe utils/lib/libmlem.so
gcc -c utils/recon/MLEM/*.c -g
gcc -o utils/lib/MLEM_exe *.o -lm
rm -f *.o
gcc -fPIC utils/recon/MLEM/*.c -shared -o utils/lib/libmlem.so
./utils/lib/MLEM_exe 134341.600_NA22_B0F1S13-B1F1S13_HV1000_2S_POSX0mm 3 60 0.8 25
# [Args]: iteration, spacing, vx, xplanes

# ====== ====== 3-angle recon ====== ======
rm -f utils/lib/3angle_exe utils/lib/lib3angle.so
gcc -c utils/recon/3angle/*.c -g
gcc -o utils/lib/3angle_exe *.o -lm
rm -f *.o
gcc -fPIC utils/recon/3angle/*.c -shared -o utils/lib/lib3angle.so
./utils/lib/3angle_exe 134341.600_NA22_B0F1S13-B1F1S13_HV1000_2S_POSX0mm 3 256 0.8 75
# [GDB] gdb -args ./utils/lib/3angle_exe 134341.600_NA22_B0F1S13-B1F1S13_HV1000_2S_POSX0mm 10 256 0.8 320

# ====== ====== SRM generation ====== ======
rm -f utils/lib/SRM_exe utils/lib//libsrm.so
gcc -c utils/recon/SRM/*.c
gcc -o utils/lib/SRM_exe *.o -lm
rm -f *.o
gcc -fPIC utils/recon/SRM/*.c -shared -o utils/lib//libsrm.so
./utils/lib/SRM_exe 60 0.8 75
# [Args]: spacing, vx, xplanes

# ====== ====== SRM threading ====== ======
cd utils/recon/thread/
rm -rf utils/lib/srmthr
gcc *.c -o utils/lib/srmthr -lpthread -lm -g
gcc *.c -o utils/lib/srmthr -lm -g
./utils/lib/srmthr 60 0.8 75
```

## 3. Setup main software

```bash
rm -f utils/lib/*.so
rm -rf .vscode/ */__pycache__/ */*/__pycache__/ */*/*/__pycache__/
bash utils/function/build_sharelib.sh
python3 main_window.py
```
