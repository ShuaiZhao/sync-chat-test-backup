#!/usr/bin/python

"""
This script is to parse the log generated for application sync-chat-app.cc

It get the timestamp of generating content, pushing notification, 
receiving notification, sending interest, receiving content, respectively. 

Author: Xuan Liu <xuan.liu@mail.umkc.edu>

"""
import numpy as np
import scipy.io
import re

LOGFILE = "topo1-app.tr"

def get_content_gen(log_file):
    ''' Get the lines in the log regarding content generating '''
    fopen = open(log_file, 'r')
    content_gen_list = []
    for line in fopen.readlines():
        if "generateContent" and "Content prefix" in line:
            temp = line.strip().split()
            temp_item = [temp[0], temp[2], temp[6]]
            content_gen_list.append(temp_item)
    fopen.close()
    return content_gen_list


def get_info_dict(key_info):
    ''' create a dictionary to stores the content generation info '''
    content_dict = {}
    for item in key_info:
        key = item[1][0:-1]
        value = (float(item[0][0:-1]), item[2])
        if content_dict.has_key(key):
            content_dict[key].append(value)
        else:
            content_dict[key] = [value]
    return content_dict

def get_info_dict_by_msg(key_info):
    ''' create a dictionary to stores the content generation info '''
    info_dict = {}
    for item in key_info:
        msg_seq = re.split('/', item[-1])[-1]
        sender = re.split('/', item[-1])[-2][1:]
        key = ''.join([sender, '_' ,msg_seq])
        value = (item[1], float(item[0][0:-1]))
        if info_dict.has_key(key):
            info_dict[key].append(value)
        else:
            info_dict[key] = [value]
    return info_dict



def get_rcv_note(log_file):
    ''' Get the key infomation about receiving notification '''
    fopen = open(log_file, 'r')
    rcv_note_list = []
    for line in fopen.readlines():
        if "syncChatApp:OnNotification" in line:
            temp = line.strip().split()
            temp_item = [temp[0], temp[2], temp[-1]]
            rcv_note_list.append(temp_item)
    fopen.close()
    return rcv_note_list

def get_interest_send(log_file):
    ''' Get the key information about interest sending '''
    fopen = open(log_file, 'r')
    interest_send_list = []
    for line in fopen.readlines():
        if "Sending Interest" in line:
            temp = line.strip().split()
            temp_item = [temp[0], temp[2], temp[-1]]
            interest_send_list.append(temp_item)
    fopen.close()
    return interest_send_list

def get_send_content(log_file):
    ''' Get the key information about contentobject sending '''
    fopen = open(log_file, 'r')
    content_send_list = []
    for line in fopen.readlines():
        if "OnInterest" and "Sending ContentObject" in line:
            temp = line.strip().split()
            temp_item = [temp[0], temp[2], temp[-1]]
            content_send_list.append(temp_item)
    fopen.close()
    return content_send_list


def get_rcv_content(log_file):
    ''' Get the key infomration about receiving content object '''
    fopen = open(log_file, 'r')
    rcv_content_list = []
    for line in fopen.readlines():
        if "OnContentObject" in line:
            temp = line.strip().split()
            temp_item = [temp[0], temp[2], temp[-1]]
            rcv_content_list.append(temp_item)
    fopen.close()
    return rcv_content_list


def search_entry(info_dict, target_tuple):
    ''' find the corresponding entry in the dictionary '''
    new_dict = {}
    split_target = re.split('/', target_tuple[1])
    target_senderid = split_target[-2]
    target_msgid = split_target[-1]
    for item in info_dict:
        for tuple_entry in info_dict[item]:
            split_tuple = re.split('/', tuple_entry[1])
            tuple_sendid = split_tuple[-2]
            tuple_msgid = split_tuple[-1]
            if target_senderid == tuple_sendid and target_msgid == tuple_msgid:
                new_dict[item] = tuple_entry[0]
                break
    return (target_tuple[1], new_dict)

def search_timestamp(content_name, info_dict, ue_id):
    ''' search timestamp for corresponding event in the dictionary '''
    timestamp = 0
    # print info_dict.keys()
    #if ue_id in info_dict.keys():
    for item in info_dict[ue_id]:
        if content_name in item[1]:
            timestamp = item[0]
            break
    if timestamp == 0:
        sender_id = re.split('/',content_name)[1]
        for item in info_dict[sender_id]:
            if content_name in item[1]:
                timestamp = item[0]
                break
    return timestamp
    

    

def gen_res_dict(content_gen_dict, 
                rcv_note_dict, 
                send_content_dict,
                rcv_content_dict):
    ''' Generate the dictionary for events regarding each content generated '''
    res_dict = {}
    for ue_id in content_gen_dict:
        for content_gen_tuple in content_gen_dict[ue_id]:
            if (float(content_gen_tuple[0])>5.0):
                break
            else:
                rcv_note_tuple = search_entry(rcv_note_dict, content_gen_tuple)
                rcv_content_tuple = search_entry(rcv_content_dict, 
                                                content_gen_tuple)
                send_content_tuple = search_entry(send_content_dict, 
                                            content_gen_tuple)
                res_dict[content_gen_tuple[1]] = {}
                res_dict[content_gen_tuple[1]]['content_gen_time'] \
                            = [content_gen_tuple[0]]
                res_dict[content_gen_tuple[1]]['rcv_note_time'] \
                            = rcv_note_tuple[1]
                res_dict[content_gen_tuple[1]]['send_content_time'] \
                            = send_content_tuple[1]
                res_dict[content_gen_tuple[1]]['rcv_content_time'] \
                            = rcv_content_tuple[1]
    return res_dict

def get_note_converge_dict(content_gen_dict, rcv_note_dict):
    """
    generate dictionary for the notification convergence time
    The key of the dictionary is content name
    The entry to a key is a n by n matrix, where n is the number of participants,
    the diagnal values are the content generating time, or the
    time when the notification is generated      
    """
    #print rcv_note_dict.keys()
    note_converge_dict = {}
    matrix_dim = len(content_gen_dict.keys())
    init_matrix = [[0 for i in range(matrix_dim)] for j in range(matrix_dim)]
    max_msg_ue = get_converge_dict_key(content_gen_dict)
    print "minmum", max_msg_ue
    for msg_seq in range(max_msg_ue):
        key = ''.join(['msg_', str(msg_seq)])
        init_matrix = [[0 for i in range(matrix_dim)] for j in range(matrix_dim)]
        for ue_id in content_gen_dict:
            sender = int(ue_id[1:])
            if msg_seq < len(content_gen_dict[ue_id]):
                init_matrix[sender-1][sender-1] = float(content_gen_dict[ue_id]\
                        [msg_seq][0])
                query_key = ''.join([ue_id[1:], '_', str(msg_seq)])
                #print query_key
                if query_key in rcv_note_dict.keys():
                    for tuple_info in rcv_note_dict[query_key]:
                        col_index = int(tuple_info[0][1:-1])
                        #print col_index
                        init_matrix[sender-1][col_index-1] = tuple_info[1]
                else:
                    pass
        note_converge_dict[key] = init_matrix

    return note_converge_dict

                 
def get_converge_dict_key(content_gen_dict):
    ''' get the convergence time dictionary key, which is the msg seq '''
    msg_num_per_ue = []
    for ue_id in content_gen_dict:
        msg_num = len(content_gen_dict[ue_id])
        msg_num_per_ue.append(msg_num)
    return max(msg_num_per_ue)


def output_data(res_dict):
    ''' output data into .m file '''
    content_name = res_dict.keys()
    content_name.sort()

def get_data_dict(content_gen_dict,
                    rcv_note_dict,
                    send_content_dict,
                    rcv_content_dict):
    """
    generate a dictionary of two-dimentional matrix 
    The format for each row in each dictionary value is:
    [producer_id, content_id, content_gen_time, rcv_note_time, 
                 send_content_time, rcv_content_time]
    The key of the returned dictionary is receiver's id
    All variables in the returned dictoionary is numerical
    """
    ue_list = rcv_content_dict.keys()
    ue_list.sort()
    data_dict = {}
    for ue_id in ue_list:
        data_matrix = []
        for rcv_time, content_id in rcv_content_dict[ue_id]:
            timestamp_list = []
            #timestamp_list.append(content_id)
            split_contentid = re.split('/', content_id)
            timestamp_list.append(int(split_contentid[1][1:]))
            timestamp_list.append(int(split_contentid[-1]))            
            content_gen_time = search_timestamp(content_id, content_gen_dict, ue_id)
            rcv_note_time = search_timestamp(content_id, rcv_note_dict, ue_id)
            send_content_time = search_timestamp(content_id, 
                                    send_content_dict,ue_id)
            timestamp_list.append(content_gen_time)
            timestamp_list.append(rcv_note_time)
            timestamp_list.append(send_content_time)
            timestamp_list.append(rcv_time)
            data_matrix.append(timestamp_list)
        data_dict[ue_id] = data_matrix
    return data_dict


def process_data(data_dict):
    ''' Process data'''
    rcver_matrix_dict = {}
    for key in data_dict:
        temp_matrix = np.matrix(data_dict[key])
        rcver_matrix_dict[key] = temp_matrix

#def plot_data(rcver_matrix_dict):
#    ''' plot data '''

    #  b = numpy.matrix(numpy.arange(48))

    #plot(b.getT(), u1_msg_array[:,1:], 'r+')

def get_cont_sender_name(content_gen_dict):
    ''' Stores the content sender and content seq '''
    content_list = []
    for sender_id in content_gen_dict:
        for send_time, content_id in content_gen_dict[sender_id]:
            split_contentid = re.split('/', content_id)
            new_list = [int(split_contentid[1][1:]), 
                            int(split_contentid[-1]), 
                            send_time]
            content_list.append(new_list)
    return content_list

def get_convergence_time(sr_dict):
    ''' Get convergence time '''
    cont_cov_matrix = []
    note_cov_matrix = []
    for content in sr_dict:
        msg_sr_time = []
        note_sr_time = []
        split_contentid = re.split('/', content)
        msg_sr_time.append(int(split_contentid[1][1:]))
        msg_sr_time.append(int(split_contentid[-1]))
        msg_sr_time.append(sr_dict[content]
                ['content_gen_time'][0])
        note_sr_time.append(int(split_contentid[1][1:]))
        note_sr_time.append(int(split_contentid[-1]))
        note_sr_time.append(sr_dict[content]
                ['content_gen_time'][0])
        for rcver_id in sr_dict[content]['rcv_content_time']:
            msg_sr_time.append(sr_dict[content]['rcv_content_time'][rcver_id])
            note_sr_time.append(sr_dict[content]['rcv_note_time'][rcver_id])
        cont_cov_matrix.append(msg_sr_time)
        note_cov_matrix.append(note_sr_time)
    return cont_cov_matrix, note_cov_matrix

def run1(log_file):
    ''' program wrapper '''
    content_gen_list = get_content_gen(log_file)
    rcv_note_list = get_rcv_note(log_file)
    send_content_list = get_send_content(log_file)
    rcv_content_list = get_rcv_content(log_file)
    content_gen_dict = get_info_dict(content_gen_list)
    rcv_note_dict = get_info_dict(rcv_note_list)
    send_content_dict = get_info_dict(send_content_list)
    rcv_content_dict = get_info_dict(rcv_content_list)
    sr_dict = gen_res_dict(content_gen_dict, 
                            rcv_note_dict, 
                            send_content_dict, 
                            rcv_content_dict)
    #data_dict = get_data_dict(content_gen_dict, 
    #                        rcv_note_dict, 
    #                        send_content_dict, 
    #                        rcv_content_dict)
    #return new_dict, data_dict
    #scipy.io.savemat('rcv_msg.mat', mdict={'rcv_msg':data_dict})
    #content_list = get_cont_sender_name(content_gen_dict)
    #return sr_dict, data_dict, content_list
    #cont_cov_matrix, note_cov_matrix = get_convergence_time(sr_dict)
    return sr_dict


def run(log_file):
    ''' wrapper 2'''
    # get a dictonary regarding content generating time
    content_gen_list = get_content_gen(log_file)
    content_gen_dict = get_info_dict(content_gen_list)
    # get a dictionary regarding notification receiving time
    rcv_note_list = get_rcv_note(log_file)
    rcv_note_dict = get_info_dict_by_msg(rcv_note_list)
    # get a dictionary regarding content sending time
    send_content_list = get_send_content(log_file)
    send_content_dict = get_info_dict(send_content_list)
    # get a dictionary regarding content receiving time
    rcv_content_list = get_rcv_content(log_file)
    rcv_content_dict = get_info_dict_by_msg(rcv_content_list)
    note_converge_dict = get_note_converge_dict(content_gen_dict, rcv_note_dict)
    content_converge_dict = get_note_converge_dict(content_gen_dict, rcv_content_dict)
    return note_converge_dict, content_converge_dict




