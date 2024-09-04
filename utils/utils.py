import os
import time
import json
import ctypes
import numpy as np
from ctypes import *
from PIL import Image
import matplotlib
import matplotlib.pyplot as plt


class Utility ():
    
    # ====== [module] data preprocessing ====== #
    def data_preprocess (self):
        '''
        For binary data: 
        ** Seperate the process from utils.FBP_recon() originally **
        1. extract_data() -> frame data & info
        2. sorting() -> LOR Z-Y pairs
        '''
        start = time.time()
        input_name = os.path.basename(self.input_file)
        input_path = f"./data/bin/{self.input_file}"
        
        # (1) Extract binary data 
        lib = CDLL("utils/lib/extract_data.so")
        lib.extract_data(bytes(input_path, encoding='utf-8'))
        print("[dbug] finish step 1 of data process")
        
        # (2) Load frame data 
        with open('data/src/tmp/frame_info.txt','r') as fFrame_info:
            frame_info = (fFrame_info.readlines()[0]).split('\t') 
        fFrame_info.close()

        c_array = []
        for i in range(len(frame_info) - 1):
            if int(frame_info[i]) > 0:
                c_array.append(int(frame_info[i]))
        
        # (3) Sort LORs with global IDs 
        sorting_lib = CDLL("utils/lib/sorting.so")
        sorting_lib.argtypes = [c_char_p, c_char_p, POINTER(c_int)]

        frame_path = 'data/src/tmp/frame'
        frame_info_c = (c_int * 64)(*c_array)
        sorting_lib.sorting(bytes(frame_path, encoding='utf-8'), bytes(input_name, encoding='utf-8'), frame_info_c)
        print(f"\n[Save] LOR global ID pairs: LOR_B0B1_{input_name}.txt")

        # (4) Sort LORs into Z-Y coordinates
        LOR_ZY = []

        # - Load global ID pairs of Board 0, 1 -
        with open(f"data/recon/LOR/{input_name}/LOR_B0B1_{input_name}.txt","r") as fLOR_B0B1:
            LOR_IDs = fLOR_B0B1.readlines()
        fLOR_B0B1.close()
        
        for LOR in LOR_IDs:
            LOR_B0 = int(LOR.split('\t')[0])       # - Read global ID (0   -  511) of 1st column -
            LOR_B1 = int(LOR.split('\t')[1][:-1])  # - Read global ID (512 - 1023) of 2nd column -
            
            # - Parse gmsl/stic/channel ID in board 0 -
            gmsl_0    = (LOR_B0 // 64) // 4
            stic_0    = (LOR_B0 // 64) % 4
            channel_0 = LOR_B0 % 64
            
            # - Convert into Z-Y axis of board 0 -
            Z_1 = self.board_map_z(channel_0, stic_0, gmsl_0)
            Y_1 = self.board_map_y(channel_0, stic_0, gmsl_0)
            
            # - Parse gmsl/stic/channel ID in board 1 -
            gmsl_1    = (LOR_B1 // 64) // 4
            stic_1    = (LOR_B1 // 64) % 4
            channel_1 = LOR_B1 % 64
            
            # - Convert into Z-Y axis of board 1 -
            Z_2 = self.board_map_z(channel_1, stic_1, gmsl_1)
            Y_2 = self.board_map_y(channel_1, stic_1, gmsl_1)
            
            # - Collect LOR Z-Y pairs (Z1 Y1 Z2 Y2) for MLEM -
            LOR_ZY.append("%2d\t%2d\t%2d\t%2d\n" % (Z_1, Y_1, Z_2, Y_2))
        
        # - Dump LOR Z-Y pairs into text file -
        with open(f"data/recon/LOR/{input_name}/LOR_ZY_{input_name}.txt","w") as fLOR_zy:
            fLOR_zy.writelines(LOR_ZY)
        fLOR_zy.close()

        end = time.time()
        print("[Time] Extract file: %.2f" % (end - start))

    # ====== [function] map index z of board ====== #
    def board_map_z (self, channel, sticID, gmslID):

        if   channel in [58, 57, 54, 52, 46, 44, 38, 37]:
            z = 0
        elif channel in [60, 59, 56, 50, 48, 42, 40, 36]:
            z = 1
        elif channel in [61, 55, 53, 47, 45, 39, 32, 35]:
            z = 2
        elif channel in [63, 62, 51, 49, 43, 41, 34, 33]:
            z = 3
        elif channel in [ 0,  1, 12, 14, 20, 22, 29, 30]:
            z = 4
        elif channel in [ 2,  8, 10, 16, 18, 24, 31, 28]:
            z = 5
        elif channel in [ 3,  4,  7, 13, 15, 21, 23, 27]:
            z = 6
        elif channel in [ 5,  6,  9, 11, 17, 19, 25, 26]:
            z = 7
        # print(f"\nsticID = {sticID}, gmslID = {gmslID}, \nchannel = {channel}, z = {z}")
        
        if   (sticID // 2) == 0:
            z = z
        elif (sticID // 2) == 1:
            z = 7 - z
        z = (gmslID % 2) * 16 + (sticID % 2) * 8 + z

        return z

    # ====== [function] map index y of board ====== #
    def board_map_y (self, channel, sticID, gmslID):
        
        if   channel in [58, 60, 61, 63,  0,  2,  3,  5]:
            y = 0
        elif channel in [57, 59, 55, 62,  1,  8,  4,  6]:
            y = 1
        elif channel in [54, 56, 53, 51, 12, 10,  7,  9]:
            y = 2
        elif channel in [52, 50, 47, 49, 14, 16, 13, 11]:
            y = 3
        elif channel in [46, 48, 45, 43, 20, 18, 15, 17]:
            y = 4
        elif channel in [44, 42, 39, 41, 22, 24, 21, 19]:
            y = 5
        elif channel in [38, 40, 32, 34, 29, 31, 23, 25]:
            y = 6
        elif channel in [37, 36, 35, 33, 30, 28, 27, 26]:
            y = 7
        # print(f"\nsticID = {sticID}, gmslID = {gmslID},\n channel = {channel}, y = {y}")
        
        if   (sticID // 2) ^ (gmslID //2) == 0:
            y = y
        elif (sticID // 2) ^ (gmslID //2) == 1:
            y = 7 - y
        y = ((sticID // 2) ^ (gmslID // 2)) * 8 + y

        return y
    
    # ====== [module] update content of config file ====== #
    def config_onchange (self):
        # - set configuration of input file in config.json -
        self.config['task'] = self.task
        self.config['name'] = self.input_file[:-4]
        self.config['type'] = self.type
        
        if self.task != 'input':
            self.config['sp'] = self.spacing
            self.config['vx'] = self.vx
            self.config['xp'] = self.xplane
        
        with open("data/src/tmp/config.json","w") as fConfig:
            json.dump(self.config, fConfig)
        fConfig.close()

    
    ### ====== ====== ====== ====== ====== ====== ====== ====== ====== ###
    ### ====== ====== ======  FBP reconstruction  ====== ====== ====== ###
    ### ====== ====== ====== ====== ====== ====== ====== ====== ====== ###

    def FBP_reconstruction (self):
        """
        # ====== ====== 1. Load frame data from text file ====== ======

        with open('data/src/tmp/frame_info.txt','r') as fFrame_info:
            frame_info = (fFrame_info.readlines()[0]).split('\t')
        fFrame_info.close()

        c_array = []
        for i in range(len(frame_info) - 1):
            if int(frame_info[i]) > 0:
                c_array.append(int(frame_info[i]))
        
        # ====== ====== 2. Coincidence sorting (global ID pairs) ====== ======
        
        # - Sorting coincidences to global ID B0, B1 -
        print("\n[Stat] Sorting coincidence data: ")
        start = time.time()
        sorting_lib = CDLL("utils/lib/sorting.so")
        sorting_lib.argtypes = [c_char_p, c_char_p, POINTER(c_int)]

        frame_path = 'data/src/tmp/frame'
        frame_info_c = (c_int * 64)(*c_array)
        sorting_lib.sorting(bytes(frame_path, encoding='utf-8'), bytes(self.input_name, encoding='utf-8'), frame_info_c)
        
        end = time.time()
        print(f"\n[Save] Global LOR pairs: data/recon/LOR/LOR_B0B1_{self.input_name}.txt")
        print(f"[Time] Global coincidence sorting: {end-start:.2f}")
        
        # ====== Initialize recon image ======
        recon_img = np.zeros((16*4, 32*4))

        resol = [[ 4, 12, 12,  4],
                [12, 36, 36, 12],
                [12, 36, 36, 12],
                [ 4, 12, 12,  4]]
        # sum_resol = np.sum(resol)  # [Unused]

        # ====== ====== ====== 3. Z-Y pairs mapping ====== ====== ======
        
        LOR_zy    = list()
        key_pairs = dict()
        start = time.time()
        
        # - Load LOR global ID pairs of Board 0 & 1 -
        with open(f"data/recon/LOR/{self.input_name}/LOR_B0B1_{self.input_name}.txt","r") as fLOR_B0B1:
            LOR_B0B1s = fLOR_B0B1.readlines()
            fLOR_B0B1.close()
        
        for LOR_B0B1 in LOR_B0B1s:
            LOR_B0 = int(LOR_B0B1.split('\t')[0])       # - Read global ID (0   -  511) of 1st column -
            LOR_B1 = int(LOR_B0B1.split('\t')[1][:-1])  # - Read global ID (512 - 1023) of 2nd column -
            
            # - Parse gmsl/stic/channel ID in board 0 -
            gmsl_0    = (LOR_B0 // 64) // 4
            stic_0    = (LOR_B0 // 64) % 4
            channel_0 = LOR_B0 % 64
            
            # - Convert into Z-Y axis of board 0 -
            Z_1 = board_map_z(channel_0, stic_0, gmsl_0)
            Y_1 = board_map_y(channel_0, stic_0, gmsl_0)
            
            # - Parse gmsl/stic/channel ID in board 1 -
            gmsl_1    = (LOR_B1 // 64) // 4
            stic_1    = (LOR_B1 // 64) % 4
            channel_1 = LOR_B1 % 64
            
            # - Convert into Z-Y axis of board 1 -
            Z_2 = board_map_z(channel_1, stic_1, gmsl_1)
            Y_2 = board_map_y(channel_1, stic_1, gmsl_1)
            
            # - Collect LOR pairs (Z1 Y1 Z2 Y2) for MLEM -
            LOR_zy.append("%2d\t%2d\t%2d\t%2d\n" % (Z_1, Y_1, Z_2, Y_2))
            
            # [Debug] - Print calculation -
            # print(f"\n\nd_0 = {d_0} \n gmsl_0({gmsl_0}) = (({d_0}) // 64) // 4 \n stic_0({stic_0}) = (({d_0}) // 64) % 4 \n channel_0({channel_0}) = ({d_0}) % 64")
            # print(f"\nabs_loc_0 in C logic: {abs_loc_0}")
            # print(f"\n\nd_1 = {d_1} \n gmsl_1({gmsl_1}) = (({d_1}) // 64) // 4 \n stic_1({stic_1}) = (({d_1}) // 64) % 4 \n channel_1({channel_1}) = ({d_1}) % 64")
            # print(f"\nabs_loc_1 in C: {abs_loc_1}")
            # print(f"sum_z({sum_z}) = abs_loc_0[0]({abs_loc_0[0]}) + abs_loc_1[0]({abs_loc_1[0]})")
            # print(f"sum_y({sum_y}) = abs_loc_0[1]({abs_loc_0[1]}) + abs_loc_1[1]({abs_loc_1[1]})")
            
            # [Unused] - Record coordinates -
            pairs_name = str(sum_z) + " " + str(sum_y)
            if pairs_name not in key_pairs.keys():
                key_pairs[pairs_name] = 1
            else:
                key_pairs[pairs_name] += 1
        """
        
        ### ====== ====== ====== [FBP] 4. FBP reconstruction ====== ====== ====== ###
        '''
        [Args]: self.input_name
        '''
        start = time.time()
        print("\n[Stat] Start FBP reconstruction ...")

        # === Initialize matrix for recon image ===
        FBP_image = np.zeros((16*4, 32*4))
        # - Defined filter -
        resol = [[ 4, 12, 12,  4],
                 [12, 36, 36, 12],
                 [12, 36, 36, 12],
                 [ 4, 12, 12,  4]]
        
        # === Read LOR ZY pairs from file ===
        with open(f"data/recon/LOR/{self.input_name}/LOR_ZY_{self.input_name}.txt","r") as fLORs_ZY:
            LORs_ZY = fLORs_ZY.readlines().split('\t')
        fLORs_ZY.close()
        print(f"\n[Test] @FBP LORs_ZY: {LORs_ZY[:10]}")

        for LORZY in LORs_ZY:
            # - Load an LOR Z-Y pair -
            LOR_Z = LORZY[  :2]
            LOR_Y = LORZY[-2: ]
            # - Sum Z & Y set as input of FBP -
            sum_z = LOR_Z[0] + LOR_Z[1]
            sum_y = LOR_Y[0] + LOR_Y[1]

            if (sum_z % 2 != 0) and (sum_y % 2 != 0):
                for i in range(16):
                    FBP_image[(sum_y // 2) * 4 + 2 + (i // 4)][(sum_z // 2) * 4 + 2 + (i % 4)] += (resol[i // 4][i % 4])
            elif (sum_y % 2 != 0):
                for i in range(16):
                    FBP_image[(sum_y // 2) * 4 + 2 + (i // 4)][(sum_z // 2) * 4 + (i % 4)] += (resol[i // 4][i % 4])
            elif (sum_z % 2 != 0):
                for i in range(16):
                    FBP_image[(sum_y // 2) * 4 + (i // 4)][(sum_z // 2) * 4 + 2 + (i % 4)] += (resol[i // 4][i % 4])
            else:
                for i in range(16):
                    FBP_image[(sum_y // 2) * 4 + (i // 4)][(sum_z // 2) * 4 + (i % 4)] += (resol[i // 4][i % 4])
        
        """# - Dump LOR Z-Y pairs into text file -
        with open(f"data/recon/LOR/{self.input_name}/LOR_ZY_{self.input_name}.txt","w") as fLOR_zy:
            fLOR_zy.writelines(LOR_zy)"""
        
        end = time.time()
        print(f"[Time] FBP reconstruction: {end-start:.2f}s")
        self.log_label.setText(f'[FBP] Finish! Total time: {end-start:.0f}s')
        
        # - Display FBP recon image on Tab4 window -
        fig = plt.figure(figsize = (8,4))
        plt.imshow(np.array(FBP_image), cmap='gray')
        plt.colorbar()
        plt.savefig(f"data/src/tmp/FBP_recon.png")

        # ====== ====== [Test] Generate numpy file for DeepPET ====== ====== **
        # - Convert PIL images into NumPy arrays -
        img = Image.open(f'data/src/tmp/FBP_recon.png')
        numpydata = np.asarray(img)
        # - Save as .npy file -
        np.save(f'data/recon/FBP/FBP_{self.input_name}.npy', numpydata)
        
        print(f"\n[Save] NumPy file: data/src/tmp/FBP_{self.input_name}.npy")
        print(type(numpydata))  # Type: <class 'numpy.ndarray'>
        print(numpydata.shape)


    ### ====== ====== ====== ====== ====== ====== ====== ====== ====== ###
    ### ====== ====== ======    SRM generation    ====== ====== ====== ###
    ### ====== ====== ====== ====== ====== ====== ====== ====== ====== ###

    def SRM_generate (self):
        """ - Generate required SRM files according to system geometry. - """
        
        print("\n[Main] Start function of SRM generation by libsrm.so... \n")
        self.log_label.setText(f'[SRM] Start SRM gen with spacing {self.spacing}mm, vx {self.vx}mm and {self.xplanes} planes...')
        start = time.time()
        
        srm_lib = CDLL("utils/lib/libsrm.so")
        srm_lib.main.restype = None
        srm_lib.main.argtypes = [c_int, POINTER(c_char_p)]
        argv = (c_char_p * 4)(b'libsrm.so', bytes(self.spacing,encoding='utf-8'), bytes(self.vx,encoding='utf-8'), bytes(self.xplanes,encoding='utf-8'))
        srm_lib.main(len(argv), argv)
        
        end = time.time()
        print(f"\n[Time] Total time of SRM generation: {end-start:.2f}s")
        self.log_label.setText(f'[SRM] Finish! Total time of SRM generation: {end-start:.0f}s')


    ### ====== ====== ====== ====== ====== ====== ====== ====== ====== ###
    ### ====== ====== ======      MLEM recon      ====== ====== ====== ###
    ### ====== ====== ====== ====== ====== ====== ====== ====== ====== ###

    def MLEM_reconstruction (self):
        
        # ====== Sorting hist before MLEM ======
        self.log_label.setText(f'Running MLEM recon with {self.iterations} iters, spacing {self.spacing}mm, vx {self.vx}mm and {self.xplanes} planes.')
        # - Initialize zero array of hist -
        Hist_o = np.zeros(512 * 512).astype('float32')
        
        # - Load sorted LOR (Z1,Y1,Z2,Y2) pairs from text file -
        LORzy_path = f"data/recon/LOR/{self.input_name}/LOR_ZY_{self.input_name}.txt"
        LORzy_i = np.loadtxt(LORzy_path, dtype=np.int64)
        
        for idx in range(len(LORzy_i)):
            Z1, Y1, Z2, Y2 = LORzy_i[idx]
            
            # - Hist global mapping in col-major (Matlab) -
            cID1 = Z1 * 16 + Y1
            cID2 = Z2 * 16 + Y2
            
            # - Histogram accumulation -
            Hist_o[cID2*32*16 + cID1] += 1
        
        # - Save hist to .dat file -
        Hist_o.tofile(f"data/recon/MLEM/INPUT/Hist_sort_{self.input_name}.dat")  # [Mod] Save to each bin folder

        print(f"\n[Save] Sorted Hist: data/recon/MLEM/INPUT/Hist_sort_{self.input_name}.dat")
        print(f"[Dialog] Total self.iterations: {self.iterations}, recon planes: {self.xplanes}")
        
        '''
        # [Debug] -- Check array size of hist .dat --
        fHist_i = "data/recon/LOR/{self.input_name}/Hist_sort_{self.input_name}.dat"
        Hist_i = np.fromfile(fHist_i, dtype=np.float32)
        print(f"\n[Check] fHist_i = {fHist_i}")
        print(f"np.shape(Hist_o) = {np.shape(Hist_o)}")
        print(f"np.shape(Hist_i) = {np.shape(Hist_i)}\n")
        '''
        
        # ====== ====== Run progress of MLEM ====== ====== #
        print("\n[Main] Start function of libmlem.so...\n")
        start = time.time()
        
        mlem_lib = CDLL("utils/lib/libmlem.so")
        mlem_lib.main.restype = None
        mlem_lib.main.argtypes = [c_int, POINTER(c_char_p)]
        argv = (c_char_p * 6)(b'libmlem.so', bytes(self.input_name, encoding='utf-8'), bytes(self.iterations, encoding='utf-8'), \
                bytes(self.spacing, encoding='utf-8'), bytes(self.vx, encoding='utf-8'), bytes(self.xplanes, encoding='utf-8'))
        mlem_lib.main(len(argv), argv)
        
        end = time.time()
        hours = (end - start) / (60*60)
        print(f"\n[Time] MLEM recon with {self.iterations} iters: {hours:.2f} hrs")
        self.log_label.setText(f'[MLEM] Total iters: {self.iterations}, spacing: {self.spacing}, vx: {self.vx}, xplane: {self.xplanes}, time: {hours:.2f} hrs')


    ### ====== ====== ====== ====== ====== ====== ====== ====== ====== ###
    ### ====== ====== ======  MLEM visualization  ====== ====== ====== ###
    ### ====== ====== ====== ====== ====== ====== ====== ====== ====== ###
        
    def MLEM_visualization (self):
        
        mlemdir = "data/recon/MLEM/OUTPUT"
        self.list_mlem = os.listdir(f'{mlemdir}/{self.datn_mlem}')
        self.list_mlem.sort(key = lambda f: int(''.join(filter(str.isdigit, f))))
        datp_mlem = f"{mlemdir}/{self.datn_mlem}/{self.list_mlem[int(self.iters_mlem)-1]}"
        
        print(f"\n[Stat] Visualization of MLEM recon: \n{datp_mlem}\n")
        self.log_label.setText(f'Vis of MLEM recon: {self.list_mlem[int(self.iters_mlem)-1][:20]}...DAT')
        
        ### ****** [Mod] implement with 3D visualization ******
        # - Implement via Matplotlib 3D widget
        # - Need to render data to 3D volumn
        
        # - [Arg] axis, slice -
        axis = self.axis_mlem
        s    = self.slice_mlem
        print(f"Axis: {axis}, Slice: {s}")
        
        # - Set full image geometry -
        nimgx = self.nimgx_mlem  # MLEM: 75 = 60 / 0.8
        nimgy = self.nimgy_mlem  # Y:    64  = 16 * 4
        nimgz = self.nimgz_mlem  # Z:    128 = 32 * 4
        print(f"- nimgx:{nimgx}, nimgy:{nimgy}, nimgz:{nimgz}")
        
        # - Load MLEM recon .DAT file -
        dat_3D = np.fromfile(datp_mlem, dtype=np.float32)
        # - Reshape input data to voxel geometry -
        dat_3D = np.reshape(dat_3D, (nimgx, nimgz, nimgy))
        
        # -- Return recon_mlem_img with a slice --
        # f_2DD = f_2D1[60, :, :]
        
        ### ****** [Mod] Normalization by custom threshold ******
        # - Set default by finding global peak intensity value -
        # - Develop a sliding bar to choose from a threshold -
        
        # - Normalization of global peak value by default -
        max3D = np.max(dat_3D)  # func: norm_value -> self.thresh
        min3D = np.min(dat_3D)
        # dat_3D = np.divide(dat_3D, max3D)

        print(f"- max3D:{max3D:.2f}, min3D:{min3D:.2f}")
        
        if axis == 'X':
            #print("in case X")
            dat_2D = dat_3D[s-1, :, :]
            recon_mlem_img = np.transpose(np.reshape(dat_2D, (nimgz, nimgy)))
        elif axis == 'Y':
            dat_2D = dat_3D[:, :, s-1]  #.swapaxes(0, 2)
            recon_mlem_img = np.transpose(np.reshape(dat_2D, (nimgx, nimgz)))
        elif axis == 'Z':
            dat_2D = dat_3D[:, s-1, :]
            recon_mlem_img = np.transpose(np.reshape(dat_2D, (nimgx, nimgy)))
        
        # f1 = np.transpose(np.reshape(f_2DD, (nimgz, nimgy)))
        # recon_mlem_img = f1[0:63, 0:127]
        
        fig = plt.figure(figsize = (8,4))
        plt.imshow(np.array(recon_mlem_img), cmap='gray')
        plt.clim(min3D, max3D) # limits on colorbar
        plt.colorbar()
        plt.savefig('data/src/tmp/MLEM_recon.png')


    ### ====== ====== ====== ====== ====== ====== ====== ====== ====== ###
    ### ====== ====== ======     3-angle recon    ====== ====== ====== ###
    ### ====== ====== ====== ====== ====== ====== ====== ====== ====== ###

    def ThreeAngle_reconstruction (self):
        
        # ====== ====== Sorting hist before 3-angle recon ====== ====== #

        self.log_label.setText(f'Running 3-angle recon with {self.iterations} iters, spacing {self.spacing}mm, vx {self.vx}mm and {self.xplanes} planes.')
        
        # - Config of file paths -
        input_3ang, file_extension = os.path.splitext(self.input_name)
        idir_3ang = f"data/recon/3angle/INPUT/{input_3ang}"
        odir_3ang = f"data/recon/3angle/OUTPUT/{input_3ang}"
        
        os.system(f"mkdir -p {idir_3ang}")
        # if not os.path.isdir(input_3ang):
        # -- Run preprocessing steps to gen sorted Hist files --

        # ====== ====== ====== Preprocessing ====== ====== ====== #

        for Acq in range(3):
            Acq += 1
            LORB0B1_p =  f"{idir_3ang}/LOR_B0B1_{input_3ang}_aq{Acq}.txt"
            LORZY_p   =  f"{idir_3ang}/LOR_ZY_{input_3ang}_aq{Acq}.txt"
            HIST_p    =  f"{idir_3ang}/Hist_sort_alongY_aq{Acq}.dat"
            
            # - Initialize zero array of hist -
            Hist_o = np.zeros(512 * 512).astype('float32')
            
            # (1) Extract bin file and output frame data
            lib_extract = CDLL("utils/lib/extract_data.so")
            lib_extract.extract_data(bytes(self.input_name, encoding='utf-8'))
            # - Saved at "data/src/tmp/frame/%d.txt" & "data/src/tmp/frame_info.txt" -

            # (2) Load frame data into array --
            with open('data/src/tmp/frame_info.txt', 'r') as fFrame_info:
                frame_info = (fFrame_info.readlines()[0]).split('\t')
            fFrame_info.close()
            c_array = []
            for i in range(len(frame_info) - 1):
                if int(frame_info[i]) > 0:
                    c_array.append(int(frame_info[i]))
            print(f"\n[Save] LOR B0B1 pairs: {LORB0B1_p}")
            
            # (3) Sort coincidence events to global ID B0, B1 
            frame_path = 'data/src/tmp/frame/'
            frame_info_c = (c_int * 64)(*c_array)
            lib_sorting = CDLL("utils/lib/sorting.so")
            lib_sorting.argtypes = [c_char_p, c_char_p, POINTER(c_int)]
            lib_sorting.sorting(bytes(frame_path, encoding='utf-8'), bytes(input_3ang, encoding='utf-8'), frame_info_c)
            os.system(f"cp data/recon/LOR/{input_3ang}/LOR_B0B1_{input_3ang}.txt {LORB0B1_p}")
            
            # (4) Sort to LOR Z-Y pairs from global-ID pairs
            # -- Load global ID pairs of Board 0, 1 --
            LOR_zy = []
            with open(LORB0B1_p,"r") as fLOR_B0B1:
                lines_B0B1 = fLOR_B0B1.readlines()
                fLOR_B0B1.close()
            for lnB0B1 in lines_B0B1:
                d_0 = int(lnB0B1.split('\t')[0])       # - Read global ID (0-511)    of 1st column -
                d_1 = int(lnB0B1.split('\t')[1][:-1])  # - Read global ID (512-1023) of 2nd column -
                # - Parse gmsl/stic/channel ID in board 0 -
                gmsl_0 = (d_0 // 64) // 4
                stic_0 = (d_0 // 64) % 4
                channel_0 = d_0 % 64
                # - Convert into Z-Y axis of board 0 -
                Z_1 = board_map_z(channel_0, stic_0, gmsl_0)
                Y_1 = board_map_y(channel_0, stic_0, gmsl_0)
                # - Parse gmsl/stic/channel ID in board 1 -
                gmsl_1 = (d_1 // 64) // 4
                stic_1 = (d_1 // 64) % 4
                channel_1 = d_1 % 64
                # - Convert into Z-Y axis of board 1 -
                Z_2 = board_map_z(channel_1, stic_1, gmsl_1)
                Y_2 = board_map_y(channel_1, stic_1, gmsl_1)
                # - Sum Z, Y of image -
                sum_z = Z_1 + Z_2
                sum_y = Y_1 + Y_2
                # - Collect LOR pairs (Z1 Y1 Z2 Y2) for MLEM -
                LOR_zy.append("%2d\t%2d\t%2d\t%2d\n" % (Z_1, Y_1, Z_2, Y_2))
            # - Dump LOR Z-Y pairs into text file -
            with open(LORZY_p, "w") as fLOR_zy:
                fLOR_zy.writelines(LOR_zy)
                print(f"[Save] LOR Z-Y pairs: {LORZY_p}")
            
            # (5) Load sorted LOR (Z1,Y1,Z2,Y2) pairs from text file
            LORzy_i = np.loadtxt(LORZY_p, dtype=np.int64)
            for idx in range(len(LORzy_i)):
                Z1, Y1, Z2, Y2 = LORzy_i[idx]
                # - Hist global mapping in col-major (Matlab) -
                cID1 = Z1 * 16 + Y1
                cID2 = Z2 * 16 + Y2
                # - Histogram counting -
                Hist_o[cID2*32*16 + cID1] += 1
            
            # - Save hist to .dat file -
            Hist_o.tofile(HIST_p)
            print(f"[Save] Sorted Hist: {HIST_p}")
        print(f'\n[3-angle] Finish preprocessing steps. \n- Source: {self.input_name}, iters: {self.iterations}, spacing: {self.spacing}, vx: {self.vx}, xplane: {self.xplanes}')
        
        # === [Function] new process of 3-angle preprocess === #
        """
        for Acq in range(3):
            Acq += 1
            self.data_preprocess()
        """
        # ====== ====== Run progress of 3-angle ====== ====== #
        
        print("\n[Main] Start function of lib3angle.so...\n")
        start = time.time()
        
        lib_3angle = CDLL("utils/lib/lib3angle.so")
        lib_3angle.main.restype = None
        lib_3angle.main.argtypes = [c_int, POINTER(c_char_p)]
        argv = (c_char_p * 6)(b'lib3angle.so', bytes(input_3ang, encoding='utf-8'), bytes(self.iterations, encoding='utf-8'), \
                bytes(self.spacing, encoding='utf-8'), bytes(self.vx, encoding='utf-8'), bytes(self.spacing, encoding='utf-8'))
        lib_3angle.main(len(argv), argv)
        
        end = time.time()
        hours = (end - start) / (60*60)
        print(f"\n[Time] 3-angle recon ({self.iterations} iters): {hours:.2f} hrs")
        self.log_label.setText(f'[3-angle] Total iters: {self.iterations}, spacing: {self.spacing}, vx: {self.vx}, xplane: {xplanes}, time: {hours:.2f} hrs')
        

    ### ====== ====== ====== ====== ====== ====== ====== ====== ====== ====== ###
    ### ====== ====== ======    3-angle visualization    ====== ====== ====== ###
    ### ====== ====== ====== ====== ====== ====== ====== ====== ====== ====== ###

    def ThreeAngle_visualization (self):
        odir_3ang = "data/recon/3angle/OUTPUT"
        self.list_3ang = os.listdir(f'{odir_3ang}/{self.datn_3ang}/Aq{self.acq_3ang}')
        self.list_3ang.sort(key = lambda f: int(''.join(filter(str.isdigit, f))))
        datp_3ang = f"{odir_3ang}/{self.datn_3ang}/Aq{self.acq_3ang}/{self.list_3ang[int(self.iters_3ang)-1]}"
        
        print(f"\n[Stat] Visualization of 3-angle recon: \n{datp_3ang}\n")
        self.log_label.setText(f'Vis of 3-angle recon: {self.list_3ang[int(self.iters_3ang)-1]}...DAT')
        
        # - Set full image geometry -
        nimgx = self.nimgx_3ang  # 3-angle: 320 = 256 / 0.8
        nimgy = self.nimgy_3ang  #          64  = 16 * 4
        nimgz = self.nimgz_3ang  #          128 = 32 * 4
        print(f"- nimgx:{nimgx}, nimgy:{nimgy}, nimgz:{nimgz}")
        
        # - Load .DAT file -
        dat_3D = np.fromfile(datp_3ang, dtype=np.float32)
        # - Reshape input data to voxel geometry -
        dat_3D = np.reshape(dat_3D, (nimgx, nimgz, nimgy))
        
        ### *** [Mod] Normalization by custom threshold ***
        # - Set default by finding global peak intensity value
        # - Develop a sliding bar to choose from a threshold
        # - Normalization of global peak value by default
        
        # - Normalization of global peak value by default -
        max3D = np.max(dat_3D)  # func: norm_value -> self.thresh
        min3D = np.min(dat_3D)

        dat_3D = np.divide(dat_3D, max3D)

        max3D if max3D < 1000 else 1000
        print(f"- max3D:{max3D:.2f}, min3D:{min3D:.2f}")
        
        # - [Arg] axis, slice -
        axis = self.axis_3ang
        s    = self.slice_3ang
        print(f"Acq: {self.acq_3ang}, Axis: {axis}, Slice: {s}")
        
        if axis == 'X':
            dat_2D = dat_3D[s-1, :, :]
            recon_3ang_img = np.transpose(np.reshape(dat_2D, (nimgz, nimgy)))
        elif axis == 'Y':
            dat_2D = dat_3D[:, :, s-1]
            recon_3ang_img = np.transpose(np.reshape(dat_2D, (nimgx, nimgz)))
        elif axis == 'Z':
            dat_2D = dat_3D[:, s-1, :]
            recon_3ang_img = np.transpose(np.reshape(dat_2D, (nimgx, nimgy)))
        
        fig = plt.figure(figsize = (8,4))
        plt.imshow(np.array(recon_3ang_img), cmap='gray')
        plt.clim(min3D, max3D)
        plt.colorbar()
        plt.savefig(f'data/src/tmp/ThreeAngle_recon.png')
