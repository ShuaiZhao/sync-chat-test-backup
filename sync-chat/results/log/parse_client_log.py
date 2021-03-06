#!/usr/bin/python

"""
This script is to parse the log generated by SyncClient 
application on the UE

The major information to be collected is about digest log 
and digest tree updates

Author: Xuan Liu <xuan.liu@mail.umkc.edu>

"""

LOGFILE = "topo1-client.tr"
SYNTAX1 = "Digest log size is"
SYNTAX2 = "fpUpdateDB size is"

def grep_log_update(log_file):
    ''' grep the lines about log update information '''
    fopen = open(log_file, 'r')
    root_update = []
    fp_update = []
    for line in fopen.readlines():
        if SYNTAX1 in line:
            root_update.append(line)
        elif SYNTAX2 in line:
            fp_update.append(line)
    fopen.close()
    return fp_update, root_update
    
def get_update_list(fp_update):
    ''' Divide the fp update based on sync-client '''
    update_list = []
    for item in fp_update:
        temp = item.strip().split()
        temp_item = [temp[0], temp[5], temp[-1]]
        update_list.append(temp_item)
    return update_list

def get_info_dict(key_info_list):
    ''' create a dictionary to stores log/fp udpate information '''
    info_dict = {}
    for item in key_info_list:
        key = item[1]
        value = (item[0], item[2])
        if info_dict.has_key(key):
            info_dict[key].append(value)
        else:
            info_dict[key] = [value]
    return info_dict



