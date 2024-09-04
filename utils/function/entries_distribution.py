from ctypes import *

def entries_distribution ():
    '''
    class stic(Structure):
        _fields_ = [("total", c_uint32),
                    ("channels", c_uint32 * 64)]

    class gmsl(Structure):
        _fields_ = [("total", c_uint32),
                    ("stics", stic * 4)]

    class board(Structure):
        _fields_ = [("total", c_uint32),
                    ("gmsls", gmsl * 4)]
    
    lib = CDLL("./function/extract_entries_distribution.so")
    lib.extract_entries_distribution(bytes("./data/data.bin", encoding='utf-8'))
    
    with open('./data/entries_distribution.bin', 'rb') as file:
        data = board()
        entries = {}
        count = 0
        
        for i in range(32):
            #file.readinto(data) == sizeof(data):
            file.readinto(data)
            entries[count] = {}
            entries[count]['total'] = data.total
            entries[count]['gmsls'] = {}
            
            for gmsl in range(4):
                entries[count]['gmsls'][gmsl] = {}
                entries[count]['gmsls'][gmsl]['total'] = data.gmsls[gmsl].total
                entries[count]['gmsls'][gmsl]['stics'] = {}
                
                for stic in range(4):
                    entries[count]['gmsls'][gmsl]['stics'][stic] = {}
                    entries[count]['gmsls'][gmsl]['stics'][stic]['total'] = data.gmsls[gmsl].stics[stic].total
                    entries[count]['gmsls'][gmsl]['stics'][stic]['channels'] = []
                    
                    for channel in range(64):
                        entries[count]['gmsls'][gmsl]['stics'][stic]['channels'].append(data.gmsls[gmsl].stics[stic].channels[channel])
            
            count += 1
            # print(data.total)
            # print(data.gmsls[0].total)
            # print(data.gmsls[0].stics[0].total)
            # print(data.gmsls[0].stics[0].channels[63])
            # break
    '''
    with open('data/src/tmp/active.txt') as file:
        lines = file.readlines()
    file.close()
    return lines
