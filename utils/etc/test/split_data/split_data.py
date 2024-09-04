import numpy as np

datalist = []
with open('./155623.139_B0all4-B1all8_ESPEC_NA22_T10E110_HVmin1200_D5.5CM_10S_POSL-52.0MM.bin', 'rb') as file: # main file 的相對位置
    bytes = file.read(8)
    while bytes:
        datalist.append(bytes)
        bytes = file.read(8)
    print(len(datalist))
file.close()

slice = int(len(datalist) / 10)
for i in range(10):
    try:
        datas = datalist[i * slice:(i + 1) * slice]
    except:
        datas = datalist[i * slice:]
    with open('./split_data/' + str(i) + '.bin', 'wb') as file:
        for data in datas:
            file.write(data)
