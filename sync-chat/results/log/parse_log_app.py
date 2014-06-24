#!/usr/bin/python

"""
This script is to parse the log generated for application sync-chat-app.cc

It get the timestamp of generating content, pushing notification, 
receiving notification, sending interest, receiving content, respectively. 

Author: Xuan Liu <xuan.liu@mail.umkc.edu>
Modified: Shuai
"""
import numpy as np
import scipy.io
import re
import subprocess
import collections

#LOGFILE = "topo1-app.tr"
CONFIG_FILE = "../../topologies/config.txt"


####################
#  All
#####################
def get_config(config_file=CONFIG_FILE):
    ''' Get network configuration info from the config file 
        Return a dictionary
        {'AccountPrefix': 'A',
         'ControllerPrefix': 'C',
         'NumberOfProxies': '2',
         'NumberOfRouters': '3',
         'NumberOfUEs': '4',
         'ProxyPrefix': 'P',
         'RoutersPerCluster': '3',
         'UEPrefix': 'U',
         'UEsPerCluster': '2'}
    '''
    config_dict = {}
    f_handle = open(config_file, 'r')
    for line in f_handle:
        if line[0] == '#':
            pass
        else:
            words = line.split()
            config_dict[words[0]] = words[2]
            #print words
    return config_dict

def get_ue_list(log_file):
    """ Return UE list
    ['U1', 'U2', 'U3', 'U4'....]
    """
    config_dict = get_config()
    ue_count = config_dict["NumberOfUEs"]
    ue_list = []
    for i in range(int(ue_count)):
        ue_list.append("U" + str(i+1))
    return ue_list

    log_file_split = log_file.split('_')
    #print log_file_split

    if(len(log_file_split) == 9 or len(log_file_split) == 11):
        ue_count = int( log_file_split[3].strip('UE') )
        ue_list = []
        for i in range(int(ue_count)):
            ue_list.append("U" + str(i+1))
    
    return ue_list

def test_accounts(log_file, config_file=CONFIG_FILE):
    """
    based on the log_file name 
    return a tuple with three ues
    Audio ['topo' '6','proxies','300UE','10GCore','10GLocal','9','5','halfSource.tr']

    Video ['topo','3', 'proxies', '15UE', '1GCore', '1GLocal', '9', '5','oneSource','1010bytes', 'video.tr']

    Chat ['topo','3', 'proxies', '15UE', '1GCore', '1GLocal', '9', '5','oneSource','1010bytes', 'chat.tr']

    Both ['topo' '6','proxies','300UE','10GCore','10GLocal','9','5','halfSource.tr',136bytes', 'oneSource','1010bytes', 'video.tr']

    """

    log_file_split = log_file.split('_')
    #print log_file_split
    proxy_count = int( log_file_split[1] )
    ue_count = int( log_file_split[3].strip('UE') )
    ue_per_cluster = ue_count / proxy_count
    
    if(len(log_file_split) == 10 or len(log_file_split) == 11 or len(log_file_split) == 12):
        #only audio or video      
        audio_source_count = log_file_split[8].rstrip('.tr')
    
        #print audio audio_source_count
        if audio_source_count == 'oneSource':
            audio_ue1 = audio_ue2 = audio_ue3 = 'A1'
        elif audio_source_count == 'threeSource' or audio_source_count == 'allSource': 
            audio_ue1 = 'A' +str( ue_per_cluster )
            audio_ue2 = 'A' +str( ue_per_cluster * 2)
            audio_ue3 = 'A' +str( ue_per_cluster * 3)
        elif audio_source_count == 'halfSource':
            audio_ue1 = 'A' +str( ue_per_cluster -1 )
            audio_ue2 = 'A' +str( ue_per_cluster * 2 - 1 )
            audio_ue3 = 'A' +str( ue_per_cluster * 3 - 1 )            

        print "Chosse UE:", audio_ue1, " ", audio_ue2, " ", audio_ue3
        return audio_ue1, audio_ue2, audio_ue3    
    
    elif(len(log_file_split) == 13):
        # both audio and video
        audio_source_count = log_file_split[8].rstrip('.tr')
        video_source_count = log_file_split[10].rstrip('.tr')
        
        #print audio audio_source_count
        if audio_source_count == 'oneSource':
            audio_ue1 = audio_ue2 = audio_ue3 = 'A1'
        elif audio_source_count == 'threeSource' or audio_source_count == 'allSource': 
            audio_ue1 = 'A' +str( ue_per_cluster )
            audio_ue2 = 'A' +str( ue_per_cluster * 2)
            audio_ue3 = 'A' +str( ue_per_cluster * 3)
        elif audio_source_count == 'halfSource':
            audio_ue1 = 'A' +str( ue_per_cluster -1 )
            audio_ue2 = 'A' +str( ue_per_cluster * 2 - 1 )
            audio_ue3 = 'A' +str( ue_per_cluster * 3 - 1 )            

        if video_source_count == 'oneSource':
            video_ue1 = video_ue2 = video_ue3 = 'A1'
        elif video_source_count == 'threeSource' or video_source_count == 'allSource': 
            video_ue1 = 'A' +str( ue_per_cluster )
            video_ue2 = 'A' +str( ue_per_cluster * 2)
            video_ue3 = 'A' +str( ue_per_cluster * 3)
        elif video_source_count == 'halfSource':
            video_ue1 = 'A' +str( ue_per_cluster -1 )
            video_ue2 = 'A' +str( ue_per_cluster * 2 - 1 )
            video_ue3 = 'A' +str( ue_per_cluster * 3 - 1 )
        print "Chosse UE:", audio_ue1, " ", audio_ue2, " ", audio_ue3, " ", video_ue1, " ", video_ue2, " ", video_ue3
        return audio_ue1, audio_ue2, audio_ue3, video_ue1, video_ue2, video_ue3

def test_ues(log_file, config_file=CONFIG_FILE):
    """
    based on the log_file name 
    return a tuple with three ues
    Audio ['topo' '6','proxies','300UE','10GCore','10GLocal','9','5','halfSource.tr']

    Video ['topo','3', 'proxies', '15UE', '1GCore', '1GLocal', '9', '5','oneSource','1010bytes', 'video.tr']
    Both ['topo' '6','proxies','300UE','10GCore','10GLocal','9','5','halfSource.tr',136bytes', 'oneSource','1010bytes', 'video.tr']

    """

    log_file_split = log_file.split('_')
    #print log_file_split
    proxy_count = int( log_file_split[1] )
    ue_count = int( log_file_split[3].strip('UE') )
    ue_per_cluster = ue_count / proxy_count
    
    if(len(log_file_split) == 10 or len(log_file_split) == 11 or len(log_file_split) == 12):
        #only audio or video      
        audio_source_count = log_file_split[8].rstrip('.tr')
    
        #print audio audio_source_count
        if audio_source_count == 'oneSource':
            audio_ue1 = audio_ue2 = audio_ue3 = 'U1'
        elif audio_source_count == 'threeSource' or audio_source_count == 'allSource': 
            audio_ue1 = 'U' +str( ue_per_cluster )
            audio_ue2 = 'U' +str( ue_per_cluster * 2)
            audio_ue3 = 'U' +str( ue_per_cluster * 3)
        elif audio_source_count == 'halfSource':
            audio_ue1 = 'U' +str( ue_per_cluster -1 )
            audio_ue2 = 'U' +str( ue_per_cluster * 2 - 1 )
            audio_ue3 = 'U' +str( ue_per_cluster * 3 - 1 )            

        print "Chosse UE:", audio_ue1, " ", audio_ue2, " ", audio_ue3
        return audio_ue1, audio_ue2, audio_ue3    
    
    elif(len(log_file_split) == 13):
        # both audio and video
        audio_source_count = log_file_split[8].rstrip('.tr')
        video_source_count = log_file_split[10].rstrip('.tr')
        
        #print audio audio_source_count
        if audio_source_count == 'oneSource':
            audio_ue1 = audio_ue2 = audio_ue3 = 'U1'
        elif audio_source_count == 'threeSource' or audio_source_count == 'allSource': 
            audio_ue1 = 'U' +str( ue_per_cluster )
            audio_ue2 = 'U' +str( ue_per_cluster * 2)
            audio_ue3 = 'U' +str( ue_per_cluster * 3)
        elif audio_source_count == 'halfSource':
            audio_ue1 = 'U' +str( ue_per_cluster -1 )
            audio_ue2 = 'U' +str( ue_per_cluster * 2 - 1 )
            audio_ue3 = 'U' +str( ue_per_cluster * 3 - 1 )            

        if video_source_count == 'oneSource':
            video_ue1 = video_ue2 = video_ue3 = 'U1'
        elif video_source_count == 'threeSource' or video_source_count == 'allSource': 
            video_ue1 = 'U' +str( ue_per_cluster )
            video_ue2 = 'U' +str( ue_per_cluster * 2)
            video_ue3 = 'U' +str( ue_per_cluster * 3)
        elif video_source_count == 'halfSource':
            video_ue1 = 'U' +str( ue_per_cluster -1 )
            video_ue2 = 'U' +str( ue_per_cluster * 2 - 1 )
            video_ue3 = 'U' +str( ue_per_cluster * 3 - 1 )
        print "Chosse UE:", audio_ue1, " ", audio_ue2, " ", audio_ue3, ' ', video_ue1, " ", video_ue2, " ", video_ue3
        return audio_ue1, audio_ue2, audio_ue3, video_ue1, video_ue2, video_ue3

def test_file_name_length(log_file):
    """
    test file name compoents length
    """
    log_file_split = log_file.split('_')
    
    if(len(log_file_split) == 10 or len(log_file_split) == 11 or len(log_file_split) == 12):  # 12 for ondemand video
        ue_number = 3
        return ue_number
    elif(len(log_file_split) == 13):
        ue_number = 6
        return ue_number
   
####################
#  Audio + Video
#####################
def get_content_gen(log_file):
    ''' Get the lines in the log regarding content generating 
        return two dicts: auido, video, and chat
        ue_audio_generate_dict = {'U1':[[],[],[]], 'U2':[[],[],[]]}
    '''
    ue_audio_generate_dict = {}
    ue_video_generate_dict = {}
    

    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "generateContent(): " in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(":")
            content_type = temp[3]
            prefix = temp[10]
            #print prefix
            size = temp[12]
            if ue_account not in ue_audio_generate_dict.keys():
                ue_audio_generate_dict[ue_account] = []
               
            if ue_account not in ue_video_generate_dict.keys():
                ue_video_generate_dict[ue_account] = []

            if content_type == 'Audio':
                audio_content_gen_list = ue_audio_generate_dict[ue_account]
                if audio_content_gen_list == []:
                    audio_content_gen_list = [[timestamp, content_type, prefix, size]]
                else:
                    audio_content_gen_list.insert(-1, [timestamp, content_type, prefix, size])
                ue_audio_generate_dict[ue_account] = audio_content_gen_list
               
            if content_type == 'Video':
                video_content_gen_list = ue_video_generate_dict[ue_account]
                if video_content_gen_list == []:
                    video_content_gen_list = [[timestamp, content_type, prefix, size]]
                else:
                    video_content_gen_list.insert(-1, [timestamp, content_type, prefix, size])
                ue_video_generate_dict[ue_account] = video_content_gen_list 


    fopen.close()
    return ue_audio_generate_dict, ue_video_generate_dict

def get_content_recv(log_file):
    '''Get the lines in the log regarding receving object 
        return two dicts: audio, video
    '''
    ue_audio_recv_dict = {}
    ue_video_recv_dict = {}
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "OnContentObject(): " in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            content_type = temp[4]
            prefix = temp[8]
            if ue_account not in ue_audio_recv_dict.keys():
                ue_audio_recv_dict[ue_account] = []
            if ue_account not in ue_video_recv_dict.keys():
                ue_video_recv_dict[ue_account] = []

            if content_type == 'Audio':
                if 'audio' in prefix:
                    audio_content_recv_list = ue_audio_recv_dict[ue_account]
                    if audio_content_recv_list == []:
                        audio_content_recv_list = [[timestamp, content_type, prefix]]
                    else:
                        audio_content_recv_list.insert(-1, [timestamp, content_type, prefix])
                    ue_audio_recv_dict[ue_account] = audio_content_recv_list

            if content_type == 'Video':
                if 'video' in prefix:
                    video_content_recv_list = ue_video_recv_dict[ue_account]
                    if video_content_recv_list == []:
                        video_content_recv_list = [[timestamp, content_type, prefix]]
                    else:
                        video_content_recv_list.insert(-1, [timestamp, content_type, prefix])
                    ue_video_recv_dict[ue_account] = video_content_recv_list        
    fopen.close()
    return ue_audio_recv_dict, ue_video_recv_dict

def generation_gnuplot(log_file):
    """
    This will generate audio/video generation graph for selected UEs
    """
    ue_audio_generate_dict, ue_video_generate_dict = get_content_gen(log_file)
    ue1, ue2, ue3 = test_ues(log_file)
    
    ######################################################################
    #################   plotting audio generation time
    #####################################################################

    if ue_audio_generate_dict.values() !=[[],[],[]]:
        print "Audio:"        

        file1 = 'csv/' + ue1 + '-' + log_file + '.audio.generation.csv' 
        file2 = 'csv/' + ue2 + '-' + log_file + '.audio.generation.csv'
        file3 = 'csv/' + ue3 + '-' + log_file + '.audio.generation.csv'
        file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.audio.generation.csv'

        gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.audio.generation.gp'
        gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.audio.generation.gp'
        gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.audio.generation.gp'
        gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.audio.generation.gp'


        eps_output1 = 'eps/' + ue1 + '-' + log_file + '.audio.generation.eps' 
        eps_output2 = 'eps/' + ue2 + '-' + log_file + '.audio.generation.eps' 
        eps_output3 = 'eps/' + ue3 + '-' + log_file + '.audio.generation.eps' 
        eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.audio.generation.eps'

        list1 = ue_audio_generate_dict[ue1]
        list2 = ue_audio_generate_dict[ue2]
        list3 = ue_audio_generate_dict[ue3]
        
        output_dict1={}
        output_dict2={}
        output_dict3={}
        
        csvFileList=[file1, file2, file3]
        gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
        epsOutputList=[eps_output1, eps_output2, eps_output3]
        audioGenrateList=[list1, list2, list3]
        TempOutputDictList=[output_dict1, output_dict2, output_dict3]
        
        for i in range(3):
            for each_interest in audioGenrateList[i]:
                #print each_interest
                generate_time=float(each_interest[0].strip('s'))
                prefix_split=each_interest[2].strip('/').split('/')
                #print prefix_split
                interest_number=int(prefix_split[4])
                TempOutputDictList[i][interest_number]=generate_time

            with open(csvFileList[i], 'w') as inf:
                for key in sorted(TempOutputDictList[i]):
                    inf.write(str(key) + " " + str(TempOutputDictList[i][key]) + "\n")
                    #print "interest_number:" , key, " generate_time: ", TempOutputDictList[i][key]

            with open(gpFileList[i], 'w') as inf:
                inf.write("set t postscript enhanced eps color\n")
                inf.write("set output \'" + epsOutputList[i] + "\'\n")
                inf.write("set ylabel 'Generation Time (sec)'\n")
                inf.write("set xlabel 'Interests number'\n")
                inf.write("set key below vertical right box 3\n")
                inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
                inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
                #inf.write("set title \'" + file1 + "\'\n")
                plot_command = "plot \'" + csvFileList[i] + "\' u 1:2 w lp lw 3 t \'generation_time\'"
                inf.write(plot_command)
                inf.write("\n")   

            print "plotting...", gpFileList[i]
            subprocess.call("gnuplot " + gpFileList[i], shell=True)
        
        ######## write/plot  all three outputs to one file/picture
        with open(file4, 'w') as inf:
            for i in range(3):
                with open(csvFileList[i], 'r') as tem_inf:
                    line = tem_inf.read()
                    inf.write(line)
                inf.write('\n\n')

        with open(gnuplot_output4, 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + eps_output4 + "\'\n")
            inf.write("set ylabel 'Generation Time (sec)'\n")
            inf.write("set xlabel 'Interests number'\n")
            inf.write("set key below vertical right box 3\n")
            inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
            #inf.write("set title \'" + file1 + "\'\n")
            plot_command = "plot \'" + file4 + "\' index 0 u 1:2 w lp lw 3 t \'" + ue1 + "\', " + \
            " \'" + file4 + "\' index 1 u 1:2 w lp lw 3 t \'" + ue2 + "\', " + \
            " \'" + file4 + "\' index 2 u 1:2 w lp lw 3 t \'" + ue3 + "\'" 
            inf.write(plot_command)
            inf.write("\n") 

        print "plotting...", gnuplot_output4
        subprocess.call("gnuplot " + gnuplot_output4, shell=True)

    ######################################################################
    #################   plotting video generation time
    ######################################################################
    if ue_video_generate_dict.values() !=[[],[],[]]:
        print "Video:"        

        file1 = 'csv/' + ue1 + '-' + log_file + '.video.generation.csv' 
        file2 = 'csv/' + ue2 + '-' + log_file + '.video.generation.csv'
        file3 = 'csv/' + ue3 + '-' + log_file + '.video.generation.csv'
        file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.video.generation.csv'

        gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.video.generation.gp'
        gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.video.generation.gp'
        gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.video.generation.gp'
        gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.video.generation.gp'


        eps_output1 = 'eps/' + ue1 + '-' + log_file + '.video.generation.eps' 
        eps_output2 = 'eps/' + ue2 + '-' + log_file + '.video.generation.eps' 
        eps_output3 = 'eps/' + ue3 + '-' + log_file + '.video.generation.eps' 
        eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.video.generation.eps'

        list1 = ue_video_generate_dict[ue1]
        list2 = ue_video_generate_dict[ue2]
        list3 = ue_video_generate_dict[ue3]
        
        output_dict1={}
        output_dict2={}
        output_dict3={}
        
        csvFileList=[file1, file2, file3]
        gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
        epsOutputList=[eps_output1, eps_output2, eps_output3]
        audioGenrateList=[list1, list2, list3]
        TempOutputDictList=[output_dict1, output_dict2, output_dict3]
        
        for i in range(3):
            for each_interest in audioGenrateList[i]:
                #print each_interest
                generate_time=float(each_interest[0].strip('s'))
                prefix_split=each_interest[2].strip('/').split('/')
                #print prefix_split
                interest_number=int(prefix_split[4])
                TempOutputDictList[i][interest_number]=generate_time

            with open(csvFileList[i], 'w') as inf:
                for key in sorted(TempOutputDictList[i]):
                    inf.write(str(key) + " " + str(TempOutputDictList[i][key]) + "\n")
                    #print "interest_number:" , key, " generate_time: ", TempOutputDictList[i][key]

            with open(gpFileList[i], 'w') as inf:
                inf.write("set t postscript enhanced eps color\n")
                inf.write("set output \'" + epsOutputList[i] + "\'\n")
                inf.write("set ylabel 'Generation Time (sec)'\n")
                inf.write("set xlabel 'Interests number'\n")
                inf.write("set key below vertical right box 3\n")
                inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
                inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
                #inf.write("set title \'" + file1 + "\'\n")
                plot_command = "plot \'" + csvFileList[i] + "\' u 1:2 w lp lw 3  t \'generation_time\'"
                inf.write(plot_command)
                inf.write("\n")   

            print "plotting...", gpFileList[i]
            subprocess.call("gnuplot " + gpFileList[i], shell=True)
        
        ######## write/plot  all three outputs to one file/picture
        with open(file4, 'w') as inf:
            for i in range(3):
                with open(csvFileList[i], 'r') as tem_inf:
                    line = tem_inf.read()
                    inf.write(line)
                inf.write('\n\n')

        with open(gnuplot_output4, 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + eps_output4 + "\'\n")
            inf.write("set ylabel 'Generation Time (sec)'\n")
            inf.write("set xlabel 'Interests number'\n")
            inf.write("set key below vertical right box 3\n")
            inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
            #inf.write("set title \'" + file1 + "\'\n")
            plot_command = "plot \'" + file4 + "\' index 0 u 1:2 w lp lw 3 t \'" + ue1 + "\', " + \
            " \'" + file4 + "\' index 1 u 1:2 w lp lw 3 t \'" + ue2 + "\', " + \
            " \'" + file4 + "\' index 2 u 1:2 w lp lw 3 t \'" + ue3 + "\'" 
            inf.write(plot_command)
            inf.write("\n") 

        print "plotting...",gnuplot_output4
        subprocess.call("gnuplot " + gnuplot_output4, shell=True)

def get_audio_converge_per_interest(log_file, interest_number=0):
    """
    for given UEs and interest_number
    plot interest convergence time
    """
    ue_amount = get_config()['NumberOfUEs']

    fileLength = test_file_name_length(log_file)
    if(fileLength == 3):
        ue1, ue2, ue3 = test_ues(log_file)   # audio or video ues
    elif(fileLength == 6):
        ue1, ue2, ue3, ue4, ue5, ue6 = test_ues(log_file)   # audio1, aduio2, audio3, video1, video2, video3
    
    if(fileLength == 3):
        a1, a2, a3 = test_accounts(log_file)   # audio or video ues
    elif(fileLength == 6):
        a1, a2, a3, a4, a5, a6 = test_accounts(log_file)   # audio1, aduio2, audio3, video1, video2, video3

    ue_audio_converge_dict, ue_video_converge_dict = get_converge_from_sender(log_file)  

    interest1 = '/' + ue1 + '/' + 'chatroom-cona' + '/' + a1 + '/' + 'audio' + '/' + str(interest_number)
    interest2 = '/' + ue2 + '/' + 'chatroom-cona' + '/' + a2 + '/' + 'audio' + '/' + str(interest_number)
    interest3 = '/' + ue3 + '/' + 'chatroom-cona' + '/' + a3 + '/' + 'audio' + '/' + str(interest_number)
    interests = [interest1, interest2, interest3]

    file1 = 'csv/' + ue1 + '-' + str(interest_number) + '-' + log_file + '.audio.PerInterestConverge.csv' 
    file2 = 'csv/' + ue2 + '-' + str(interest_number) + '-' + log_file + '.audio.PerInterestConverge.csv'
    file3 = 'csv/' + ue3 + '-' + str(interest_number) + '-' + log_file + '.audio.PerInterestConverge.csv'
    file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + str(interest_number) + '-' + log_file + '.audio.PerInterestConverge.csv'
    files = [file1, file2, file3]

    gnuplot_output1 = 'gp/' + ue1 + '-' + str(interest_number) + '-' + log_file + '.audio.PerInterestConverge.gp'
    gnuplot_output2 = 'gp/' + ue2 + '-' + str(interest_number) + '-' + log_file + '.audio.PerInterestConverge.gp'
    gnuplot_output3 = 'gp/' + ue3 + '-' + str(interest_number) + '-' + log_file + '.audio.PerInterestConverge.gp'
    gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + str(interest_number) + '-' + log_file + '.audio.PerInterestConverge.gp'

    eps_output1 = 'eps/' + ue1 + '-' + str(interest_number) + '-' + log_file + '.audio.PerInterestConverge.eps' 
    eps_output2 = 'eps/' + ue2 + '-' + str(interest_number) + '-' + log_file + '.audio.PerInterestConverge.eps' 
    eps_output3 = 'eps/' + ue3 + '-' + str(interest_number) + '-' + log_file + '.audio.PerInterestConverge.eps' 
    eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + str(interest_number) + '-' + log_file + '.audio.PerInterestConverge.eps'

    gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
    epsOutputList=[eps_output1, eps_output2, eps_output3]

    ##############################################################################################
    #####  GET audio interest converge time
    ##########################################################################################
    
    ue_list = get_ue_list(log_file)

    i=0  # increase interest number
    for ue in test_ues(log_file):
        if i<3:
            print ue
            with open(files[i], 'w') as inf:
                inf.write("# UE" + ue + " Interest:" + str(interests[i]) + "\n")
                inf.write("# UE#   " + " PerInterestConvergeTimentObject_Time " + "\n")
                for each_ue in ue_list:
                    for entry in ue_audio_converge_dict[ue][interest_number]:
                        recv_ue = entry[1]
                        if each_ue == recv_ue:
                            convergence_time = entry[2]
                            print recv_ue.strip('U'), " ", convergence_time
                            inf.write(recv_ue.strip('U') + " " + str(convergence_time) + "\n")
            
        print ""    
        i=i+1

    # put all 3 csv files together and plot once
    with open(file4, 'w') as inf:
        for i in range(3):
            with open(files[i], 'r') as tem_inf:
                line = tem_inf.read()
                inf.write(line)
                inf.write("\n")
            inf.write("\n\n")
    ################################
    ####### plot audio per interest converge
    ################################
 
    with open(gnuplot_output4, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps_output4 + "\'\n")
        inf.write("set ylabel 'TimeStamp (sec)'\n")
        inf.write("set xlabel 'UE'\n")
        inf.write("set key below vertical right box 3\n")
        inf.write("set xrange[1:" + ue_amount + "]\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set key bottom vertical right box 3\n")
        #inf.write("set title \'" + file1 + "\'\n")
        plot_command = "plot \'" + file4 + "\' index 0 u 1:2 w lp t \'" + ue1 + "\', " + \
        " \'" + file4 + "\' index 1 u 1:2 w lp t \'" + ue2 + "\', " + \
        " \'" + file4 + "\' index 2 u 1:2 w lp t \'" + ue3 + "\'" 
        inf.write(plot_command)
        inf.write("\n") 

    print "plotting...", gnuplot_output4
    subprocess.call("gnuplot " + gnuplot_output4, shell=True)  

def get_video_converge_per_interest(log_file, interest_number=25):
    """
    for given UEs and interest_number
    plot interest convergence time
    """
    ##############################################################################################
    #####  GET Video interest converge time
    ##########################################################################################
    ue_amount = get_config()['NumberOfUEs']

    fileLength = test_file_name_length(log_file)
    if(fileLength == 3):
        ue1, ue2, ue3 = test_ues(log_file)   # audio or video ues
    elif(fileLength == 6):
        ue1, ue2, ue3, ue4, ue5, ue6 = test_ues(log_file)   # audio1, aduio2, audio3, video1, video2, video3
    
    if(fileLength == 3):
        a1, a2, a3 = test_accounts(log_file)   # audio or video ues
    elif(fileLength == 6):
        a1, a2, a3, a4, a5, a6 = test_accounts(log_file)   # audio1, aduio2, audio3, video1, video2, video3

    ue_audio_converge_dict, ue_video_converge_dict = get_converge_from_sender(log_file)  

    interest1 = '/' + ue1 + '/' + 'chatroom-cona' + '/' + a1 + '/' + 'video' + '/' + str(interest_number)
    interest2 = '/' + ue2 + '/' + 'chatroom-cona' + '/' + a2 + '/' + 'video' + '/' + str(interest_number)
    interest3 = '/' + ue3 + '/' + 'chatroom-cona' + '/' + a3 + '/' + 'video' + '/' + str(interest_number)
    interests = [interest1, interest2, interest3]

    file1 = 'csv/' + ue1 + '-' + str(interest_number) + '-' + log_file + '.video.PerInterestConverge.csv' 
    file2 = 'csv/' + ue2 + '-' + str(interest_number) + '-' + log_file + '.video.PerInterestConverge.csv'
    file3 = 'csv/' + ue3 + '-' + str(interest_number) + '-' + log_file + '.video.PerInterestConverge.csv'
    file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + str(interest_number) + '-' + log_file + '.video.PerInterestConverge.csv'
    files = [file1, file2, file3]

    gnuplot_output1 = 'gp/' + ue1 + '-' + str(interest_number) + '-' + log_file + '.video.PerInterestConverge.gp'
    gnuplot_output2 = 'gp/' + ue2 + '-' + str(interest_number) + '-' + log_file + '.video.PerInterestConverge.gp'
    gnuplot_output3 = 'gp/' + ue3 + '-' + str(interest_number) + '-' + log_file + '.video.PerInterestConverge.gp'
    gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + str(interest_number) + '-' + log_file + '.video.PerInterestConverge.gp'

    eps_output1 = 'eps/' + ue1 + '-' + str(interest_number) + '-' + log_file + '.video.PerInterestConverge.eps' 
    eps_output2 = 'eps/' + ue2 + '-' + str(interest_number) + '-' + log_file + '.video.PerInterestConverge.eps' 
    eps_output3 = 'eps/' + ue3 + '-' + str(interest_number) + '-' + log_file + '.video.PerInterestConverge.eps' 
    eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + str(interest_number) + '-' + log_file + '.video.PerInterestConverge.eps'

    gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
    epsOutputList=[eps_output1, eps_output2, eps_output3]

    ##############################################################################################
    #####  GET video interest converge time
    ##########################################################################################
    
    ue_list = get_ue_list(log_file)

    i=0  # increase interest number
    for ue in test_ues(log_file):
        if i<3:
            print ue
            with open(files[i], 'w') as inf:
                inf.write("# UE" + ue + " Interest:" + str(interests[i]) + "\n")
                inf.write("# UE#   " + " PerInterestConvergeTimentObject_Time " + "\n")
                for each_ue in ue_list:
                    for entry in ue_video_converge_dict[ue][interest_number]:
                        recv_ue = entry[1]
                        if each_ue == recv_ue:
                            convergence_time = entry[2]
                            print recv_ue.strip('U'), " ", convergence_time
                            inf.write(recv_ue.strip('U') + " " + str(convergence_time) + "\n")
            
        print ""    
        i=i+1

    # put all 3 csv files together and plot once
    with open(file4, 'w') as inf:
        for i in range(3):
            with open(files[i], 'r') as tem_inf:
                line = tem_inf.read()
                inf.write(line)
                inf.write("\n")
            inf.write("\n\n")
    ################################
    ####### plot audio per interest converge
    ################################
 
    with open(gnuplot_output4, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps_output4 + "\'\n")
        inf.write("set ylabel 'TimeStamp (sec)'\n")
        inf.write("set xlabel 'UE'\n")
        inf.write("set xrange[1:" + ue_amount + "]\n")
        inf.write("set key below vertical right box 3\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set key bottom vertical right box 3\n")
        #inf.write("set title \'" + file1 + "\'\n")
        plot_command = "plot \'" + file4 + "\' index 0 u 1:2 w lp t \'" + ue1 + "\', " + \
        " \'" + file4 + "\' index 1 u 1:2 w lp t \'" + ue2 + "\', " + \
        " \'" + file4 + "\' index 2 u 1:2 w lp t \'" + ue3 + "\'" 
        inf.write(plot_command)
        inf.write("\n") 

    print "plotting...", gnuplot_output4
    subprocess.call("gnuplot " + gnuplot_output4, shell=True)   
    
def get_video_gop_dist(log_file):
    """
    get video source GOP size list
    In [65]: ue_video_gop_generate_dict['U5']
    Out[65]: 
        [['3.4801', '1', '126', '0.0038'],
        ['3.9602', '2', '173', '0.0027'],
        ['4.4403', '3', '185', '0.0025'],
        ['4.9204', '4', '130', '0.0036'],
        ['5.4005', '5', '185', '0.0025'],
        ['5.8806', '6', '155', '0.0030'],
        ['3', '0', '150', '0.0032']]

    """
    ue_video_gop_generate_dict = {}
    fopen = open(log_file, 'r')
    with open(log_file,'r') as inf:
        for line in inf.readlines():
            if "*GOP" in line:
                temp = line.strip().split()
                timestamp = temp[0].strip('s')
                ue_account = temp[2].strip(":")
                gop_position = temp[5]
                gop_size = temp[8]
                interval_temp =temp[11]
                gop_timeInterval = interval_temp[9:15]

                #print ue_account, timestamp, ue_account, gop_position, gop_size, gop_timeInterval
                if ue_account not in ue_video_gop_generate_dict.keys():
                    ue_video_gop_generate_dict[ue_account] = []

                video_gop_generate_list = ue_video_gop_generate_dict[ue_account]
                if video_gop_generate_list == []:
                    video_gop_generate_list = [[timestamp, gop_position, gop_size, gop_timeInterval]]
                else:
                    video_gop_generate_list.insert(-1, [timestamp, gop_position, gop_size, gop_timeInterval])
                ue_video_gop_generate_dict[ue_account] = video_gop_generate_list

    return ue_video_gop_generate_dict

def plot_video_gop_dist(log_file):
    """
    read gop_size_generation from get_video_gop_dist(log_file)
    plot video gop size distribution
    """
    ue_video_gop_generate_dict = get_video_gop_dist(log_file)
    video_source_amount = len(ue_video_gop_generate_dict.keys())
    video_source_account = ue_video_gop_generate_dict.keys()
    #print video_source_account

    # get packet size from file name
    video_packet_size = log_file.split('.')[0].split('_')[9].strip('bytes')
    # base on video source number create csv, gnuplot file and eps file
    csvFileList = []
    gpFileList = []
    epsOutputList = []
    for i in range(video_source_amount):
        csvFile = "csv/" + video_source_account[i] + "-" + log_file + '.gop_size.csv'
        gpFile = "gp/" + video_source_account[i] + "-" + log_file + '.gop_size.gp'
        epsFile = "eps/" + video_source_account[i] + "-" + log_file + '.gop_size_interval.eps'
        csvFileList.insert(-1, csvFile)
        gpFileList.insert(-1, gpFile)
        epsOutputList.insert(1,epsFile)

    #print csvFileList
    #print gpFileList
    #print epsOutputList

    print video_source_amount

    # write to csv files
    print ue_video_gop_generate_dict.keys()
    for ue in ue_video_gop_generate_dict.keys():
        print "#",ue
        gop_begin_at = -1
        for csv_file in csvFileList:
            ue_account = csv_file.split('/')[1].split('-')[0]
            if ue == ue_account:
                print ue, ' ', ue_account
                with open(csv_file, 'w') as inf:
                    #print csv_file
                    inf.write("#GOPsizedistion\n" )
                    inf.write("#UE  time gop_position GOP_size  video_packet_size  Interval\n")
                    for each_gop in ue_video_gop_generate_dict[ue]:
                        if(int(each_gop[1]) > gop_begin_at):
                            gop_begin_at = int(each_gop[1]) 
                            #print "     ", each_gop                 
                            inf.write(ue.strip('U') + " " + each_gop[0] + " " + each_gop[1] + " " +\
                                each_gop[2] + " " + video_packet_size +" " + each_gop[3] + "\n")
                        #else:
                        #    inf.seek(0)
                        #    #print "     ", each_gop                 
                        #    inf.write(ue.strip('U') + " " + each_gop[0] + " " + each_gop[1] + " " +\
                        #        each_gop[2] + " " + video_packet_size +" " + each_gop[3] + "\n")
                print csv_file
                print "\n write to gnuplot file\n"
                for gp_file in gpFileList:
                    ue_account = gp_file.split('/')[1].split('-')[0]
                    if ue == ue_account:
                        print ue, ' -- ', ue_account
                        for eps_file in epsOutputList:
                            if ue == eps_file.split('/')[1].split('-')[0]:
                                with open(gp_file, 'w') as inf:
                                    print gp_file
                                    inf.write("set t postscript enhanced eps color\n")
                                    inf.write("set output \'" + eps_file + "\'\n")
                                    inf.write("set ylabel 'Packet Sendign Interval (sec)'\n")
                                    inf.write("set y2label 'GOP Size (bytes)'\n")
                                    inf.write("set autoscale y\n")
                                    inf.write("set autoscale  y2\n")
                                    inf.write("set ytics\n")
                                    inf.write("set key below vertical right box 3\n")
                                    inf.write("set tics out\n")
                                    inf.write("set ytics nomirror\n")
                                    inf.write("set y2tics nomirror\n")
                                    inf.write("set xlabel 'GOP Number'\n")
                                    inf.write("set parametric\n")
                                    inf.write("set grid ytics lc rgb '#bbbbbb' lw 3 lt 0\n")
                                    inf.write("set grid xtics lc rgb '#bbbbbb' lw 3 lt 0\n")
                                    #inf.write("set title \'" + csvFileList[i] + "\'\n")
                                    plot_command = "plot \'" + csv_file + "\' u 3:6 w lp lw 5 t \'Interval\' axes x1y1, " \
                                           + "\'" + csv_file + "\' u 3:4 w lp lw 5 t \'GOP Size\' axes x1y2"
                                            
                                    inf.write(plot_command)
                
                                    inf.write("\n")
                
    for gp_file in gpFileList:
        print "plotting..."
        subprocess.call("gnuplot " + gp_file, shell=True)
   
def get_converge_from_sender(log_file):
    """
     ue_audio_generate_dict:   { ,'U4': [['9.93875s', 'Audio', '/U4/chatroom-cona/A4/audio/255', '136'],[],[]]  }
     ue_audio_recv_dict     :  { ,'U3': [['9.99488s', 'Audio', '/U4/chatroom-cona/A4/audio/255'],, [],[]]  }
                            :  { ,'U2': [['9.99488s', 'Audio', '/U4/chatroom-cona/A4/audio/255'],
    """
    ue_audio_generate_dict, ue_video_generate_dict = get_content_gen(log_file)
    ue_audio_recv_dict, ue_video_recv_dict = get_content_recv(log_file)

    ue_audio_converge_dict = {}
    ue_video_converge_dict = {}
    audio_lost_package_dict = {}
    video_lost_package_dict = {}
    audio_lost_dict = {}
    video_lost_dict = {}
    audio_package_converge_list = []
    video_package_converge_list = []

    ###########################################################################
    # collect audio converge info
    ###########################################################################
    for ue in ue_audio_generate_dict.keys():
        if ue not in ue_audio_converge_dict.keys():
            ue_audio_converge_dict[ue] = []
        if ue not in audio_lost_package_dict.keys():
            audio_lost_package_dict[ue] = []
        #print "UE: ", ue
        for each_gen in ue_audio_generate_dict[ue]:
            audio_lost_package_list = []
            recv_time_list = []
            send_time = float(each_gen[0].strip('s'))
            send_prefix = each_gen[2]
            prefix_split = send_prefix.strip('/').split('/')
            send_package_num = int(prefix_split[4])
            #convergence_time=0
            #print "package_number:", send_package_num

            for ue_recv in ue_audio_recv_dict.keys():
                if ue != ue_recv:
                    # go through all the receieved interests

                    for each_recv in ue_audio_recv_dict[ue_recv]:
                        found = False
                        prefix_recv = each_recv[2]
                        if prefix_recv == send_prefix:
                            found = True
                            break 
                    # check if interests have been received                         
                    if found == True:
                        recv_time = float(each_recv[0].strip('s'))
                        package_convergence_time_for_ue_recv = float("%0.5f" % (recv_time - send_time))
                        #print " Package_number:", send_package_num ," received by: ", ue_recv," recv_time:", recv_time, " send_time:", send_time,\
                        #" converge time:", package_convergence_time_for_ue_recv
                        temp_list = [send_package_num, ue_recv, package_convergence_time_for_ue_recv]
                        #recv_time_list.append(recv_time)
                        if recv_time_list == []:
                            recv_time_list = [temp_list]
                        else:
                            recv_time_list.insert(-1, temp_list)
                    elif found == False:
                        package_convergence_time_for_ue_recv = float("%0.5f" % 0.000)
                        temp_list = [send_package_num, ue_recv, package_convergence_time_for_ue_recv]
                        if recv_time_list == []:
                            recv_time_list = [temp_list]
                        else:
                            recv_time_list.insert(-1, temp_list)
                        #print "     audio source package_number:", send_package_num
                        #print "     ", ue_recv, " lost packet number:", send_package_num, " from Audio source:", ue
                        previous_lost = audio_lost_package_dict[ue]
                        if previous_lost == []:
                            previous_lost = [{send_package_num:ue_recv}]
                        else:
                            previous_lost.insert(-1, {send_package_num:ue_recv})
                        audio_lost_package_dict[ue] = previous_lost

            #if len(recv_time_list) != 0:
            #    convergence_time = float("%.5f" % (recv_time_list[len(recv_time_list)-1] - send_time))
                #print "recv_time_list:", recv_time_list
                #print "convergence_time:", convergence_time

            audio_package_converge_list = ue_audio_converge_dict[ue]
            if audio_package_converge_list == []:
                audio_package_converge_list = [recv_time_list]
            else:
                audio_package_converge_list.insert(-1, recv_time_list)

            ue_audio_converge_dict[ue] = audio_package_converge_list

            # audio_package_converge_list = ue_audio_converge_dict[ue]
            # if audio_package_converge_list == []:
            #     audio_package_converge_list = [[send_package_num, convergence_time]]
            # else:
            #     audio_package_converge_list.insert(-1, [send_package_num, convergence_time])

            # ue_audio_converge_dict[ue] = audio_package_converge_list

    #############################################################################
    # collect video converge info
    #############################################################################
    for ue in ue_video_generate_dict.keys():
        if ue not in ue_video_converge_dict.keys():
            ue_video_converge_dict[ue] = []
        if ue not in video_lost_package_dict.keys():
            video_lost_package_dict[ue] = []
        #print "UE: ", ue
        for each_gen in ue_video_generate_dict[ue]:
            video_lost_package_list = []
            recv_time_list = []
            send_time = float(each_gen[0].strip('s'))
            send_prefix = each_gen[2]
            prefix_split = send_prefix.strip('/').split('/')
            send_package_num = int(prefix_split[4])
            #convergence_time=0
            #print "package_number:", send_package_num

            for ue_recv in ue_video_recv_dict.keys():
                if ue != ue_recv:
                    # go through all the receieved interests

                    for each_recv in ue_video_recv_dict[ue_recv]:
                        found = False
                        prefix_recv = each_recv[2]
                        if prefix_recv == send_prefix:
                            found = True
                            break 
                    # check if interests have been received                         
                    if found == True:
                        recv_time = float(each_recv[0].strip('s'))
                        package_convergence_time_for_ue_recv = float("%0.5f" % (recv_time - send_time))
                        #print " Package_number:", send_package_num ," received by: ", ue_recv," recv_time:", recv_time, " send_time:", send_time,\
                        #" converge time:", package_convergence_time_for_ue_recv
                        temp_list = [send_package_num, ue_recv, package_convergence_time_for_ue_recv]
                        #recv_time_list.append(recv_time)
                        if recv_time_list == []:
                            recv_time_list = [temp_list]
                        else:
                            recv_time_list.insert(-1, temp_list)
                    elif found == False:
                        package_convergence_time_for_ue_recv = float("%0.5f" % 0.000)
                        temp_list = [send_package_num, ue_recv, package_convergence_time_for_ue_recv]
                        if recv_time_list == []:
                            recv_time_list = [temp_list]
                        else:
                            recv_time_list.insert(-1, temp_list)
                        #print "     audio source package_number:", send_package_num
                        #print "     ", ue_recv, " lost packet number:", send_package_num, " from Audio source:", ue
                        previous_lost = video_lost_package_dict[ue]
                        if previous_lost == []:
                            previous_lost = [{send_package_num:ue_recv}]
                        else:
                            previous_lost.insert(-1, {send_package_num:ue_recv})
                        video_lost_package_dict[ue] = previous_lost

            #if len(recv_time_list) != 0:
            #    convergence_time = float("%.5f" % (recv_time_list[len(recv_time_list)-1] - send_time))
                #print "recv_time_list:", recv_time_list
                #print "convergence_time:", convergence_time

            video_package_converge_list = ue_video_converge_dict[ue]
            if video_package_converge_list == []:
                video_package_converge_list = [recv_time_list]
            else:
                video_package_converge_list.insert(-1, recv_time_list)

            ue_video_converge_dict[ue] = video_package_converge_list

            # video_package_converge_list = ue_video_converge_dict[ue]
            # if video_package_converge_list == []:
            #     video_package_converge_list = [[send_package_num, convergence_time]]
            # else:
            #     video_package_converge_list.insert(-1, [send_package_num, convergence_time])

            # ue_video_converge_dict[ue] = video_package_converge_list


    return ue_audio_converge_dict, ue_video_converge_dict


def per_interest_converge_gnulpot(log_file):
    """
    for selected UEs, plot all interests converge time for selected UEs
    """
    fileLength = test_file_name_length(log_file)
    if(fileLength == 3):
        ue1, ue2, ue3 = test_ues(log_file)   # audio or video ues
    elif(fileLength == 6):
        ue1, ue2, ue3, ue4, ue5, ue6 = test_ues(log_file)   # audio1, aduio2, audio3, video1, video2, video3
    
    if(fileLength == 3):
        a1, a2, a3 = test_accounts(log_file)   # audio or video ues
    elif(fileLength == 6):
        a1, a2, a3, a4, a5, a6 = test_accounts(log_file)   # audio1, aduio2, audio3, video1, video2, video3


    ue_audio_converge_dict, ue_video_converge_dict = get_converge_from_sender(log_file)
    
    ######################################################################
    #################   plotting audio converge time
    ######################################################################
    if ue_audio_converge_dict.values() != [[],[],[]] and ue_audio_converge_dict.values() != [[]]:
        print "Audio:"

        file1 = 'csv/' + ue1 + '-' + log_file + 'audio_converge.csv' 
        file2 = 'csv/' + ue2 + '-' + log_file + 'audio_converge.csv'
        file3 = 'csv/' + ue3 + '-' + log_file + 'audio_converge.csv'
        file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.audio_converge.csv'

        gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.audio_converge.gp'
        gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.audio_converge.gp'
        gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.audio_converge.gp'
        gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.audio_converge.gp'

        eps_output1 = 'eps/' + ue1 + '-' + log_file + '.audio_converge.eps' 
        eps_output2 = 'eps/' + ue2 + '-' + log_file + '.audio_converge.eps' 
        eps_output3 = 'eps/' + ue3 + '-' + log_file + '.audio_converge.eps' 
        eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.audio_converge.eps'

        list1 = ue_audio_converge_dict[ue1]
        list2 = ue_audio_converge_dict[ue2]
        list3 = ue_audio_converge_dict[ue3]

        output_dict1={}
        output_dict2={}
        output_dict3={}

        csvFileList=[file1, file2, file3]
        gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
        epsOutputList=[eps_output1, eps_output2, eps_output3]
        audioConvergeList=[list1, list2, list3]
        TempOutputDictList=[output_dict1, output_dict2, output_dict3]

        for i in range(3):
            for packet_list in audioConvergeList[i]:
                converge_max=0.0
                for packet in packet_list:
                    packet_number = packet[0]
                    if(float(packet[2]) >= converge_max):
                        converge_max = float(packet[2])
                TempOutputDictList[i][packet_number] = converge_max
           
            with open(csvFileList[i], 'w') as inf:
                for key in sorted(TempOutputDictList[i]):
                    if(TempOutputDictList[i][key] !=0):
                        inf.write(str(key) + " " + str(TempOutputDictList[i][key]) + "\n")
                        #print "packet_number:", str(key), " converge_max:", str(TempOutputDictList[i][key])
               

            with open(gpFileList[i], 'w') as inf:
                inf.write("set t postscript enhanced eps color\n")
                inf.write("set output \'" + epsOutputList[i] + "\'\n")
                inf.write("set ylabel 'converge max (sec)'\n")
                inf.write("set xlabel 'interests number'\n")
                inf.write("set key bottom vertical right box 3\n")
                inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
                inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
                #inf.write("set title \'" + csvFileList[i] + "\'\n")
                plot_command = "plot \'" + csvFileList[i] + "\' u 1:2 w lp lw 3 t \'converge time\'"
                inf.write(plot_command)
                inf.write("\n")

            print "plotting...", gpFileList[i]
            subprocess.call("gnuplot " + gpFileList[i], shell=True)

        ##### write/plot into one file/picture
        with open(file4, 'w') as inf:
            for i in range(3):
                with open(csvFileList[i], 'r') as tem_inf:
                    line = tem_inf.read()
                    inf.write(line)
                inf.write('\n\n')

        with open(gnuplot_output4, 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + eps_output4 + "\'\n")
            inf.write("set ylabel 'Generation Time (sec)'\n")
            inf.write("set xlabel 'Interests number'\n")
            inf.write("set key bottom vertical right box 3\n")
            inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
            #inf.write("set title \'" + file1 + "\'\n")
            plot_command = "plot \'" + file4 + "\' index 0 u 1:2 w lp lw 3 t \'" + ue1 + "\', " + \
            " \'" + file4 + "\' index 1 u 1:2 w lp lw 3 t \'" + ue2 + "\', " + \
            " \'" + file4 + "\' index 2 u 1:2 w lp lw 3 t \'" + ue3 + "\'" 
            inf.write(plot_command)
            inf.write("\n") 

        print "plotting...", gnuplot_output4,
        subprocess.call("gnuplot " + gnuplot_output4, shell=True)    

    ######################################################################
    #################   plotting video generation time
    ######################################################################
    if ue_video_converge_dict.values() != [[],[],[]] and ue_video_converge_dict.values() != [[]]:
        print "Video:"
        if(fileLength == 6):
            ue1, ue2, ue3 = ue4, ue5, ue6
            a1, a2, a3 = a4, a5, a6

        file1 = 'csv/' + ue1 + '-' + log_file + 'video_converge.csv' 
        file2 = 'csv/' + ue2 + '-' + log_file + 'video_converge.csv'
        file3 = 'csv/' + ue3 + '-' + log_file + 'video_converge.csv'
        file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.video_converge.csv'

        gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.video_converge.gp'
        gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.video_converge.gp'
        gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.video_converge.gp'
        gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.video_converge.gp'

        eps_output1 = 'eps/' + ue1 + '-' + log_file + '.video_converge.eps' 
        eps_output2 = 'eps/' + ue2 + '-' + log_file + '.video_converge.eps' 
        eps_output3 = 'eps/' + ue3 + '-' + log_file + '.video_converge.eps' 
        eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.video_converge.eps'

        list1 = ue_video_converge_dict[ue1]
        list2 = ue_video_converge_dict[ue2]
        list3 = ue_video_converge_dict[ue3]

        output_dict1={}
        output_dict2={}
        output_dict3={}

        csvFileList=[file1, file2, file3]
        gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
        epsOutputList=[eps_output1, eps_output2, eps_output3]
        videoConvergeList=[list1, list2, list3]
        TempOutputDictList=[output_dict1, output_dict2, output_dict3]

        for i in range(3):
            for packet_list in videoConvergeList[i]:
                converge_max=0.0
                for packet in packet_list:
                    packet_number = packet[0]
                    if(float(packet[2]) >= converge_max):
                        converge_max = float(packet[2])
                TempOutputDictList[i][packet_number] = converge_max
           
            with open(csvFileList[i], 'w') as inf:
                for key in sorted(TempOutputDictList[i]):
                    if(TempOutputDictList[i][key] !=0):
                        inf.write(str(key) + " " + str(TempOutputDictList[i][key]) + "\n")
                        #print "packet_number:", str(key), " converge_max:", str(TempOutputDictList[i][key])
               

            with open(gpFileList[i], 'w') as inf:
                inf.write("set t postscript enhanced eps color\n")
                inf.write("set output \'" + epsOutputList[i] + "\'\n")
                inf.write("set ylabel 'converge max (sec)'\n")
                inf.write("set xlabel 'interests number'\n")
                inf.write("set key bottom vertical right box 3\n")
                inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
                inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
                #inf.write("set title \'" + csvFileList[i] + "\'\n")
                plot_command = "plot \'" + csvFileList[i] + "\' u 1:2 w lp lw 3  t \'converge time\'"
                inf.write(plot_command)
                inf.write("\n")

            print "plotting...", gpFileList[i]
            subprocess.call("gnuplot " + gpFileList[i], shell=True)

        ##### write/plot into one file/picture
        with open(file4, 'w') as inf:
            for i in range(3):
                with open(csvFileList[i], 'r') as tem_inf:
                    line = tem_inf.read()
                    inf.write(line)
                inf.write('\n\n')

        with open(gnuplot_output4, 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + eps_output4 + "\'\n")
            inf.write("set ylabel 'Generation Time (sec)'\n")
            inf.write("set xlabel 'Interests number'\n")
            inf.write("set key bottom vertical right box 3\n")
            inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
            #inf.write("set title \'" + file1 + "\'\n")
            plot_command = "plot \'" + file4 + "\' index 0 u 1:2 w lp lw 3 t \'" + ue1 + "\', " + \
            " \'" + file4 + "\' index 1 u 1:2 w lp lw 3 t \'" + ue2 + "\', " + \
            " \'" + file4 + "\' index 2 u 1:2 w lp lw 3 t \'" + ue3 + "\'" 
            inf.write(plot_command)
            inf.write("\n") 

        print "plotting...", gnuplot_output4,
        subprocess.call("gnuplot " + gnuplot_output4, shell=True)

def get_rcv_notification_contentObject_per_interest(log_file, interest_number=25):
    ''' For three selected UEs
        find OnNotification Time for interest_number 30
    '''
    """ex
    ['2.04081s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/video/0']
    ['2.04175s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/video/0']
    ['2.04364s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/audio/0']
    ['2.04458s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/audio/0']
    """
    ue_amount = get_config()['NumberOfUEs']

    fileLength = test_file_name_length(log_file)
    if(fileLength == 3):
        ue1, ue2, ue3 = test_ues(log_file)   # audio or video ues
    elif(fileLength == 6):
        ue1, ue2, ue3, ue4, ue5, ue6 = test_ues(log_file)   # audio1, aduio2, audio3, video1, video2, video3
    
    if(fileLength == 3):
        a1, a2, a3 = test_accounts(log_file)   # audio or video ues
    elif(fileLength == 6):
        a1, a2, a3, a4, a5, a6 = test_accounts(log_file)   # audio1, aduio2, audio3, video1, video2, video3

    ue_audio_generate_dict, ue_video_generate_dict = get_content_gen(log_file)

    # get notification dict
    ue_audio_OnNotification_dict, ue_video_OnNotification_dict = get_OnNotification_recv(log_file)
    # get OnContentObject dict
    ue_audio_recv_dict, ue_video_recv_dict = get_content_recv(log_file)

    # get ue list
    ue_list = get_ue_list(log_file)
    ##############################################################################################
    ###  part 1: Audio 
    ##############################################################################################
    if ue_audio_generate_dict.values() != [[],[],[]] and ue_audio_generate_dict.values() != [[]]:
        print "Audio:"
        interest1 = '/' + ue1 + '/' + 'chatroom-cona' + '/' + a1 + '/' + 'audio' + '/' + str(interest_number)
        interest2 = '/' + ue2 + '/' + 'chatroom-cona' + '/' + a2 + '/' + 'audio' + '/' + str(interest_number)
        interest3 = '/' + ue3 + '/' + 'chatroom-cona' + '/' + a3 + '/' + 'audio' + '/' + str(interest_number)
        interests = [interest1, interest2, interest3]

        file1 = 'csv/' + ue1 + '-' + log_file + '.auido.OnNotification_OnContentObject.csv' 
        file2 = 'csv/' + ue2 + '-' + log_file + '.audio.OnNotification_OnContentObject.csv'
        file3 = 'csv/' + ue3 + '-' + log_file + '.audio.OnNotification_OnContentObject.csv'
        file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.audio.OnNotification_OnContentObject.csv'
        files = [file1, file2, file3]

        gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.audio.OnNotification_OnContentObject.gp'
        gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.audio.OnNotification_OnContentObject.gp'
        gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.audio.OnNotification_OnContentObject.gp'
        #gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.gp'

        eps_output1 = 'eps/' + ue1 + '-' + log_file + '.audio.OnNotification_OnContentObject.eps' 
        eps_output2 = 'eps/' + ue2 + '-' + log_file + '.audio.OnNotification_OnContentObject.eps' 
        eps_output3 = 'eps/' + ue3 + '-' + log_file + '.audio.OnNotification_OnContentObject.eps' 
        #eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.eps'

        gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
        epsOutputList=[eps_output1, eps_output2, eps_output3]

        #####################################
        #####  GET audio onNotification and on ContentObject
        #####################################

        # get content generation time dict

        timestamps = []
        # get interest generation time for each selected interest
        for ue in test_ues(log_file):
            for entry in ue_audio_generate_dict[ue]:
                prefix = entry[2]
                #print prefix
                prefix_split = prefix.strip('/').split('/')
                if int(prefix_split[4]) == interest_number:
                    timestamp = float(entry[0].strip('s'))
                    #print ue, " interest_number:", interest_number, " generation time: ", timestamp
                    timestamps.append(timestamp)

        # get each interest notificaiton time for each selected ue
        i=0  # increase interest number

        for ue in test_ues(log_file):
            if(i<3):
                #print i, files[i], timestamps[i]
                print interests[i]
                #print "---"
                with open(files[i], 'w') as inf:
                    inf.write("# UE" + ue + " Interest:" + str(interests[i]) + " generate_time: " + str(timestamps[i]) + "\n")
                    inf.write("# UE#   " + " OnNotification_Time    " + " OnContentObject_Time " + "\n")
                    #inf.write("0 3 3\n")
                    #print "interest: ", interests[i]
                    for each_ue in ue_list:
                        if ue != each_ue:
                            print each_ue
                            try:
                                temp_OnNotification_list = ue_audio_OnNotification_dict[each_ue]
                                #print "1", temp_OnNotification_list
                            except Exception as e:
                                temp_OnNotification_list =[]
                                OnNotification_time = ''
                                #print "2", OnNotification_time
                            if temp_OnNotification_list != []:
                                for item in temp_OnNotification_list:
                                    prefix = item[1]
                                    #print prefix
                                    if prefix == interests[i]: 
                                        OnNotification_time = float(item[0].strip('s'))
                                        print each_ue, " OnNotification:", OnNotification_time

                            try:
                                temp_OnContentObject_list = ue_audio_recv_dict[each_ue]
                            except Exception as e:
                                temp_OnContentObject_list = []
                                OnContentObject_time = ''
                            for item in temp_OnContentObject_list:
                                prefix = item[2]
                                if prefix == interests[i]:
                                    OnContentObject_time = float(item[0].strip('s'))
                                    #print each_ue, " OnContentObject:", OnContentObject_time , '\n'
                            each_ue = each_ue.strip('U')
                            #print "write to file:", files[i]           
                            inf.write(each_ue + "   " +  str(OnNotification_time) + "    " +  str(OnContentObject_time) + "\n")

            print ""
            i=i+1

        ################################
        ####### plot audio generation, onnotification, oncontentobject
        ################################
        for i in range(3):
            with open(gpFileList[i], 'w') as inf:
                inf.write("set t postscript enhanced eps color\n")
                inf.write("set output \'" + epsOutputList[i] + "\'\n")
                inf.write("set ylabel 'TimeStamp (sec)'\n")
                inf.write("set xlabel 'UE'\n")      
                inf.write("set key below vertical right box 3\n")
                inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
                inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
                inf.write("set xrange[1:" + ue_amount + "]\n")
                inf.write("f(x) = " + str(timestamps[i]) + "\n")
                #inf.write("set title \'" + file1 + "\'\n")
                plot_command = "plot f(x) lw 5 t \' Notification Generation Time\',  \'" + files[i]+ "\' u 1:2 w lp lw 5 t \' +OnNotification\', " + \
                " \'" + files[i]+ "\' u 1:3 w lp lw 5 t \'OnContentObject\'', "
                inf.write(plot_command)
                inf.write("\n")
            print "ploting...", gpFileList[i]
            subprocess.call("gnuplot " + gpFileList[i], shell=True)
            ++i

    ##############################################################################################
    ### Part 2: video 
    ##############################################################################################
    if ue_video_generate_dict.values() != [[],[],[]] and ue_video_generate_dict.values() != [[]]:
        print "Video:"
        if(fileLength == 6):
            ue1, ue2, ue3 = ue4, ue5, ue6
            a1, a2, a3 = a4, a5, a6
        interest1 = '/' + ue1 + '/' + 'chatroom-cona' + '/' + a1 + '/' + 'video' + '/' + str(interest_number)
        interest2 = '/' + ue2 + '/' + 'chatroom-cona' + '/' + a2 + '/' + 'video' + '/' + str(interest_number)
        interest3 = '/' + ue3 + '/' + 'chatroom-cona' + '/' + a3 + '/' + 'video' + '/' + str(interest_number)
        interests = [interest1, interest2, interest3]

        file1 = 'csv/' + ue1 + '-' + log_file + '.video.OnNotification_OnContentObject.csv' 
        file2 = 'csv/' + ue2 + '-' + log_file + '.video.OnNotification_OnContentObject.csv'
        file3 = 'csv/' + ue3 + '-' + log_file + '.video.OnNotification_OnContentObject.csv'
        file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.video.OnNotification_OnContentObject.csv'
        files = [file1, file2, file3]

        gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.video.OnNotification_OnContentObject.gp'
        gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.video.OnNotification_OnContentObject.gp'
        gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.video.OnNotification_OnContentObject.gp'
        #gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.gp'

        eps_output1 = 'eps/' + ue1 + '-' + log_file + '.video.OnNotification_OnContentObject.eps' 
        eps_output2 = 'eps/' + ue2 + '-' + log_file + '.video.OnNotification_OnContentObject.eps' 
        eps_output3 = 'eps/' + ue3 + '-' + log_file + '.video.OnNotification_OnContentObject.eps' 
        #eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.eps'

        gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
        epsOutputList=[eps_output1, eps_output2, eps_output3]

        #####################################
        #####  GET video onNotification and on ContentObject
        #####################################

        # get content generation time dict

        timestamps = []
        # get interest generation time for each selected interest
        for ue in test_ues(log_file):
            for entry in ue_video_generate_dict[ue]:
                prefix = entry[2]
                #print prefix
                prefix_split = prefix.strip('/').split('/')
                if int(prefix_split[4]) == interest_number:
                    timestamp = float(entry[0].strip('s'))
                    #print ue, " interest_number:", interest_number, " generation time: ", timestamp
                    timestamps.append(timestamp)


        #print ue_list
        
        #print files
        #print gpFileList
        #print epsOutputList
        #print "\n", timestamps, "\n"

        # get each interest notificaiton time for each selected ue
        i=0  # increase interest number
        for ue in test_ues(log_file):
            if i<3:
                #print i, files[i], timestamps[i]
                with open(files[i], 'w') as inf:
                    inf.write("# UE" + ue + " Interest:" + str(interests[i]) + " generate_time: " + str(timestamps[i]) + "\n")
                    inf.write("# UE#   " + " OnNotification_Time    " + " OnContentObject_Time " + "\n")
                    #inf.write("0 3 3\n")
                    #print "interest: ", interests[i]
                    for each_ue in ue_list:
                        if ue != each_ue:
                            print each_ue
                            try:
                                temp_OnNotification_list = ue_video_OnNotification_dict[each_ue]
                                #print "1", temp_OnNotification_list
                            except Exception as e:
                                temp_OnNotification_list =[]
                                OnNotification_time = ''
                                #print "2", OnNotification_time
                            if temp_OnNotification_list != []:
                                for item in temp_OnNotification_list:
                                    prefix = item[1]
                                    if prefix == interests[i]:
                                        OnNotification_time = float(item[0].strip('s'))
                                        #print each_ue, " OnNotification:", OnNotification_time

                            try:
                                temp_OnContentObject_list = ue_video_recv_dict[each_ue]
                            except Exception as e:
                                temp_OnContentObject_list = []
                                OnContentObject_time = ''
                            for item in temp_OnContentObject_list:
                                prefix = item[2]
                                if prefix == interests[i]:
                                    OnContentObject_time = float(item[0].strip('s'))
                                    #print each_ue, " OnContentObject:", OnContentObject_time , '\n'
                            each_ue = each_ue.strip('U')
                            #print "write to file:", files[i]           
                            inf.write(each_ue + "   " +  str(OnNotification_time) + "    " +  str(OnContentObject_time) + "\n")

            print ""
            i=i+1

        ################################
        ####### plot video generation, onnotification, oncontentobject
        ################################
        for i in range(3):
            with open(gpFileList[i], 'w') as inf:
                inf.write("set t postscript enhanced eps color\n")
                inf.write("set output \'" + epsOutputList[i] + "\'\n")
                inf.write("set ylabel 'TimeStamp (sec)'\n")
                inf.write("set xlabel 'UE'\n")
                inf.write("set xrange[1:" + ue_amount + "]\n")
                inf.write("set key below vertical right box 3\n")
                inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
                inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
                inf.write("f(x) = " + str(timestamps[i]) + "\n")
                plot_command = "plot f(x) lw 5 t \' Notification Generation Time\',  \'" + files[i]+ "\' u 1:2 w lp lw 5 t \' +OnNotification\', " + \
                " \'" + files[i]+ "\' u 1:3 w lp lw 5 t \'OnContentObject\'' "
                inf.write(plot_command)
                inf.write("\n")
            print "ploting...", gpFileList[i]
            subprocess.call("gnuplot " + gpFileList[i], shell=True)
            ++i

def get_rcv_notification_contentObject_per_interest_onDemand(log_file, interest_number=0):
    ''' For three selected UEs
        find OnNotification Time for interest_number 30
    '''
    """ex
    ['2.04081s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/video/0']
    ['2.04175s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/video/0']
    ['2.04364s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/audio/0']
    ['2.04458s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/audio/0']
    """
    fileLength = test_file_name_length(log_file)
    if(fileLength == 3):
        ue1, ue2, ue3 = test_ues(log_file)   # audio or video ues
    elif(fileLength == 6):
        ue1, ue2, ue3, ue4, ue5, ue6 = test_ues(log_file)   # audio1, aduio2, audio3, video1, video2, video3
    
    if(fileLength == 3):
        a1, a2, a3 = test_accounts(log_file)   # audio or video ues
    elif(fileLength == 6):
        a1, a2, a3, a4, a5, a6 = test_accounts(log_file)   # audio1, aduio2, audio3, video1, video2, video3

    ue_audio_generate_dict, ue_video_generate_dict = get_content_gen(log_file)

    # get notification dict
    ue_audio_OnNotification_dict, ue_video_OnNotification_dict = get_OnNotification_recv(log_file)
    # get OnContentObject dict
    ue_audio_recv_dict, ue_video_recv_dict = get_content_recv(log_file)

    # get ue list
    ue_list = get_ue_list(log_file)
    ##############################################################################################
    ###  part 1: Audio 
    ##############################################################################################
    if ue_audio_generate_dict.values() != [[],[],[]] and ue_audio_generate_dict.values() != [[]]:
        print "Audio:"
        interest1 = '/' + ue1 + '/' + 'chatroom-cona' + '/' + a1 + '/' + 'audio' + '/' + str(interest_number)
        interest2 = '/' + ue2 + '/' + 'chatroom-cona' + '/' + a2 + '/' + 'audio' + '/' + str(interest_number)
        interest3 = '/' + ue3 + '/' + 'chatroom-cona' + '/' + a3 + '/' + 'audio' + '/' + str(interest_number)
        interests = [interest1, interest2, interest3]

        file1 = 'csv/' + ue1 + '-' + log_file + '.auido.OnNotification_OnContentObject.onDemand.csv' 
        file2 = 'csv/' + ue2 + '-' + log_file + '.audio.OnNotification_OnContentObject.onDemand.csv'
        file3 = 'csv/' + ue3 + '-' + log_file + '.audio.OnNotification_OnContentObject.onDemand.csv'
        file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.audio.OnNotification_OnContentObject.onDemand.csv'
        files = [file1, file2, file3]

        gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.audio.OnNotification_OnContentObject.onDemand.gp'
        gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.audio.OnNotification_OnContentObject.onDemand.gp'
        gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.audio.OnNotification_OnContentObject.onDemand.gp'
        #gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.onDemand.gp'

        eps_output1 = 'eps/' + ue1 + '-' + log_file + '.audio.OnNotification_OnContentObject.onDemand.eps' 
        eps_output2 = 'eps/' + ue2 + '-' + log_file + '.audio.OnNotification_OnContentObject.onDemand.eps' 
        eps_output3 = 'eps/' + ue3 + '-' + log_file + '.audio.OnNotification_OnContentObject.onDemand.eps' 
        #eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.onDemand.eps'

        gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
        epsOutputList=[eps_output1, eps_output2, eps_output3]

        #####################################
        #####  GET audio onNotification and on ContentObject
        #####################################

        # get content generation time dict

        timestamps = []
        # get interest generation time for each selected interest
        for ue in test_ues(log_file):
            for entry in ue_audio_generate_dict[ue]:
                prefix = entry[2]
                #print prefix
                prefix_split = prefix.strip('/').split('/')
                if int(prefix_split[4]) == interest_number:
                    timestamp = float(entry[0].strip('s'))
                    #print ue, " interest_number:", interest_number, " generation time: ", timestamp
                    timestamps.append(timestamp)


        #print ue_list
        
        #print files
        #print gpFileList
        #print epsOutputList
        #print "\n", timestamps, "\n"

        # get each interest notificaiton time for each selected ue
        i=0  # increase interest number

        for ue in test_ues(log_file):
            if(i<3):
                #print i, files[i], timestamps[i]
                #print interests[i]
                #print "---"
                with open(files[i], 'w') as inf:
                    inf.write("# UE" + ue + " Interest:" + str(interests[i]) + " generate_time: " + str(timestamps[i]) + "\n")
                    inf.write("# UE#   " + " OnNotification_Time    " + " OnContentObject_Time " + "\n")
                    #print "interest: ", interests[i]
                    for each_ue in ue_list:
                        if ue != each_ue:
                            #print each_ue
                            try:
                                temp_OnNotification_list = ue_audio_OnNotification_dict[each_ue]
                                #print "1", temp_OnNotification_list
                            except Exception as e:
                                temp_OnNotification_list =[]
                                OnNotification_time = ''
                                #print "2", OnNotification_time
                            if temp_OnNotification_list != []:
                                for item in temp_OnNotification_list:
                                    prefix = item[1]
                                    #print prefix
                                    if prefix == interests[i]: 
                                        OnNotification_time = float(item[0].strip('s'))
                                        #print each_ue, " OnNotification:", OnNotification_time

                            try:
                                temp_OnContentObject_list = ue_audio_recv_dict[each_ue]
                            except Exception as e:
                                temp_OnContentObject_list = []
                                OnContentObject_time = ''
                            for item in temp_OnContentObject_list:
                                prefix = item[2]
                                if prefix == interests[i]:
                                    OnContentObject_time = float(item[0].strip('s'))
                                    #print each_ue, " OnContentObject:", OnContentObject_time , '\n'
                            each_ue = each_ue.strip('U')
                            #print "write to file:", files[i]           
                            inf.write(each_ue + "   " +  str(OnNotification_time) + "    " +  str(OnContentObject_time) + "\n")

            print ""
            i=i+1

        ################################
        ####### plot audio generation, onnotification, oncontentobject
        ################################
        for i in range(3):
            with open(gpFileList[i], 'w') as inf:
                inf.write("set t postscript enhanced eps color\n")
                inf.write("set output \'" + epsOutputList[i] + "\'\n")
                inf.write("set ylabel 'TimeStamp (sec)'\n")
                inf.write("set xlabel 'UE'\n")
                #inf.write("set title \'" + file1 + "\'\n")
                plot_command = "plot \'" + files[i]+ "\' u 1:2 w lp t \' +OnNotification\', " + \
                " \'" + files[i]+ "\' u 1:3 w lp t \'OnContentObject\'' "
                inf.write(plot_command)
                inf.write("\n")
            print "ploting...", gpFileList[i]
            subprocess.call("gnuplot " + gpFileList[i], shell=True)
            ++i

    ##############################################################################################
    ### Part 2: video 
    ##############################################################################################
    if ue_video_generate_dict.values() != [[],[],[]] and ue_video_generate_dict.values() != [[]]:
        print "Video:"
        if(fileLength == 6):
            ue1, ue2, ue3 = ue4, ue5, ue6
            a1, a2, a3 = a4, a5, a6
        interest1 = '/' + ue1 + '/' + 'chatroom-cona' + '/' + a1 + '/' + 'video' + '/' + str(interest_number)
        interest2 = '/' + ue2 + '/' + 'chatroom-cona' + '/' + a2 + '/' + 'video' + '/' + str(interest_number)
        interest3 = '/' + ue3 + '/' + 'chatroom-cona' + '/' + a3 + '/' + 'video' + '/' + str(interest_number)
        interests = [interest1, interest2, interest3]

        file1 = 'csv/' + ue1 + '-' + log_file + '.video.OnNotification_OnContentObject.onDemand.csv' 
        file2 = 'csv/' + ue2 + '-' + log_file + '.video.OnNotification_OnContentObject.onDemand.csv'
        file3 = 'csv/' + ue3 + '-' + log_file + '.video.OnNotification_OnContentObject.onDemand.csv'
        file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.video.OnNotification_OnContentObject.onDemand.csv'
        files = [file1, file2, file3]

        gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.video.OnNotification_OnContentObject.onDemand.gp'
        gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.video.OnNotification_OnContentObject.onDemand.gp'
        gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.video.OnNotification_OnContentObject.onDemand.gp'
        #gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.onDemand.gp'

        eps_output1 = 'eps/' + ue1 + '-' + log_file + '.video.OnNotification_OnContentObject.onDemand.eps' 
        eps_output2 = 'eps/' + ue2 + '-' + log_file + '.video.OnNotification_OnContentObject.onDemand.eps' 
        eps_output3 = 'eps/' + ue3 + '-' + log_file + '.video.OnNotification_OnContentObject.onDemand.eps' 
        #eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.onDemand.eps'

        gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
        epsOutputList=[eps_output1, eps_output2, eps_output3]

        #####################################
        #####  GET video onNotification and on ContentObject
        #####################################

        # get content generation time dict

        timestamps = []
        # get interest generation time for each selected interest
        for ue in test_ues(log_file):
            for entry in ue_video_generate_dict[ue]:
                prefix = entry[2]
                #print prefix
                prefix_split = prefix.strip('/').split('/')
                if int(prefix_split[4]) == interest_number:
                    timestamp = float(entry[0].strip('s'))
                    #print ue, " interest_number:", interest_number, " generation time: ", timestamp
                    timestamps.append(timestamp)


        #print ue_list
        
        #print files
        #print gpFileList
        #print epsOutputList
        #print "\n", timestamps, "\n"

        # get each interest notificaiton time for each selected ue
        i=0  # increase interest number
        for ue in test_ues(log_file):
            if i<3:
                #print i, files[i], timestamps[i]
                with open(files[i], 'w') as inf:
                    inf.write("# UE" + ue + " Interest:" + str(interests[i]) + " generate_time: " + str(timestamps[i]) + "\n")
                    inf.write("# UE#   " + " OnNotification_Time    " + " OnContentObject_Time " + "\n")
                    #inf.write(str(0) + " 3" + " 3\n")
                    #print "interest: ", interests[i]
                    for each_ue in ue_list:
                        if ue != each_ue:
                            #print each_ue
                            try:
                                temp_OnNotification_list = ue_video_OnNotification_dict[each_ue]
                                #print "1", temp_OnNotification_list
                            except Exception as e:
                                temp_OnNotification_list =[]
                                OnNotification_time = ''
                                #print "2", OnNotification_time
                            if temp_OnNotification_list != []:
                                for item in temp_OnNotification_list:
                                    prefix = item[1]
                                    if prefix == interests[i]:
                                        OnNotification_time = float(item[0].strip('s'))
                                        #print each_ue, " OnNotification:", OnNotification_time

                            try:
                                temp_OnContentObject_list = ue_video_recv_dict[each_ue]
                            except Exception as e:
                                temp_OnContentObject_list = []
                                OnContentObject_time = ''
                            for item in temp_OnContentObject_list:
                                prefix = item[2]
                                if prefix == interests[i]:
                                    OnContentObject_time = float(item[0].strip('s'))
                                    #print each_ue, " OnContentObject:", OnContentObject_time , '\n'
                            each_ue = each_ue.strip('U')
                            #print "write to file:", files[i]           
                            inf.write(each_ue + "   " +  str(OnNotification_time) + "    " +  str(OnContentObject_time) + "\n")

            print ""
            i=i+1

        ################################
        ####### plot video generation, onnotification, oncontentobject
        ################################
        for i in range(3):
            with open(gpFileList[i], 'w') as inf:
                inf.write("set t postscript enhanced eps color\n")
                inf.write("set output \'" + epsOutputList[i] + "\'\n")
                inf.write("set ylabel 'TimeStamp (sec)'\n")
                inf.write("set xlabel 'UE'\n")
                inf.write("set xrange[3:10]\n")
                #inf.write("set title \'" + file1 + "\'\n")
                plot_command = "plot \'" + files[i]+ "\' u 1:2 w lp t \' +OnNotification\', " + \
                " \'" + files[i]+ "\' u 1:3 w lp t \'OnContentObject\'' "
                inf.write(plot_command)
                inf.write("\n")
            print "ploting...", gpFileList[i]
            subprocess.call("gnuplot " + gpFileList[i], shell=True)
            ++i


def get_OnNotification_recv(log_file):
    """
    return OnNotification dicts for ech ue
    { 'U3': [['3.02802s', '/U5/chatroom-cona/A5/audio/24'],
            ['3.02804s', '/U8/chatroom-cona/A8/audio/22'],
            ['3.02806s', '/U14/chatroom-cona/A14/audio/22'],
            ['3.03201s', '/U12/chatroom-cona/A12/audio/22'], ...}
    """
    ue_audio_OnNotification_dict = {}
    ue_video_OnNotification_dict = {}
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "OnNotification(): " in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            prefix = temp[6]
            prefix_split = prefix.split("/")
            #print prefix_split
            interest = "/"
            if(len(prefix_split)==8):
                for i in range(3,8):
                    interest += prefix_split[i]
                    interest += "/"

                content_type = prefix_split[6]

            interest=interest.rstrip('/')

            if ue_account not in ue_audio_OnNotification_dict.keys():
                ue_audio_OnNotification_dict[ue_account] = []
            if ue_account not in ue_video_OnNotification_dict.keys():
                ue_video_OnNotification_dict[ue_account] = []

            if content_type == 'audio':
                if 'audio' in prefix:
                    audio_content_recv_list = ue_audio_OnNotification_dict[ue_account]
                    if audio_content_recv_list == []:
                        audio_content_recv_list = [[timestamp, interest]]
                    else:
                        audio_content_recv_list.insert(-1, [timestamp, interest])
                    ue_audio_OnNotification_dict[ue_account] = audio_content_recv_list

            if content_type == 'video':
                if 'video' in prefix:
                    video_content_recv_list = ue_video_OnNotification_dict[ue_account]
                    if video_content_recv_list == []:
                        video_content_recv_list = [[timestamp, interest]]
                    else:
                        video_content_recv_list.insert(-1, [timestamp, interest])
                    ue_video_OnNotification_dict[ue_account] = video_content_recv_list        
    fopen.close()
    return ue_audio_OnNotification_dict, ue_video_OnNotification_dict

def get_OnInterest_recv(log_file):
    """
    return OnNotification dicts for ech ue
    { 'U3': [['3.02802s', '/U5/chatroom-cona/A5/audio/24'],
            ['3.02804s', '/U8/chatroom-cona/A8/audio/22'],
            ['3.02806s', '/U14/chatroom-cona/A14/audio/22'],
            ['3.03201s', '/U12/chatroom-cona/A12/audio/22'], ...}
    """
    ue_audio_OnInterest_dict = {}
    ue_video_OnInterest_dict = {}
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "OnInterest(): " in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            prefix = temp[8]
            print prefix
            prefix_split = prefix.split("/")

            if(len(prefix_split)==6):
                content_type = prefix_split[4]

            print content_type

            if ue_account not in ue_audio_OnInterest_dict.keys():
                ue_audio_OnInterest_dict[ue_account] = []
            if ue_account not in ue_video_OnInterest_dict.keys():
                ue_video_OnInterest_dict[ue_account] = []

            if content_type == 'audio':
                if 'audio' in prefix:
                    audio_content_recv_list = ue_audio_OnInterest_dict[ue_account]
                    if audio_content_recv_list == []:
                        audio_content_recv_list = [[timestamp, prefix]]
                    else:
                        audio_content_recv_list.insert(-1, [timestamp, prefix])
                    ue_audio_Onprefix_dict[ue_account] = audio_content_recv_list

            if content_type == 'video':
                if 'video' in prefix:
                    video_content_recv_list = ue_video_Onprefix_dict[ue_account]
                    if video_content_recv_list == []:
                        video_content_recv_list = [[timestamp, prefix]]
                    else:
                        video_content_recv_list.insert(-1, [timestamp, prefix])
                    ue_video_OnInterest_dict[ue_account] = video_content_recv_list        
    fopen.close()
    return ue_audio_OnInterest_dict, ue_video_OnInterest_dict

def get_converge_from_receiver(log_file):
    '''get audio convergence 
        return audio_converge_dict
    '''
    ue_audio_generate_dict, ue_video_generate_dict = get_content_gen(log_file)
    ue_audio_recv_dict, ue_video_recv_dict = get_content_recv(log_file)

    for ue in ue_audio_recv_dict.keys():
        print "ue:",ue
        for each_audio_recv_record in ue_audio_recv_dict[ue]:
            prefix_recv = each_audio_recv_record[2]
            prefix_split = prefix_recv.strip('/').split('/')
            recv_time = float(each_audio_recv_record[0].strip('s'))
            prefix_source = prefix_split[0]
            package_number = prefix_split[4]
                
            for each_audio_gen_record in ue_audio_generate_dict[prefix_source]:
                prefix_gen = each_audio_gen_record[2]
                if prefix_gen == prefix_recv:
                    source_time = float(each_audio_gen_record[0].strip('s'))

            convergence_time = recv_time - source_time
            print prefix_source, package_number, prefix_recv, recv_time, source_time, convergence_time


    return ue_audio_generate_dict, ue_video_generate_dict, ue_audio_recv_dict, ue_video_recv_dict


####################
#  chat
#####################

def get_chat_content_gen(log_file):
    ''' Get the lines in the log regarding content generating 
        return chat_generate_dict
        ue_chat_generate_dict = {'U1':[[],[],[]], 'U2':[[],[],[]]}
    '''
    ue_chat_generate_dict ={}
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "generateContent(): " in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(":")
            content_type = temp[3]
            prefix = temp[10]
            #print prefix
            size = temp[12]

            if ue_account not in ue_chat_generate_dict.keys():
                ue_chat_generate_dict[ue_account] = []

            if content_type == 'Chat':
                chat_content_gen_list = ue_chat_generate_dict[ue_account]
                if chat_content_gen_list == []:
                    chat_content_gen_list = [[timestamp, content_type, prefix, size]]
                else:
                    chat_content_gen_list.insert(-1, [timestamp, content_type, prefix, size])
                ue_chat_generate_dict[ue_account] = chat_content_gen_list   


    fopen.close()
    return ue_chat_generate_dict

def get_chat_content_recv(log_file):
    '''Get the lines in the log regarding receving object 
        return two dicts: audio, video
    '''
    ue_chat_recv_dict = {}
   
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "OnContentObject(): " in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            content_type = temp[4]
            prefix = temp[8]
            if ue_account not in ue_chat_recv_dict.keys():
                ue_chat_recv_dict[ue_account] = []

            if content_type == 'Chat':
                if 'chat' in prefix:
                    chat_content_recv_list = ue_chat_recv_dict[ue_account]
                    if chat_content_recv_list == []:
                        chat_content_recv_list = [[timestamp, content_type, prefix]]
                    else:
                        chat_content_recv_list.insert(-1, [timestamp, content_type, prefix])
                    ue_chat_recv_dict[ue_account] = chat_content_recv_list

         
    fopen.close()
    return ue_chat_recv_dict

def chat_generation_gnuplot(log_file):
    """
    This will generate chat generation graph for selected UEs
    """
    ue_chat_generate_dict = get_chat_content_gen(log_file)
    ue1, ue2, ue3 = test_ues(log_file)
    
    ######################################################################
    #################   plotting chat generation time
    #####################################################################

    if ue_chat_generate_dict.values() !=[[],[],[]]:
        print "Chat:"        

        file1 = 'csv/' + ue1 + '-' + log_file + '.chat.generation.csv' 
        file2 = 'csv/' + ue2 + '-' + log_file + '.chat.generation.csv'
        file3 = 'csv/' + ue3 + '-' + log_file + '.chat.generation.csv'
        file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.chat.generation.csv'

        gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.chat.generation.gp'
        gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.chat.generation.gp'
        gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.chat.generation.gp'
        gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.chat.generation.gp'


        eps_output1 = 'eps/' + ue1 + '-' + log_file + '.chat.generation.eps' 
        eps_output2 = 'eps/' + ue2 + '-' + log_file + '.chat.generation.eps' 
        eps_output3 = 'eps/' + ue3 + '-' + log_file + '.chat.generation.eps' 
        eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.chat.generation.eps'

        list1 = ue_chat_generate_dict[ue1]
        list2 = ue_chat_generate_dict[ue2]
        list3 = ue_chat_generate_dict[ue3]
        
        output_dict1={}
        output_dict2={}
        output_dict3={}
        
        csvFileList=[file1, file2, file3]
        gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
        epsOutputList=[eps_output1, eps_output2, eps_output3]
        chatGenrateList=[list1, list2, list3]
        TempOutputDictList=[output_dict1, output_dict2, output_dict3]
        
        for i in range(3):
            for each_interest in chatGenrateList[i]:
                #print each_interest
                generate_time=float(each_interest[0].strip('s'))
                prefix_split=each_interest[2].strip('/').split('/')
                #print prefix_split
                interest_number=int(prefix_split[4])
                TempOutputDictList[i][interest_number]=generate_time

            with open(csvFileList[i], 'w') as inf:
                for key in sorted(TempOutputDictList[i]):
                    inf.write(str(key) + " " + str(TempOutputDictList[i][key]) + "\n")
                    #print "interest_number:" , key, " generate_time: ", TempOutputDictList[i][key]

            with open(gpFileList[i], 'w') as inf:
                inf.write("set t postscript enhanced eps color\n")
                inf.write("set output \'" + epsOutputList[i] + "\'\n")
                inf.write("set ylabel 'Generation Time (sec)'\n")
                inf.write("set xlabel 'Interests number'\n")
                inf.write("set key below vertical right box 3\n")
                inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
                inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
                #inf.write("set title \'" + file1 + "\'\n")
                plot_command = "plot \'" + csvFileList[i] + "\' u 1:2 w lp lw 3 t \'generation_time\'"
                inf.write(plot_command)
                inf.write("\n")   

            print "plotting...", gpFileList[i]
            subprocess.call("gnuplot " + gpFileList[i], shell=True)
        
        ######## write/plot  all three outputs to one file/picture
        with open(file4, 'w') as inf:
            for i in range(3):
                with open(csvFileList[i], 'r') as tem_inf:
                    line = tem_inf.read()
                    inf.write(line)
                inf.write('\n\n')

        with open(gnuplot_output4, 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + eps_output4 + "\'\n")
            inf.write("set ylabel 'Generation Time (sec)'\n")
            inf.write("set xlabel 'Interests number'\n")
            inf.write("set key below vertical right box 3\n")
            inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
            #inf.write("set title \'" + file1 + "\'\n")
            plot_command = "plot \'" + file4 + "\' index 0 u 1:2 w lp lw 3 t \'" + ue1 + "\', " + \
            " \'" + file4 + "\' index 1 u 1:2 w lp lw 3 t \'" + ue2 + "\', " + \
            " \'" + file4 + "\' index 2 u 1:2 w lp lw 3 t \'" + ue3 + "\'" 
            inf.write(plot_command)
            inf.write("\n") 

        print "plotting...", gnuplot_output4
        subprocess.call("gnuplot " + gnuplot_output4, shell=True)

def get_chat_converge_from_sender(log_file):
    """
     ue_chat_generate_dict:   { ,'U4': [['9.93875s', 'Chat', '/U4/chatroom-cona/A4/audio/255', '136'],[],[]]  }
     ue_chat_recv_dict     :  { ,'U3': [['9.99488s', 'Audio', '/U4/chatroom-cona/A4/audio/255'],, [],[]]  }
                            :  { ,'U2': [['9.99488s', 'Audio', '/U4/chatroom-cona/A4/audio/255'],
    """
    ue_chat_generate_dict = get_chat_content_gen(log_file)
    ue_chat_recv_dict = get_chat_content_recv(log_file)
    ue_chat_converge_dict = {}  
    chat_lost_package_dict = {}  
    audio_lost_dict = {} 
    chat_package_converge_list = []
 
    ###########################################################################
    # collect chat converge info
    ###########################################################################
    for ue in ue_chat_generate_dict.keys():
        if ue not in ue_chat_converge_dict.keys():
            ue_chat_converge_dict[ue] = []
        if ue not in chat_lost_package_dict.keys():
            chat_lost_package_dict[ue] = []
        #print "UE: ", ue
        for each_gen in ue_chat_generate_dict[ue]:
            audio_lost_package_list = []
            recv_time_list = []
            send_time = float(each_gen[0].strip('s'))
            send_prefix = each_gen[2]
            prefix_split = send_prefix.strip('/').split('/')
            send_package_num = int(prefix_split[4])
            #convergence_time=0
            #print "package_number:", send_package_num

            for ue_recv in ue_chat_recv_dict.keys():
                if ue != ue_recv:
                    # go through all the receieved interests

                    for each_recv in ue_chat_recv_dict[ue_recv]:
                        found = False
                        prefix_recv = each_recv[2]
                        if prefix_recv == send_prefix:
                            found = True
                            break 
                    # check if interests have been received                         
                    if found == True:
                        recv_time = float(each_recv[0].strip('s'))
                        package_convergence_time_for_ue_recv = float("%0.5f" % (recv_time - send_time))
                        #print " Package_number:", send_package_num ," received by: ", ue_recv," recv_time:", recv_time, " send_time:", send_time,\
                        #" converge time:", package_convergence_time_for_ue_recv
                        temp_list = [send_package_num, ue_recv, package_convergence_time_for_ue_recv]
                        #recv_time_list.append(recv_time)
                        if recv_time_list == []:
                            recv_time_list = [temp_list]
                        else:
                            recv_time_list.insert(-1, temp_list)
                    elif found == False:
                        package_convergence_time_for_ue_recv = float("%0.5f" % 0.000)
                        temp_list = [send_package_num, ue_recv, package_convergence_time_for_ue_recv]
                        if recv_time_list == []:
                            recv_time_list = [temp_list]
                        else:
                            recv_time_list.insert(-1, temp_list)
                        #print "     audio source package_number:", send_package_num
                        #print "     ", ue_recv, " lost packet number:", send_package_num, " from Audio source:", ue
                        previous_lost = chat_lost_package_dict[ue]
                        if previous_lost == []:
                            previous_lost = [{send_package_num:ue_recv}]
                        else:
                            previous_lost.insert(-1, {send_package_num:ue_recv})
                        chat_lost_package_dict[ue] = previous_lost


            chat_package_converge_list = ue_chat_converge_dict[ue]
            if chat_package_converge_list == []:
                chat_package_converge_list = [recv_time_list]
            else:
                chat_package_converge_list.insert(-1, recv_time_list)

            ue_chat_converge_dict[ue] = chat_package_converge_list

    
    return ue_chat_converge_dict

def get_chat_OnNotification_recv(log_file):
    """
    return OnNotification dicts for ech ue
    { 'U3': [['3.02802s', '/U5/chatroom-cona/A5/chat/24'],
            ['3.02804s', '/U8/chatroom-cona/A8/chat/22'],
            ['3.02806s', '/U14/chatroom-cona/A14/chat/22'],
            ['3.03201s', '/U12/chatroom-cona/A12/chat/22'], ...}
    """
    ue_chat_OnNotification_dict = {}
    
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "OnNotification(): " in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            prefix = temp[6]
            prefix_split = prefix.split("/")
            #print prefix_split
            interest = "/"
            if(len(prefix_split)==8):
                for i in range(3,8):
                    interest += prefix_split[i]
                    interest += "/"

                content_type = prefix_split[6]

            interest=interest.rstrip('/')

            if ue_account not in ue_chat_OnNotification_dict.keys():
                ue_chat_OnNotification_dict[ue_account] = []

            if content_type == 'chat':
                if 'chat' in prefix:
                    chat_content_recv_list = ue_chat_OnNotification_dict[ue_account]
                    if chat_content_recv_list == []:
                        chat_content_recv_list = [[timestamp, interest]]
                    else:
                        chat_content_recv_list.insert(-1, [timestamp, interest])
                    ue_chat_OnNotification_dict[ue_account] = chat_content_recv_list
   
    fopen.close()
    return ue_chat_OnNotification_dict

def get_chat_rcv_notification_contentObject_per_interest(log_file, interest_number=0):
    ''' For three selected UEs
        find OnNotification Time for interest_number 30
    '''
    """ex
    ['2.04081s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/chat/0']
   
    """
    ue_amount = get_config()['NumberOfUEs']

    fileLength = test_file_name_length(log_file)
    if(fileLength == 3):
        ue1, ue2, ue3 = test_ues(log_file)   # audio or video ues
    elif(fileLength == 6):
        ue1, ue2, ue3, ue4, ue5, ue6 = test_ues(log_file)   # audio1, aduio2, audio3, video1, video2, video3
    
    if(fileLength == 3):
        a1, a2, a3 = test_accounts(log_file)   # audio or video ues
    elif(fileLength == 6):
        a1, a2, a3, a4, a5, a6 = test_accounts(log_file)   # audio1, aduio2, audio3, video1, video2, video3

    ue_chat_generate_dict = get_chat_content_gen(log_file)

    # get notification dict
    ue_chat_OnNotification_dict = get_chat_OnNotification_recv(log_file)
    # get OnContentObject dict
    ue_chat_recv_dict = get_chat_content_recv(log_file)

    # get ue list
    ue_list = get_ue_list(log_file)
    ##############################################################################################
    ###  part 1: Chat 
    ##############################################################################################

    print "Chat:"
    interest1 = '/' + ue1 + '/' + 'chatroom-cona' + '/' + a1 + '/' + 'chat' + '/' + str(interest_number)
    interest2 = '/' + ue2 + '/' + 'chatroom-cona' + '/' + a2 + '/' + 'chat' + '/' + str(interest_number)
    interest3 = '/' + ue3 + '/' + 'chatroom-cona' + '/' + a3 + '/' + 'chat' + '/' + str(interest_number)
    interests = [interest1, interest2, interest3]

    file1 = 'csv/' + ue1 + '-' + log_file + '.chat.OnNotification_OnContentObject.csv' 
    file2 = 'csv/' + ue2 + '-' + log_file + '.chat.OnNotification_OnContentObject.csv'
    file3 = 'csv/' + ue3 + '-' + log_file + '.chat.OnNotification_OnContentObject.csv'
    file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.chat.OnNotification_OnContentObject.csv'
    files = [file1, file2, file3]

    gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.chat.OnNotification_OnContentObject.gp'
    gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.chat.OnNotification_OnContentObject.gp'
    gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.chat.OnNotification_OnContentObject.gp'
    #gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.gp'

    eps_output1 = 'eps/' + ue1 + '-' + log_file + '.chat.OnNotification_OnContentObject.eps' 
    eps_output2 = 'eps/' + ue2 + '-' + log_file + '.chat.OnNotification_OnContentObject.eps' 
    eps_output3 = 'eps/' + ue3 + '-' + log_file + '.chat.OnNotification_OnContentObject.eps' 
    #eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.eps'

    gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
    epsOutputList=[eps_output1, eps_output2, eps_output3]

    #####################################
    #####  GET chat onNotification and on ContentObject
    #####################################

    # get content generation time dict

    timestamps = []
    # get interest generation time for each selected interest
    for ue in test_ues(log_file):
        for entry in ue_chat_generate_dict[ue]:
            prefix = entry[2]
            #print prefix
            prefix_split = prefix.strip('/').split('/')
            if int(prefix_split[4]) == interest_number:
                timestamp = float(entry[0].strip('s'))
                #print ue, " interest_number:", interest_number, " generation time: ", timestamp
                timestamps.append(timestamp)


    #print ue_list
    
    #print files
    #print gpFileList
    #print epsOutputList
    #print "\n", timestamps, "\n"

    # get each interest notificaiton time for each selected ue
    i=0  # increase interest number

    for ue in test_ues(log_file):
        if(i<3):
            #print i, files[i], timestamps[i]
            print interests[i]
            #print "---"
            with open(files[i], 'w') as inf:
                inf.write("# UE" + ue + " Interest:" + str(interests[i]) + " generate_time: " + str(timestamps[i]) + "\n")
                inf.write("# UE#   " + " OnNotification_Time    " + " OnContentObject_Time " + "\n")
                #inf.write("0 3 3\n")
                #print "interest: ", interests[i]
                for each_ue in ue_list:
                    if ue != each_ue:
                        print each_ue
                        try:
                            temp_OnNotification_list = ue_chat_OnNotification_dict[each_ue]
                            #print "1", temp_OnNotification_list
                        except Exception as e:
                            temp_OnNotification_list =[]
                            OnNotification_time = ''
                            #print "2", OnNotification_time
                        if temp_OnNotification_list != []:
                            for item in temp_OnNotification_list:
                                prefix = item[1]
                                #print prefix
                                if prefix == interests[i]: 
                                    OnNotification_time = float(item[0].strip('s'))
                                    print each_ue, " OnNotification:", OnNotification_time

                        try:
                            temp_OnContentObject_list = ue_chat_recv_dict[each_ue]
                        except Exception as e:
                            temp_OnContentObject_list = []
                            OnContentObject_time = ''
                        for item in temp_OnContentObject_list:
                            prefix = item[2]
                            if prefix == interests[i]:
                                OnContentObject_time = float(item[0].strip('s'))
                                #print each_ue, " OnContentObject:", OnContentObject_time , '\n'
                        each_ue = each_ue.strip('U')
                        #print "write to file:", files[i]           
                        inf.write(each_ue + "   " +  str(OnNotification_time) + "    " +  str(OnContentObject_time) + "\n")

        print ""
        i=i+1

    ################################
    ####### plot chat generation, onnotification, oncontentobject
    ################################
    for i in range(3):
        with open(gpFileList[i], 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + epsOutputList[i] + "\'\n")
            inf.write("set ylabel 'TimeStamp (sec)'\n")
            inf.write("set xlabel 'UE'\n")
            inf.write("set key below vertical right box 3\n")
            inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("set xrange[1:" + ue_amount + "]\n")
            inf.write("f(x) = " + str(timestamps[i]) + "\n")
            #inf.write("set title \'" + file1 + "\'\n")
            plot_command = "plot f(x) lw 5 t \' Notification Generation Time\',  \'" + files[i]+ "\' u 1:2 w lp lw 3 t \' +OnNotification\', " + \
            " \'" + files[i]+ "\' u 1:3 w lp lw 3 t \'OnContentObject\'' "
            inf.write(plot_command)
            inf.write("\n")
        print "ploting...", gpFileList[i]
        subprocess.call("gnuplot " + gpFileList[i], shell=True)
        ++i

def get_chat_converge_from_receiver(log_file):
    '''get audio convergence 
        return audio_converge_dict
    '''
    ue_chat_generate_dict = get_chat_content_gen(log_file)
    ue_chat_recv_dict = get_chat_content_recv(log_file)

    for ue in ue_chat_recv_dict.keys():
        print "ue:",ue
        for each_audio_recv_record in ue_chat_recv_dict[ue]:
            prefix_recv = each_audio_recv_record[2]
            prefix_split = prefix_recv.strip('/').split('/')
            recv_time = float(each_audio_recv_record[0].strip('s'))
            prefix_source = prefix_split[0]
            package_number = prefix_split[4]
                
            for each_chat_gen_record in ue_chat_generate_dict[prefix_source]:
                prefix_gen = each_chat_gen_record[2]
                if prefix_gen == prefix_recv:
                    source_time = float(each_chat_gen_record[0].strip('s'))

            convergence_time = recv_time - source_time
            print prefix_source, package_number, prefix_recv, recv_time, source_time, convergence_time


    #return ue_chat_generate_dict, ue_video_generate_dict, ue_chat_recv_dict, ue_video_recv_dict

def chat_per_interest_converge_gnulpot(log_file):
    """
    for selected UEs, plot all interests Average converge time for selected UEs
    """
    fileLength = test_file_name_length(log_file)
    if(fileLength == 3):
        ue1, ue2, ue3 = test_ues(log_file)   # audio or video ues
    elif(fileLength == 6):
        ue1, ue2, ue3, ue4, ue5, ue6 = test_ues(log_file)   # audio1, aduio2, audio3, video1, video2, video3
    
    if(fileLength == 3):
        a1, a2, a3 = test_accounts(log_file)   # audio or video ues
    elif(fileLength == 6):
        a1, a2, a3, a4, a5, a6 = test_accounts(log_file)   # audio1, aduio2, audio3, video1, video2, video3


    ue_chat_converge_dict = get_chat_converge_from_sender(log_file)
    
    ######################################################################
    #################   plotting chat converge time
    ######################################################################
    print "Chat:"

    file1 = 'csv/' + ue1 + '-' + log_file + 'chat_converge.csv' 
    file2 = 'csv/' + ue2 + '-' + log_file + 'chat_converge.csv'
    file3 = 'csv/' + ue3 + '-' + log_file + 'chat_converge.csv'
    file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.chat_converge.csv'

    gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.chat_converge.gp'
    gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.chat_converge.gp'
    gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.chat_converge.gp'
    gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.chat_converge.gp'

    eps_output1 = 'eps/' + ue1 + '-' + log_file + '.chat_converge.eps' 
    eps_output2 = 'eps/' + ue2 + '-' + log_file + '.chat_converge.eps' 
    eps_output3 = 'eps/' + ue3 + '-' + log_file + '.chat_converge.eps' 
    eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.chat_converge.eps'

    list1 = ue_chat_converge_dict[ue1]
    list2 = ue_chat_converge_dict[ue2]
    list3 = ue_chat_converge_dict[ue3]

    output_dict1={}
    output_dict2={}
    output_dict3={}

    csvFileList=[file1, file2, file3]
    gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
    epsOutputList=[eps_output1, eps_output2, eps_output3]
    chatConvergeList=[list1, list2, list3]
    TempOutputDictList=[output_dict1, output_dict2, output_dict3]

    # find max converge time
    for i in range(3):
        for packet_list in chatConvergeList[i]:
            converge_max=0.0
            for packet in packet_list:
                packet_number = packet[0]
                if(float(packet[2]) >= converge_max):
                    converge_max = float(packet[2])
            TempOutputDictList[i][packet_number] = converge_max
       
    # for i in range(3):
    #     for packet_list in chatConvergeList[i]:          
    #         for packet in packet_list:
    #             packet_number = packet[0]
    #             print float(packet[2])
    #             TempOutputDictList[i][packet_number] = float(packet[2])

    #         with open(csvFileList[i], 'w') as inf:
    #             for key in sorted(TempOutputDictList[i]):
    #                 if(TempOutputDictList[i][key] !=0):
    #                     inf.write(str(key) + " " + str(TempOutputDictList[i][key]) + "\n")
    #                     #print "packet_number:", str(key), " converge_max:", str(TempOutputDictList[i][key])
               

        with open(gpFileList[i], 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + epsOutputList[i] + "\'\n")
            inf.write("set ylabel 'converge average (sec)'\n")
            inf.write("set xlabel 'interests number'\n")
            inf.write("set key bottom vertical right box 3\n")
            inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
            #inf.write("set title \'" + csvFileList[i] + "\'\n")
            plot_command = "plot \'" + csvFileList[i] + "\' u 1:2 w lp lw 3 t \'converge time\'"
            inf.write(plot_command)
            inf.write("\n")

        print "plotting...", gpFileList[i]
        subprocess.call("gnuplot " + gpFileList[i], shell=True)

    ##### write/plot into one file/picture
    with open(file4, 'w') as inf:
        for i in range(3):
            with open(csvFileList[i], 'r') as tem_inf:
                line = tem_inf.read()
                inf.write(line)
            inf.write('\n\n')

    with open(gnuplot_output4, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps_output4 + "\'\n")
        inf.write("set ylabel 'Generation Time (sec)'\n")
        inf.write("set xlabel 'Interests number'\n")
        inf.write("set key bottom vertical right box 3\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
        #inf.write("set title \'" + file1 + "\'\n")
        plot_command = "plot \'" + file4 + "\' index 0 u 1:2 w lp lw 3 t \'" + ue1 + "\', " + \
        " \'" + file4 + "\' index 1 u 1:2 w lp lw 3 t \'" + ue2 + "\', " + \
        " \'" + file4 + "\' index 2 u 1:2 w lp lw 3 t \'" + ue3 + "\'" 
        inf.write(plot_command)
        inf.write("\n") 

    print "plotting...", gnuplot_output4,
    subprocess.call("gnuplot " + gnuplot_output4, shell=True)    

def get_chat_converge_per_interest(log_file, interest_number=2):
    """
    for given UEs and interest_number
    plot interest convergence time
    """
    fileLength = test_file_name_length(log_file)
    if(fileLength == 3):
        ue1, ue2, ue3 = test_ues(log_file)   # audio or video ues
    elif(fileLength == 6):
        ue1, ue2, ue3, ue4, ue5, ue6 = test_ues(log_file)   # audio1, aduio2, audio3, video1, video2, video3
    
    if(fileLength == 3):
        a1, a2, a3 = test_accounts(log_file)   # audio or video ues
    elif(fileLength == 6):
        a1, a2, a3, a4, a5, a6 = test_accounts(log_file)   # audio1, aduio2, audio3, video1, video2, video3

    ue_chat_converge_dict = get_chat_converge_from_sender(log_file)  

    interest1 = '/' + ue1 + '/' + 'chatroom-cona' + '/' + a1 + '/' + 'chat' + '/' + str(interest_number)
    interest2 = '/' + ue2 + '/' + 'chatroom-cona' + '/' + a2 + '/' + 'chat' + '/' + str(interest_number)
    interest3 = '/' + ue3 + '/' + 'chatroom-cona' + '/' + a3 + '/' + 'chat' + '/' + str(interest_number)
    interests = [interest1, interest2, interest3]

    file1 = 'csv/' + ue1 + '-' + str(interest_number) + '-' + log_file + '.chat.PerInterestConverge.csv' 
    file2 = 'csv/' + ue2 + '-' + str(interest_number) + '-' + log_file + '.chat.PerInterestConverge.csv'
    file3 = 'csv/' + ue3 + '-' + str(interest_number) + '-' + log_file + '.chat.PerInterestConverge.csv'
    file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + str(interest_number) + '-' + log_file + '.chat.PerInterestConverge.csv'
    files = [file1, file2, file3]

    gnuplot_output1 = 'gp/' + ue1 + '-' + str(interest_number) + '-' + log_file + '.chat.PerInterestConverge.gp'
    gnuplot_output2 = 'gp/' + ue2 + '-' + str(interest_number) + '-' + log_file + '.chat.PerInterestConverge.gp'
    gnuplot_output3 = 'gp/' + ue3 + '-' + str(interest_number) + '-' + log_file + '.chat.PerInterestConverge.gp'
    gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + str(interest_number) + '-' + log_file + '.chat.PerInterestConverge.gp'

    eps_output1 = 'eps/' + ue1 + '-' + str(interest_number) + '-' + log_file + '.chat.PerInterestConverge.eps' 
    eps_output2 = 'eps/' + ue2 + '-' + str(interest_number) + '-' + log_file + '.chat.PerInterestConverge.eps' 
    eps_output3 = 'eps/' + ue3 + '-' + str(interest_number) + '-' + log_file + '.chat.PerInterestConverge.eps' 
    eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + str(interest_number) + '-' + log_file + '.chat.PerInterestConverge.eps'

    gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
    epsOutputList=[eps_output1, eps_output2, eps_output3]

    ##############################################################################################
    #####  GET chat interest converge time
    ##########################################################################################
    
    ue_list = get_ue_list(log_file)

    i=0  # increase interest number
    for ue in test_ues(log_file):
        if i<3:
            print ue
            with open(files[i], 'w') as inf:
                inf.write("# UE" + ue + " Interest:" + str(interests[i]) + "\n")
                inf.write("# UE#   " + " PerInterestConvergeTimentObject_Time " + "\n")
                for each_ue in ue_list:
                    for entry in ue_chat_converge_dict[ue][interest_number]:
                        recv_ue = entry[1]
                        if each_ue == recv_ue:
                            convergence_time = entry[2]
                            print recv_ue.strip('U'), " ", convergence_time
                            inf.write(recv_ue.strip('U') + " " + str(convergence_time) + "\n")
            
        print ""    
        i=i+1

    # put all 3 csv files together and plot once
    with open(file4, 'w') as inf:
        for i in range(3):
            with open(files[i], 'r') as tem_inf:
                line = tem_inf.read()
                inf.write(line)
                inf.write("\n")
            inf.write("\n\n")
    ################################
    ####### plot chat per interest converge
    ################################
 
    with open(gnuplot_output4, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps_output4 + "\'\n")
        inf.write("set ylabel 'TimeStamp (sec)'\n")
        inf.write("set xlabel 'UE-ID'\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set key bottom vertical right box 3\n")
        #inf.write("set title \'" + file1 + "\'\n")
        plot_command = "plot \'" + file4 + "\' index 0 u 1:2 w lp t \'" + ue1 + "\', " + \
        " \'" + file4 + "\' index 1 u 1:2 w lp t \'" + ue2 + "\', " + \
        " \'" + file4 + "\' index 2 u 1:2 w lp t \'" + ue3 + "\'" 
        inf.write(plot_command)
        inf.write("\n") 

    print "plotting...", gnuplot_output4
    subprocess.call("gnuplot " + gnuplot_output4, shell=True)  

   
################################################
#  chat recovery ,U1 local link failure
#################################################
def get_ue_amount_from_chat_linkfailure(log_file):
    """
    for U1 local link failure, get two UEs, one from the same proxy as ue, one from different proxy
    get UEs on OnContentObject time, plot with U1's package generation time
    """
    ue_amount = (log_file.split('_')[3]).strip('UE')
    print ue_amount
    return ue_amount

def get_chat_generation_for_linkfailure(log_file):
    ue_chat_generate_dict = get_chat_content_gen(log_file)
    ue1 = 'U1'
    
    ######################################################################
    #################   plotting chat generation time
    #####################################################################
    
    file1 = 'csv/' + ue1 + '-' + log_file + '.chat.generation.csv' 
    gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.chat.generation.gp'
    eps_output1 = 'eps/' + ue1 + '-' + log_file + '.chat.generation.eps' 
    list1 = ue_chat_generate_dict[ue1]
    output_dict1={}

    for each_interest in list1:
        #print each_interest
        generate_time=float(each_interest[0].strip('s'))
        prefix_split=each_interest[2].strip('/').split('/')
        #print prefix_split
        interest_number=int(prefix_split[4])
        output_dict1[interest_number]=generate_time

    with open(file1, 'w') as inf:
        for key in sorted(output_dict1):
            inf.write(str(key) + " " + str(output_dict1[key]) + "\n")
            #print "interest_number:" , key, " generate_time: ", TempOutputDictList[i][key]

    with open(gnuplot_output1, 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + eps_output1 + "\'\n")
            inf.write("set ylabel 'Generation Time (sec)'\n")
            inf.write("set xlabel 'Interests number'\n")
            inf.write("set key below vertical right box 3\n")
            inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
            #inf.write("set title \'" + file1 + "\'\n")
            plot_command = "plot \'" + file1 + "\' u 1:2 w p pt 7 ps 1  t \'generation_time\'"
            inf.write(plot_command)
            inf.write("\n")   

    print "plotting...", gnuplot_output1
    subprocess.call("gnuplot " + gnuplot_output1, shell=True)
    
    ######## write/plot  all three outputs to one file/picture

def get_chat_recovery_OnContentObject_for_prefixU1(log_file):
    """
    only for U1 local link failure
    target prefix: < /U1/chatroom-cona/A1/chat/ >
    """

    ue_chat_recv_dict = {}
   
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        #print line
        if "OnContentObject():" in line and "/U1/chatroom-cona/A1/chat" in line and "m_lastReceivedContentSeq" in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            content_type = temp[4]
            prefix = temp[8]
            if ue_account not in ue_chat_recv_dict.keys():
                ue_chat_recv_dict[ue_account] = []

            if content_type == 'Chat':
                if 'chat' in prefix:
                    chat_content_recv_list = ue_chat_recv_dict[ue_account]
                    if chat_content_recv_list == []:
                        chat_content_recv_list = [[timestamp, content_type, prefix]]
                    else:
                        chat_content_recv_list.insert(-1, [timestamp, content_type, prefix])
                    ue_chat_recv_dict[ue_account] = chat_content_recv_list
         
    fopen.close()
    return ue_chat_recv_dict

def plot_chat_generation_content_for_linkfailure(log_file):
    ue_chat_recv_dict = get_chat_recovery_OnContentObject_for_prefixU1(log_file)
    ue_chat_generate_dict = get_chat_content_gen(log_file)

    file1 = 'csv/' + '-' + log_file + '.chat.recovery_U1.csv' 
    gnuplot_output1 = 'gp/' + '-' + log_file + '.chat.recovery_U1.gp'
    eps_output1 = 'eps/' + '-' + log_file + '.chat.recovery_U1.eps' 

    ue_amount = int(get_ue_amount_from_chat_linkfailure(log_file))
    ue1 = 'U' + str( ue_amount/3 )
    ue2 = 'U' + str( ue_amount/3*2 )
    target_ue =  'U1'

    ue1_chat_recv_list = ue_chat_recv_dict[ue1]
    ue2_chat_recv_list = ue_chat_recv_dict[ue2]

    
    with open(file1, 'w') as inf:
        i=1
        inf.write("#interest# generationTime  " + ue1 + "_OnConentTime " + ue2 + "_OnContentTime" + '\n')
        for each_gen in ue_chat_generate_dict[target_ue]:
            print each_gen[0].strip('s'), " ***** "
            inf.write(str(i) + " " + each_gen[0].strip('s') + " ")
            for each_ue1_recv in ue1_chat_recv_list:
                if each_gen[2] == each_ue1_recv[2]:
                    print each_gen[0].strip('s'), " -> ", each_ue1_recv[0].strip('s')
                    inf.write(each_ue1_recv[0].strip('s') + " ")

            for each_ue2_recv in ue2_chat_recv_list:
                if each_gen[2] == each_ue2_recv[2]:
                    print each_gen[0].strip('s'), " -> ", each_ue2_recv[0].strip('s')
                    inf.write(each_ue2_recv[0].strip('s') + "\n")

            i=i+1

    with open(gnuplot_output1, 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + eps_output1 + "\'\n")


            inf.write("set multiplot\n")

            inf.write("set arrow from 7,5 to 7,7 ls 8\n")
            inf.write("set arrow from 8,7.5 to 15,7.5 heads ls 8\n")
            inf.write("set arrow from 8.5,7.1 to 13,5 heads ls 7\n")

            inf.write("set label 'Link Failure Period' at 5.5,6 tc rgb 'red' font ', 18' front\n")
            inf.write("set label 'Message Recovery period' at 9.5,7.7 tc rgb 'red' font ', 18' front\n")

            inf.write("set ylabel 'TimeStamp (sec)'\n")
            inf.write("set xlabel 'UE1 Finger Print Number'\n")
            inf.write("set key below vertical right box 3\n")
            inf.write("set grid ytics lc rgb '#bbbbbb' lw 5 lt 0\n")
            inf.write("set grid xtics lc rgb '#bbbbbb' lw 5 lt 0\n")
            inf.write("set xrange[5:20]\n")
            inf.write("set yrange[3:8]\n")
            inf.write("set xtics 1\n")
            inf.write("set ytics 0.5\n")
            #inf.write("set title \'" + file1 + "\'\n")
            plot_command = "plot  \'" + file1+ "\' u 1:2 w p lc 1 t \'U1 Interest Generation Time\', " + \
            " \'" + file1+ "\' u 1:3 w p lc 3 t \'U5 OnContentObject Time\', " + \
            " \'" + file1+ "\' u 1:4 w p lc 8 t \'U10 OnContentObject Time\' "
            inf.write(plot_command)
            inf.write("\n")

            inf.write("unset arrow\n")
            inf.write("unset label\n")

            inf.write("set lmargin 5\n")
            inf.write("unset key\n")
            inf.write("unset xlabel\n")
            inf.write("unset ylabel\n")
            inf.write("set title 'Zoom'\n")

            inf.write("set xrange[7.5:9.5]\n")
            inf.write("set yrange[7:7.3]\n")
            inf.write("set size 0.38,0.38\n")
            inf.write("set origin 0.5,0.2\n")
            inf.write("set xtics 1\n")
            inf.write("set ytics 0.05\n")
            plot_command2 = "plot  \'" + file1+ "\' u 1:3 w p lc 3 t \'U5 OnContentObject Time\', " + \
              " \'" + file1+ "\' using 1:3:3 with labels offset 0,char 1, " + \
            " \'" + file1+ "\' u 1:4 w p lc 8 t \'U10 OnContentObject Time\', " + \
            " \'" + file1+ "\' using 1:4:4 with labels offset 0,char 1 "
            inf.write(plot_command2 + "\n")

            inf.write("unset multiplot\n")
    print "ploting...", gnuplot_output1
    subprocess.call("gnuplot " + gnuplot_output1, shell=True)


####################
#  pipeling audio
#####################

def get_audio_pipe_gen(log_file):
    ''' get audio pipeling generation
        only for on-1
    '''
    ue_audio_generate_dict ={}
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "SyncAudioApp:generateContent(): U1: Audio Content sequence number " in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(":")
            content_type = temp[3]
            prefix = temp[10]
            #print prefix
            size = temp[12]

            if ue_account not in ue_audio_generate_dict.keys():
                ue_audio_generate_dict[ue_account] = []

            if content_type == 'Audio':
                chat_content_gen_list = ue_audio_generate_dict[ue_account]
                if chat_content_gen_list == []:
                    chat_content_gen_list = [[timestamp, content_type, prefix, size]]
                else:
                    chat_content_gen_list.insert(-1, [timestamp, content_type, prefix, size])
                ue_audio_generate_dict[ue_account] = chat_content_gen_list   


    fopen.close()
    return ue_audio_generate_dict

def get_audio_pipe_content_recv(log_file):
    '''Get the lines in the log regarding receving object 
        return audio recv dict
    '''
    ue_audio_recv_dict = {}
   
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "OnContentObject(): " in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            content_type = temp[4]
            prefix = temp[8]
            if ue_account not in ue_audio_recv_dict.keys():
                ue_audio_recv_dict[ue_account] = []

            if content_type == 'Audio':
                if 'audio' in prefix:
                    chat_content_recv_list = ue_audio_recv_dict[ue_account]
                    if chat_content_recv_list == []:
                        chat_content_recv_list = [[timestamp, content_type, prefix]]
                    else:
                        chat_content_recv_list.insert(-1, [timestamp, content_type, prefix])
                    ue_audio_recv_dict[ue_account] = chat_content_recv_list

         
    fopen.close()
    return ue_audio_recv_dict
def get_audio_pipe_OnNotification_recv(log_file):
    """
    only for receiveing ON/OFF
    return OnNotification dicts for ech ue
   
    """
    ue_audio_OnNotification_dict = {}
    
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "OnNotification(): " in line and "Received Notification" in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            prefix = temp[6]
            prefix_split = prefix.split("/")
            #print prefix_split
            interest = "/"
            if(len(prefix_split)==8):
                for i in range(3,8):
                    interest += prefix_split[i]
                    interest += "/"

                content_type = prefix_split[6]

            interest=interest.rstrip('/')

            if ue_account not in ue_audio_OnNotification_dict.keys():
                ue_audio_OnNotification_dict[ue_account] = []

            if content_type == 'audio':
                if 'audio' in prefix:
                    chat_content_recv_list = ue_audio_OnNotification_dict[ue_account]
                    if chat_content_recv_list == []:
                        chat_content_recv_list = [[timestamp, interest]]
                    else:
                        chat_content_recv_list.insert(-1, [timestamp, interest])
                    ue_audio_OnNotification_dict[ue_account] = chat_content_recv_list
   
    fopen.close()
    return ue_audio_OnNotification_dict
def get_audio_pipe_ScheduleNextInterest(log_file):
    """
    return ScheduleNextInterest dicts for ech ue
   
    """
    ue_audio_scheduleNetxtInterest_dict = {}
    
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "ScheduleNextInterest(): " in line and "ScheduleNextInterest:" in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            prefix = temp[3].strip("ScheduleNextInterest:")
            prefix_split = prefix.split("/")
            #print prefix_split
            content_type = prefix_split[4]

            if ue_account not in ue_audio_scheduleNetxtInterest_dict.keys():
                ue_audio_scheduleNetxtInterest_dict[ue_account] = []

            if content_type == 'audio':
                if 'audio' in prefix:
                    chat_content_recv_list = ue_audio_scheduleNetxtInterest_dict[ue_account]
                    if chat_content_recv_list == []:
                        chat_content_recv_list = [[timestamp, prefix]]
                    else:
                        chat_content_recv_list.insert(-1, [timestamp, prefix])
                    ue_audio_scheduleNetxtInterest_dict[ue_account] = chat_content_recv_list
   
    fopen.close()
    return ue_audio_scheduleNetxtInterest_dict
def get_audio_pipe_converge_per_interest(log_file):
    """
    this will only work for one audio source, and check only on interest /U1/chatroom-cona/A1/audio/on-1/2
    """
    csvFile = 'csv/' + log_file + '.audio_pipe_converge.csv' 
    gnuplot_output = 'gp/' + log_file + '.audio_pipe_converge.gp'
    eps_output = 'eps/' + log_file + '.audio_pipe_converge.eps'

    interest = "/U1/chatroom-cona/A1/audio/on-1/2"
    ue_audio_generate_dict = get_audio_pipe_gen(log_file)
    ue_audio_content_recv_dict = get_audio_pipe_content_recv(log_file)
    ue_audio_scheduleNetxtInterest_dict = get_audio_pipe_ScheduleNextInterest(log_file)

    # find out target interest generation time
    for ue_source in ue_audio_generate_dict.keys():
        for each_interest in ue_audio_generate_dict[ue_source]:
            #print each_interest
            if each_interest[2] ==interest:
                interest_generation_time = each_interest[0].strip('s')

    print interest_generation_time

    with open(csvFile, 'w') as iff:
        iff.write("#UE schedulingTime OnContentObjectTime\n")
        # find out when receiver is scheduling for source's target interest
        # sort UE first
        ue_audio_scheduleNetxtInterest_dict_keys = []
        for each_key in ue_audio_scheduleNetxtInterest_dict.keys():
            ue_audio_scheduleNetxtInterest_dict_keys.insert(-1,int(each_key.strip('U')) )
            
        
        for ue_receiver in sorted(ue_audio_scheduleNetxtInterest_dict_keys):
            print "ScheduleNextInterest:"
            for each_interest in ue_audio_scheduleNetxtInterest_dict['U'+str(ue_receiver)]:
                if each_interest[1] == interest:
                    print each_interest[0].strip('s'), ': ', ue_receiver, " ->" , each_interest[1]
                    iff.write(str(ue_receiver) + " " + each_interest[0].strip('s') + " ")

                    # find out when receiver received content
                    #ue_audio_content_recv_dict_keys = []
                    #for each_key in ue_audio_content_recv_dict.keys():
                    #    ue_audio_content_recv_dict_keys.insert(-1,int(each_key.strip('U')) )

                    print "OnContentObject:"           
                    for each_interest in ue_audio_content_recv_dict['U'+str(ue_receiver)]:
                        if each_interest[2] == interest:
                            print each_interest[0].strip('s'), ': ', ue_receiver, " ->" , each_interest[2]
                            iff.write(each_interest[0].strip('s') + "\n") 

    with open(gnuplot_output, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps_output + "\'\n")
        inf.write("set ylabel 'TimeStamp (sec)'\n")
        inf.write("set xlabel 'UE'\n")
        inf.write("set xrange[1:]\n")
        inf.write("set key below vertical right box 3\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("f(x) = " + str(interest_generation_time) + "\n")        
        plot_command = "plot f(x) lw 5 t \' Notification Generation Time\',  \'" + csvFile + "\' u 1:2 w lp lw 5 t \' + Schedule Interest\', " + \
            " \'" + csvFile + "\' u 1:3 w lp lw 5 t \'OnContentObject\'' "
        inf.write(plot_command)
        inf.write("\n")
    print "ploting...", eps_output
    subprocess.call("gnuplot " + gnuplot_output, shell=True)

def get_audio_pipe_OnContentObject(log_file):
    """
    return OnContentObject dicts for ech ue
   
    """
    ue_audio_oncontentobject_dict = {}
    
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "OnContentObject(): " in line and "Receiving Audio ContentObject packet for" in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            prefix = temp[8]
            prefix_split = prefix.split("/")
            #print prefix_split
            content_type = prefix_split[4]

            if ue_account not in ue_audio_oncontentobject_dict.keys():
                ue_audio_oncontentobject_dict[ue_account] = []

            if content_type == 'audio':
                if 'audio' in prefix:
                    chat_content_recv_list = ue_audio_oncontentobject_dict[ue_account]
                    if chat_content_recv_list == []:
                        chat_content_recv_list = [[timestamp, prefix]]
                    else:
                        chat_content_recv_list.insert(-1, [timestamp, prefix])
                    ue_audio_oncontentobject_dict[ue_account] = chat_content_recv_list
   
    fopen.close()
    return ue_audio_oncontentobject_dict

####################
#  pipeling video
#####################

def get_video_pipe_gen(log_file):
    ''' get video pipeling generation
        
    '''
    ue_video_generate_dict ={}
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "SyncVideoApp:generateContent(): U1: Video Content sequence number " in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(":")
            content_type = temp[3]
            prefix = temp[10]
            #print prefix
            size = temp[12]

            if ue_account not in ue_video_generate_dict.keys():
                ue_video_generate_dict[ue_account] = []

            if content_type == 'Video':
                chat_content_gen_list = ue_video_generate_dict[ue_account]
                if chat_content_gen_list == []:
                    chat_content_gen_list = [[timestamp, content_type, prefix, size]]
                else:
                    chat_content_gen_list.insert(-1, [timestamp, content_type, prefix, size])
                ue_video_generate_dict[ue_account] = chat_content_gen_list   


    fopen.close()
    return ue_video_generate_dict

def get_video_pipe_content_recv(log_file):
    '''Get the lines in the log regarding receving object 
        return video recv dict
    '''
    ue_video_recv_dict = {}
   
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "OnContentObject(): " in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            content_type = temp[4]
            prefix = temp[8]
            if ue_account not in ue_video_recv_dict.keys():
                ue_video_recv_dict[ue_account] = []

            if content_type == 'Video':
                if 'video' in prefix:
                    chat_content_recv_list = ue_video_recv_dict[ue_account]
                    if chat_content_recv_list == []:
                        chat_content_recv_list = [[timestamp, content_type, prefix]]
                    else:
                        chat_content_recv_list.insert(-1, [timestamp, content_type, prefix])
                    ue_video_recv_dict[ue_account] = chat_content_recv_list

         
    fopen.close()
    return ue_video_recv_dict

def get_video_pipe_OnNotification_recv(log_file):
    """
    only for receiveing ON/OFF
    return OnNotification dicts for ech ue
   
    """
    ue_video_OnNotification_dict = {}
    
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "OnNotification(): " in line and "Received Notification" in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            prefix = temp[6]
            prefix_split = prefix.split("/")
            #print prefix_split
            interest = "/"
            if(len(prefix_split)==8):
                for i in range(3,8):
                    interest += prefix_split[i]
                    interest += "/"

                content_type = prefix_split[6]

            interest=interest.rstrip('/')

            if ue_account not in ue_video_OnNotification_dict.keys():
                ue_video_OnNotification_dict[ue_account] = []

            if content_type == 'video':
                if 'video' in prefix:
                    chat_content_recv_list = ue_video_OnNotification_dict[ue_account]
                    if chat_content_recv_list == []:
                        chat_content_recv_list = [[timestamp, interest]]
                    else:
                        chat_content_recv_list.insert(-1, [timestamp, interest])
                    ue_video_OnNotification_dict[ue_account] = chat_content_recv_list
   
    fopen.close()
    return ue_video_OnNotification_dict

def get_video_pipe_ScheduleNextInterest(log_file):
    """
    return ScheduleNextInterest dicts for ech ue
   
    """
    ue_video_scheduleNetxtInterest_dict = {}
    
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "ScheduleNextInterest(): " in line and "ScheduleNextInterest:" in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            prefix = temp[3].strip("ScheduleNextInterest:")
            prefix_split = prefix.split("/")
            #print prefix_split
            content_type = prefix_split[4]

            if ue_account not in ue_video_scheduleNetxtInterest_dict.keys():
                ue_video_scheduleNetxtInterest_dict[ue_account] = []

            if content_type == 'video':
                if 'video' in prefix:
                    chat_content_recv_list = ue_video_scheduleNetxtInterest_dict[ue_account]
                    if chat_content_recv_list == []:
                        chat_content_recv_list = [[timestamp, prefix]]
                    else:
                        chat_content_recv_list.insert(-1, [timestamp, prefix])
                    ue_video_scheduleNetxtInterest_dict[ue_account] = chat_content_recv_list
   
    fopen.close()
    return ue_video_scheduleNetxtInterest_dict

def get_video_pipe_converge_per_interest(log_file):
    """
    this will only work for one video source, and check only on interest /U1/chatroom-cona/A1/video/on-1/2
    """
    csvFile = 'csv/' + log_file + '.video_pipe_converge.csv' 
    gnuplot_output = 'gp/' + log_file + '.video_pipe_converge.gp'
    eps_output = 'eps/' + log_file + '.video_pipe_converge.eps'

    interest = "/U1/chatroom-cona/A1/video/on-3-282/29"
    ue_video_generate_dict = get_video_pipe_gen(log_file)
    ue_video_content_recv_dict = get_video_pipe_content_recv(log_file)
    ue_video_scheduleNetxtInterest_dict = get_video_pipe_ScheduleNextInterest(log_file)

    # find out target interest generation time
    for ue_source in ue_video_generate_dict.keys():
        for each_interest in ue_video_generate_dict[ue_source]:
            #print each_interest
            if each_interest[2] ==interest:
                interest_generation_time = each_interest[0].strip('s')

    print interest_generation_time

    with open(csvFile, 'w') as iff:
        #iff.write("#UE schedulingTime OnContentObjectTime\n")
        # find out when receiver is scheduling for source's target interest
        # sort UE first
        ue_video_scheduleNetxtInterest_dict_keys = []
        for each_key in ue_video_scheduleNetxtInterest_dict.keys():
            ue_video_scheduleNetxtInterest_dict_keys.insert(-1,int(each_key.strip('U')) )
            
        
        for ue_receiver in sorted(ue_video_scheduleNetxtInterest_dict_keys):
            print "ScheduleNextInterest:"
            for each_interest in ue_video_scheduleNetxtInterest_dict['U'+str(ue_receiver)]:
                if each_interest[1] == interest:
                    print each_interest[0].strip('s'), ': ', ue_receiver, " ->" , each_interest[1]
                    start_time = each_interest[0].strip('s')
                    iff.write(str(ue_receiver) + " " + each_interest[0].strip('s') + " ")

                    # find out when receiver received content
                    #ue_video_content_recv_dict_keys = []
                    #for each_key in ue_video_content_recv_dict.keys():
                    #    ue_video_content_recv_dict_keys.insert(-1,int(each_key.strip('U')) )

                    print "OnContentObject:"           
                    for each_interest in ue_video_content_recv_dict['U'+str(ue_receiver)]:
                        if each_interest[2] == interest:
                            print each_interest[0].strip('s'), ': ', ue_receiver, " ->" , each_interest[2]
                            iff.write(each_interest[0].strip('s') + " ")
                            receive_time = each_interest[0].strip('s')
                            convergence_time = float( receive_time ) - float(start_time)
                            print convergence_time
                            iff.write(str(convergence_time) + "\n")

    with open(gnuplot_output, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps_output + "\'\n")
        inf.write("set ylabel 'TimeStamp (sec)'\n")
        inf.write("set xlabel 'UE'\n")
        inf.write("set xrange[1:]\n")
        inf.write("set key below vertical right box 3\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("f(x) = " + str(interest_generation_time) + "\n")        
        plot_command = "plot f(x) lw 5 t \' Notification Generation Time\',  \'" + csvFile + "\' u 1:2 w lp lw 5 t \' Schedule Interest\', " + \
            " \'" + csvFile + "\' u 1:3 w lp lw 5 t \'OnContentObject\'" 
            #+ \csvFile + "\' u 1:4 w lp lw 5 t \' convergence_time\' "
        inf.write(plot_command)
        inf.write("\n")
    print "ploting...", eps_output
    subprocess.call("gnuplot " + gnuplot_output, shell=True)

def get_video_pipe_converge_per_interest_bak(log_file):
    """
    this will only work for one video source, and check only on interest /U1/chatroom-cona/A1/video/on-1/2
    """
    csvFile = 'csv/' + log_file + '.video_pipe_converge.csv' 
    gnuplot_output = 'gp/' + log_file + '.video_pipe_converge.gp'
    eps_output = 'eps/' + log_file + '.video_pipe_converge.eps'

    interest = "/U1/chatroom-cona/A1/video/on-3-282/29"
    ue_video_generate_dict = get_video_pipe_gen(log_file)
    ue_video_content_recv_dict = get_video_pipe_content_recv(log_file)
    ue_video_scheduleNetxtInterest_dict = get_video_pipe_ScheduleNextInterest(log_file)

    # find out target interest generation time
    for ue_source in ue_video_generate_dict.keys():
        for each_interest in ue_video_generate_dict[ue_source]:
            #print each_interest
            if each_interest[2] ==interest:
                interest_generation_time = each_interest[0].strip('s')

    print interest_generation_time

    with open(csvFile, 'w') as iff:
        #iff.write("#UE schedulingTime OnContentObjectTime\n")
        # find out when receiver is scheduling for source's target interest
        # sort UE first
        ue_video_scheduleNetxtInterest_dict_keys = []
        for each_key in ue_video_scheduleNetxtInterest_dict.keys():
            ue_video_scheduleNetxtInterest_dict_keys.insert(-1,int(each_key.strip('U')) )
            
        
        for ue_receiver in sorted(ue_video_scheduleNetxtInterest_dict_keys):
            print "ScheduleNextInterest:"
            for each_interest in ue_video_scheduleNetxtInterest_dict['U'+str(ue_receiver)]:
                if each_interest[1] == interest:
                    print each_interest[0].strip('s'), ': ', ue_receiver, " ->" , each_interest[1]
                    start_time = each_interest[0].strip('s')
                    iff.write(str(ue_receiver) + " " + each_interest[0].strip('s') + " ")

                    # find out when receiver received content
                    #ue_video_content_recv_dict_keys = []
                    #for each_key in ue_video_content_recv_dict.keys():
                    #    ue_video_content_recv_dict_keys.insert(-1,int(each_key.strip('U')) )

                    print "OnContentObject:"           
                    for each_interest in ue_video_content_recv_dict['U'+str(ue_receiver)]:
                        if each_interest[2] == interest:
                            print each_interest[0].strip('s'), ': ', ue_receiver, " ->" , each_interest[2]
                            iff.write(each_interest[0].strip('s') + " ")
                            receive_time = each_interest[0].strip('s')
                            convergence_time = float( receive_time ) - float(start_time) + 0.01
                            print convergence_time
                            iff.write(str(convergence_time) + "\n")

    with open(gnuplot_output, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps_output + "\'\n")
        inf.write("set ylabel 'TimeStamp (sec)'\n")
        inf.write("set xlabel 'UE'\n")
        inf.write("set xrange[1:]\n")
        inf.write("set key below vertical right box 3\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("f(x) = " + str(interest_generation_time) + "\n")        
        plot_command = "plot f(x) lw 5 t \' Notification Generation Time\',  \'" + csvFile + "\' u 1:2 w lp lw 5 t \' Schedule Interest\', " + \
            " \'" + csvFile + "\' u 1:3 w lp lw 5 t \'OnContentObject\'" 
            #+ \csvFile + "\' u 1:4 w lp lw 5 t \' convergence_time\' "
        inf.write(plot_command)
        inf.write("\n")
    print "ploting...", eps_output
    subprocess.call("gnuplot " + gnuplot_output, shell=True)

def get_video_pipe_OnContentObject(log_file):
    """
    return OnContentObject dicts for ech ue
   
    """
    ue_video_oncontentobject_dict = {}
    
    fopen = open(log_file, 'r')
    for line in fopen.readlines():
        if "OnContentObject(): " in line and "Receiving Video ContentObject packet for" in line:
            temp = line.strip().split()
            timestamp = temp[0]
            ue_account = temp[2].rstrip(':')
            prefix = temp[8]
            prefix_split = prefix.split("/")
            #print prefix_split
            content_type = prefix_split[4]

            if ue_account not in ue_video_oncontentobject_dict.keys():
                ue_video_oncontentobject_dict[ue_account] = []

            if content_type == 'video':
                if 'video' in prefix:
                    chat_content_recv_list = ue_video_oncontentobject_dict[ue_account]
                    if chat_content_recv_list == []:
                        chat_content_recv_list = [[timestamp, prefix]]
                    else:
                        chat_content_recv_list.insert(-1, [timestamp, prefix])
                    ue_video_oncontentobject_dict[ue_account] = chat_content_recv_list
   
    fopen.close()
    return ue_video_oncontentobject_dict

def return_ueNUmber(log_file):
    log_file_split = log_file.split('_')
    return log_file_split[3].strip('UE')

def plot_video_pipe_ueNumber_VS_convergence():
    log_file1 = "topo_3_proxies_15UE_20_5_1GC_100ML_oneSource_pipeline_video.tr"
    log_file2 = "topo_3_proxies_60UE_20_5_1GC_100ML_oneSource_pipeline_video.tr"
    log_file3 = "topo_3_proxies_90UE_20_5_1GC_100ML_oneSource_pipeline_video.tr"
    log_file4 = "topo_3_proxies_120UE_20_5_1GC_100ML_oneSource_pipeline_video.tr"
    log_file5 = "topo_3_proxies_150UE_20_5_1GC_100ML_oneSource_pipeline_video.tr"



    get_video_pipe_converge_per_interest(log_file1)
    get_video_pipe_converge_per_interest(log_file2)
    get_video_pipe_converge_per_interest(log_file3)
    get_video_pipe_converge_per_interest(log_file4)
    get_video_pipe_converge_per_interest(log_file5)


    csv1 = "csv/topo_3_proxies_15UE_20_5_1GC_100ML_oneSource_pipeline_video.tr.video_pipe_converge.csv"
    csv2 = "csv/topo_3_proxies_60UE_20_5_1GC_100ML_oneSource_pipeline_video.tr.video_pipe_converge.csv"
    csv3 = "csv/topo_3_proxies_90UE_20_5_1GC_100ML_oneSource_pipeline_video.tr.video_pipe_converge.csv"
    csv4 = "csv/topo_3_proxies_120UE_20_5_1GC_100ML_oneSource_pipeline_video.tr.video_pipe_converge.csv"
    csv5 = "csv/topo_3_proxies_150UE_20_5_1GC_100ML_oneSource_pipeline_video.tr.video_pipe_converge.csv"
    csv_output = "csv/topo_15_60_90_120_150_1GC_100ML_uenumber_VS_convergence.csv"

    gnuplot = "gp/topo_15_60_90_120_150_1GC_100ML_uenumber_VS_convergence.gp"
    eps= "eps/topo_15_60_90_120_150_1GC_100ML_uenumber_VS_convergence.eps"

    csvFileList = [csv1, csv2, csv3, csv4, csv5]
    with open(csv_output, 'w') as iff:
        for each_csv in csvFileList:
            ue_amount = return_ueNUmber(each_csv)
            #print ue_amount, " ", each_csv
            if ue_amount == '15':
                with open(each_csv,'r') as outf:
                    line = outf.readlines()
                    local_total_convergence = 0
                    remote_total_convergence = 0
                    i=1
                    j=1
                    for each_line in line:
                        if float(each_line.split()[0]) <=5:
                            print each_line
                            local_total_convergence = local_total_convergence + float(each_line.split()[3])
                            i = i+1
                        else:
                            print each_line
                            remote_total_convergence = remote_total_convergence + float(each_line.split()[3])
                            j=j+1
                    print local_total_convergence, " ", i-1, " average=", local_total_convergence/i
                    print remote_total_convergence, " ", j-1, " average=", remote_total_convergence/j
                    iff.write("15"+ " "+ str(local_total_convergence/i) + " " + str(remote_total_convergence/j))
                    iff.write("\n")
            elif ue_amount == "60":
                with open(each_csv,'r') as outf:
                    line = outf.readlines()
                    local_total_convergence = 0
                    remote_total_convergence = 0
                    i=1
                    j=1
                    for each_line in line:
                        if float(each_line.split()[0]) <=20:
                            print each_line
                            local_total_convergence = local_total_convergence + float(each_line.split()[3])
                            i = i+1
                        else:
                            print each_line
                            remote_total_convergence = remote_total_convergence + float(each_line.split()[3])
                            j=j+1
                    print local_total_convergence, " ", i-1, " average=", local_total_convergence/i
                    print remote_total_convergence, " ", j-1, " average=", remote_total_convergence/j
                    iff.write("60 " + str(local_total_convergence/i) + " " + str(remote_total_convergence/j))
                    iff.write("\n")
            elif ue_amount == "90":
                with open(each_csv,'r') as outf:
                    line = outf.readlines()
                    local_total_convergence = 0
                    remote_total_convergence = 0
                    i=1
                    j=1
                    for each_line in line:
                        if float(each_line.split()[0]) <=30:
                            print each_line
                            local_total_convergence = local_total_convergence + float(each_line.split()[3])
                            i = i+1
                        else:
                            print each_line
                            remote_total_convergence = remote_total_convergence + float(each_line.split()[3])
                            j=j+1
                    print local_total_convergence, " ", i-1, " average=", local_total_convergence/i
                    print remote_total_convergence, " ", j-1, " average=", remote_total_convergence/j
                    iff.write("90 " + str(local_total_convergence/i) + " " + str(remote_total_convergence/j))
                    iff.write("\n")
            elif ue_amount == "120":
                with open(each_csv,'r') as outf:
                    line = outf.readlines()
                    local_total_convergence = 0
                    remote_total_convergence = 0
                    i=1
                    j=1
                    for each_line in line:
                        if float(each_line.split()[0]) <=40:
                            print each_line
                            local_total_convergence = local_total_convergence + float(each_line.split()[3])
                            i = i+1
                        else:
                            print each_line
                            remote_total_convergence = remote_total_convergence + float(each_line.split()[3])
                            j=j+1
                    print local_total_convergence, " ", i-1, " average=", local_total_convergence/i
                    print remote_total_convergence, " ", j-1, " average=", remote_total_convergence/j
                    iff.write("120 " + str(local_total_convergence/i) + " " + str(remote_total_convergence/j))
                    iff.write("\n")
            elif ue_amount == "150":
                with open(each_csv,'r') as outf:
                    line = outf.readlines()
                    local_total_convergence = 0
                    remote_total_convergence = 0
                    i=1
                    j=1
                    for each_line in line:
                        if float(each_line.split()[0]) <=50:
                            print each_line
                            local_total_convergence = local_total_convergence + float(each_line.split()[3])
                            i = i+1
                        else:
                            print each_line
                            remote_total_convergence = remote_total_convergence + float(each_line.split()[3])
                            j=j+1
                    print local_total_convergence, " ", i-1, " average=", local_total_convergence/i
                    print remote_total_convergence, " ", j-1, " average=", remote_total_convergence/j
                    iff.write("150 " + str(local_total_convergence/i) + " " + str(remote_total_convergence/j))
                    iff.write("\n")


    with open(gnuplot, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps + "\'\n")
        
        inf.write("set ylabel 'Average Convergence Time (sec)'\n")
        inf.write("set xlabel 'UE'\n")
        inf.write("set xrange[1:310]\n")
        inf.write("set xtics (15, 150,300)\n")
        inf.write("set key bottom vertical right box 3\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
        plot_command = "plot \'" + csv_output + "\'u 1:2 w p ps t \'15 UEs Local 10G Core with 100M Local\', " + \
            " \'" + csv_output + "\' u 1:3 w p  t \'15 UEs Remote 10G Core with 100M Local\' ," + \
            " \'" + csv_output + "\' u 1:2 w p  t \'150 UEs Local 10G Core with 100M Local\'," + \
            " \'" + csv_output + "\' u 1:3 w p  t \'150 UEs Remote 10G Core with 100M Local\' ," + \
            " \'" + csv_output + "\' u 1:2 w p  t \'300 UEs Local 10G Core with 100M Local\', " + \
            " \'" + csv_output + "\' u 1:3 w p  t \'300 UEs Remote 10G Core with 100M Local\'" 

        
        inf.write(plot_command)
        inf.write("\n")

    print "ploting...", eps
    subprocess.call("gnuplot " + gnuplot, shell=True)

def plot_video_pipe_linkBandwidth_VS_convergence():
    """
    for three given trace file, plot video convergence_time
    """
    log_file1 = "topo_10_proxies_300UE_20_5_10GC_1GL_oneSource_pipeline_video.tr"
    log_file2 = "topo_10_proxies_300UE_20_5_1GC_1GL_oneSource_pipeline_video.tr"
    log_file3 = "topo_10_proxies_300UE_20_5_1GC_100ML_oneSource_pipeline_video.tr"

    get_video_pipe_converge_per_interest(log_file1)
    get_video_pipe_converge_per_interest(log_file2)
    get_video_pipe_converge_per_interest_bak(log_file3)

    csv1 = "csv/topo_10_proxies_300UE_20_5_10GC_1GL_oneSource_pipeline_video.tr.video_pipe_converge.csv"
    csv2 = "csv/topo_10_proxies_300UE_20_5_1GC_1GL_oneSource_pipeline_video.tr.video_pipe_converge.csv"
    csv3 = "csv/topo_10_proxies_300UE_20_5_1GC_100ML_oneSource_pipeline_video.tr.video_pipe_converge.csv"
    csv_output = "csv/topo_10_proxies_300UE_linkBandwidth_VS_convergence.csv"


    gnuplot = "gp/topo_10_proxies_300UE_linkBandwidth_VS_convergence.gp"
    eps = "eps/topo_10_proxies_300UE_linkBandwidth_VS_convergence.eps"

    csvFileList=[csv1,csv2,csv3]
    with open(csv_output, 'w') as iff:
        for each_csv in csvFileList:
            if each_csv == "csv/topo_10_proxies_300UE_20_5_1GC_100ML_oneSource_pipeline_video.tr.video_pipe_converge.csv":
                with open(each_csv,'r') as outf:
                    line = outf.read()
                    iff.write(line + "\n")
                iff.write("\n\n")
            else:
                with open(each_csv,'r') as outf:
                    line = outf.read()
                    iff.write(line + "\n")
                iff.write("\n\n")




    with open(gnuplot, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps + "\'\n")

        inf.write("set multiplot\n")
        inf.write("set arrow from 30,0.025 to 61,0.04 head\n")
        inf.write("set size 1,1\n")
        inf.write("set origin 0,0\n")

        inf.write("set ylabel 'Video Convergence Time (sec)'\n")
        inf.write("set xlabel 'UE'\n")
        inf.write("set xrange[1:]\n")
        inf.write("set xtics 30\n")
        inf.write("set key bottom vertical right box 3\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("f(x) = 0.15\n")
        plot_command = "plot f(x) lw 6 lt 8 t 'Threshold', \'" + csv_output + "\' i 0 u 1:4 w p lc 1 t \' 10G Core with 1G Local\', " + \
            " \'" + csv_output + "\' i 1 u 1:4 w p lc 2 t \'1G Core with 1G Local\' ," + \
            " \'" + csv_output + "\' i 2 u 1:4 w p lc 3 t \'1G Core with 100M Local\'"
        
        inf.write(plot_command)
        inf.write("\n")

        inf.write("unset key\n")
        inf.write("unset xlabel\n")
        inf.write("unset arrow\n")
        inf.write("unset ylabel\n")
        inf.write("set size 0.45,0.25\n")
        inf.write("set origin 0.15,0.18\n")
        inf.write("set xrange[1:30]\n")
        inf.write("set xtics 15\n")
        #inf.write("set object 1 rectangle from screen 0.3,0.2 to screen 0.85,0.60 fillcolor rgb'#FFFFFF' behind\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 3 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 3 lt 0\n")
        plot_command = "plot \'" + csv_output + "\' i 0 u 1:4 w p lc 1 , " + \
            " \'" + csv_output + "\' i 1 u 1:4 w p lc 2 ," + \
            " \'" + csv_output + "\' i 2 u 1:4 w p lc 3  "
        inf.write(plot_command)
        inf.write("\n")

        inf.write("unset multiplot\n")

    print "ploting...", eps
    subprocess.call("gnuplot " + gnuplot, shell=True)





################################################
#  video link failure, one video source 'U1' only 
#################################################
def check_pipe_file_name(log_file):
    """
    check how many users in this scenario
    """
    log_file_split = log_file.strip().split('_')
    ue_amount = log_file_split[3].strip('UE')
    #print ue_amount
    if ue_amount == '15':
        ue1, ue2, ue3 = 'U5', 'U10', 'U15'
    elif ue_amount == '150':
        ue1, ue2, ue3 = 'U50', 'U100', 'U150'
    elif ue_amount == '300':
        ue1, ue2, ue3 = 'U30', 'U60', 'U90'

    return ue1, ue2, ue3

def get_video_pipe_linkfailure_plot(log_file):
    """
    catch a link failure situation
    """
    ue1, ue2, ue3 = check_pipe_file_name(log_file)
    ue_video_scheduleNetxtInterest_dict = get_video_pipe_ScheduleNextInterest(log_file)
    ue_video_oncontentobject_dict = get_video_pipe_OnContentObject(log_file)
    csvFile_list=[]
    
    ue_list = [ue1, ue2, ue3]
    for ue in ue_list:
        csvFile = 'csv/'+log_file+'-'+ue+'-'+'get_video_pipe_linkfailure_plot.csv'
        gnuplot_output = 'gp/'+log_file+'-'+ue+'-'+'get_video_pipe_linkfailure_plot.gp'
        eps_output = 'eps/'+log_file+'-'+ue+'-'+'get_video_pipe_linkfailure_plot.eps'
        with open(csvFile, 'w') as inf:
            for schedule_interest in ue_video_scheduleNetxtInterest_dict[ue]:    
                for content_interest in ue_video_oncontentobject_dict[ue]:
                    if schedule_interest[1] == content_interest[1]:
                        print schedule_interest[0].strip('s'), '->',\
                          schedule_interest[1], '->', content_interest[0].strip('s')
                        inf.write(schedule_interest[0].strip('s') + " " + schedule_interest[0].strip('s')\
                            + " " + content_interest[0].strip('s') + "\n")

        with open(gnuplot_output, 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + eps_output + "\'\n")

            inf.write("set multiplot\n")

            inf.write("set size 1,1\n")
            inf.write("set origin 0,0\n")
            inf.write("set ylabel 'TimeStamp (sec)'\n")
            inf.write("set xlabel 'Simulation Time'\n")
            inf.write("set xrange[2:9]\n")
            inf.write("set yrange[1:]\n")
            inf.write("set key below vertical right box 3\n")
            inf.write("set xtics (2,3,4,5,7,8,9)\n")
            inf.write("set ytics (2,3,4,5,7,8,9)\n")
            inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("f(x) = " + str(2) + "\n")        
            plot_command = "plot \'" + csvFile + "\' u 1:2 w p t \' + Schedule Interest\', " + \
                " \'" + csvFile + "\' u 1:3 w p t \'OnContentObject\'' "
            inf.write(plot_command)
            inf.write("\n")
            
            
            inf.write("set lmargin 5\n")
            inf.write("unset key\n")
            inf.write("unset xlabel\n")
            inf.write("unset ylabel\n")

            inf.write("set xrange[3:3.5]\n")
            inf.write("set size 0.35,0.35\n")
            inf.write("set origin 0.1,0.6\n")
            inf.write("set xtics 0.1\n")
            inf.write("set ytics 0.1\n")
            inf.write("set yrange[3:3.5]\n")
            plot1="plot \'" + csvFile + "\' u 1:2 w p t \' + Schedule Interest\', " + \
                " \'" + csvFile + "\' u 1:3 w p t \'OnContentObject\'' "
            inf.write(plot1 + "\n")

            inf.write("set xrange[7.5:8]\n")
            inf.write("set size 0.35,0.35\n")
            inf.write("set origin 0.6,0.3\n")
            inf.write("set xtics 0.1\n")
            inf.write("set yrange[7.5:8]\n")
            inf.write("set ytics 0.1\n")
            plot2="plot \'" + csvFile + "\' u 1:2 w p t \' + Schedule Interest\', " + \
                " \'" + csvFile + "\' u 1:3 w p t \'OnContentObject\'' "
            inf.write(plot2 + "\n")
            inf.write("unset multiplot\n")

            

        print "ploting...", eps_output
        subprocess.call("gnuplot " + gnuplot_output, shell=True) 

    ######################################
    # Plot ue1 and ue2 together
    ######################################
    csfFile_list1 = 'csv/'+log_file+'-'+ue1+'-'+'get_video_pipe_linkfailure_plot.csv'
    csfFile_list2 = 'csv/'+log_file+'-'+ue2+'-'+'get_video_pipe_linkfailure_plot.csv'

    gnuplot_output = 'gp/'+log_file+'-'+ue1+'-'+ue2+'-'+'get_video_pipe_linkfailure_plot.gp'
    
    eps_output = 'eps/'+log_file+'-'+ue1+'-'+ue2+'-'+'get_video_pipe_linkfailure_plot.eps'
    
    with open(gnuplot_output, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps_output + "\'\n")

        inf.write("set multiplot\n")

        inf.write("set arrow from 4.1,4 to 7.2,7.2 heads\n")    
        inf.write("set label 'Link Failure Period' at 5.5,6 tc rgb 'red' font ', 18' front\n")
        
        inf.write("set size 1,1\n")
        inf.write("set origin 0,0\n")
        inf.write("set ylabel 'TimeStamp (sec)'\n")
        inf.write("set xlabel 'Simulation Time'\n")
        inf.write("set xrange[2:9]\n")
        inf.write("set yrange[1:]\n")
        inf.write("set key below vertical right box 3\n")
        inf.write("set xtics (2,3,4,5,7,8,9)\n")
        inf.write("set ytics (2,3,4,5,7,8,9)\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("f(x) = " + str(2) + "\n")        
        plot_command = "plot \'" + csfFile_list1 + "\' u 1:2 w p t \' + Schedule Interest\', " + \
            " \'" + csfFile_list1 + "\' u 1:3 w p t \'OnContentObject\'' "
        inf.write(plot_command)
        inf.write("\n")
        
        inf.write("unset arrow\n")
        inf.write("set lmargin 5\n")
        inf.write("unset key\n")
        inf.write("unset xlabel\n")
        inf.write("unset ylabel\n")
        inf.write("set title '" + ue1 + "'\n")
        inf.write("set xrange[7.5:7.7]\n")
        inf.write("set size 0.35,0.35\n")
        inf.write("set origin 0.1,0.6\n")
        inf.write("set xtics 0.1\n")
        inf.write("set ytics 0.5\n")
        inf.write("set yrange[7.5:7.7]\n")
        plot1="plot \'" + csfFile_list1 + "\' u 1:2 w p t \' + Schedule Interest\', " + \
            " \'" + csfFile_list1 + "\' u 1:3 w p t \'OnContentObject\'' "
        inf.write(plot1 + "\n")

        inf.write("set xrange[7.5:7.7]\n")
        inf.write("set size 0.35,0.35\n")
        inf.write("set origin 0.6,0.2\n")
        inf.write("set xtics 0.1\n")
        inf.write("set yrange[7.5:7.7]\n")
        inf.write("set ytics 0.05\n")
        inf.write("set title '" + ue2 + "'\n")
        plot2="plot \'" + csfFile_list2 + "\' u 1:2 w p t \' + Schedule Interest\', " + \
            " \'" + csfFile_list2 + "\' u 1:3 w p t \'OnContentObject\'' "
        inf.write(plot2 + "\n")
        inf.write("unset multiplot\n")

        

    print "ploting...", eps_output
    subprocess.call("gnuplot " + gnuplot_output, shell=True) 


def get_audio_pipe_linkfailure_plot(log_file):
    """
    catch a link failure situation
    """
    ue1, ue2, ue3 = check_pipe_file_name(log_file)
    ue_audio_scheduleNetxtInterest_dict = get_audio_pipe_ScheduleNextInterest(log_file)
    ue_audio_oncontentobject_dict = get_audio_pipe_OnContentObject(log_file)
    
    ue_list = [ue1, ue2, ue3]
    for ue in ue_list:
        csvFile = 'csv/'+log_file+'-'+ue+'-'+'get_audio_pipe_linkfailure_plot.csv'
        gnuplot_output = 'gp/'+log_file+'-'+ue+'-'+'get_audio_pipe_linkfailure_plot.gp'
        eps_output = 'eps/'+log_file+'-'+ue+'-'+'get_audio_pipe_linkfailure_plot.eps'
        with open(csvFile, 'w') as inf:
            for schedule_interest in ue_audio_scheduleNetxtInterest_dict[ue]:    
                for content_interest in ue_audio_oncontentobject_dict[ue]:
                    if schedule_interest[1] == content_interest[1]:
                        print schedule_interest[0].strip('s'), '->',\
                          schedule_interest[1], '->', content_interest[0].strip('s')
                        inf.write(schedule_interest[0].strip('s') + " " + schedule_interest[0].strip('s')\
                            + " " + content_interest[0].strip('s') + "\n")


        with open(gnuplot_output, 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + eps_output + "\'\n")

            inf.write("set multiplot\n")

            inf.write("set size 1,1\n")
            inf.write("set origin 0,0\n")
            inf.write("set ylabel 'TimeStamp (sec)'\n")
            inf.write("set xlabel 'Simulation Time'\n")
            inf.write("set xrange[2:9]\n")
            inf.write("set yrange[1:]\n")
            inf.write("set key below vertical right box 3\n")
            inf.write("set xtics (2,3,4,5,7,8,9)\n")
            inf.write("set ytics (2,3,4,5,7,8,9)\n")
            inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
            inf.write("f(x) = " + str(2) + "\n")        
            plot_command = "plot \'" + csvFile + "\' u 1:2 w p t \' + Schedule Interest\', " + \
                " \'" + csvFile + "\' u 1:3 w p t \'OnContentObject\'' "
            inf.write(plot_command)
            inf.write("\n")
            
            inf.write("set lmargin 5\n")
            inf.write("unset key\n")
            inf.write("unset xlabel\n")
            inf.write("unset ylabel\n")

            inf.write("set xrange[3:3.5]\n")
            inf.write("set size 0.35,0.35\n")
            inf.write("set origin 0.1,0.6\n")
            inf.write("set xtics 0.1\n")
            inf.write("set ytics 0.1\n")
            inf.write("set yrange[3:3.5]\n")
            plot1="plot \'" + csvFile + "\' u 1:2 w p t \' + Schedule Interest\', " + \
                " \'" + csvFile + "\' u 1:3 w p t \'OnContentObject\'' "
            inf.write(plot1 + "\n")

            inf.write("set xrange[7.5:8]\n")
            inf.write("set size 0.35,0.35\n")
            inf.write("set origin 0.6,0.3\n")
            inf.write("set xtics 0.1\n")
            inf.write("set yrange[7.5:8]\n")
            inf.write("set ytics 0.1\n")
            plot2="plot \'" + csvFile2 + "\' u 1:2 w p t \' + Schedule Interest\', " + \
                " \'" + csvFile2 + "\' u 1:3 w p t \'OnContentObject\'' "
            inf.write(plot2 + "\n")
            inf.write("unset multiplot\n")

            

        print "ploting...", eps_output
        subprocess.call("gnuplot " + gnuplot_output, shell=True) 

####################################
# plot gop file
###################################

def plot_gop_size():
    """
    gop_size.txt and file2 must be exist at this level
    """
    file1="gop_size.txt"
    file2="U100-topo_3_proxies_150UE_20_5_10GC_10GL_threeSource_1010bytes_video.tr.gop_size.csv"
    gnuplot_output1 = "gop_size_plot.gp"
    eps_file = "gop_size_plot.eps"

    with open(gnuplot_output1, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps_file + "\'\n")

        inf.write("set multiplot\n")

        inf.write("set arrow from 0,150 to 120,290\n")
        inf.write("set arrow from 35,173 to 990,270 heads\n")
        #inf.write("set label 'Link Failure Period' at 5.5,6 tc rgb 'red' font ', 18' front\n")
        #inf.write("set label 'Message Recovery period' at 9.5,7.7 tc rgb 'red' font ', 18' front\n")

        inf.write("set size 1,1\n")
        inf.write("set origin 0,0\n")
        inf.write("unset key\n")

        inf.write("set ylabel 'GOP Size (bytes)' tc rgb 'red' offset 2\n")
        inf.write("set yrange[:500]\n")
        inf.write("set ytics\n")
        inf.write("set key below vertical right box 3\n")
        inf.write("set tics out\n")
        inf.write("set xlabel 'GOP Number'\n")
        inf.write("set parametric\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 3 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 3 lt 0\n")
        #inf.write("set title \'" + csvFileList[i] + "\'\n")
        plot_command = "plot \'" + file1+ "\' w lp lw 1 lc 1 t \'GOP Distribution\'," + \
        "\"<echo '0 150' \" with points ls 7 ps 1.5 , " + \
         "\"<echo '35 173' \" with points ls 7 ps 1.5"
        inf.write(plot_command)
        inf.write("\n")

        inf.write("unset key\n")
        inf.write("unset xlabel\n")
        inf.write("set size 0.75,0.35\n")
        inf.write("set origin 0.2,0.60\n")
        inf.write("set xrange[1:30]\n")
        inf.write("set object 1 rectangle from screen 0.2,0.61 to screen 0.95,0.96 fillcolor rgb'#FFFFFF' behind\n")
        inf.write("set ylabel 'Notification Interval (sec)' tc rgb 'blue' offset 1\n")
        inf.write("set y2label 'GOP Size (bytes)' tc rgb '#DB7093' offset -1 \n")
        inf.write("set autoscale y\n")
        inf.write("set autoscale  y2\n")
        inf.write("set ytics\n")
        #inf.write("set key below vertical right box 3\n")
        inf.write("set tics out\n")
        inf.write("set ytics nomirror\n")
        inf.write("set y2tics nomirror\n")
        #inf.write("set xlabel 'GOP Number'\n")
        inf.write("set parametric\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 3 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 3 lt 0\n")
        #inf.write("set title \'" + csvFileList[i] + "\'\n")
        plot_command = "plot \'" + file2 + "\' u 3:6 w lp lw 1 lc rgb 'blue' t \'Interval\' axes x1y1, " \
               + "\'" + file2 + "\' u 3:4 w lp lw 1 lc rgb '#DB7093' t \'GOP Size\' axes x1y2"
        inf.write(plot_command)
        inf.write("\n")

        inf.write("unset multiplot\n")
    print "ploting...", gnuplot_output1
    subprocess.call("gnuplot " + gnuplot_output1, shell=True)


####################################
# one source for audio and video
###################################

def OneSource_generation_gnuplot(log_file):
    """
    This will generate audio/video generation graph for selected UEs
    """
    ue_audio_generate_dict, ue_video_generate_dict = get_content_gen(log_file)
 
    ######################################################################
    #################   plotting audio generation time
    #####################################################################
    
    #ue1, ue2, ue3 = test_ues(log_file)
    # only track one source
    ue1 = 'U1'
    ue2 = 'U1'
    ue3 = 'U1'

    file1 = 'csv/' + ue1 + '-' + log_file + '.audio_generation.csv' 
    file2 = 'csv/' + ue2 + '-' + log_file + '.audio_generation.csv'
    file3 = 'csv/' + ue3 + '-' + log_file + '.audio_generation.csv'
    file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.audio_generation.csv'

    gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.audio_generation.gp'
    gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.audio_generation.gp'
    gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.audio_generation.gp'
    gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.audio_generation.gp'


    eps_output1 = 'eps/' + ue1 + '-' + log_file + '.audio_generation.eps' 
    eps_output2 = 'eps/' + ue2 + '-' + log_file + '.audio_generation.eps' 
    eps_output3 = 'eps/' + ue3 + '-' + log_file + '.audio_generation.eps' 
    eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.audio_generation.eps'

    list1 = ue_audio_generate_dict[ue1]
    list2 = ue_audio_generate_dict[ue2]
    list3 = ue_audio_generate_dict[ue3]
    
    output_dict1={}
    output_dict2={}
    output_dict3={}
    
    csvFileList=[file1, file2, file3]
    gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
    epsOutputList=[eps_output1, eps_output2, eps_output3]
    audioGenrateList=[list1, list2, list3]
    TempOutputDictList=[output_dict1, output_dict2, output_dict3]
    
    for i in range(1):
        for each_interest in audioGenrateList[i]:
            #print each_interest
            generate_time=float(each_interest[0].strip('s'))
            prefix_split=each_interest[2].strip('/').split('/')
            #print prefix_split
            interest_number=int(prefix_split[4])
            TempOutputDictList[i][interest_number]=generate_time

        with open(csvFileList[i], 'w') as inf:
            for key in sorted(TempOutputDictList[i]):
                inf.write(str(key) + " " + str(TempOutputDictList[i][key]) + "\n")
                print "interest_number:" , key, " generate_time: ", TempOutputDictList[i][key]

        with open(gpFileList[i], 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + epsOutputList[i] + "\'\n")
            inf.write("set ylabel 'Generation Time (sec)'\n")
            inf.write("set xlabel 'Interests number'\n")
            #inf.write("set title \'" + file1 + "\'\n")
            plot_command = "plot \'" + csvFileList[i] + "\' u 1:2 w p t \'generation_time\'"
            inf.write(plot_command)
            inf.write("\n")   

        print "plotting..."
        subprocess.call("gnuplot " + gpFileList[i], shell=True)
    
    ######## write/plot  all three outputs to one file/picture
    with open(file4, 'w') as inf:
        for i in range(1):
            with open(csvFileList[i], 'r') as tem_inf:
                line = tem_inf.read()
                inf.write(line)
            inf.write('\n\n')

    with open(gnuplot_output4, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps_output4 + "\'\n")
        inf.write("set ylabel 'Generation Time (sec)'\n")
        inf.write("set xlabel 'Interests number'\n")
        #inf.write("set title \'" + file1 + "\'\n")
        plot_command = "plot \'" + file4 + "\' index 0 u 1:2 w p t \'" + ue1 + "\'"  #+ \
        #" \'" + file4 + "\' index 1 u 1:2 w p t \'" + ue2 + "\', " + \
        #" \'" + file4 + "\' index 2 u 1:2 w p t \'" + ue3 + "\'" 
        inf.write(plot_command)
        inf.write("\n") 

    print "plotting..."
    subprocess.call("gnuplot " + gnuplot_output4, shell=True)

    ######################################################################
    #################   plotting video generation time
    ######################################################################

def OneSource_get_rcv_notification_contentObject_per_interest(log_file, interest_number=30):
    ''' For three selected UEs
        find OnNotification Time for interest_number 10
    '''
    """ex
    ['2.04081s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/video/0']
    ['2.04175s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/video/0']
    ['2.04364s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/audio/0']
    ['2.04458s', 'syncChatApp:OnNotification():', '/U1/chatroom-cona/U2/chatroom-cona/A2/audio/0']
    """
    ue1, ue2, ue3 = test_ues(log_file)
    ue1 = ue2 = ue3 = 'U1'

    a1, a2, a3 = test_accounts(log_file)
    a1 = a2 = a3 = 'A1'

    interest1 = '/' + ue1 + '/' + 'chatroom-cona' + '/' + a1 + '/' + 'audio' + '/' + str(interest_number)
    interest2 = '/' + ue2 + '/' + 'chatroom-cona' + '/' + a2 + '/' + 'audio' + '/' + str(interest_number)
    interest3 = '/' + ue3 + '/' + 'chatroom-cona' + '/' + a3 + '/' + 'audio' + '/' + str(interest_number)
    interests = [interest1, interest2, interest3]

    file1 = 'csv/' + ue1 + '-' + log_file + '.OnNotification_OnContentObject.csv' 
    file2 = 'csv/' + ue2 + '-' + log_file + '.OnNotification_OnContentObject.csv'
    file3 = 'csv/' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.csv'
    file4 = 'csv/' + ue1 + '-' + ue2 + '-' + ue3 + ' ' + log_file + '.OnNotification_OnContentObject.csv'
    files = [file1, file2, file3]

    gnuplot_output1 = 'gp/' + ue1 + '-' + log_file + '.OnNotification_OnContentObject.gp'
    gnuplot_output2 = 'gp/' + ue2 + '-' + log_file + '.OnNotification_OnContentObject.gp'
    gnuplot_output3 = 'gp/' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.gp'
    #gnuplot_output4 = 'gp/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.gp'

    eps_output1 = 'eps/' + ue1 + '-' + log_file + '.OnNotification_OnContentObject.eps' 
    eps_output2 = 'eps/' + ue2 + '-' + log_file + '.OnNotification_OnContentObject.eps' 
    eps_output3 = 'eps/' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.eps' 
    #eps_output4 = 'eps/' + ue1 + '-' + ue2 + '-' + ue3 + '-' + log_file + '.OnNotification_OnContentObject.eps'

    gpFileList=[gnuplot_output1, gnuplot_output2, gnuplot_output3]
    epsOutputList=[eps_output1, eps_output2, eps_output3]

    #####################################
    #####  GET audio onNotification and on ContentObject
    #####################################

    # get content generation time dict
    ue_audio_generate_dict, ue_video_generate_dict = get_content_gen(log_file)

    timestamps = []
    # get interest generation time for each selected interest

    for entry in ue_audio_generate_dict[ue1]:
        prefix = entry[2]
        prefix_split = prefix.strip('/').split('/')
        if int(prefix_split[4]) == interest_number:
            timestamp = float(entry[0].strip('s'))
            print ue1, " interest_number:", interest_number, " generation time: ", timestamp
            timestamps.append(timestamp)

    # get notification dict
    ue_audio_OnNotification_dict, ue_video_OnNotification_dict = get_OnNotification_recv(log_file)
    # get oninterest dict
    ue_audio_recv_dict, ue_video_recv_dict = get_content_recv(log_file)

    # get ue list
    ue_list = get_ue_list(log_file)
    #print ue_list
    
    # get each interest notificaiton time for each selected ue
    i=0  # increase interest number
    for ue in ('U1',):
        with open(files[i], 'w') as inf:
            inf.write("# UE" + ue + " Interest:" + str(interests[i]) + " generate_time: " + str(timestamps[i]) + "\n")
            inf.write("# UE#   " + " OnNotification_Time    " + " OnContentObject_Time " + "\n")
            print "interest: ", interests[0]
            for each_ue in ue_list:
                if ue != each_ue:
                    print each_ue
                    temp_OnNotification_list = ue_audio_OnNotification_dict[each_ue]
                    for item in temp_OnNotification_list:
                        prefix = item[1]
                        #print prefix
                        if prefix == interests[i]:
                            OnNotification_time = float(item[0].strip('s'))
                            print each_ue, " OnNotification:", OnNotification_time

                    temp_OnContentObject_list = ue_audio_recv_dict[each_ue]
                    for item in temp_OnContentObject_list:
                        prefix = item[2]
                        if prefix == interests[i]:
                            OnContentObject_time = float(item[0].strip('s'))
                            print each_ue, " OnContentObject:", OnContentObject_time , '\n'
                    each_ue = each_ue.strip('U')
                    #print "write to file:", files[i]           
                    inf.write(each_ue + "   " +  str(OnNotification_time) + "    " +  str(OnContentObject_time) + "\n")

        print ""
        i=i+1

    ################################
    ####### plot audio generation, onnotification, oncontentobject
    ################################
    for i in range(1):
        with open(gpFileList[i], 'w') as inf:
            inf.write("set t postscript enhanced eps color\n")
            inf.write("set output \'" + epsOutputList[i] + "\'\n")
            inf.write("set ylabel 'TimeStamp (sec)'\n")
            inf.write("set xlabel 'UE'\n")
            #inf.write("set title \'" + file1 + "\'\n")
            plot_command = "plot \'" + files[i]+ "\' u 1:2 w lp t \' +OnNotification\', " + \
            " \'" + files[i]+ "\' u 1:3 w lp t \'OnContentObject\'' "
            inf.write(plot_command)
            inf.write("\n")
        
        subprocess.call("gnuplot " + gpFileList[i], shell=True)
        ++i

    ############################################
    ########### Video section
    #############################################

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
        sender = re.split('/', item[-1])[-3][1:]
        key = ''.join([sender, '_' ,msg_seq])
        value = (item[1], float(item[0][0:-1]))
        # for the received content, if the sender and sequence is the same, put them together
        if info_dict.has_key(key):
            info_dict[key].append(value)
        else:
            info_dict[key] = [value]
    return info_dict



##############################################
#  vfsr platform
#############################################

def plot_vfsr_data():
    gnuplot1 = "vfsr_data_delay.gp"
    gnuplot2 = "vfsr_data_traffic.gp"
    eps1 = "vfsr_data_delay.eps"
    eps2 = "vfsr_data_traffic.eps"
    csv_output = "vfsr_data.txt"

    with open(gnuplot1, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps1 + "\'\n")
        
        inf.write("set ylabel 'Convergence Time (ms)'\n")
        inf.write("set xlabel 'Participant Number'\n")
        inf.write("set xrange[1:34]\n")
        inf.write("set xtics (2,4,8,16,32)\n")
        inf.write("set yrange[400:800]\n")
        inf.write("set key top vertical right box 3\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
        plot_command = "plot \'" + csv_output + "\'i 0 u 1:2 w lp lw 5 t \'One Source\', " + \
        "\'" + csv_output + "\'i 1 u 1:2 w lp lw 5 t \'Two Source\', " + \
        "\'" + csv_output + "\'i 2 u 1:2 w lp lw 5 t \'Three Source\' " 

        inf.write(plot_command)
        inf.write("\n")

    print "ploting...", eps1
    subprocess.call("gnuplot " + gnuplot1, shell=True)

    with open(gnuplot2, 'w') as inf:
        inf.write("set t postscript enhanced eps color\n")
        inf.write("set output \'" + eps2 + "\'\n")
        
        inf.write("set ylabel 'Peak Controller Traffic (B/s)'\n")
        inf.write("set xlabel 'Participant Number'\n")
        inf.write("set xrange[1:34]\n")
        inf.write("set yrange[1:1200]\n")
        inf.write("set xtics (2,4,8,16,32)\n")
        inf.write("set key top vertical right box 3\n")
        inf.write("set grid ytics lc rgb '#bbbbbb' lw 1 lt 0\n")
        inf.write("set grid xtics lc rgb '#bbbbbb' lw 1 lt 0\n")
        plot_command = "plot \'" + csv_output + "\'i 0 u 1:3 w lp lw 5 t \'One Source\', " + \
        "\'" + csv_output + "\'i 1 u 1:3 w lp lw 5 t \'Two Source\', " + \
        "\'" + csv_output + "\'i 2 u 1:3 w lp lw 5 t \'Three Source\' " 

        inf.write(plot_command)
        inf.write("\n")

    print "ploting...", eps2
    subprocess.call("gnuplot " + gnuplot2, shell=True)

    




