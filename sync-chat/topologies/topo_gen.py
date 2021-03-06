#!/usr/bin/python

"""
This is scripts is to generate topology file for the simulation, 
by reading the network configuaration file.

The configuration file has information such as the network type, 
number of nodes, number of porxy, number of UEs. 

Based on these information, this script will create a network 
that includes a number of UE clusters, a core network that 
contains proxies connected with UE clusters and controller. 

In order to run this script, python module networkx has to be installed
For ubuntu users, run apt-get install python-networkx

Author: Xuan Liu <xuan.liu@mail.umkc.edu>

"""
import random
import networkx as nx
import sys
from optparse import OptionParser
from copy import deepcopy

import matplotlib.pyplot as plt  # added by shuai

#global vairables 

CORE_LINK_ATTR = {'bw':'1000Mbps', 'metric': '1', 
                    'delay': '5ms', 'queue': '10000'}
EDGE_LINK_ATTR = {'bw':'1000Mbps', 'metric': '1', 
                    'delay': '5ms', 'queue':'10000'}
LOCAL_LINK_ATTR = {'bw':'100Mbps', 'metric': '1', 
                    'delay': '5ms', 'queue':'10000'}



def get_config(config_file):
    ''' Get network configuration info from the config file '''
    config_dict = {}
    f_handle = open(config_file, 'r')
    for line in f_handle:
        if line[0] == '#':
            #print line
            pass
        else:
            #print line
            words = line.split()
            config_dict[words[0]] = words[2]
    return config_dict


def create_option(parser):
    '''Add the options to the parser'''
    parser.add_option("-r", dest="config_file",
                    type="str", default="config.txt",
                    help="Read the network configuration file")
    parser.add_option("-w", dest="topo_file",
                    type="str", default="topoinfo.txt",
                    help="Generate the topology information file")
 

def core_network_gen(config_dict):
    ''' Genearte the core network topology '''
    num_router = int(config_dict['NumberOfRouters'])
    name_mapping = {}
    for i in range(num_router):
        name_mapping[i] = 'cr-' + str(i)
    # print name_mapping
    good = False
    while not good:
        core_net = nx.gnp_random_graph(num_router, 0.5, random.seed())
        good = is_valid(core_net)
    core_net = nx.relabel_nodes(core_net, name_mapping)
    add_link_attr(core_net, CORE_LINK_ATTR)
    print "core_net:", core_net
    return core_net


def add_link_attr(network_topo, attr_dict):
    ''' add link attributes to the network '''
    links = network_topo.edges()
    network_topo.add_edges_from(links, attr_dict)


def local_network_gen(config_dict):
    ''' Genearte the local network topology '''
    num_local_router = int(config_dict['RoutersPerCluster'])
    good = False
    while not good:
        local_net = nx.gnp_random_graph(num_local_router, 0.5, random.seed())
        good = is_valid(local_net)
    add_link_attr(local_net, LOCAL_LINK_ATTR)
    print "local_net:", local_net
    return local_net

def is_valid(core_net):
    ''' All nodes in the core_net should have degree value no less than 2 '''
    node_degree = core_net.degree(core_net.nodes())
    """{'cr-0': 2, 'cr-1': 2, 'cr-2': 2}"""
    check_list = [key for (key, value) in node_degree.items() if value < 2]
    if check_list != []:
        return False
    else:
        return True

def add_ctrl_nodes(core_net, config_dict):
    ''' Add proxy nodes to the edge of the core network '''
    num_proxy = int(config_dict['NumberOfProxies'])
    proxy_prefix = config_dict['ProxyPrefix']
    ctrler_prefix = config_dict['ControllerPrefix']
    new_node_list = []
    for index in range(num_proxy):
        new_node_list.append(proxy_prefix+str(index+1))
    new_node_list.append(ctrler_prefix)
    print new_node_list
    router_list = core_net.nodes()
    edge_routers = random.sample(set(router_list), len(new_node_list))
    print "edge_routes:", edge_routers
    new_links = []
    for i in range(len(new_node_list)):
        new_links.append((new_node_list[i], edge_routers[i]))
    # print new_links
    core_net.add_edges_from(new_links, EDGE_LINK_ATTR)
    print "core_net.edges:" , core_net.edges()

def add_ue(local_net, ue_sublist, local_routers):
    ''' Add UEs to the cluster network '''
    # access_routers = random.sample(set(local_routers), len(ue_sublist))
    new_links = []
    for i in range(len(ue_sublist)):
        random_router = random.choice(local_routers)
        # new_links.append((access_routers[i], ue_sublist[i]))
        new_links.append((random_router, ue_sublist[i]))
    local_net.add_edges_from(new_links, LOCAL_LINK_ATTR)
 

def get_ue_full_list(config_dict):
    ''' Create UE list based on the number of clusters (proxies) '''
    num_of_ue = int(config_dict['NumberOfUEs'])
    ue_prefix = config_dict['UEPrefix']
    ue_per_cluster = int(config_dict['UEsPerCluster'])
    ue_full_list = []
    count = 0
    gw_index = 1
    temp_sublist = []
    for index in range(num_of_ue):
        temp_sublist.append(ue_prefix+str(index + 1))
        count = count + 1
        if count == ue_per_cluster:
            temp_sublist.append('gw'+str(gw_index))
            count = 0
            ue_full_list.append(temp_sublist)
            temp_sublist = []
            gw_index = gw_index + 1
    print "ue_full_list:", ue_full_list
    return ue_full_list


def network_union(core_net, local_net):
    ''' Add links to connect core_net to local_net '''
    # note that the number of proxies is equal to the number of local networks
    full_network = nx.Graph()
    full_network = deepcopy(core_net)
    for i in range(len(local_net)):
        temp_proxy = 'P' + str(i+1)
        temp_gw = 'gw' + str(i+1)
        full_network.add_edge(temp_proxy, temp_gw, LOCAL_LINK_ATTR)
        full_network = nx.compose(full_network, local_net[i])
    return full_network

def write_topofile_heading(topo_file):
    '''generate the topology file '''
    f_handle = open(topo_file, 'w')
    heading_lines = ["# any empty lines and lines starting " \
            "with '#' symbol is ignored\n",
                     "# The file should contain exactly two sections: "\
                              "router and link,\n",
                     "# each starting with the corresponding keyword\n",
                     "\n",
                     "# router section defines topology nodes and " \
                                "their relative positions\n",
                     "# (e.g., to use in visualizer)\n\n"]
    for line in heading_lines:
        f_handle.writelines(line)
    f_handle.close()

def write_router_info(topo_file, full_network):
    ''' wirte router information to the topo file '''
    f_handle = open(topo_file, 'a')
    f_handle.writelines("router\n")
    f_handle.writelines("# each line in this section represents one" \
            "router and should have the following data: \n\n")
    f_handle.writelines("# nodeId\tcity\tyPos\txPos\n")
    nodes_sorted = full_network.nodes()
    nodes_sorted.sort()
    for node in nodes_sorted:
        node_info = node + '\t' + "NA\t0\t0" + '\n'
        f_handle.writelines(node_info)
    f_handle.writelines("\n\n")
    f_handle.close()


def write_link_info(topo_file, full_network):
    ''' write link information to the topo file '''
    f_handle = open(topo_file, 'a')
    f_handle.writelines("link\n\n")
    f_handle.writelines("# link section defines point-to-point links "\
            "between nodes and characteristics of these links: \n")
    f_handle.writelines(["# each line should be in the following format "\
            "(only first two are required, the rest can be ommitted)\n",
            "# bandwidth: link bandwidth\n",
            "# metric: routing metric\n",
            "# delay: link delay\n",
            "# queue: MaxPackets for transmission " \
            "queue on the link (both direction)\n",
            "# srcNode	dstNode		bandwidth	metric	delay	queue \n",
            ])
    for src, dst in full_network.edges():
        attr_list = [full_network[src][dst]['bw'], 
                     full_network[src][dst]['metric'],
                     full_network[src][dst]['delay'],
                     full_network[src][dst]['queue']]
        link_info = [src, dst]
        link_info.extend(attr_list)
        line = '\t'.join(link_info)
        f_handle.writelines(line + '\n')
    f_handle.close()
        
    
    
def run(config_dict, topo_file):
    ''' generate network '''
    # create the core network with proxies and controller at the edges
    core_net = core_network_gen(config_dict)
    """core_net is a networkx objects, core_net.nodes(). core_net.edges()..."""
    add_ctrl_nodes(core_net, config_dict)
    
    # create a full list of UEs, containing a number of ue_sublists. 
    ue_full_list = get_ue_full_list(config_dict)
    """ue_full_list: [['U1', 'U2', 'gw1'], ['U3', 'U4', 'gw2']]"""
    local_net = []
    for i in range(len(ue_full_list)):
        temp_net = local_network_gen(config_dict)
        local_router_mapping = {}
        for j in temp_net.nodes():
            local_router_mapping[j] = 'lc' + str(i) + '-r' + str(j)
        #print local_router_mapping
        temp_net = nx.relabel_nodes(temp_net, local_router_mapping)
        add_ue(temp_net, ue_full_list[i], temp_net.nodes())
        local_net.append(temp_net)
    full_network = network_union(core_net, local_net)

    write_topofile_heading(topo_file)
    write_router_info(topo_file, full_network)
    write_link_info(topo_file, full_network)
    
    print "full_network:", full_network
    return full_network



def main(argv = None):
    '''Program Wrapper'''
    if not argv:
        argv  = sys.argv[1:]
    usage = ("%prog [-r config_file] [-w topo_file]")
    parser = OptionParser(usage=usage)
    create_option(parser)
    (options, _) = parser.parse_args(argv)
    config_file = options.config_file
    topo_file = options.topo_file

    config_dict = get_config(config_file)
    """ reslt examples
    {'AccountPrefix': 'A',
     'ControllerPrefix': 'C',
     'NumberOfProxies': '2',
     'NumberOfRouters': '3',
     'NumberOfUEs': '4',
     'ProxyPrefix': 'P',
     'RoutersPerCluster': '3',
     'UEPrefix': 'U',
     'UEsPerCluster': '2'}
        
     """
    print "config_dict:", config_dict
    full_network = run(config_dict, topo_file)
    nx.draw(full_network)
    plt.savefig('ego_graph.png')
    plt.show()
    


if __name__ == '__main__':
    sys.exit(main())
