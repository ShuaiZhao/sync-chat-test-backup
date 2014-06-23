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

using namespace ns3;

/**
 * This scenario simulates a one-node two-app scenario:
 *
 *      +------+ <-----> (CustomApp1)
 *      | Node |
 *      +------+ <-----> (CustomApp2)
 *
 *     NS_LOG=CustomApp ./waf --run=ndn-simple-with-custom-app
 */

int main(int argc, char *argv[])
{
	// Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
	CommandLine cmd;
	cmd.Parse(argc, argv);

	// Creating nodes
	Ptr<Node> node = CreateObject<Node> ();
	node->GetId();
	Names::Add("Node1", node);
	std::string nodeName = Names::FindName(node);

	std::cout<<"node Name is "<< nodeName<<std::endl;

	// Install CCNx stack on all nodes
	ndn::StackHelper ccnxHelper;
	ccnxHelper.InstallAll();

	// Installing application
	ndn::AppHelper consumerHelper("CustomApp");
	ApplicationContainer app1 = consumerHelper.Install(node);
	ApplicationContainer app2 = consumerHelper.Install(node);

	app1.Start(Seconds(1.0)); //will send out Interest, which nobody will receive
	app2.Start(Seconds(2.0)); //will sent out an INterest, which will be received and satisficed by app1

	Simulator::Stop(Seconds(30.0));

	Simulator::Run();
	Simulator::Destroy();

	return 0;

}
