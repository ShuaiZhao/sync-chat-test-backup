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

// sync-proxy.cc

#include "sync-controller.h"
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
//#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndn-fib.h"
#include <boost/foreach.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

NS_LOG_COMPONENT_DEFINE ("SyncController");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (SyncController);

// register NS-3 type
TypeId
SyncController::GetTypeId()
{
	static TypeId tid = TypeId ("SyncController")
	    .SetParent<ndn::App> ()
	    .AddConstructor<SyncController> ()
	    .AddAttribute("Prefix", "Prefix for the controller",
	    			StringValue("/"),
	    			ndn::MakeNameAccessor(&SyncController::m_controllerPrefix),
	        		ndn::MakeNameChecker())
	    .AddAttribute("NumOfProxy", "Number of proxies",
	    			IntegerValue(0),
	    			MakeIntegerAccessor(&SyncController::m_numProxy),
	    			MakeIntegerChecker<uint32_t>())
	    ;
	return tid;
}

// constructor
SyncController::SyncController()
	: m_frequency(1.0)
	, m_start (true)
	, m_random(0)
{
	NS_LOG_FUNCTION_NOARGS ();

	//m_proxyIdList.push_back("P1");
	//m_proxyIdList.push_back("P2");
	//m_proxyIdList.push_back("P3");

}

// destructor
SyncController::~SyncController()
{
	NS_LOG_FUNCTION_NOARGS ();
	if (m_random)
	    delete m_random;
}

//////////////////////////////////////////////////////////////////////////
//                    Start or Stop the application            			//
//////////////////////////////////////////////////////////////////////////

// Processing upon start of the application
void
SyncController::StartApplication ()
{
	NS_LOG_FUNCTION_NOARGS();
	NS_ASSERT(GetNode()->GetObject<ndn::Fib>()!=0);
	// initialize ndn::App
	ndn::App::StartApplication ();


	//NS_LOG_DEBUG("NodeID: " <<GetNode()->GetId());

	// Get Node Name
	Ptr<Node> node = GetNode();
	m_controllerId = Names::FindName(node);
	NS_LOG_INFO(m_controllerId<<": Application starts");

	NS_LOG_INFO(m_controllerId<<": Contoller has "<<m_numProxy<<" proxies");
	for (int i = 1; i <= m_numProxy; i++)
	{
		std::string proxy_Id = "P" + boost::lexical_cast<std::string>(i);
		m_proxyIdList.push_back(proxy_Id);
	}
	InitDigestTree(); // Initialize the digest tree
	InitialDigestLog();	// Initialize the digest log

	Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();

	// Add sync controller prefix to fib
	Ptr<ndn::fib::Entry> fibEntry = fib->Add (m_controllerPrefix, m_face, 0);

	fibEntry->UpdateStatus (m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);


}

// Processing when application is stopped
void
SyncController::StopApplication ()
{
	NS_LOG_FUNCTION_NOARGS ();
	NS_ASSERT (GetNode ()->GetObject<ndn::Fib> () != 0);
	// cancel periodic packet generation
	//Simulator::Cancel (m_sendContentEvent);

	// cleanup ndn::App
	NS_LOG_INFO(m_controllerId<<": Application stops");
	ndn::App::StopApplication ();
}

//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Notification		           		//
//////////////////////////////////////////////////////////////////////////
void SyncController::PushNotification(Ptr<ndn::Name> m_fingerprint, std::string notificationType)
{
	NS_LOG_INFO(m_controllerId<<":Start to generate notification prefix");


	//NS_LOG_DEBUG("Push notification to Proxies");
	for(int i = 0; i < m_proxyIdList.size(); i++)
	{
		m_toProxySyncPrefix.routing_prefix = m_proxyIdList[i];
		m_toProxySyncPrefix.proxyId = "VM01";
		m_toProxySyncPrefix.serviceId = "chatroom-cona";

		m_toProxySyncPrefix.msg_type = notificationType;
		m_toProxySyncPrefix.m_fingerprint = *m_fingerprint;
		//Take off the first '/' from the content name
		std::string str_fingerprint;
		std::stringstream ss;
		ss<<m_toProxySyncPrefix.m_fingerprint;
		ss>>str_fingerprint;
		str_fingerprint.erase(0,1);

		int keyLogSize = m_keyForProxy.size();
		/*LogKey dc_entry1 = m_keyForProxy[keyLogSize-1];
		dc_entry1.digest.getDigest()*/
		m_toProxySyncPrefix.dc_curr = m_keyForProxy[keyLogSize-1].digest.getDigest();
		m_toProxySyncPrefix.dc_pre = m_keyForProxy[keyLogSize-2].digest.getDigest();

		m_toProxySyncPrefix.dp_curr = m_controllerDigestTree.FindNode(m_toProxySyncPrefix.routing_prefix)->GetData()->getDigest();

		// form the notification interest
		Ptr<ndn::Name> m_syncToProxyPrefix = Create<ndn::Name> (m_toProxySyncPrefix.routing_prefix);
		//NS_LOG_DEBUG(m_controllerId<<": The notification prefix is "<<*m_syncToProxyPrefix);
		m_syncToProxyPrefix->Add(m_toProxySyncPrefix.proxyId);
		m_syncToProxyPrefix->Add(m_toProxySyncPrefix.serviceId);
		m_syncToProxyPrefix->Add(m_toProxySyncPrefix.msg_type);
		m_syncToProxyPrefix->Add(m_toProxySyncPrefix.dc_pre); // Need to declare the root digest
		m_syncToProxyPrefix->Add(m_toProxySyncPrefix.dc_curr); // Need to declare the root digest
		m_syncToProxyPrefix->Add(m_toProxySyncPrefix.dp_curr); // Need to declare the root digest
		m_syncToProxyPrefix->Add(str_fingerprint);

		NS_LOG_INFO(m_controllerId<<": The notification prefix to proxy is "<<*m_syncToProxyPrefix);

		SendSyncNotification(m_syncToProxyPrefix);
	}


}

ndn::InterestHeader SyncController::GenerateSyncNotificationHeader(Ptr<ndn::Name> m_syncPrefix)
{
	NS_LOG_INFO(m_controllerId<<": Generate the notification header");
	//NS_LOG_DEBUG(m_controllerId<<": Sync Notification Prefix is "<<*m_syncPrefix);
	ndn::InterestHeader syncNotificationHeader;
	UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
	syncNotificationHeader.SetNonce (rand.GetValue ());
	syncNotificationHeader.SetName (m_syncPrefix);
	syncNotificationHeader.SetInterestLifetime (Seconds (1.0));
	return syncNotificationHeader;
}

void SyncController::SendSyncNotification(Ptr<ndn::Name> m_syncPrefix)
{
	NS_LOG_INFO(m_controllerId<<": Ready to send the sync notification");
	ndn::InterestHeader syncNotificationHeader;
	syncNotificationHeader = GenerateSyncNotificationHeader(m_syncPrefix);

	//NS_LOG_DEBUG(m_controllerId<<": Create the sync notification packet");
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
//          			Process incoming packets       			      //
////////////////////////////////////////////////////////////////////////


// Callback that will be called when Interest arrives
void
SyncController::OnInterest (const Ptr<const ndn::InterestHeader> &interest, Ptr<Packet> origPacket)
{
	NS_LOG_INFO (m_controllerId<<": Received Interest packet for " << interest->GetName ());

	OnNotification(interest,origPacket);
}





//Determine the notification type: join/leave/update
std::string SyncController::GetNotificationType(ndn::Name notificationPrefix)
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



// Receives notification
void
SyncController::OnNotification(const Ptr<const ndn::InterestHeader> &interest,
		Ptr<Packet> origPacket)
{
	//std::string senderType = "proxy"; // TBD
	std::string localClientId;  //the local clientID who sends the notification to the proxy
	std::string notificationType; // determine the type of destination component ("client" or "controller")
	uint32_t trackSeq;
	size_t hashValue;
	ndn::Name notificationPrefix;
	notificationPrefix = interest->GetName();
	NS_LOG_INFO(m_controllerId<<": the notification is "<<notificationPrefix);
	notificationType = GetNotificationType(notificationPrefix);
	NS_LOG_INFO(m_controllerId<<": the notification type is "<<notificationType);
	m_proxyId = FindProxyID(notificationPrefix);
	if (notificationType != "Empty")
	{
		Ptr<ndn::Name> newArrival_FP = Create<ndn::Name>();
		*newArrival_FP = interest->GetName().GetLastComponent();
		NS_LOG_INFO(m_controllerId<<": New arrival fp is "<<*newArrival_FP);
		//const DigestTree *newLogEntryKey = new DigestTree();
		// Update DigestTree and DigestLog
		localClientId = newArrival_FP->GetComponents().front();
		//NS_LOG_DEBUG(m_controllerId<<": the sender of new fp is "<<localClientId);
		if (notificationType == "join")
		{
			NS_LOG_DEBUG(m_controllerId<<": This is a join message.");
			//m_proxyIdList.push_back(localClientId);  // Could be used for enable/disable proxy later
 			trackSeq = 1;
			Digest *newDfp = new Digest(localClientId, "0000");
			std::string name_str = NameToString(*newArrival_FP);
			/*hashValue = newDfp.hash(name_str.c_str());
			newDfp.setDigest(localClientId, trackSeq, hashValue);*/
			newDfp->DigestSeqIncrement();
			newDfp->updateDigestValue(name_str);
			newDfp->finalizeDigestValue();
			newDfp->getDigestInfo()->digestValue = newDfp->getHashValue();
			DigestNode *newNode = new DigestNode(newDfp);
			//NS_LOG_DEBUG(m_proxyId<<": new digest is "<<newDfp->getDigest());
			//DigestNode newClient = new DigestNode(&newDfp);
			m_controllerDigestTree.InsertNode(newNode, m_proxyId);
			// Update Digest Tree
			UpdateDigestTree();
			// Update Digest Log
			UpdateDigestLog(newArrival_FP);
			TraverseLog();
		}
		if (notificationType == "leave")
		{
			// Remove Node
			// Update Digest Tree
		}
		if (notificationType == "update")
		{
			//NS_LOG_DEBUG(m_controllerId<<": This is a update message sent from "<<localClientId);
			DigestNode *findClient = m_controllerDigestTree.FindNode(localClientId);
			trackSeq = atoi(findClient->GetData()->getDigestInfo()->trackSeq.c_str())+1;
			std::string name_str = NameToString(*newArrival_FP);
			hashValue = findClient->GetData()->hash(name_str.c_str());
			findClient->GetData()->setDigest(localClientId, trackSeq, hashValue); // update the digestnode in the tree
			// Update Digest Tree
			UpdateDigestTree();
			// Update Digest Log
			UpdateDigestLog(newArrival_FP);
		}
		//}

		//NS_LOG_DEBUG(m_controllerId<<": Received Notification for"<<interest->GetName());
		//NS_LOG_DEBUG(m_controllerId<<": The content name is: "<<*newArrival_FP);
		PushNotification(newArrival_FP,notificationType);

	}

}




////////////////////////////////////////////////////////////////////////
// 			Parse components within a ndn::Name type Name		      //
////////////////////////////////////////////////////////////////////////

std::string SyncController::SearchNameComponent(int index, ndn::Name name)
{
	int count = 1;
	std::string target="none";
	BOOST_FOREACH(const std::string &component, name.GetComponents())
	{
		if (count == index)
		{
			target = component;
			break;
		}
		count++;
	}
	return target;
}


std::string SyncController::FindProxyID(ndn::Name name)
{
	//std::cout<<name<<std::endl;
	std::string proxyInfo, proxyId;

	std::vector<std::string> key_value;

	proxyInfo = SearchNameComponent(4, name);
	//std::cout<<proxyInfo<<std::endl;
	std::stringstream strstr(proxyInfo);
	std::string word;
	while(getline(strstr, word, '-'))
	{
		key_value.push_back(word);
	}
	proxyId = key_value[0];
	//NS_LOG_DEBUG(m_controllerId<<": the update is sent from "<<proxyId);
	return proxyId;

}

std::string SyncController::NameToString(ndn::Name name) // convert a name to string
{
	std::stringstream ss;
	std::string name_str;
	ss<<name;
	ss>>name_str;
	return name_str;
}

////////////////////////////////////////////////////////////////////////
// 						 Digest Log Processing			 		      //
////////////////////////////////////////////////////////////////////////
void SyncController::InitialDigestLog()
{
	NS_LOG_INFO(m_controllerId<<": Initilize Digest Log.");
	// initial dr, dp, and defualtFP are stored with index 0
	Ptr<ndn::Name> defaultName = Create<ndn::Name> ();
	//NS_LOG_DEBUG(m_controllerId<<*defaultName);
	m_fpUpdateDB.push_back(*defaultName);
	LogKey *dc = new LogKey();
	dc->digest = *m_controllerDigestTree.GetRoot()->GetData();
	dc->index = m_fpUpdateDB.size()-1;
	m_keyForProxy.push_back(*dc);


}

void SyncController::TraverseLog()
{
	for(int i = 0; i<m_keyForProxy.size();i++)
	{
		LogKey dc = m_keyForProxy[i];
		NS_LOG_DEBUG(m_controllerId<<": DigestLog Key: "<<dc.digest.getDigest()<<": "<<m_fpUpdateDB[dc.index]);
	}
	for(int i = 0; i<m_fpUpdateDB.size();i++)
	{
		NS_LOG_DEBUG(m_controllerId<<": DigestLog Value: "<<m_fpUpdateDB[i]);
	}
}

void SyncController::UpdateDigestLog(Ptr<ndn::Name> newArrival_FP)
{
	NS_LOG_INFO(m_controllerId<<": Update Digest Log");
	CriticalSection cs(m_logMutex);
	m_fpUpdateDB.push_back(*newArrival_FP);
	LogKey *dc = new LogKey();
	dc->digest = *m_controllerDigestTree.GetRoot()->GetData();
	dc->index = m_fpUpdateDB.size()-1;
	m_keyForProxy.push_back(*dc);
}


////////////////////////////////////////////////////////////////////////
// 						 Digest Tree Processing			 		      //
////////////////////////////////////////////////////////////////////////

// Initialize the proxy digest tree: droot(local_dproxy, dcontroller)
void SyncController::InitDigestTree()
{
	NS_LOG_INFO(m_controllerId<<": Initialize Digest Tree");
	Digest *dc = new Digest(m_controllerId, "0000");
	DigestNode *root = new DigestNode(dc);
	// Create a vector of proxy digest, and then run build tree
	InitProxyDigestVector(m_proxyIdList);

	/*for (int i = 0; i<m_proxyDigestVector.size();i++)
	{
		Digest d = m_proxyDigestVector[i];
		std::string temp = m_proxyDigestVector[i].getDigestInfo()->nodeId;
		std::cout<<temp<<std::endl;
	}*/

	m_controllerDigestTree.SetRoot(root);
	m_controllerDigestTree.BuildTree(m_proxyDigestVector, m_controllerId);

	// Tracing Debug
	//NS_LOG_DEBUG(m_controllerId<<"Traverse the digest tree.");
	//m_controllerDigestTree.Traverse();

}


void SyncController::InitProxyDigestVector(std::vector<std::string> ProxyIdVector)
{
	for(int i = 0; i < ProxyIdVector.size(); i++)
	{
		std::string tempNodeId = ProxyIdVector[i];
		Digest *tempdigest = new Digest(tempNodeId, "0000");
		m_proxyDigestVector.push_back(*tempdigest);
	}

}




// Recompute the digest through the tree
void SyncController::UpdateDigestTree()
{

	NS_LOG_INFO(m_controllerId<<": Start updating digest tree");
	// TBD
	// Find the proxy where the new FP is generated, update that proxy digest
	//NS_LOG_DEBUG(m_controllerId<<": find proxy subtree "<<m_proxyId);
	DigestTree proxySubTree = m_controllerDigestTree.FindNode(m_proxyId);
	Digest *dc = m_controllerDigestTree.GetRoot()->GetData();
	Digest *dp = proxySubTree.GetRoot()->GetData();
	// Update local proxy digest
	DigestNode *cursor = proxySubTree.GetChild().GetRoot();
	dp->reset();
	dp->updateDigestValue(cursor->GetData()->getDigestInfo()->digestValue);
	cursor = cursor->GetRightSibling();
	while(cursor != 0)
	{
		dp->updateDigestValue(cursor->GetData()->getDigestInfo()->digestValue);
		cursor = cursor->GetRightSibling();
	}
	dp->finalizeDigestValue();
	dp->getDigestInfo()->digestValue = dp->getHashValue();
	dp->DigestSeqIncrement();
	NS_LOG_INFO(m_controllerId<<": new proxy digest is "<<dp->getDigest());
	// update controller digest root
	DigestNode *cursor2 = m_controllerDigestTree.GetChild().GetRoot();
	dc->reset();
	dc->updateDigestValue(cursor2->GetData()->getDigestInfo()->digestValue);
	cursor2 = cursor2->GetRightSibling();
	while(cursor2 != 0)
	{
		dc->updateDigestValue(cursor2->GetData()->getDigestInfo()->digestValue);
		cursor2 = cursor2->GetRightSibling();
	}
	dc->finalizeDigestValue();
	dc->getDigestInfo()->digestValue = dc->getHashValue();
	dc->DigestSeqIncrement();
	NS_LOG_INFO(m_controllerId<<": new controller digest is "<<dc->getDigest());

}





} //ns3

