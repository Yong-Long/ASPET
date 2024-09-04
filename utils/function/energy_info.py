import os
import time
import fnmatch
from ctypes import *
from pathlib import Path


def read_energy_info (filepath, mode):
    if os.path.isdir("data/src/tmp/" + mode):
        pass
    else:
        start = time.time()
        lib = CDLL("utils/lib/extract_energy_info.so")
        lib.extract_energy_info(bytes(filepath, encoding='utf-8'), bytes(mode, encoding='utf-8'))
        end = time.time()
        print("[Time] extract energy info: %.2f" % (end - start))


def read_record ():
    with open('data/src/tmp/active.txt','r') as file:
        lines = file.readlines()
    file.close()
    return lines


def show_energy_info (record, mode, board_num = 'None', gmsl_num = 'None', stic_num = 'None', channel_set = 'None'):
    if board_num != 'None' and gmsl_num == 'None' and stic_num == 'None' and channel_set == 'None':
        dirpath  = 'data/src/tmp/' + mode + '/board' + board_num
        txtfiles = [str(folder) for folder in Path(dirpath).glob('**/**/*.[Tt][Xx][Tt]')]
        num      = int(record[int(board_num) * 4].split('\n')[0])
        data     = [None] * num
        count    = 0
        # print(num)
        
        for txtfile in txtfiles:
            with open(txtfile,'r') as file:
                lines = file.readlines()
            file.close()
            
            for i, line in enumerate(lines):
                ds    =  line.split('\n')[0]
                count += 1
                data[count - 1] = int(ds)
        # return data
    
    elif gmsl_num != 'None' and stic_num == 'None':
        dirpath  = 'data/src/tmp/' + mode + '/board' + board_num + '/gmsl' + gmsl_num
        txtfiles = [str(folder) for folder in Path(dirpath).glob('**/**/*.[Tt][Xx][Tt]')]
        num      = int(record[int(board_num) * 4 + 1].split('\t')[int(gmsl_num)])
        data     = [None] * num
        count    = 0
        
        for txtfile in txtfiles:
            with open(txtfile, 'r') as file:
                lines = file.readlines()
            file.close()
            
            for line in lines:
                ds = line.split('\n')[0]
                count += 1
                data[count - 1] = int(ds)
        # return data
    
    elif stic_num != 'None' and channel_set == 'None':
        dirpath = 'data/src/tmp/' + mode + '/board' + board_num + '/gmsl' + gmsl_num + '/stic' + stic_num
        txtfiles = [str(folder) for folder in Path(dirpath).glob('**/**/*.[Tt][Xx][Tt]')]
        num = int(record[int(board_num) * 4 + 2].split('\t')[int(gmsl_num) * 4 + int(stic_num)])
        data = [None] * num
        count = 0
        
        for txtfile in txtfiles:
            with open(txtfile,'r') as file:
                lines = file.readlines()
            file.close()
            
            for line in lines:
                ds = line.split('\n')[0]
                count += 1
                data[count - 1] = int(ds)
        # return data
    
    elif channel_set != 'None':
        dirpath    = 'data/src/tmp/' + mode + '/board' + board_num + '/gmsl' + gmsl_num + '/stic' + stic_num
        target_set = [str(i) for i in range(int(channel_set) * 16, int(channel_set) * 16 + 16)]
        txtfiles   = [str(folder) for folder in Path(dirpath).glob('**/**/*.[Tt][Xx][Tt]') if any(fnmatch.fnmatch((str(folder).split('/')[-1]).split('.')[0], p) for p in target_set)]
        data       = [None] * 16
        
        if len(txtfiles) == 0:
            for i in range(16):
                data[i] = []
        else:
            for txtfile in txtfiles:
                count = 0
                id    = int((txtfile.split('/')[-1]).split('.')[0])
                num   = int(record[int(board_num) * 4 + 3].split('\t')[int(gmsl_num) * 256 + int(stic_num) * 64 + id])
                data[id % 16] = [None] * num
                
                with open(txtfile,'r') as file:
                    lines = file.readlines()
                file.close()
                
                for line in lines:
                    ds    =  line.split('\n')[0]
                    count += 1
                    data[id % 16][count - 1] = int(ds)
    return data
