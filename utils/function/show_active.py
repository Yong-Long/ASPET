from ctypes import Structure, c_uint16, c_uint8, sizeof, CDLL
import numpy as np
import time

def show_active_boards (filepath):
    start = time.time()
    lib = CDLL("utils/lib/extract_data.so")
    lib.extract_data(bytes(filepath, encoding='utf-8'))
    end = time.time()
    print("[Time] Extracting bin file: %.2f" % (end - start))
    
    # - Recording active boards in array -
    active = np.array(np.zeros(32), dtype=bool)

    with open('data/src/tmp/active.txt','r') as file:
        lines = file.readlines()
        for i, line in enumerate(lines):
            if i % 4 == 0:
                a = int(line.split('\t')[0])
                if a > 0:
                    active[i // 4] = True
    file.close()

    return active
