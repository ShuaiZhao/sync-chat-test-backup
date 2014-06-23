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

//#include "ns3/ndn-link-control-helper.h"

#include "ns3/ndnSIM/utils/tracers/ndn-l3-rate-tracer.h"
#include "ns3/ndnSIM/utils/tracers/ndn-l3-aggregate-tracer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>

using namespace ns3;

  
////////////////////////////////////////////////////////////////  setup ///////////////////////////////////////////
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
			std::cout<<key<<" "<<value<<std::endl;
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
		std::cout<<temp<<std::endl;
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

void FailLink (Ptr<Node> node1, Ptr<Node> node2){
	//NS_LOG_FUNCTION (node1 << node2);

	  NS_ASSERT (node1 != 0);
	  NS_ASSERT (node2 != 0);

	  Ptr<ndn::L3Protocol> ndn1 = node1->GetObject<ndn::L3Protocol> ();
	  Ptr<ndn::L3Protocol> ndn2 = node2->GetObject<ndn::L3Protocol> ();

	  NS_ASSERT (ndn1 != 0);
	  NS_ASSERT (ndn2 != 0);

	  // iterate over all faces to find the right one
	  for (uint32_t faceId = 0; faceId < ndn1->GetNFaces (); faceId++)
	    {
	      Ptr<ndn::NetDeviceFace> ndFace = ndn1->GetFace (faceId)->GetObject<ndn::NetDeviceFace> ();
	      if (ndFace == 0) continue;

	      Ptr<PointToPointNetDevice> nd1 = ndFace->GetNetDevice ()->GetObject<PointToPointNetDevice> ();
	      if (nd1 == 0) continue;

	      Ptr<Channel> channel = nd1->GetChannel ();
	      if (channel == 0) continue;

	      Ptr<PointToPointChannel> ppChannel = DynamicCast<PointToPointChannel> (channel);

	      Ptr<NetDevice> nd2 = ppChannel->GetDevice (0);
	      if (nd2->GetNode () == node1)
	        nd2 = ppChannel->GetDevice (1);

	      if (nd2->GetNode () == node2)
	        {
	          Ptr<ndn::Face> face1 = ndn1->GetFaceByNetDevice (nd1);
	          Ptr<ndn::Face> face2 = ndn2->GetFaceByNetDevice (nd2);

	          face1->SetUp (false);
	          face2->SetUp (false);
	          break;
	        }
	    }
}
void
UpLink (Ptr<Node> node1, Ptr<Node> node2)
{
  //NS_LOG_FUNCTION (node1 << node2);

  NS_ASSERT (node1 != 0);
  NS_ASSERT (node2 != 0);

  Ptr<ndn::L3Protocol> ndn1 = node1->GetObject<ndn::L3Protocol> ();
  Ptr<ndn::L3Protocol> ndn2 = node2->GetObject<ndn::L3Protocol> ();

  NS_ASSERT (ndn1 != 0);
  NS_ASSERT (ndn2 != 0);

  // iterate over all faces to find the right one
  for (uint32_t faceId = 0; faceId < ndn1->GetNFaces (); faceId++)
    {
      Ptr<ndn::NetDeviceFace> ndFace = ndn1->GetFace (faceId)->GetObject<ndn::NetDeviceFace> ();
      if (ndFace == 0) continue;

      Ptr<PointToPointNetDevice> nd1 = ndFace->GetNetDevice ()->GetObject<PointToPointNetDevice> ();
      if (nd1 == 0) continue;

      Ptr<Channel> channel = nd1->GetChannel ();
      if (channel == 0) continue;

      Ptr<PointToPointChannel> ppChannel = DynamicCast<PointToPointChannel> (channel);

      Ptr<NetDevice> nd2 = ppChannel->GetDevice (0);
      if (nd2->GetNode () == node1)
        nd2 = ppChannel->GetDevice (1);

      if (nd2->GetNode () == node2)
        {
          Ptr<ndn::Face> face1 = ndn1->GetFaceByNetDevice (nd1);
          Ptr<ndn::Face> face2 = ndn2->GetFaceByNetDevice (nd2);

          face1->SetUp (true);
          face2->SetUp (true);
          break;
        }
    }
}

int main (int argc, char *argv[])
{
	/////////////////////////////////                 configuration files                                  /////////////////////////////////////////////////
	std::string topo_path = "topologies/";
	std::string topo_filename = "config.txt";
	std::string config_filename = "";
	std::string gop_size_filename="/home/shuai/workspace/ndnsim-sync-chat/sync-chat/results/log/gop_size.txt";
	double sim_time = 0.0;




/////////////////////////////////                 cmd arguments                                    /////////////////////////////////////////////////
    CommandLine cmd;
    // filename has to be given along the path from the parent directory.
    cmd.AddValue("TopoInfoFile", "Topology File", topo_filename);
    cmd.AddValue("ConfigInfo", "network configuration file", config_filename);
    cmd.AddValue("SimulationTime", "simulation time", sim_time);
   //cmd.AddValue("GopSizeInfo", "GoP size file", gop_size_filename );
    cmd.Parse (argc, argv);

    // config.txt's path
    std::string config_path = topo_path + config_filename;
    std::cout<<"Config information is imported from "<< config_path <<std::endl;

/////////////////////////////////                network topologyr                                     /////////////////////////////////////////////////
    // Parse the topology file
    std::map<std::string, std::string> configInfo;
    configInfo = parseConfigFile(topo_path+config_filename);  //configInfo is a map: (configuratonParas, value)
    std::vector<std::string> ue_list, proxy_list, router_list, account_list;
    ue_list = GetNameList(configInfo["UEPrefix"], configInfo["NumberOfUEs"]);  // return( U1, U2, U3....)
    proxy_list = GetNameList(configInfo["ProxyPrefix"], configInfo["NumberOfProxies"]); // return (P1, P2,...)
    account_list = GetNameList(configInfo["AccountPrefix"], configInfo["NumberOfUEs"]); // return (A1, A2..)
    int max_local_ue = atoi(configInfo["UEsPerCluster"].c_str());     // Max UE for each cluster

    // printing debug information
    std::cout << "topo_path:" << topo_path << std::endl;
    std::cout << "topo_filename:" << topo_filename << std::endl;
    std::cout << "config_filename:" << config_filename << std::endl;
    PrintNames(ue_list);
    PrintNames(proxy_list);
    PrintNames(account_list);
    std::cout << "max_local_ue:" << max_local_ue << std::endl;

    std::string controllerId = configInfo["ControllerPrefix"];    // return 'C'
    //std::cout << "controllerId:" <<  controllerId << std::endl;

    AnnotatedTopologyReader topologyReader ("", 100);
    topologyReader.SetFileName(topo_path+topo_filename);
    topologyReader.Read ();

/////////////////////////////////                Nodes                                     /////////////////////////////////////////////////
    
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

///////////////////////////////////////////                 routing                               //////////////////////////////////////////////////////
    // Installing global routing interface on all nodes
    ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
    ndnGlobalRoutingHelper.InstallAll ();
    ndnGlobalRoutingHelper.Install(Controller);
    ndnGlobalRoutingHelper.Install(ProxyNodes);
    ndnGlobalRoutingHelper.Install(UE);


    // Add / prefix origins to ndn::GlobalRouter
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

//////////////////////////////////                 Application Container                                     //////////////////////////////////////////////////
    // Calculate and install FIBs
    ndnGlobalRoutingHelper.CalculateRoutes ();

    // installing sync controller to Controller node
    ndn::AppHelper syncControllerAppHelper("SyncController");
    syncControllerAppHelper.SetPrefix("C");
    syncControllerAppHelper.SetAttribute("NumOfProxy", IntegerValue(proxy_list.size()));
    ApplicationContainer syncController = syncControllerAppHelper.Install(Controller.Get(0));

    ApplicationContainer syncProxy;
    ApplicationContainer syncClient;
    ApplicationContainer chatApp;
    ApplicationContainer videoApp;
    ApplicationContainer audioApp;
    ApplicationContainer startApp;

//////////////////////////////////////////////////////      Proxy + client    /////////////////////////////////////////////////////////////////////////
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
        	std::cout<<"Track: "<<tempUEId<<"<->"<<tempProxyId<<"<->"<<local_ue_count<<"<->"<<ue_index<<"<->"<<max_local_ue<<std::endl;
        	local_ue_count++;
        	ue_index++;
        }
    }



//////////////////////////////////////////////////////      Start/Stop App    /////////////////////////////////////////////////////////////////////////
int leave_time = sim_time;

    
 	for (int i = 0;i<ue_list.size();i++)
  	{
  		std::string tempUEId = ue_list[i];
  	  	ndn::AppHelper startAppHelper("SyncStartApp");
  	  	startAppHelper.SetPrefix(tempUEId);
  	  	startAppHelper.SetAttribute("AccountID", StringValue(account_list[i]));
  	    startAppHelper.SetAttribute("Frequency", StringValue("5"));
  	    startAppHelper.SetAttribute("PayloadSize", StringValue("5"));
  	    startAppHelper.SetAttribute("StartSeq", IntegerValue(0));
  	    startAppHelper.SetAttribute("SyncClientID", StringValue(tempUEId+"/sync"));
  	    startAppHelper.SetAttribute("LeaveTime", IntegerValue(leave_time) );
  	  	startApp.Add(startAppHelper.Install(UE.Get(i)));
  	 }



////////////////////////////////////////////////  chat app /////////////////////////////////////////

    // Installing chat applications to UE nodes
    // Installing chat applications to UE nodes
 /*
    for (int i = 0;i<ue_list.size();i++)
    {
      // all chating through text
    	std::string tempUEId = ue_list[i];
        ndn::AppHelper chatAppHelper("syncChatApp");
        chatAppHelper.SetPrefix(tempUEId);
        chatAppHelper.SetAttribute("AccountID", StringValue(account_list[i]));
        chatAppHelper.SetAttribute("Frequency", StringValue("4"));
        chatAppHelper.SetAttribute("PayloadSize", StringValue("1024"));
        chatAppHelper.SetAttribute("StartSeq", IntegerValue(0));
        chatAppHelper.SetAttribute("SyncClientID", StringValue(tempUEId+"/sync"));
        chatApp.Add(chatAppHelper.Install(UE.Get(i)));
     }


*/
//////////////////////////////////////////////////////      AudioApp    /////////////////////////////////////////////////////////////////////////

   /*
      std::string audio_interArrivalRate = "0.012";  // 30ms
      uint32_t audio_packetSize = 136;  //bytes per packet

      for (int i = 0;i<ue_list.size();i++)
  	  	{
    		//if( i ==ue_list.size()/proxy_list.size()-1 ||  i ==ue_list.size()/proxy_list.size()*2-1 ||
    			//	  i ==ue_list.size()/proxy_list.size()*3-1){				// 2. three audio sources
  			//if ( i%2 == 0){																	// 3. half audio sources
  			 if (i==0) {   																		// 1. one audio source
							std::cout << "Audio Source: " << i+1 << std::endl;
							std::string tempUEId = ue_list[i];
							ndn::AppHelper audioAppHelper("SyncAudioApp");
							audioAppHelper.SetPrefix(tempUEId);
							audioAppHelper.SetAttribute("AccountID", StringValue(account_list[i]));
							audioAppHelper.SetAttribute("PayloadSize", IntegerValue(audio_packetSize));
							audioAppHelper.SetAttribute("StartSeq", IntegerValue(0));
							audioAppHelper.SetAttribute("SyncClientID", StringValue(tempUEId+"/sync"));
							audioAppHelper.SetAttribute("LeaveTime", IntegerValue(leave_time) );
							audioAppHelper.SetAttribute("InterArrival", StringValue(audio_interArrivalRate) );
							audioAppHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.352]") );
							audioAppHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.65]") );
							audioApp.Add(audioAppHelper.Install(UE.Get(i)));
  			}
  			else{
  			 	// audio receive,
							std::cout << "Audio Receiver: " << i+1 << std::endl;
							std::string tempUEId = ue_list[i];
							ndn::AppHelper audioAppHelper("SyncAudioApp");
							audioAppHelper.SetPrefix(tempUEId);
							audioAppHelper.SetAttribute("AccountID", StringValue(account_list[i]));
							audioAppHelper.SetAttribute("PayloadSize", IntegerValue(audio_packetSize));
							audioAppHelper.SetAttribute("StartSeq", IntegerValue(0));
							audioAppHelper.SetAttribute("SyncClientID", StringValue(tempUEId+"/sync"));
							audioAppHelper.SetAttribute("LeaveTime", IntegerValue(1000) );
							audioAppHelper.SetAttribute("InterArrival", StringValue("0.012") );
							audioAppHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.36]") );
							audioAppHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.64]") );
							audioApp.Add(audioAppHelper.Install(UE.Get(i)));
					}
  		  }
*/
//////////////////////////////////////////////////////      Videoapp Node /////////////////////////////////////////////////////////////////////////


	std::string video_interArrivalRate = "0.012";  // 30ms
	for (int i = 0;i<ue_list.size();i++)
  	  	{
		     // if( i ==ue_list.size()/proxy_list.size()-1 ||  i ==ue_list.size()/proxy_list.size()*2-1 ||
  				//    i ==ue_list.size()/proxy_list.size()*3-1){					// 2. three sources
            
				  //if( i%2 == 0){																	// 3. half sources.
			   if (i==0  )  {																			// 1. one source
					std::cout << "Video Source: " << i +1 << std::endl;
					std::string tempUEId = ue_list[i];
					ndn::AppHelper videoAppHelper("SyncVideoApp");
					videoAppHelper.SetPrefix(tempUEId);
					videoAppHelper.SetAttribute("AccountID", StringValue(account_list[i]));
					videoAppHelper.SetAttribute("PayloadSize", IntegerValue(1010));
					videoAppHelper.SetAttribute("StartSeq", IntegerValue(0));
					videoAppHelper.SetAttribute("SyncClientID", StringValue(tempUEId+"/sync"));
					videoAppHelper.SetAttribute("LeaveTime", IntegerValue(leave_time) );
					videoAppHelper.SetAttribute("InterArrival", StringValue(video_interArrivalRate) );
					videoAppHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.48]") );
					videoAppHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0001]") );
					videoApp.Add(videoAppHelper.Install(UE.Get(i)));
				//videoApp.Get(i)->SetStartTime(Seconds(3.0));
  			}
		else{
					std::cout << "Video Receiver: " << i +1 << std::endl;
					std::string tempUEId = ue_list[i];
					ndn::AppHelper videoAppHelper("SyncVideoApp");
					videoAppHelper.SetPrefix(tempUEId);
					videoAppHelper.SetAttribute("AccountID", StringValue(account_list[i]));
					videoAppHelper.SetAttribute("PayloadSize", IntegerValue(0));
					videoAppHelper.SetAttribute("StartSeq", IntegerValue(0));
					videoAppHelper.SetAttribute("SyncClientID", StringValue(tempUEId+"/sync"));
					videoAppHelper.SetAttribute("LeaveTime", IntegerValue(leave_time) );
					videoAppHelper.SetAttribute("InterArrival", StringValue(video_interArrivalRate) );
					videoAppHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.48]") );
					videoAppHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.00001]") );
					videoApp.Add(videoAppHelper.Install(UE.Get(i)));
			}
  	 }



//////////////////////////    setStartTime for controller, proxy, client, StartApp ////////////////////////////////////////////////
  	// Start controller
  	syncController.Start(Seconds(0.0));

  	// Start proxy
  	for (int i = 0; i<proxy_list.size(); i++)
  	{
  		syncProxy.Get(i)->SetStartTime(Seconds(0.0));
  	}

  	// start client
  	for (int i = 0; i<ue_list.size(); i++)
  	{
  		syncClient.Get(i)->SetStartTime(Seconds(0.0));
  	}

/*

	   // start App
  	for (int i = 0; i<ue_list.size(); i++)
  	{
  		startApp.Get(i)->SetStartTime(Seconds(1.0));
  	}

*/

//////////////////////////     chat app       ////////////////////////////////////////////////////

/*
    // start chat
    for (int i = 0; i<ue_list.size(); i++)
    {
      chatApp.Get(i)->SetStartTime(Seconds(2.0));
    }
*/

//////////////////////////    setStartTime for  audio app ////////////////////////////////////////////////

 /*
     // start audio
  	for (int i = 0; i<ue_list.size(); i++)
  	{
  		//if(i ==ue_list.size()/proxy_list.size()-1 ||  i ==ue_list.size()/proxy_list.size()*2-1 ||
  			//  i ==ue_list.size()/proxy_list.size()*3-1 )     // 2. three souces
  		//if ( i%2 == 0)                                        //3. half source
      if(i==0)                                            // 1 one source
  			audioApp.Get(i)->SetStartTime(Seconds(2.0));
  		else
  			audioApp.Get(i)->SetStartTime(Seconds(2.0));
  	}
  */

//////////////////////////    setStartTime for  video app ////////////////////////////////////////////////

	// start video
 	for (int i = 0; i<ue_list.size(); i++)
  	{
  	    //if(i ==ue_list.size()/proxy_list.size()-1 ||  i ==ue_list.size()/proxy_list.size()*2-1 ||
  			// i ==ue_list.size()/proxy_list.size()*3-1 )    // 2. three souces
  		//
      //if(i%2==0)                                         // 3 half source
     if(i==0)                                            //1. one source
      //if(i ==ue_list.size()/proxy_list.size()*3-1)
  			videoApp.Get(i)->SetStartTime(Seconds(3.0));
  		else
  			videoApp.Get(i)->SetStartTime(Seconds(3.0));
  	}

//////////////////////////   link failer  ////////////////////////////////////////////////

  //Ptr<Node> node_1 = Names::Find<Node> ("gw1");
  //Ptr<Node> node_2 = Names::Find<Node> ("P1");

  //Ptr<Node> node_1 = Names::Find<Node> ("gw3");
  //Ptr<Node> node_2 = Names::Find<Node> ("P3");

   
  Ptr<Node> node_1 = Names::Find<Node> ("U1");
  Ptr<Node> node_2 = Names::Find<Node> ("lc0-r3");

 // Simulator::Schedule (Seconds (5.0), FailLink, node_1, node_2);
 // Simulator::Schedule (Seconds (7.0), UpLink,  node_1, node_2);



//////////////////////////////////////////////////////      Begin  Simulation  ////////////////////////////////////////////////
  	Simulator::Stop(Seconds(sim_time));
    boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::L3RateTracer> > > 
        rateTracers = ndn::L3RateTracer::InstallAll ("topo_10_proxies_3000UE_20_5_1GC_100ML_oneSource_pipeline_video-rate-trace.txt", Seconds (0.1));
    boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::L3AggregateTracer> > > 
        aggTracers = ndn::L3AggregateTracer::InstallAll ("topo_10_proxies_300UE_20_5_1GC_100ML_oneSource_pipeline_video-aggregate-trace.txt", Seconds (0.1));


    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
