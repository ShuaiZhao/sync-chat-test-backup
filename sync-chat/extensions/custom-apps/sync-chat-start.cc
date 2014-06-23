/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 University of California, Los Angeles
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
 * Modified from ndn-consumer.h, ndn-producer.h and custom-app.h
 * Author: Xuan Liu <xuan.liu@mail.umkc.edu>
 *
 */

// sync-chat-app.cc

#include "sync-chat-start.h"
#include "sync-client.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"

#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/random-variable.h"
#include "ns3/core-module.h"

#include "ns3/ndn-name.h"
#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-content-object.h"
#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndn-fib.h"

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <boost/foreach.hpp>

NS_LOG_COMPONENT_DEFINE("SyncStartApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(SyncStartApp);

// register NS-3 type
TypeId SyncStartApp::GetTypeId() {
	static TypeId tid =
			TypeId("SyncStartApp")
.SetParent<ndn::App>()
.AddConstructor<SyncStartApp>().AddAttribute("Prefix",
					"Prefix, for which producer has the data", StringValue("/"),
					ndn::MakeNameAccessor(&SyncStartApp::m_nodePrefix),
					ndn::MakeNameChecker())
.AddAttribute("AccountID",
					"AccoundID for the Application", StringValue("none"),
					MakeStringAccessor(&SyncStartApp::m_accountId),
					MakeStringChecker())
.AddAttribute("PayloadSize",
					"Virtual payload size for content packets",
					StringValue("1024"),
					MakeUintegerAccessor(&SyncStartApp::m_virtualPayloadSize),
					MakeUintegerChecker<uint32_t>())
.AddAttribute("StartSeq",
					"Initial sequence number", IntegerValue(0),
					MakeIntegerAccessor(&SyncStartApp::m_contentSeq),
					MakeIntegerChecker<uint32_t>())
.AddAttribute("Frequency",
					"Frequency of packets", StringValue("1.0"),
					MakeDoubleAccessor(&SyncStartApp::m_frequency),
					MakeDoubleChecker<double>())
.AddAttribute("Randomize",
					"Type of send time randomization: none (default), uniform, exponential",
					StringValue("none"),
					MakeStringAccessor(&SyncStartApp::SetRandomize,
							&SyncStartApp::GetRandomize), MakeStringChecker())
.AddAttribute(
					"SyncClientID", "The sync proxy ID the client is connected",
					StringValue("none"),
					MakeStringAccessor(&SyncStartApp::m_syncClientId),
					MakeStringChecker())
.AddAttribute("LeaveTime",
							"When User will leave", IntegerValue(3),
							MakeIntegerAccessor(&SyncStartApp::m_leaveTime),
							MakeIntegerChecker<uint32_t>());
	return tid;
}

// constructor
SyncStartApp::SyncStartApp() :
		m_frequency(0.25), m_start(true), m_random() {
	NS_LOG_FUNCTION_NOARGS ();
}

// destructor
SyncStartApp::~SyncStartApp() {
	if (m_random)
		delete m_random;
}

//////////////////////////////////////////////////////////////////////////
//                    Start or Stop the application            			//
//////////////////////////////////////////////////////////////////////////

// Processing upon start of the application
void SyncStartApp::StartApplication() {
	NS_LOG_FUNCTION_NOARGS();
	NS_ASSERT(GetNode()->GetObject<ndn::Fib>() != 0);
	// initialize ndn::App
	ndn::App::StartApplication();

	//NS_LOG_DEBUG("NodeID: " <<GetNode()->GetId());

	// Get Node Name
	Ptr<Node> node = GetNode();
	m_nodeName = Names::FindName(node);
	//NS_LOG_DEBUG("This node's name is "<<m_nodeName);

	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();

	// Add application prefix to the fib
	Ptr<ndn::fib::Entry> fibEntry = fib->Add(m_nodePrefix, m_face, 0);

	fibEntry->UpdateStatus(m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);

	NS_LOG_DEBUG("This node's prefix "<<m_nodePrefix);
	ScheduleNextContent();

}

// Processing when application is stopped
void SyncStartApp::StopApplication() {
	NS_LOG_FUNCTION_NOARGS ();
	NS_ASSERT(GetNode()->GetObject<ndn::Fib>() != 0);
	// cancel periodic packet generation
	Simulator::Cancel(m_sendContentEvent);

	// cleanup ndn::App
	ndn::App::StopApplication();
}

//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Content		           			//
//////////////////////////////////////////////////////////////////////////

//  Generate Content <contentPrefix, contentSize>
void SyncStartApp::generateContent() {
	// convert uint32_t to string;
	NS_LOG_INFO("GenerateContent()");
	std::stringstream ss2;
	ss2 << m_ContentSeq;
	std::string str_ContentSeq;
	ss2 >> str_ContentSeq;

	Ptr<ndn::Name> joinCotentName = Create<ndn::Name>(m_nodePrefix);
	joinCotentName->Add("chatroom-cona");
	joinCotentName->Add(m_accountId);
	joinCotentName->Add("Join");
	joinCotentName->Add(str_ContentSeq);

	m_contentSize = HelloPacketSize();

	//NS_LOG_INFO(m_nodeName<<": Join  prefix is "<<*joinCotentName<<" and its size is "<<m_contentSize);
	m_contentDict[*joinCotentName] = m_contentSize;

	// Get FIB object
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();

	Ptr<ndn::fib::Entry> fibEntry2 = fib->Add(joinCotentName, m_face, 0);
	fibEntry2->UpdateStatus(m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);
	GenerateNotificationPrefix(joinCotentName);
}

//  Generate Content <contentPrefix, contentSize>
void SyncStartApp::generateHelloContent() {
	// convert uint32_t to string;
	NS_LOG_INFO("GenerateHelloContent()");
	Ptr<ndn::Name> joinCotentName = Create<ndn::Name>(m_nodePrefix);
	joinCotentName->Add("chatroom-cona");
	joinCotentName->Add(m_accountId);
	joinCotentName->Add("Hello");

	m_contentSize = HelloPacketSize();

	//NS_LOG_INFO(m_nodeName<<": Hello  prefix is "<<*joinCotentName<<" and its size is "<<m_contentSize);
	m_contentDict[*joinCotentName] = m_contentSize;

	// Get FIB object
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();

	Ptr<ndn::fib::Entry> fibEntry2 = fib->Add(joinCotentName, m_face, 0);
	fibEntry2->UpdateStatus(m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);
	GenerateNotificationPrefix(joinCotentName);
}

//  Generate Content <contentPrefix, contentSize>
void SyncStartApp::generateLeaveContent() {
	// convert uint32_t to string;
	NS_LOG_INFO("GenerateLeaveContent()");
	Ptr<ndn::Name> joinCotentName = Create<ndn::Name>(m_nodePrefix);
	joinCotentName->Add("chatroom-cona");
	joinCotentName->Add(m_accountId);
	joinCotentName->Add("Leave");

	m_contentSize = HelloPacketSize();

	//NS_LOG_INFO(m_nodeName<<": Hello  prefix is "<<*joinCotentName<<" and its size is "<<m_contentSize);
	m_contentDict[*joinCotentName] = m_contentSize;

	// Get FIB object
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();

	Ptr<ndn::fib::Entry> fibEntry2 = fib->Add(joinCotentName, m_face, 0);
	fibEntry2->UpdateStatus(m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);
	GenerateNotificationPrefix(joinCotentName);
	//Simulator::Stop();
	SyncStartApp::StopApplication();
}

// publish content locally, not to other remote service node
void SyncStartApp::ScheduleNextContent() {
	if (m_start) {
		//added by shuai
		/*m_videoContentSeq = 0;*/
		m_ContentSeq=0;
		m_sendContentEvent = Simulator::Schedule(Seconds(1.0),
				&SyncStartApp::generateContent, this);
		//NS_LOG_INFO(m_nodeName<<": Schedule the join content generating");
		m_start = false;
		NS_LOG_INFO("Current Simulator Time: " << Simulator::Now().GetSeconds());
	} else if (!m_sendContentEvent.IsRunning()) {
		m_ContentSeq++;
		hello_time_interval = 5;  //3s

		m_sendContentEvent = Simulator::Schedule(Seconds(hello_time_interval),
				&SyncStartApp::generateHelloContent, this);
		NS_LOG_INFO("Current Simulator Time: " << Simulator::Now().GetSeconds() << " Leaving System time set to: " << m_leaveTime);

		if( Simulator::Now().GetSeconds() > m_leaveTime){
			m_sendContentEvent = Simulator::Schedule(Seconds(1.0),
							&SyncStartApp::generateLeaveContent, this);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Interest		           			//
//////////////////////////////////////////////////////////////////////////

ndn::InterestHeader SyncStartApp::GenerateInterestHeader(
		ndn::Name m_interestName) {
	// Create and configure ndn::InterestHeader
	ndn::InterestHeader interestHeader;
	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
	interestHeader.SetNonce(rand.GetValue());
	interestHeader.SetName(m_interestName);
	interestHeader.SetInterestLifetime(Seconds(1.0));
	NS_LOG_INFO(m_nodeName<<": InterestHeader " << interestHeader);
	return interestHeader;
}

//Sending one Interest packet out
void SyncStartApp::SendInterest(ndn::Name m_interestName) {
	NS_LOG_INFO(m_nodeName<<": Start sending interest! " << m_interestName);
	//NS_LOG_INFO(m_nodeName<<": Interest Name" << m_interestName);
	// Create and configure ndn::InterestHeader
	ndn::InterestHeader interestHeader;
	interestHeader = GenerateInterestHeader(m_interestName);
	// NS_LOG_DEBUG (m_nodeName<<": Generate interest for content: " << m_interestName);

	// Create packet and add ndn::InterestHeader
	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(interestHeader);

	NS_LOG_INFO (m_nodeName<<": Sending Interest packet for " << m_interestName);

	//ndn::FwHopCountTag hopCountTag;
	//packet->AddPacketTag (hopCountTag);
	//NS_LOG_DEBUG("hopCountTag is "<<hopCountTag.Get());

	// Forward packet to lower (network) layer
	m_protocolHandler(packet);

	// Call trace (for logging purposes)
	m_transmittedInterests(&interestHeader, this, m_face);
}

//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Notification		           			//
//////////////////////////////////////////////////////////////////////////
void SyncStartApp::GenerateNotificationPrefix(Ptr<ndn::Name> m_contentName) {
	m_outSyncNotificationPrefix.routing_prefix = m_syncClientId;
	m_outSyncNotificationPrefix.serviceId = "chatroom-cona";
	m_outSyncNotificationPrefix.m_fingerprint = *m_contentName;

	//Take off the first '/' from the content name
	std::string str_fingerprint;
	std::stringstream ss;
	ss << m_outSyncNotificationPrefix.m_fingerprint;
	ss >> str_fingerprint;
	str_fingerprint.erase(0, 1);

	// form the notification interest
	Ptr<ndn::Name> m_syncPrefix = Create<ndn::Name>(
			m_outSyncNotificationPrefix.routing_prefix);
	//NS_LOG_INFO(m_nodeName<<": The notification prefix is "<<*m_syncPrefix);
	m_syncPrefix->Add(m_outSyncNotificationPrefix.serviceId);
	m_syncPrefix->Add(str_fingerprint);

	//NS_LOG_INFO(m_nodeName<<": Now: The notification prefix is "<<*m_syncPrefix);
	SendSyncNotification(m_syncPrefix);

}

void SyncStartApp::SendSyncNotification(Ptr<ndn::Name> m_syncPrefix) {
	//NS_LOG_DEBUG(m_nodeName<<": Sending hello notification to sync client");
	ndn::InterestHeader syncNotificationHeader;
	syncNotificationHeader = GenerateSyncNotificationHeader(m_syncPrefix);

	// Create packet and add ndn::syncNotificationHeader
	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(syncNotificationHeader);

	// Forward packet to lower (network) layer
	m_protocolHandler(packet);

	// Call trace (for logging purposes)
	m_transmittedInterests(&syncNotificationHeader, this, m_face);
	ScheduleNextContent();

}

ndn::InterestHeader SyncStartApp::GenerateSyncNotificationHeader(
		Ptr<ndn::Name> m_syncPrefix) {
	//NS_LOG_INFO(m_nodeName<<": Generate the notification header");
	//NS_LOG_DEBUG(m_nodeName<<": Sync Notification Prefix is "<<*m_syncPrefix);
	ndn::InterestHeader syncNotificationHeader;
	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
	syncNotificationHeader.SetNonce(rand.GetValue());
	syncNotificationHeader.SetName(m_syncPrefix);
	syncNotificationHeader.SetInterestLifetime(Seconds(1.0));
	NS_LOG_INFO(m_nodeName<<": notification header " << syncNotificationHeader );
	return syncNotificationHeader;
}


////////////////////////////////////////////////////////////////////////
//          			Process incoming packets       			      //
////////////////////////////////////////////////////////////////////////

// Callback that will be called when Interest arrives
void SyncStartApp::OnInterest(const Ptr<const ndn::InterestHeader> &interest,
		Ptr<Packet> origPacket) {
	//NS_LOG_INFO (m_nodeName<<": Received Interest packet  " << interest->GetName ());
}

// Callback that will be called when Data arrives
void SyncStartApp::OnContentObject(
		const Ptr<const ndn::ContentObjectHeader> &contentObject,
		Ptr<Packet> payload) {
	//NS_LOG_INFO (m_nodeName<<": Receiving Hello ContentObject packet for " << contentObject->GetName ());

	//std::cout << "DATA received for name " << contentObject->GetName () << std::endl;

	/*int hopCount = -1;
	 ndn::FwHopCountTag hopCountTag;
	 if (payload->RemovePacketTag (hopCountTag))
	 {
	 hopCount = hopCountTag.Get ();
	 NS_LOG_DEBUG( "hopCount is " << hopCount );
	 }*/
}
void SyncStartApp::OnNotification(const Ptr<const ndn::InterestHeader> &interest,
		Ptr<Packet> origPacket) {
	/*
	ndn::Name remote_FP;
	remote_FP = interest->GetName().GetLastComponent();
	NS_LOG_INFO(m_nodeName<<": Received Notification for "<<interest->GetName());
	//NS_LOG_DEBUG(m_nodeName<<": The content name is: "<< remote_FP);
	//ndn::Name new_interestPrefix;
	ndn::Name m_interestName = remote_FP;
	SendInterest(m_interestName);  // send new interest for content*/
}

/**
 * @brief Set type of frequency randomization
 * @param value Either 'none', 'uniform', or 'exponential'
 */
void SyncStartApp::SetRandomize(const std::string &value) {
	if (m_random)
		delete m_random;
	if (value == "uniform") {
		m_random = new UniformVariable(0.0, 2 * 1.0 / m_frequency);
	} else if (value == "exponential") {
		m_random = new ExponentialVariable(1.0 / m_frequency,
				50 * 1.0 / m_frequency);
	} else
		m_random = 0;

	m_randomType = value;
}

/**
 * @brief Get type of frequency randomization
 * @returns either 'none', 'uniform', or 'exponential'
 */
std::string SyncStartApp::GetRandomize() const {
	return m_randomType;
}


/*
 * A test function
 */
int  SyncStartApp::HelloPacketSize(){
	int hello_packet_size = 10;
	return hello_packet_size;
}

//////////////////////////////////////
//  	CSV File Read & Write  		//
//////////////////////////////////////

// Write to CSV file
int SyncStartApp::WriteFile(std::string fname,
		std::map<ndn::Name, uint32_t> m_contentDict) {
	if (m_contentDict.empty())
		return 0;
	std::ofstream myfile;
	myfile.open(fname.c_str(), std::ios::out | std::ios::app);
	if (!myfile)
		return -1;

	for (std::map<ndn::Name, uint32_t>::iterator it = m_contentDict.begin();
			it != m_contentDict.end(); it++) {
		myfile << it->first << "," << it->second;
	}
	myfile.close();
	return 0;
}

// Read CSV file into map
std::map<ndn::Name, std::string> SyncStartApp::ReadFile(std::string fname) {
	std::ifstream infile(fname.c_str());
	std::string line = "";
	std::map<ndn::Name, std::string> my_contentDict;
	while (getline(infile, line)) {
		std::vector<std::string> key_value;
		std::stringstream strstr(line);
		std::string word;
		ndn::Name temp_Name;
		while (getline(strstr, word, ',')) {
			key_value.push_back(word);
		}
		temp_Name(key_value[0]);
		my_contentDict[temp_Name] = key_value[1];
	}
	return my_contentDict;
}

} // ns3
