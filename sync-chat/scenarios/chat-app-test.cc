/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
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
 * This is a example from ndn-simple-with-custom-app.cc
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

/**
 * This scenario simulates a one-node two-app scenario:
 *
 *       +----------+   	      +--------+             +----------+
 * 		 | consumer | <---------> | router | <---------> | producer |
 * 		 +----------+             +--------+             +----------+
 *
 *     NS_LOG=myChatApp ./waf --run=chat-app-test
 */

// Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize

int main(int argc, char *argv[])
{
	// setting default parameters for PointToPoint links and channels
	Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
	Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
	Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));

	// Read optional command-line parameters (e.g., enable visualizaer with ./waf -- run=<> --visualize)
	CommandLine cmd;
	cmd.Parse(argc, argv);

	// Creating nodes

	// Creating nodes
	NodeContainer nodes;
	nodes.Create(2);
	Names::Add("Node1", nodes.Get(0));
	Names::Add("Node2", nodes.Get(1));

	//Creating routers
	NodeContainer routers;
	routers.Create(1);
	// Connecting nodes using two links
	PointToPointHelper p2p;
	p2p.Install(nodes.Get(0), routers.Get(0));
	p2p.Install(routers.Get(0), nodes.Get(1));
	// Install NDN stack on all nodes
	ndn::StackHelper ndnHelper;
	ndnHelper.SetDefaultRoutes(true);
	ndnHelper.InstallAll();

	// Installing application
	ndn::AppHelper chat1AppHelper("myChatApp");
	chat1AppHelper.SetPrefix("node01");
	chat1AppHelper.SetAttribute("AccountID", StringValue("Alice"));
	chat1AppHelper.SetAttribute("Frequency", StringValue("2"));
	chat1AppHelper.SetAttribute("PayloadSize", StringValue("1024"));
	chat1AppHelper.SetAttribute("StartSeq", IntegerValue(0));
	chat1AppHelper.SetAttribute("ProxyID", StringValue("node02"));
	ApplicationContainer app1 = chat1AppHelper.Install(nodes.Get(0));

	ndn::AppHelper chat2AppHelper("myChatApp");
	chat2AppHelper.SetPrefix("node02");
	chat2AppHelper.SetAttribute("AccountID", StringValue("Bob"));
	chat2AppHelper.SetAttribute("Frequency", StringValue("2"));
	chat2AppHelper.SetAttribute("PayloadSize", StringValue("1024"));
	chat2AppHelper.SetAttribute("StartSeq", IntegerValue(0));
	chat2AppHelper.SetAttribute("ProxyID", StringValue("node01"));
	ApplicationContainer app2 = chat2AppHelper.Install(nodes.Get(1));

	app1.Start(Seconds(1.0));
	app2.Start(Seconds(2.0));

	Simulator::Stop(Seconds(4.0));

	Simulator::Run();
	Simulator::Destroy();

	return 0;
}





