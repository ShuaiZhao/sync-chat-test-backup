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

#include "sync-chat-app.h"
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
#include <boost/lexical_cast.hpp>
#include <string>


NS_LOG_COMPONENT_DEFINE ("syncChatApp");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (syncChatApp);

// register NS-3 type
TypeId
syncChatApp::GetTypeId ()
{
  static TypeId tid = TypeId ("syncChatApp")
    .SetParent<ndn::App> ()
    .AddConstructor<syncChatApp> ()
    .AddAttribute("Prefix", "Prefix, for which producer has the data",
    				StringValue("/"),
    				ndn::MakeNameAccessor(&syncChatApp::m_nodePrefix),
    				ndn::MakeNameChecker())
    .AddAttribute("AccountID", "AccoundID for the Application",
       				StringValue("none"),
       				MakeStringAccessor(&syncChatApp::m_accountId),
      				MakeStringChecker())
    .AddAttribute("PayloadSize", "Virtual payload size for content packets",
    				StringValue("1024"),
    				MakeUintegerAccessor(&syncChatApp::m_virtualPayloadSize),
    				MakeUintegerChecker<uint32_t>())
    .AddAttribute("StartSeq", "Initial sequence number",
    				IntegerValue(0),
    				MakeIntegerAccessor(&syncChatApp::m_contentSeq),
    				MakeIntegerChecker<uint32_t>())
    .AddAttribute("Frequency", "Frequency of packets",
    				StringValue("1.0"),
    				MakeDoubleAccessor(&syncChatApp::m_frequency),
    				MakeDoubleChecker<double>())
    .AddAttribute("Randomize", "Type of send time randomization: none (default), uniform, exponential",
    				StringValue ("none"),
    				MakeStringAccessor (&syncChatApp::SetRandomize, &syncChatApp::GetRandomize),
    				MakeStringChecker ())
    .AddAttribute("SyncClientID", "The sync proxy ID the client is connected",
    				StringValue ("none"),
    				MakeStringAccessor (&syncChatApp::m_syncClientId),
    				MakeStringChecker ())
    ;
  return tid;
}

// constructor
syncChatApp::syncChatApp()
	: m_frequency(1.0)
	, m_start (true)
	, m_random(0)
	, m_lastReceivedContentSeq(-1)
	, m_recovery(false)
{
	NS_LOG_FUNCTION_NOARGS ();
}

// destructor
syncChatApp::~syncChatApp()
{
	if (m_random)
	    delete m_random;
}

//////////////////////////////////////////////////////////////////////////
//                    Start or Stop the application            			//
//////////////////////////////////////////////////////////////////////////

// Processing upon start of the application
void
syncChatApp::StartApplication ()
{
	NS_LOG_FUNCTION_NOARGS();
	NS_ASSERT(GetNode()->GetObject<ndn::Fib>()!=0);
	// initialize ndn::App
	ndn::App::StartApplication ();

	//NS_LOG_INFO("NodeID: " <<GetNode()->GetId());

	// Get Node Name
	Ptr<Node> node = GetNode();
	m_nodeName = Names::FindName(node);
	//NS_LOG_INFO("This node's name is "<<m_nodeName);

	Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();

	// Add application prefix to the fib
	Ptr<ndn::fib::Entry> fibEntry = fib->Add (m_nodePrefix, m_face, 0);

	fibEntry->UpdateStatus (m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);

	ScheduleNextContent();

}

// Processing when application is stopped
void
syncChatApp::StopApplication ()
{
	NS_LOG_FUNCTION_NOARGS ();
	NS_ASSERT (GetNode ()->GetObject<ndn::Fib> () != 0);
	// cancel periodic packet generation
	Simulator::Cancel (m_sendContentEvent);

	// cleanup ndn::App
	ndn::App::StopApplication ();
}

//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Content		           			//
//////////////////////////////////////////////////////////////////////////


// publish content locally, not to other remote service node
void syncChatApp::ScheduleNextContent()
{
	if (m_start)
	{
		m_contentSeq = 0; // join the chatroom
		m_sendContentEvent = Simulator::Schedule(Seconds(1.0),
				&syncChatApp::generateContent, this);
		NS_LOG_INFO(m_nodeName<<": Schedule the first chat content generating");
		m_start = false;
	}
	else if (!m_sendContentEvent.IsRunning())
	{
		m_contentSeq++;
		m_sendContentEvent = Simulator::Schedule(
										(m_random == 0) ?
											Seconds(1.0 / m_frequency)
										:
											Seconds(m_random->GetValue()),
										&syncChatApp::generateContent, this);
		//NS_LOG_INFO(m_nodeName<<": Send the content with sequence number: "<<m_contentSeq);
	}
}

//  Generate Content <contentPrefix, contentSize>
void syncChatApp::generateContent()
{
	// convert uint32_t to string
	
	std::stringstream ss;
	ss << m_contentSeq;
	std::string str_contentSeq;
	ss >> str_contentSeq;

	Ptr<ndn::Name> m_contentName = Create<ndn::Name> (m_nodePrefix);
	//NS_LOG_INFO(m_nodeName<<": New Content prefix is "<<*m_contentName);
	m_contentName->Add("chatroom-cona");
	m_contentName->Add(m_accountId);
	m_contentName->Add("chat");
	m_contentName->Add(str_contentSeq);


	//m_contentSize = m_random->GetInteger();
	m_contentSize = 1024;
	//std::cout<<"content size is as"<<m_contentSize<<std::endl;
	//NS_LOG_INFO(m_nodeName<<": Content prefix is "<<*m_contentName<<" and its size is "<<m_contentSize);
	//NS_LOG_INFO(m_nodeName<<": Content size is "<<m_contentSize);
	m_contentDict[*m_contentName] = m_contentSize;

	// write new generated content into file
	//int count;
	//count = WriteFile("contentdb.csv", m_contentDict);


	// Get FIB object
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();
	// add entry to FIB
	Ptr<ndn::fib::Entry> fibEntry = fib->Add(m_contentName, m_face, 0);
	fibEntry->UpdateStatus(m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);

	NS_LOG_INFO(m_nodeName<<": Chat Content sequence number is "<<m_contentSeq << " prefix: "
				<< *m_contentName << " size: "  << m_contentSize );

	//return *m_contentName;
	//NS_LOG_INFO(m_nodeName<<": Ready to generate notification prefix");
	GenerateNotificationPrefix(m_contentName);

}


//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Interest		           			//
//////////////////////////////////////////////////////////////////////////


ndn::InterestHeader syncChatApp::GenerateInterestHeader(ndn::Name m_interestName)
{
	// Create and configure ndn::InterestHeader
	ndn::InterestHeader interestHeader;
	UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
	interestHeader.SetNonce (rand.GetValue ());
	interestHeader.SetName (m_interestName);
	interestHeader.SetInterestLifetime (Seconds (1.0));
	return interestHeader;
}

//Sending one Interest packet out
void
syncChatApp::SendInterest (ndn::Name m_interestName)
{
  NS_LOG_INFO(m_nodeName<<": Start sending interest!");
  // Create and configure ndn::InterestHeader
  ndn::InterestHeader interestHeader;
  interestHeader = GenerateInterestHeader(m_interestName);
  // NS_LOG_INFO (m_nodeName<<": Generate interest for content: " << m_interestName);

  // Create packet and add ndn::InterestHeader
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (interestHeader);

  NS_LOG_INFO (m_nodeName<<": Sending Interest packet for " << m_interestName);


  //ndn::FwHopCountTag hopCountTag;
  //packet->AddPacketTag (hopCountTag);
  //NS_LOG_INFO("hopCountTag is "<<hopCountTag.Get());

  // Forward packet to lower (network) layer
  m_protocolHandler (packet);

  // Call trace (for logging purposes)
  m_transmittedInterests (&interestHeader, this, m_face);
}


//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Notification		           			//
//////////////////////////////////////////////////////////////////////////
void syncChatApp::GenerateNotificationPrefix(Ptr<ndn::Name> m_contentName)
{
	//S_LOG_INFO(m_nodeName<<": Start generating notification prefix:");
	m_outSyncNotificationPrefix.routing_prefix = m_syncClientId; // proxy01 on service router 01
	m_outSyncNotificationPrefix.serviceId = "chatroom-cona";
	m_outSyncNotificationPrefix.m_fingerprint = *m_contentName;

	//Take off the first '/' from the content name
	std::string str_fingerprint;
	std::stringstream ss;
	ss<<m_outSyncNotificationPrefix.m_fingerprint;
	ss>>str_fingerprint;
	str_fingerprint.erase(0,1);


	// form the notification interest
	Ptr<ndn::Name> m_syncPrefix = Create<ndn::Name> (m_outSyncNotificationPrefix.routing_prefix);
	//NS_LOG_INFO(m_nodeName<<": The notification prefix is "<<*m_syncPrefix);
	m_syncPrefix->Add(m_outSyncNotificationPrefix.serviceId);
	m_syncPrefix->Add(str_fingerprint);

	//NS_LOG_INFO(m_nodeName<<": The notification prefix is "<<*m_syncPrefix);
	//INFO notification name components
	/*BOOST_FOREACH(const std::string &component, m_syncPrefix->GetComponents())
	{
		NS_LOG_INFO(m_accountId<<" Notification components are: "<<component);
	}*/
	SendSyncNotification(m_syncPrefix);

}

ndn::InterestHeader syncChatApp::GenerateSyncNotificationHeader(Ptr<ndn::Name> m_syncPrefix)
{
	//NS_LOG_INFO(m_nodeName<<": Generate the notification header");
	//NS_LOG_INFO(m_nodeName<<": Sync Notification Prefix is "<<*m_syncPrefix);
	ndn::InterestHeader syncNotificationHeader;
	UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
	syncNotificationHeader.SetNonce (rand.GetValue ());
	syncNotificationHeader.SetName (m_syncPrefix);
	syncNotificationHeader.SetInterestLifetime (Seconds (5.0));
	return syncNotificationHeader;
}

void syncChatApp::SendSyncNotification(Ptr<ndn::Name> m_syncPrefix)
{
	ndn::InterestHeader syncNotificationHeader;
	syncNotificationHeader = GenerateSyncNotificationHeader(m_syncPrefix);

	NS_LOG_INFO(m_nodeName<<": Sending chat notification to sync client " << *m_syncPrefix);
	//NS_LOG_INFO(m_nodeName<<": Create the sync notification packet");
	// Create packet and add ndn::syncNotificationHeader
	Ptr<Packet> packet = Create<Packet> ();
	packet->AddHeader (syncNotificationHeader);

	//ndn::FwHopCountTag hopCountTag;
	//packet->AddPacketTag (hopCountTag);
	//NS_LOG_INFO("hopCountTag is "<<hopCountTag.Get());

	// Forward packet to lower (network) layer
	m_protocolHandler (packet);

	// Call trace (for logging purposes)
	m_transmittedInterests (&syncNotificationHeader, this, m_face);
	ScheduleNextContent();


}

////////////////////////////////////////////////////////////////////////
//          			Process incoming packets       			      //
////////////////////////////////////////////////////////////////////////


// Callback that will be called when Interest arrives
void
syncChatApp::OnInterest (const Ptr<const ndn::InterestHeader> &interest, Ptr<Packet> origPacket)
{
  ndn::Name interestName = interest->GetName();
  std::list<std::string> subPrefixList = interestName.GetComponents();

	std::stringstream ss2, temp;
	ss2 << interestName;
	std::string str_chatContentName;
	ss2 >> str_chatContentName;
	std::string delimiter = "/";

	std::string s;
	s=str_chatContentName;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
	    token = s.substr(0, pos);
	    //std::cout << token << std::endl;
	    s.erase(0, pos + delimiter.length());
	}
	//std::cout << "medie type: " << token <<  std::endl;
	//std::cout << str_chatContentName << std::endl;
	if(token == "video" || token== "audio")
		return;

  bool isNotification = false;

  int prefixSize = subPrefixList.size();
  if (prefixSize == 3) // notification sent by sync-client has three components
  {
	  isNotification = true;
	  //NS_LOG_INFO(m_nodeName<<": Received a sync notification from sync-client");
	  OnNotification(interest, origPacket);
	  return;
  }


  if (!isNotification)
  {
	  if (m_contentDict.find(interestName) == m_contentDict.end())
	  {
		  // not found
		  NS_LOG_INFO(m_nodeName<<": There is no content matching the interest with name "<< interestName);
	  }
	  else
	  {
		NS_LOG_INFO (m_nodeName<<": Received content request Interest packet " << interestName);
	 	ndn::ContentObjectHeader contentData;
	 	contentData.SetName (Create<ndn::Name> (interestName)); // data will have the same name as Interests

	 	ndn::ContentObjectTail trailer; // doesn't require any configuration

	 	// Create packet and add header and trailer
	 	Ptr<Packet> packet = Create<Packet> (1024);
	 	packet->AddHeader (contentData);
	 	packet->AddTrailer (trailer);

	 	//NS_LOG_INFO (m_nodeName<<": Sending ContentObject packet for " << contentData.GetName ());

	 	// Forward packet to lower (network) layer
	 	m_protocolHandler (packet);

	 	// Call trace (for logging purposes)
	 	m_transmittedContentObjects (&contentData, packet, this, m_face);
	  }
  }
}


// Callback that will be called when Data arrives
void
syncChatApp::OnContentObject (const Ptr<const ndn::ContentObjectHeader> &contentObject,
                            Ptr<Packet> payload)
{
  ndn::Name received_prefix = contentObject->GetName();
  
	// get prefix components
	std::string delimiter = "/";
	std::stringstream ss2;
	ss2 << received_prefix;
	std::string str_prefixComponents;
	ss2 >> str_prefixComponents;

	std::string s;
	s=str_prefixComponents;
	size_t pos = 0;
	std::string media_typeToken;
	std::string last_token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		media_typeToken = s.substr(0, pos);
	    //NS_LOG_INFO(m_nodeName<<": token " << media_typeToken);
	    s.erase(0, pos + delimiter.length());
	}

	//std::string num_str = boost::lexical_cast<std::string>(i);

	//get last token

	last_token = s.substr(0, pos);

	if (!m_recovery){
		m_lastSavedToken = boost::lexical_cast<uint32_t>( last_token );
	}
	uint32_t x_save = m_lastSavedToken;

	std::size_t found = 0;
	found = str_prefixComponents.find("/U1/");
	//NS_LOG_INFO(m_nodeName <<": found: " << found);

	//NS_LOG_INFO (m_nodeName<<": Receiving Chat ContentObject packet for " 
		//					<< str_prefixComponents );

	if(found!=std::string::npos){
		if ( m_lastSavedToken-m_lastReceivedContentSeq ==1)
		{
				m_lastReceivedContentSeq ++;
				NS_LOG_INFO (m_nodeName<<": Receiving Chat ContentObject packet for " 
							<< str_prefixComponents << " this token: " << m_lastSavedToken << " m_lastReceivedContentSeq: "
						<< m_lastReceivedContentSeq);
			}
			else {
					//resend interest for lost package
					//m_interestName_lost = received_prefix;
				m_recovery = true;
				//m_lastSavedToken = m_lastReceivedContentSeq;
				for(int i=m_lastReceivedContentSeq; i<=x_save; i++){
						Ptr<ndn::Name> m_lostInterestName = Create<ndn::Name> ("U1");
						//NS_LOG_INFO(m_nodeName<<": New Content prefix is "<<*m_lostInterestName);
						m_lostInterestName->Add("chatroom-cona");
						m_lostInterestName->Add("A1");
						m_lostInterestName->Add("chat");
						m_lostInterestName->Add(i);

						//NS_LOG_INFO (m_nodeName<<": recover" << " this token: " << m_lastSavedToken 
						//<< " m_lastReceivedContentSeq: " << m_lastReceivedContentSeq << " lost interestName: " << *m_lostInterestName) ;
						m_resendInterestEvent = Simulator::Schedule(Seconds(0.01),
									&syncChatApp::SendInterest, this, *m_lostInterestName);
						//SendInterest(*m_lostInterestName);
					}
					m_recovery = false;
					m_lastReceivedContentSeq = x_save;
					NS_LOG_INFO (m_nodeName<<": Recover Chat ContentObject packet for " 
							<< str_prefixComponents );
			}
		}
	else{
		NS_LOG_INFO (m_nodeName<<": Receiving Chat ContentObject packet for " 
							<< str_prefixComponents );
	}
}

void
syncChatApp::OnNotification(const Ptr<const ndn::InterestHeader> &interest,
		Ptr<Packet> origPacket)
{
	ndn::Name remote_FP;
	remote_FP = interest->GetName().GetLastComponent();
	NS_LOG_INFO(m_nodeName<<": Received Notification for "<<interest->GetName());
	//NS_LOG_INFO(m_nodeName<<": The content name is: "<< remote_FP);
	//ndn::Name new_interestPrefix;
	ndn::Name m_interestName = remote_FP;
	SendInterest(m_interestName);  // send new interest for content
}


/**
* @brief Set type of frequency randomization
* @param value Either 'none', 'uniform', or 'exponential'
*/
void syncChatApp::SetRandomize (const std::string &value)
{
	if (m_random)
		delete m_random;
	if (value == "uniform")
	{
		m_random = new UniformVariable (0.0, 2 * 1.0 / m_frequency);
	}
	else if (value == "exponential")
	{
		m_random = new ExponentialVariable (1.0 / m_frequency, 50 * 1.0 / m_frequency);
	}
	else
		m_random = 0;

	m_randomType = value;
}


/**
* @brief Get type of frequency randomization
* @returns either 'none', 'uniform', or 'exponential'
*/
std::string
syncChatApp::GetRandomize() const
{
	return m_randomType;
}


//////////////////////////////////////
//  	CSV File Read & Write  		//
//////////////////////////////////////

// Write to CSV file
int syncChatApp::WriteFile(std::string fname, std::map<ndn::Name, uint32_t> m_contentDict)
{
	if (m_contentDict.empty())
		return 0;
	std::ofstream myfile;
	myfile.open(fname.c_str(), std::ios::out | std::ios::app);
	if (!myfile)
		return -1;

	for (std::map<ndn::Name, uint32_t>::iterator it=m_contentDict.begin();
			it != m_contentDict.end(); it++)
	{
		myfile<<it->first<<","<<it->second;
	}
	myfile.close();
	return 0;
}

// Read CSV file into map
std::map<ndn::Name, std::string> syncChatApp::ReadFile(std::string fname)
{
	std::ifstream infile(fname.c_str());
	std::string line = "";
	std::map<ndn::Name, std::string> my_contentDict;
	while(getline(infile, line))
	{
		std::vector<std::string> key_value;
		std::stringstream strstr(line);
		std::string word;
		ndn::Name temp_Name;
		while(getline(strstr, word, ','))
		{
			key_value.push_back(word);
		}
		temp_Name(key_value[0]);
		my_contentDict[temp_Name] = key_value[1];
	}
	return my_contentDict;
}

} // ns3
