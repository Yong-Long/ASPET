import os
import time
import ctypes
import numpy as np
from ctypes import *
import matplotlib.pyplot as plt

def MLEM_reconstruction(self, bin_name, iterations, xplanes, spacing):
    # ====== ====== Sorting hist before ML-EM ====== ====== #
    #self.log_label.setText(f'Running ML-EM recon with {iterations} iters, {xplanes} planes and spacing {spacing}mm ...')
    # -- Initialize zero array of hist
    Hist_o = np.zeros(512*512).astype('float32')
    
    # -- Load sorted LOR (Z1,Y1,Z2,Y2) pairs from text file 
    LORzy_path = f"./data/LOR_ZY_{bin_name}.txt"
    LORzy_i = np.loadtxt(LORzy_path, dtype=np.int64)
    
    for idx in range(len(LORzy_i)):
        Z1, Y1, Z2, Y2 = LORzy_i[idx]
        
        # -- Hist global mapping in col-major (Matlab)
        cID1 = Z1 * 16 + Y1
        cID2 = Z2 * 16 + Y2
        
        # -- Histogram counting
        Hist_o[cID2*32*16 + cID1] += 1
    
    # -- Save hist to .dat file
    Hist_o.tofile(f"./MLEM/INPUT/SORTED_Hist_{bin_name}.dat")
    print(f"\n[Save] Sorted Hist: ./MLEM/INPUT/SORTED_Hist_{bin_name}.dat")
    print(f"[Dialog] Total iterations: {iterations}, recon planes: {xplanes}")
    '''
    # [Debug] -- Check array size of hist .dat
    fHist_i = "./function/_data/SORTED_Hist_PointSource.dat"
    Hist_i = np.fromfile(fHist_i, dtype=np.float32)
    print(f"\n[Check] fHist_i = {fHist_i}")
    print(f"np.shape(Hist_o) = {np.shape(Hist_o)}")
    print(f"np.shape(Hist_i) = {np.shape(Hist_i)}\n")
    '''
    
    # ====== ====== Run progress of ML-EM ====== ====== #
    print("\n[Main] Start function of libmlem.so...\n")
    start = time.time()
    
    3angle_lib = CDLL("./function/libmlem.so")
    3angle_lib.main.restype = None
    3angle_lib.main.argtypes = [c_int, POINTER(c_char_p)]
    argv = (c_char_p * 5)(b'libmlem.so', bytes(bin_name,encoding='utf-8'), bytes(iterations,encoding='utf-8'), \
            bytes(xplanes,encoding='utf-8'), bytes(spacing,encoding='utf-8'))
    3angle_lib.main(len(argv), argv)
    
    end = time.time()
    print(f"\n[Time] 3-angle ML-EM: {end-start:.2f}s")
    #self.log_label.setText(f'[ML-EM] Finish! Total iters: {iterations}, xplanes: {xplanes}, time: {end-start:.0f}s')

