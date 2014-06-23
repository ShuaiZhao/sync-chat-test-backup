/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Xuan Liu <xuan.liu@mail.umkc.edu>
 */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/point-to-point-module.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>

using namespace ns3;

std::map<std::string, std::string> parseConfigFile(std::string config_filename)
{
	std::map<std::string, std::string> configInfo;
	std::ifstream config_file_open;
	config_file_open.open(config_filename.c_str());
	if (!config_file_open.is_open() || !config_file_open.good())
	{
		std::cout<<"Cannot open file "<<config_filename<<" for reading."<<std::endl;
	}
	while (!config_file_open.eof())
	{
		std::string line, key, value;
		std::getline(config_file_open, line);
		std::vector<std::string> key_value;
		std::string word;
		if (line[0] == '#')
		{
			//std::cout<<"This is comment line."<<std::endl;
			continue;
		}
		else
		{
			std::stringstream strstr(line);
			while(getline(strstr, word, ' '))
			{
				key_value.push_back(word);
			}
			key = key_value[0];
			value = key_value[2];
			//std::cout<<key<<" "<<value<<std::endl;
			configInfo[key] = value;
		}
	}
	return configInfo;
}


std::vector<std::string> GetNameList(std::string name, std::string count)
{
	std::vector<std::string> nameList;
	int num = atoi(count.c_str());
	for(int i = 1; i<=num; i++)
	{
		std::string num_str = boost::lexical_cast<std::string>(i);
		std::string temp = name+num_str;
		//std::cout<<temp<<std::endl;
		nameList.push_back(temp);
	}

	return nameList;

}

void PrintNames(std::vector<std::string> nameList)
{
	for(int i = 0; i<nameList.size(); i++)
	{
		std::cout<<nameList[i]<<", ";
	}
	std::cout<<std::endl;
}

int main (int argc, char *argv[])
{
	std::string topo_path = "topologies/";
	std::string topo_filename = "test_topo.txt";
	std::string config_filename = "config.txt";
	double sim_time = 5.0;

    CommandLine cmd;
    // filename has to be given along the path from the parent directory.
    cmd.AddValue("TopoInfoFile", "Topology File", topo_filename);
    cmd.AddValue("ConfigInfo", "network configuration file", config_filename);
    cmd.AddValue("SimulationTime", "simulation time", sim_time);
    cmd.Parse (argc, argv);



    std::cout<<"Config information is imported from "<<topo_path + config_filename<<std::endl;

    // Parse the topology file
    std::map<std::string, std::string> configInfo;
    configInfo = parseConfigFile(topo_path+config_filename);
    std::vector<std::string> ue_list, proxy_list, router_list, account_list;
    ue_list = GetNameList(configInfo["UEPrefix"], configInfo["NumberOfUEs"]);
    proxy_list = GetNameList(configInfo["ProxyPrefix"], configInfo["NumberOfProxies"]);
    account_list = GetNameList(configInfo["AccountPrefix"], configInfo["NumberOfUEs"]);
    int max_local_ue = atoi(configInfo["UEsPerCluster"].c_str());

    PrintNames(ue_list);
    PrintNames(proxy_list);
    PrintNames(account_list);

    std::string controllerId = configInfo["ControllerPrefix"];

    AnnotatedTopologyReader topologyReader ("", 25);
    topologyReader.SetFileName(topo_path+topo_filename);
    topologyReader.Read ();

    // Install Ndn stack on all nodes
    ndn::StackHelper ndnHelper;
    ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
    //ndnHelper.SetDefaultRoutes(true);
    ndnHelper.InstallAll ();

    NodeContainer Controller, ProxyNodes, UE, routers;
    Controller.Add(Names::Find<Node> (controllerId));

    for (int i = 0; i<proxy_list.size(); i++)
    {
	    std::string proxyId = proxy_list[i];
	    std::cout<<proxyId<<std::endl;
	    ProxyNodes.Add(Names::Find<Node> (proxyId));
    }

    for (int i = 0; i<ue_list.size(); i++)
    {
  	    std::string ueId = ue_list[i];
  	    std::cout<<ueId<<std::endl;
  	    UE.Add(Names::Find<Node> (ueId));
    }

    for (int i = 0; i<router_list.size(); i++)
    {
   	    std::string routerId = router_list[i];
   	    std::cout<<routerId<<std::endl;
   	    routers.Add(Names::Find<Node> (routerId));
    }

    // Installing global routing interface on all nodes
    ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
    ndnGlobalRoutingHelper.InstallAll ();
    ndnGlobalRoutingHelper.Install(Controller);
    ndnGlobalRoutingHelper.Install(ProxyNodes);
    ndnGlobalRoutingHelper.Install(UE);


    // Add /prefix origins to ndn::GlobalRouter
  	ndnGlobalRoutingHelper.AddOrigins ("C", Controller.Get(0));

    for (int i = 0; i<ue_list.size();i++)
    {
	    std::string ueId = ue_list[i];
	    ndnGlobalRoutingHelper.AddOrigins (ueId, UE.Get(i));
    }


    for (int i = 0; i<proxy_list.size();i++)
    {
  	    std::string proxyId = proxy_list[i];
  	    ndnGlobalRoutingHelper.AddOrigins (proxyId, ProxyNodes.Get(i));
    }

    // Calculate and install FIBs
    ndnGlobalRoutingHelper.CalculateRoutes ();

    // installing sync controller to Controller node
    ndn::AppHelper syncControllerAppHelper("SyncController");
    syncControllerAppHelper.SetPrefix("C");
    syncControllerAppHelper.SetAttribute("NumOfProxy", IntegerValue(proxy_list.size()));
    ApplicationContainer syncController = syncControllerAppHelper.Install(Controller.Get(0));

    ApplicationContainer syncProxy;
    ApplicationContainer syncClient;
    ApplicationContainer ChatApp;

    int ue_index = 0;


    for (int i = 0;i<proxy_list.size();i++)
    {
    	int local_ue_count = 0;
        std::string tempProxyId = proxy_list[i];
        ndn::AppHelper syncProxyAppHelper("SyncProxy");
        syncProxyAppHelper.SetPrefix(tempProxyId);
        syncProxyAppHelper.SetAttribute("ControllerID", StringValue("C"));
        syncProxy.Add(syncProxyAppHelper.Install(ProxyNodes.Get(i)));
        while (local_ue_count < max_local_ue)
        {
        	std::string tempUEId = ue_list[ue_index];
        	ndn::AppHelper syncClientAppHelper("SyncClient");
        	syncClientAppHelper.SetPrefix(tempUEId+"/sync");
        	syncClientAppHelper.SetAttribute("ProxyID", StringValue(tempProxyId));
        	syncClient.Add(syncClientAppHelper.Install(UE.Get(ue_index)));
        	std::cout<<"Track: "<<tempUEId<<" "<<tempProxyId<<" "<<local_ue_count<<" "<<ue_index<<" "<<max_local_ue<<std::endl;
        	local_ue_count++;
        	ue_index++;
        }
    }


  	// Installing chat applications to UE nodes
  	for (int i = 0;i<ue_list.size();i++)
  	{
  		std::string tempUEId = ue_list[i];
  	  	ndn::AppHelper chatAppHelper("syncChatApp");
  	  	chatAppHelper.SetPrefix(tempUEId);
  	  	chatAppHelper.SetAttribute("AccountID", StringValue(account_list[i]));
  	    chatAppHelper.SetAttribute("Frequency", StringValue("2"));
  	    chatAppHelper.SetAttribute("PayloadSize", StringValue("1024"));
  	    chatAppHelper.SetAttribute("StartSeq", IntegerValue(0));
  	    chatAppHelper.SetAttribute("SyncClientID", StringValue(tempUEId+"/sync"));
  	  	ChatApp.Add(chatAppHelper.Install(UE.Get(i)));
  	 }


  	// Start applications
  	syncController.Start(Seconds(0.0));
  	for (int i = 0; i<proxy_list.size(); i++)
  	{
  		syncProxy.Get(i)->SetStartTime(Seconds(0.0));
  	}

  	for (int i = 0; i<ue_list.size(); i++)
  	{
  		syncClient.Get(i)->SetStartTime(Seconds(0.0));
  		ChatApp.Get(i)->SetStartTime(Seconds(1.0));
  	}

  	Simulator::Stop(Seconds(sim_time));

    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
