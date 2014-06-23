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
// sync-client.cc

#include "sync-client.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/system-mutex.h"

#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/random-variable.h"
#include "ns3/core-module.h"

#include "ns3/ndn-name.h"
#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-content-object.h"
//#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndn-fib.h"
#include <boost/foreach.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <boost/foreach.hpp>
#include <algorithm>


NS_LOG_COMPONENT_DEFINE ("SyncClient");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (SyncClient);

// register NS-3 type
TypeId
SyncClient::GetTypeId()
{
	static TypeId tid = TypeId ("SyncClient")
	    .SetParent<ndn::App> ()
	    .AddConstructor<SyncClient> ()
	    .AddAttribute("Prefix", "Prefix for the client",
	    			StringValue("/"),
	    			ndn::MakeNameAccessor(&SyncClient::m_clientPrefix),
	        		ndn::MakeNameChecker())
	    .AddAttribute("ProxyID", "The proxy's identifier that the client is connected to",
	    			StringValue("none"),
	    			MakeStringAccessor(&SyncClient::m_proxyId),
	    			MakeStringChecker())
	    ;
	return tid;
}



// constructor
SyncClient::SyncClient()
	: m_frequency(1.0)
	, m_start (true)
	, m_random(0)
{
	NS_LOG_FUNCTION_NOARGS ();

}

// destructor
SyncClient::~SyncClient()
{
	if (m_random)
	    delete m_random;
}


// Processing upon start of the application
void
SyncClient::StartApplication ()
{
	NS_LOG_FUNCTION_NOARGS();
	NS_ASSERT(GetNode()->GetObject<ndn::Fib>()!=0);
	// initialize ndn::App
	ndn::App::StartApplication ();

	//NS_LOG_DEBUG("NodeID: " <<GetNode()->GetId());

	// Get Client Name
	Ptr<Node> node = GetNode();
	m_nodeName = Names::FindName(node);
	m_clientId = m_nodeName+"/sync";
	NS_LOG_INFO(m_clientId<<": Start Application");

	InitDigestTree(); // Initialize the digest tree
	InitialDigestLog();	// Initialize the digest log


	Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();

	// Add sync client prefix to fib
	Ptr<ndn::fib::Entry> fibEntry = fib->Add (m_clientPrefix, m_face, 0);

	fibEntry->UpdateStatus (m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);


}



// Processing when application is stopped
void
SyncClient::StopApplication ()
{
	NS_LOG_FUNCTION_NOARGS ();
	NS_ASSERT (GetNode ()->GetObject<ndn::Fib> () != 0);
	// cancel periodic packet generation
	//Simulator::Cancel (m_sendContentEvent);

	// cleanup ndn::App
	NS_LOG_INFO(m_clientId<<": Application Stops");
	ndn::App::StopApplication ();
}



////////////////////////////////////////////////////////////////////////
//          			Process incoming interest packets             //
////////////////////////////////////////////////////////////////////////


// Callback that will be called when Interest arrives
void
SyncClient::OnInterest (const Ptr<const ndn::InterestHeader> &interest, Ptr<Packet> origPacket)
{
	NS_LOG_INFO (m_clientId<<": Received Interest packet for " << interest->GetName ());

 	ndn::Name notificationPrefix = interest->GetName();
 	std::string senderType = GetSenderType(notificationPrefix);
 	std::string notificationType;
 	Ptr<ndn::Name> newArrival_FP = Create<ndn::Name> () ;

 	*newArrival_FP = notificationPrefix.GetLastComponent();
 	NS_LOG_DEBUG(m_clientId<<"New arrival fp is "<<*newArrival_FP);
 	//std::cout<<"Last Comp: "<<newArrival_FP->GetLastComponent()<<std::endl;
 	if (senderType == "app")
 	{
 		NS_LOG_INFO(m_clientId<<": Received new fingerprint from application");
 		// Does not update digest tree and digest log
 		PushNotification(newArrival_FP, senderType);
 	}
 	else if (senderType == "proxy")
 	{
 		NS_LOG_INFO(m_clientId<<": Received new fingerprint from "<<m_proxyId);
 		notificationType = GetNotificationType(notificationPrefix);
 		bool new_update;
 		//NS_LOG_DEBUG(m_clientId<<": Notification is "<<notificationType);
 		if (notificationType != "Empty")
	  	{

	  		//DigestTree *newLogEntryKey = new DigestTree();
	  		DigestNode *dr = m_clientDigestTree.GetRoot();
	  		std::string dr_pre = SearchNameComponent(5, notificationPrefix);
	  		std::string dr_curr = SearchNameComponent(6, notificationPrefix);
	  		//std::cout<<m_clientId<<" Compare: "<<dr->GetData()->getDigest()<<" "<<dr_pre<<std::endl;
	  		//std::cout<<m_clientId<<" Compare: "<<dr->GetData()->getDigest()<<" "<<dr_curr<<std::endl;
	  		if (dr->GetData()->getDigest() == dr_pre)
	  		{
	  			NS_LOG_INFO(m_clientId<<": Match!");
	  			// There is no update missing
	  			dr->GetData()->RetrieveDigest(dr_curr);
	  			//std::cout<<m_clientId<<" Now: dr is "<<dr->GetData()->getDigest()<<std::endl;
	  			// Update Digest Log
	  			new_update = UpdateDigestLog(senderType, newArrival_FP);
	  			NS_LOG_INFO(m_clientId<<": New root digest is "<<dr->GetData()->getDigest());
	  			NS_LOG_INFO(m_clientId<<": fpUpdateDB size is "<<m_fpUpdateDB.size());
	  			NS_LOG_INFO(m_clientId<<": Digest log size is "<<m_keyForProxy.size());

	  			//TraverseLog();
	  		}
	  		else
	  		{
	  			NS_LOG_INFO(m_clientId<<": Not Match!");
	  			if (dr->GetData()->getDigestInfo()->trackSeq == "0")
	  			{
	  				NS_LOG_INFO(m_clientId<<": This is a new join, discard old updates");
	  				dr->GetData()->RetrieveDigest(dr_curr);
	  				//std::cout<<m_clientId<<" Now: dr is "<<dr->GetData()->getDigest()<<std::endl;
	  				// Update Digest Log
	  				new_update = UpdateDigestLog(senderType, newArrival_FP);
	  				NS_LOG_INFO(m_clientId<<": New root digest is "<<dr->GetData()->getDigest());
	  				NS_LOG_INFO(m_clientId<<": fpUpdateDB size is "<<m_fpUpdateDB.size());
	  				NS_LOG_INFO(m_clientId<<": Digest log size is "<<m_keyForProxy.size());
	  				//TraverseLog();
	  			}
	  			// Enter recovery mode, send requests for old fingerprints from controller
	  		}
	  	}
 		if (new_update)
 			PushNotification(newArrival_FP, senderType);
 	}
 	else
 	{

 	}
 	//NS_LOG_DEBUG(m_clientId<<": Received Notification for"<<interest->GetName());
 	//NS_LOG_DEBUG(m_clientId<<": The content name is: "<< *newArrival_FP);


}

//Determine the type of receiver of the new notification
std::string SyncClient::GetSenderType(ndn::Name notificationPrefix)
{
	std::list<std::string> subPrefixList = notificationPrefix.GetComponents();
	int prefixSize = subPrefixList.size();
	//debug
	NS_LOG_INFO(m_clientId<<": Notification is: "<<notificationPrefix<<" and Received notification size is "<<prefixSize);
	/*BOOST_FOREACH(const std::string &component, notificationPrefix.GetComponents())
	{
		NS_LOG_DEBUG(m_clientId<<" Notification components are: "<<component);
	}*/

	if (prefixSize == 4 ) // notification prefix sent by application has 4 components
		//new notification should be sent to the proxy
		{
			NS_LOG_INFO(m_clientId<<" Notification sender is app");
			return "app";
		}

	else if (prefixSize == 7) // notification prefix sent by client has 7 components
		//new notification should be sent to client
		{
		NS_LOG_INFO(m_clientId<<" Notification sender is proxy");
			return "proxy";
		}
	else
	{
		NS_LOG_INFO(m_clientId<<": Notification sender is none");
		return "none";
	}

}


//Determine the notification type: join/leave/update
std::string SyncClient::GetNotificationType(ndn::Name notificationPrefix)
{
	std::string notificationType = "Empty";
	BOOST_FOREACH(const std::string &component, notificationPrefix.GetComponents())
	{
		if (component == "join" || component == "leave" || component == "update")
		{
			notificationType = component;
			break;
		}
	 }
	return notificationType;
}


//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Notification		           		//
//////////////////////////////////////////////////////////////////////////
void SyncClient::PushNotification(Ptr<ndn::Name> m_fingerprint, std::string SenderType)
{
	NS_LOG_DEBUG(m_clientId<<":Start to generate notification prefix"<<*m_fingerprint);

	if (SenderType == "none")
	{
		NS_LOG_INFO(m_clientId<<": Notification is wrong!");
		return;
	}
	if (SenderType == "proxy")  //If a notification is received from either controller or client, proxy will push new notification to clients
	{
		NS_LOG_INFO(m_clientId<<": Sender is the proxy, and push new FP "<<*m_fingerprint<<" to application");

		//NS_LOG_DEBUG(m_clientId<<": new fp is from"<<m_fingerprint->GetComponents().front()<<", and the local node is"<<m_nodeName);
		if (m_nodeName == m_fingerprint->GetComponents().front())
		{
			NS_LOG_INFO(m_clientId<<": The is fp is generated locally, not from users under other proxies");
		}
		else
		{
			m_toAppPrefix.routing_prefix = m_nodeName;
			m_toAppPrefix.serviceId = "chatroom-cona";
			m_toAppPrefix.m_fingerprint = *m_fingerprint;
			//NS_LOG_DEBUG(m_clientId<<": fingerprint is "<<m_toAppPrefix.m_fingerprint);
			//Take off the first '/' from the content name
			std::string str_fingerprint;
			std::stringstream ss;
			ss<<m_toAppPrefix.m_fingerprint;
			ss>>str_fingerprint;
			str_fingerprint.erase(0,1);
			//NS_LOG_DEBUG(m_clientId<<": fingerprint is "<<str_fingerprint);
			// form the notification interest
			Ptr<ndn::Name> m_syncToAppPrefix = Create<ndn::Name> (m_toAppPrefix.routing_prefix);
			//NS_LOG_DEBUG(m_clientId<<": The notification to application prefix is "<<*m_syncToAppPrefix);
			m_syncToAppPrefix->Add(m_toAppPrefix.serviceId);
			m_syncToAppPrefix->Add(str_fingerprint);

			NS_LOG_INFO(m_clientId<<": The notification prefix to application is "<<*m_syncToAppPrefix);

			SendSyncNotification(m_syncToAppPrefix);
		}
	}
	if (SenderType == "app")  //If a notification is received from client, proxy will push new notification to controller
	{
		NS_LOG_INFO(m_clientId<<": Push notification to the Proxy");
		m_toProxySyncPrefix.routing_prefix = m_proxyId;
		//NS_LOG_DEBUG(m_clientId<<": The notification prefix to proxy is "<<m_toProxySyncPrefix.routing_prefix);
		m_toProxySyncPrefix.serviceId = "chatroom-cona";
		m_toProxySyncPrefix.proxyId = "VM01";

		// Parse Msg Type
		std::string str_contentSeq = m_fingerprint->GetLastComponent();
		//std::cout<<"Content Seq: "<<str_contentSeq<<std::endl;
		if (str_contentSeq == "0") // join
		{
			m_toProxySyncPrefix.msg_type = "join";
		}
		else if (str_contentSeq == "-1")  // leave
		{
			m_toProxySyncPrefix.msg_type = "leave";
		}
		else						// normal case, send messages
		{
			m_toProxySyncPrefix.msg_type = "update";
		}
		m_toProxySyncPrefix.dr = m_clientDigestTree.GetRoot()->GetData()->getDigest();

		m_toProxySyncPrefix.m_fingerprint = *m_fingerprint;
		//Take off the first '/' from the content name
		std::string str_fingerprint;
		std::stringstream ss;
		ss<<m_toProxySyncPrefix.m_fingerprint;
		ss>>str_fingerprint;
		str_fingerprint.erase(0,1);

		m_toProxySyncPrefix.dr = m_keyForProxy.back().digest.getDigest();

		// form the notification interest
		Ptr<ndn::Name> m_syncToProxyPrefix = Create<ndn::Name> (m_toProxySyncPrefix.routing_prefix);
		//NS_LOG_DEBUG(m_clientId<<": The notification prefix is "<<*m_syncToProxyPrefix);
		m_syncToProxyPrefix->Add(m_toProxySyncPrefix.proxyId);
		m_syncToProxyPrefix->Add(m_toProxySyncPrefix.serviceId);

		m_syncToProxyPrefix->Add(m_toProxySyncPrefix.msg_type);

		m_syncToProxyPrefix->Add(m_toProxySyncPrefix.dr);
		m_syncToProxyPrefix->Add(str_fingerprint);

		NS_LOG_INFO(m_clientId<<": The notification prefix to proxy is "<<*m_syncToProxyPrefix);

		SendSyncNotification(m_syncToProxyPrefix);

	}


}

ndn::InterestHeader SyncClient::GenerateSyncNotificationHeader(Ptr<ndn::Name> m_syncPrefix)
{
	NS_LOG_INFO(m_clientId<<": Generate the notification header");
	//NS_LOG_DEBUG(m_clientId<<": Sync Notification Prefix is "<<*m_syncPrefix);
	ndn::InterestHeader syncNotificationHeader;
	UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
	syncNotificationHeader.SetNonce (rand.GetValue ());
	syncNotificationHeader.SetName (m_syncPrefix);
	syncNotificationHeader.SetInterestLifetime (Seconds (1.0));
	return syncNotificationHeader;
}

void SyncClient::SendSyncNotification(Ptr<ndn::Name> m_syncPrefix)
{
	NS_LOG_INFO(m_clientId<<": Ready to send the sync notification");
	ndn::InterestHeader syncNotificationHeader;
	syncNotificationHeader = GenerateSyncNotificationHeader(m_syncPrefix);

	//NS_LOG_INFO(m_clientId<<": Create the sync notification packet");
	// Create packet and add ndn::syncNotificationHeader
	Ptr<Packet> packet = Create<Packet> ();
	packet->AddHeader (syncNotificationHeader);

	//ndn::FwHopCountTag hopCountTag;
	//packet->AddPacketTag (hopCountTag);
	//NS_LOG_DEBUG("hopCountTag is "<<hopCountTag.Get());

	// Forward packet to lower (network) layer
	m_protocolHandler (packet);

	// Call trace (for logging purposes)
	m_transmittedInterests (&syncNotificationHeader, this, m_face);
}


////////////////////////////////////////////////////////////////////////
// 			Parse components within a ndn::Name type Name		      //
////////////////////////////////////////////////////////////////////////

std::string SyncClient::SearchNameComponent(int index, ndn::Name name)
{
	int count = 1;
	std::string target="none";
	BOOST_FOREACH(const std::string &component, name.GetComponents())
	{
		//NS_LOG_DEBUG(m_clientId<<": Search "<<count<<" "<<component);
		if (count == index)
		{
			target = component;
			break;
		}
		count++;
	}
	return target;
}

////////////////////////////////////////////////////////////////////////
// 						 Digest Log Processing			 		      //
////////////////////////////////////////////////////////////////////////
void SyncClient::InitialDigestLog()
{
	// initial dr, dp, and defualtFP are stored with index 0
	Ptr<ndn::Name> defaultName = Create<ndn::Name> ();
	m_fpUpdateDB.push_back(*defaultName);
	LogKey *dr = new LogKey();
	dr->digest = *m_clientDigestTree.GetRoot()->GetData();
	//dr->fingerPrint = &m_fpUpdateDB.back();
	dr->index = m_fpUpdateDB.size()-1;
	m_keyForProxy.push_back(*dr);
	//NS_LOG_INFO(m_clientId<<" First entry in the log is "<<dr->digest.getDigest()<<": "<<*(dr->fingerPrint));
}

void SyncClient::TraverseLog()
{
	for(int i = 0; i<m_keyForProxy.size();i++)
	{
		LogKey dr = m_keyForProxy[i];
		NS_LOG_DEBUG(m_clientId<<": DigestLog Key: "<<dr.digest.getDigest()<<": "<<m_fpUpdateDB[dr.index]);
	}
	for(int i = 0; i<m_fpUpdateDB.size();i++)
	{
		NS_LOG_DEBUG(m_clientId<<": DigestLog Value: "<<m_fpUpdateDB[i]);
	}
}


int SyncClient::FindDigestIndex(ndn::Name targetName)
{
	//NS_LOG_DEBUG(m_clientId<<": Find digest value "<<targetName);
	int index = 0;
	for(int i = 0;i<m_fpUpdateDB.size();i++)
	{
		//std::cout<<i<<" "<<m_fpUpdateDB[i]<<std::endl;
		if (m_fpUpdateDB[i] == targetName)
		{
			index = i;
			break;
		}
		else
			index++;
	}
	return index;
}

bool SyncClient::UpdateDigestLog(std::string senderType, Ptr<ndn::Name> newArrival_FP)
{
	bool new_fp;
	NS_LOG_DEBUG(m_clientId<<": Sender type is "<<senderType);
	if (senderType == "app")
	{
		// Do nothing to digesttree and digestlog
		// May cache the new FP for some time

	}
	if (senderType == "proxy")
	{
		int index;
		index = FindDigestIndex(*newArrival_FP);
		//NS_LOG_DEBUG(m_clientId<<": new fp is "<<*newArrival_FP<<" and find the index is "<<index);
		//NS_LOG_DEBUG(m_clientId<<" Before push "<<m_fpUpdateDB.size());
		if (index == m_fpUpdateDB.size())
		{
			// not in the fpUpdateDB
			m_fpUpdateDB.push_back(*newArrival_FP);
			//NS_LOG_DEBUG(m_clientId<<" After push "<<m_fpUpdateDB.size());
			LogKey *dr = new LogKey();
			dr->digest = *m_clientDigestTree.GetRoot()->GetData();
			//dr->fingerPrint = &(m_fpUpdateDB.back());
			dr->index = m_fpUpdateDB.size()-1;
			m_keyForProxy.push_back(*dr);
			new_fp = true;
		}
		else if (index <= m_fpUpdateDB.size()-1)
		{
			NS_LOG_INFO(m_clientId<<": ALREADY had it at "<<index);
			// this new arrival FP sent from the proxy is generated locally, it has already been stored at the client
			//LogKey dr = {*m_clientDigestTree.GetRoot()->GetData(), &(*it)};
			//m_keyForProxy.push_back(dr);
			LogKey *dr = new LogKey();
			dr->digest = *m_clientDigestTree.GetRoot()->GetData();
			dr->index = index;
			m_keyForProxy.push_back(*dr);
			new_fp = false;
		}
	}
	return new_fp;
}


////////////////////////////////////////////////////////////////////////
// 						 Digest Tree Processing			 		      //
////////////////////////////////////////////////////////////////////////

// Initialize the proxy digest tree: droot(local_dproxy, dcontroller)
void SyncClient::InitDigestTree()
{
	NS_LOG_DEBUG("Initialize the Digest Tree at "<<m_clientId);
	Digest dr = Digest("root", "0000");
	DigestNode *ptr_root = new DigestNode(&dr);
	m_clientDigestTree.SetRoot(ptr_root);
	m_clientDigestTree.Traverse();
}




} //ns3





