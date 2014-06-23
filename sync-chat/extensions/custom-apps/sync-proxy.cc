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

#include "sync-proxy.h"
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
//#include "ns3/ndn-content-object.h"
//#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndn-fib.h"
#include <boost/foreach.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <boost/foreach.hpp>


NS_LOG_COMPONENT_DEFINE ("SyncProxy");

namespace ns3{

NS_OBJECT_ENSURE_REGISTERED (SyncProxy);

// register NS-3 type
TypeId
SyncProxy::GetTypeId()
{
	static TypeId tid = TypeId ("SyncProxy")
	    .SetParent<ndn::App> ()
	    .AddConstructor<SyncProxy> ()
	    .AddAttribute("Prefix", "Prefix for the proxy",
	    			StringValue("/"),
	    			ndn::MakeNameAccessor(&SyncProxy::m_proxyPrefix),
	        		ndn::MakeNameChecker())
	    .AddAttribute("ControllerID", "The controller's identifier that the proxy is connected to",
	    			StringValue("none"),
	    			MakeStringAccessor(&SyncProxy::m_controllerId),
	    			MakeStringChecker())
	    ;
	return tid;
}

// constructor
SyncProxy::SyncProxy()
	: m_frequency(1.0)
	, m_start (true)
	, m_random(0)
{
	NS_LOG_FUNCTION_NOARGS ();

}

// destructor
SyncProxy::~SyncProxy()
{
	if (m_random)
	    delete m_random;
}

//////////////////////////////////////////////////////////////////////////
//                    Start or Stop the application            			//
//////////////////////////////////////////////////////////////////////////

// Processing upon start of the application
void
SyncProxy::StartApplication ()
{
	NS_LOG_FUNCTION_NOARGS();
	NS_ASSERT(GetNode()->GetObject<ndn::Fib>()!=0);
	// initialize ndn::App
	ndn::App::StartApplication ();

	//NS_LOG_DEBUG("NodeID: " <<GetNode()->GetId());

	// Get Node Name
	Ptr<Node> node = GetNode();
	m_proxyId = Names::FindName(node);
	NS_LOG_INFO(m_proxyId<<": Application Starts");

	NS_LOG_INFO(m_proxyId<<": Initialize the digest tree");
	InitDigestTree(); // Initialize the digest tree
	NS_LOG_INFO(m_proxyId<<": Initialize the digest log");
	InitialDigestLog();	// Initialize the digest log


	Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();

	// Add sync proxy prefix to fib
	Ptr<ndn::fib::Entry> fibEntry = fib->Add (m_proxyPrefix, m_face, 0);

	fibEntry->UpdateStatus (m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);


}

// Processing when application is stopped
void
SyncProxy::StopApplication ()
{
	NS_LOG_FUNCTION_NOARGS ();
	NS_ASSERT (GetNode ()->GetObject<ndn::Fib> () != 0);
	// cancel periodic packet generation
	//Simulator::Cancel (m_sendContentEvent);

	// cleanup ndn::App
	NS_LOG_INFO(m_proxyId<<": Application Stops");
	ndn::App::StopApplication ();
}


//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Notification		           		//
//////////////////////////////////////////////////////////////////////////
void SyncProxy::PushNotification(Ptr<ndn::Name> m_fingerprint, std::string SenderType, std::string notificationType)
{
	NS_LOG_DEBUG(m_proxyId<<": Start to generate notification prefix");

	if (SenderType == "none")
	{
		NS_LOG_INFO("Notification is wrong!");
		return;
	}
	if (SenderType == "controller" || SenderType == "client")  //If a notification is received from either controller or client, proxy will push new notification to clients
	{
		NS_LOG_INFO(m_proxyId<<": Push notification to Clients");
		for(int i = 0; i < m_clientIdList.size(); i++)
		{
			m_toClientSyncPrefix.routing_prefix = m_clientIdList[i];
			m_toClientSyncPrefix.serviceId = "chatroom-cona";

			m_toClientSyncPrefix.msg_type = notificationType;

			m_toClientSyncPrefix.m_fingerprint = *m_fingerprint;
			//Take off the first '/' from the content name
			std::string str_fingerprint;
			std::stringstream ss;
			ss<<m_toClientSyncPrefix.m_fingerprint;
			ss>>str_fingerprint;
			str_fingerprint.erase(0,1);

			int keyLogSize = m_keyForClient.size();
			m_toClientSyncPrefix.dr_curr = m_keyForClient[keyLogSize-1].digest.getDigest();;
			m_toClientSyncPrefix.dr_pre = m_keyForClient[keyLogSize-2].digest.getDigest();

			// form the notification interest
			Ptr<ndn::Name> m_syncToClientPrefix = Create<ndn::Name> (m_toClientSyncPrefix.routing_prefix);
			//NS_LOG_DEBUG(m_proxyId<<": The notification prefix is "<<*m_syncToClientPrefix);
			m_syncToClientPrefix->Add(m_toClientSyncPrefix.serviceId);
			m_syncToClientPrefix->Add(m_toClientSyncPrefix.msg_type);
			m_syncToClientPrefix->Add(m_toClientSyncPrefix.dr_pre); // Need to declare the root digest
			m_syncToClientPrefix->Add(m_toClientSyncPrefix.dr_curr); // Need to declare the root digest
			m_syncToClientPrefix->Add(str_fingerprint);

			NS_LOG_INFO(m_proxyId<<": The notification prefix is "<<*m_syncToClientPrefix);

			SendSyncNotification(m_syncToClientPrefix);
		}

	}
	if (SenderType == "client")  //If a notification is received from client, proxy will push new notification to controller
	{
		NS_LOG_INFO(m_proxyId<<": Push notification to the Controller");
		m_toControllerSyncPrefix.routing_prefix = m_controllerId;
		m_toControllerSyncPrefix.serviceId = "chatroom-cona";

		m_toControllerSyncPrefix.msg_type = notificationType;

		m_toControllerSyncPrefix.m_fingerprint = *m_fingerprint;
		//Take off the first '/' from the content name
		std::string str_fingerprint;
		std::stringstream ss;
		ss<<m_toControllerSyncPrefix.m_fingerprint;
		ss>>str_fingerprint;
		str_fingerprint.erase(0,1);


		int keyLogSize = m_keyForController.size();
		m_toControllerSyncPrefix.dp_curr = m_keyForController[keyLogSize-1].digest.getDigest();
		m_toControllerSyncPrefix.dp_pre = m_keyForController[keyLogSize-2].digest.getDigest();
		// form the notification interest
		Ptr<ndn::Name> m_syncToControllerPrefix = Create<ndn::Name> (m_toControllerSyncPrefix.routing_prefix);
		//NS_LOG_DEBUG(m_proxyId<<": The notification prefix is "<<*m_syncToControllerPrefix);
		m_syncToControllerPrefix->Add(m_toControllerSyncPrefix.serviceId);
		m_syncToControllerPrefix->Add(m_toControllerSyncPrefix.msg_type);

		m_syncToControllerPrefix->Add(m_toControllerSyncPrefix.dp_pre); // Need to declare the root digest
		m_syncToControllerPrefix->Add(m_toControllerSyncPrefix.dp_curr); // Need to declare the root digest
		m_syncToControllerPrefix->Add(str_fingerprint);

		NS_LOG_INFO(m_proxyId<<": The notification prefix is "<<*m_syncToControllerPrefix);

		SendSyncNotification(m_syncToControllerPrefix);
	}


}

ndn::InterestHeader SyncProxy::GenerateSyncNotificationHeader(Ptr<ndn::Name> m_syncPrefix)
{
	NS_LOG_INFO(m_proxyId<<": Generate the notification header");
	//NS_LOG_DEBUG(m_proxyId<<": Sync Notification Prefix is "<<*m_syncPrefix);
	ndn::InterestHeader syncNotificationHeader;
	UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
	syncNotificationHeader.SetNonce (rand.GetValue ());
	syncNotificationHeader.SetName (m_syncPrefix);
	syncNotificationHeader.SetInterestLifetime (Seconds (1.0));
	return syncNotificationHeader;
}

void SyncProxy::SendSyncNotification(Ptr<ndn::Name> m_syncPrefix)
{
	NS_LOG_INFO(m_proxyId<<": Ready to send the sync notification");
	ndn::InterestHeader syncNotificationHeader;
	syncNotificationHeader = GenerateSyncNotificationHeader(m_syncPrefix);

	//NS_LOG_DEBUG(m_proxyId<<": Create the sync notification packet");
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
SyncProxy::OnInterest (const Ptr<const ndn::InterestHeader> &interest, Ptr<Packet> origPacket)
{
  NS_LOG_INFO (m_proxyId<<": Received Interest packet for " << interest->GetName ());

  bool isNotification = false;
  // check whether this interest is a notification or a content interest
  BOOST_FOREACH(const std::string &component, interest->GetName().GetComponents())
  {
	  if (component == "join" || component == "leave" || component == "update")
	  {
		  isNotification = true;
		  //NS_LOG_DEBUG(m_proxyId<<": Received a sync notification as a "<<component);
		  OnNotification(interest, origPacket);
		  break;
	  }
  }

  if (!isNotification)
  {
	  // reply the content if it's cached in the content store
	  // empty for now.
  }
}

//Determine the type of receiver of the new notification
std::string SyncProxy::GetSenderType(ndn::Name notificationPrefix)
{
	std::list<std::string> subPrefixList = notificationPrefix.GetComponents();
	int prefixSize = subPrefixList.size();

	//debug
	NS_LOG_DEBUG(m_proxyId<<": Notification is: "<<notificationPrefix<<" and Received notification size is "<<prefixSize);
	/*BOOST_FOREACH(const std::string &component, notificationPrefix.GetComponents())
	{
		NS_LOG_DEBUG(m_proxyId<<": Notification components are: "<<component);
	}*/

	if (prefixSize == 6) // notification prefix sent by client has 6 components
	{
		//new notification should be sent to the controller
		//NS_LOG_DEBUG(m_proxyId<<": Notification sender is client");
		return "client";
	}
	else if (prefixSize == 8) // notification prefix sent by client has 6 components
	{
		//new notification should be sent to both client and controller
		//NS_LOG_DEBUG(m_proxyId<<": Notification sender is controller");
		return "controller";
	}
	else
	{
		//NS_LOG_DEBUG(m_proxyId<<": Get notification sender wrong!");
		return "none";
	}
}


//Determine the notification type: join/leave/update
std::string SyncProxy::GetNotificationType(ndn::Name notificationPrefix)
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
	NS_LOG_INFO(m_proxyId<<": Notification type is "<<notificationType);
	return notificationType;
}



std::string SyncProxy::NameToString(ndn::Name name) // convert a name to string
{
	std::stringstream ss;
	std::string name_str;
	ss<<name;
	ss>>name_str;
	return name_str;
}


// Receives notification
void
SyncProxy::OnNotification(const Ptr<const ndn::InterestHeader> &interest,
		Ptr<Packet> origPacket)
{
	NS_LOG_INFO(m_proxyId<<": Process incoming notification");
	std::string senderType; // determine the type of sender who pushes this notification to the proxy
	std::string localClientId;  //the local clientID who sends the notification to the proxy
	std::string notificationType; // determine the type of destination component ("client" or "controller")
	uint32_t trackSeq;
	size_t hashValue;
	ndn::Name notificationPrefix;
	notificationPrefix = interest->GetName();
	notificationType = GetNotificationType(notificationPrefix);
	if (notificationType != "Empty")
	{
		ndn::Name newArrival_FP;
		newArrival_FP = interest->GetName().GetLastComponent();
		senderType = GetSenderType(notificationPrefix);
		const DigestTree *newLogEntryKey = new DigestTree();
		// Update DigestTree and DigestLog
		if (senderType == "client")
		{
			NS_LOG_INFO(m_proxyId<<": Sender is client");
			localClientId = *newArrival_FP.GetComponents().begin()+"/sync";
			//std::cout<<localClientId<<" "<<notificationType<<std::endl;
			if (notificationType == "join")
			{
				NS_LOG_INFO(m_proxyId<<": This is a join update");
				m_clientIdList.push_back(localClientId);
				Digest *newDfp = new Digest(localClientId, "0000");
				std::string name_str = NameToString(newArrival_FP);
				// update user digest
				newDfp->DigestSeqIncrement();
				newDfp->updateDigestValue(name_str);
				newDfp->finalizeDigestValue();
				newDfp->getDigestInfo()->digestValue = newDfp->getHashValue();
				DigestNode *newNode = new DigestNode(newDfp);
				//NS_LOG_DEBUG(m_proxyId<<": new digest is "<<newDfp->getDigestInfo()->digestValue);
				m_proxyDigestTree.InsertNode(newNode, m_proxyId);
				//NS_LOG_DEBUG(m_proxyId<<": new digest is "<<newNode->GetData()->getDigest());

				// Update Digest Tree
				UpdateDigestTree();
				//NS_LOG_DEBUG(m_proxyId<<": Traverse the new tree");
				m_proxyDigestTree.Traverse();
				// Update Digest Log
				UpdateDigestLog(senderType, newArrival_FP);


			}
			if (notificationType == "leave")
			{
				// Remove Node
				// Update Digest Tree
			}
			if (notificationType == "update")
			{
				DigestNode *findClient = m_proxyDigestTree.FindNode(localClientId);
				std::string name_str = NameToString(newArrival_FP);
				//std::cout<<"Before: "<<findClient->GetData()->getDigest()<<std::endl;
				// Update the digest node in the tree
				findClient->GetData()->DigestSeqIncrement();
				findClient->GetData()->reset();
				findClient->GetData()->updateDigestValue(name_str);
				findClient->GetData()->finalizeDigestValue();
				//std::cout<<"After: "<<findClient->GetData()->getHashValue()<<std::endl;
				findClient->GetData()->getDigestInfo()->digestValue = findClient->GetData()->getHashValue();
				//findClient->GetData()->DestroyDigestValue();
				//std::cout<<"After: "<<findClient->GetData()->getDigest()<<std::endl;
				// Update Digest Tree
				UpdateDigestTree();
				// Update Digest Log
				UpdateDigestLog(senderType, newArrival_FP);
				NS_LOG_INFO(m_proxyId<<": fpUpdateDB size is "<<m_fpUpdateDB.size());
				NS_LOG_INFO(m_proxyId<<": Digest root log size is "<<m_keyForClient.size());
				NS_LOG_INFO(m_proxyId<<": Proxy digest log size is "<<m_keyForController.size());
				//TraverseLog();
			}



		}
		if (senderType == "controller")
		{
			NS_LOG_INFO(m_proxyId<<": Sender is controller");
			DigestNode *dc = m_proxyDigestTree.GetChild().GetRSibling().GetRoot();
			std::string dc_pre = SearchNameComponent(5, notificationPrefix);
			std::string dc_curr = SearchNameComponent(6, notificationPrefix);
			//std::cout<<m_proxyId<<" Compare: "<<dc->GetData()->getDigest()<<" "<<dc_pre<<std::endl;
			//std::cout<<m_proxyId<<" Compare: "<<dc->GetData()->getDigest()<<" "<<dc_curr<<std::endl;
			if (dc->GetData()->getDigest() == dc_pre)
			{
				// There is no update missing
				NS_LOG_INFO(m_proxyId<<": Match!");
				dc->GetData()->RetrieveDigest(dc_curr);
				NS_LOG_INFO(m_proxyId<<": The new controller digest is "<<dc->GetData()->getDigest());
				// Update Tree
				UpdateDigestTree();
				// Update Digest Log
				UpdateDigestLog(senderType, newArrival_FP);
				NS_LOG_INFO(m_proxyId<<": fpUpdateDB size is "<<m_fpUpdateDB.size());
				NS_LOG_INFO(m_proxyId<<": Digest root log size is "<<m_keyForClient.size());
				NS_LOG_INFO(m_proxyId<<": Proxy digest log size is "<<m_keyForController.size());
				//TraverseLog();
			}
			else
			{
				NS_LOG_INFO(m_proxyId<<": Not Match!");
				// Enter recovery mode, send requests for old fingerprints from controller
			}
		}

		//NS_LOG_DEBUG(m_proxyId<<": Received Notification for"<<interest->GetName());
		//NS_LOG_DEBUG(m_proxyId<<": The content name is: "<< newArrival_FP);
		PushNotification(&newArrival_FP, senderType, notificationType);

	}



}


////////////////////////////////////////////////////////////////////////
// 			Parse components within a ndn::Name type Name		      //
////////////////////////////////////////////////////////////////////////

std::string SyncProxy::SearchNameComponent(int index, ndn::Name name)
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


////////////////////////////////////////////////////////////////////////
// 						 Digest Log Processing			 		      //
////////////////////////////////////////////////////////////////////////
void SyncProxy::InitialDigestLog()
{
	// initial dr, dp, and defualtFP are stored with index 0
	Ptr<ndn::Name> defaultName = Create<ndn::Name> ();
	m_fpUpdateDB.push_back(*defaultName);
	//LogKey dr = {*m_proxyDigestTree.GetRoot()->GetData(), &m_fpUpdateDB.back()};
	//LogKey dp = {*m_proxyDigestTree.GetChild().GetRoot()->GetData(), &m_fpUpdateDB.back()};
	LogKey *dr = new LogKey();
	dr->digest = *m_proxyDigestTree.GetRoot()->GetData();
	dr->index = m_fpUpdateDB.size()-1;
	m_keyForClient.push_back(*dr);
	LogKey *dp = new LogKey();
	dp->digest = *m_proxyDigestTree.GetChild().GetRoot()->GetData();
	dp->index = m_fpUpdateDB.size()-1;
	m_keyForController.push_back(*dp);
}

int SyncProxy::FindDigestIndex(ndn::Name targetName)
{
	//NS_LOG_DEBUG(m_clientId<<": Find digest value "<<targetName);
	int index = 0;
	for(int i = 0;i<m_fpUpdateDB.size();i++)
	{
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


void SyncProxy::TraverseLog()
{
	for(int i = 0; i<m_keyForClient.size();i++)
	{
		LogKey dr = m_keyForClient[i];
		NS_LOG_DEBUG(m_proxyId<<": DigestLog Key: "<<dr.digest.getDigest()<<": "<<m_fpUpdateDB[dr.index]);
	}
	for(int i = 0; i<m_keyForController.size();i++)
	{
		LogKey dp = m_keyForController[i];
		NS_LOG_DEBUG(m_proxyId<<": DigestLog Key: "<<dp.digest.getDigest()<<": "<<m_fpUpdateDB[dp.index]);
	}
	for(int i = 0; i<m_fpUpdateDB.size();i++)
	{
		NS_LOG_DEBUG(m_proxyId<<": DigestLog Value: "<<m_fpUpdateDB[i]);
	}
}


void SyncProxy::UpdateDigestLog(std::string senderType, ndn::Name newArrival_FP)
{
	CriticalSection cs(m_logMutex);
	if (senderType == "client")
	{
		// if the notification is sent by a client, both digest root vector and proxy digest needs to be updated
		m_fpUpdateDB.push_back(newArrival_FP);
		//LogKey dr = {*m_proxyDigestTree.GetRoot()->GetData(), &m_fpUpdateDB.back()};
		//LogKey dp = {*m_proxyDigestTree.GetChild().GetRoot()->GetData(), &m_fpUpdateDB.back()};
		LogKey *dr = new LogKey();
		dr->digest = *m_proxyDigestTree.GetRoot()->GetData();
		dr->index = m_fpUpdateDB.size()-1;
		m_keyForClient.push_back(*dr);
		LogKey *dp = new LogKey();
		dp->digest = *m_proxyDigestTree.GetChild().GetRoot()->GetData();
		dp->index = m_fpUpdateDB.size()-1;
		m_keyForController.push_back(*dp);
	}
	else if (senderType == "controller")
	{
		// if the notification is sent by a controller, only the root digest vector will be updated
		int index;
		index = FindDigestIndex(newArrival_FP);
		//NS_LOG_DEBUG(m_proxyId<<"Find "<<newArrival_FP<<" at "<<index);
		//NS_LOG_DEBUG(m_proxyId<<"fp db size is "<<m_fpUpdateDB.size());
		if (index == m_fpUpdateDB.size())
		{
			// new arrival FP sent from the controller is from remote proxies
			m_fpUpdateDB.push_back(newArrival_FP);
			LogKey *dr = new LogKey();
			dr->digest = *m_proxyDigestTree.GetRoot()->GetData();
			dr->index = m_fpUpdateDB.size()-1;
			m_keyForClient.push_back(*dr);
		}
		else if (index <= m_fpUpdateDB.size()-1)
		{
			NS_LOG_INFO(m_proxyId<<": ALREADY had it at "<<index);
			// this new arrival FP sent from the controller is generated locally, it has already been stored at the proxy
			LogKey *dr = new LogKey();
			dr->digest = *m_proxyDigestTree.GetRoot()->GetData();
			dr->index = index;
			m_keyForClient.push_back(*dr);
		}

	}

	/*for (int i = 0;i<m_keyForClient.size();i++)
	{
		LogKey temp = m_keyForClient[i];
		NS_LOG_DEBUG(m_clientId<<": digest value is "<<temp.digest.getDigest());
	}*/

}


////////////////////////////////////////////////////////////////////////
// 						 Digest Tree Processing			 		      //
////////////////////////////////////////////////////////////////////////

// Initialize the proxy digest tree: droot(local_dproxy, dcontroller)
void SyncProxy::InitDigestTree()
{
	Digest *dr = new Digest("root", "0000");
	Digest *dp = new Digest(m_proxyId, "0000");
	Digest *dc = new Digest(m_controllerId, "0000");

	DigestNode *ptr_root = new DigestNode(dr);
	DigestNode *ptr_proxy = new DigestNode(dp);
	DigestNode *ptr_controller = new DigestNode(dc);
	m_proxyDigestTree.SetRoot(ptr_root);
	m_proxyDigestTree.InsertNode(ptr_controller, "root"); //proxy's sibling
	//m_proxyDigestTree.Traverse();
	m_proxyDigestTree.InsertNode(ptr_proxy, "root");  // left child tree
	//m_proxyDigestTree.Traverse();
	//NS_LOG_DEBUG(m_proxyId<<" Traverse proxy digest tree.");
	//m_proxyDigestTree.Traverse();
}



// Recompute the digest through the tree
void SyncProxy::UpdateDigestTree()
{
	NS_LOG_DEBUG(m_proxyId<<": Update Digest Tree");
	DigestTree proxySubTree = m_proxyDigestTree.GetChild();
	Digest *dr = m_proxyDigestTree.GetRoot()->GetData();
	Digest *dp = proxySubTree.GetRoot()->GetData();
	Digest *dc = proxySubTree.GetRSibling().GetRoot()->GetData();

	//debug: current state
	//std::cout<<"Current Client digest is: "<<dp->getDigest()<<std::endl;


	// Update local proxy digest
	DigestNode *cursor = proxySubTree.GetChild().GetRoot();
	if (cursor == 0)
	{
		NS_LOG_INFO(m_proxyId<<": Currently there is no client join.");
	}
	else
	{
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
		NS_LOG_DEBUG(m_proxyId<<": new proxy digest is "<<dp->getDigest());
	}


	//Update digest root
	NS_LOG_INFO(m_proxyId<<": Updating the digest root");
	dr->reset();
	dr->updateDigestValue(dp->getDigestInfo()->digestValue);
	dr->updateDigestValue(dc->getDigestInfo()->digestValue);

	dr->finalizeDigestValue();
	dr->DigestSeqIncrement();
	dr->getDigestInfo()->digestValue =dr->getHashValue();
	//NS_LOG_DEBUG(m_proxyId<<": Updating the digest root as "<<dr->getDigestInfo()->digestValue);

}




} // ns3
