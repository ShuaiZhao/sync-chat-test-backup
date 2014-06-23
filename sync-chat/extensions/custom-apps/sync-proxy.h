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
 *
 * Author: Xuan Liu <xuan.liu@mail.umkc.edu>
 */

#ifndef SYNC_PROXY_H_
#define SYNC_PROXY_H_

#include "digest.h"
#include "digest-tree.h"
#include "ns3/ndn-app.h"
#include "ns3/ndn-name.h"
#include "ns3/random-variable.h"
#include "ns3/system-mutex.h"
#include <vector>
#include <map>
#include <iostream>
#include <stdexcept>

namespace ns3
{

class SyncProxy : public ndn::App
{
	/*
	 *  This is the sync application for a sync-proxy that pushes notification to both
	 *  sync-client and sync-controller
	 *  When the sync-proxy receives a notification from a sync-client, it retrieves
	 *  the fingerprint part, generates a new notification with controller's prefix and
	 *  sends to the controller; when it receives a notification from a sync-controller,
	 *  it will generate notification to the sync-client.
	 *
	 *  During the notification processing, the sync-proxy also updates its digest tree
	 *  and digest log
	 */

public:
	// register NS-3 type "myChatApp"
	static TypeId
	GetTypeId ();

	// constructor
	SyncProxy();

	// destructor
	virtual ~SyncProxy();

	// (overridden from ndn::App) Processing upon start of the application
	virtual void
	StartApplication ();

	// (overridden from ndn::App) Processing when application is stopped
	virtual void
	StopApplication ();

	/*------------------------------------------------------------------*
	 * 			Parse the components within a ndn::Name type prefix     *
	 *-----------------------------------------------------------------*/
	// Search prefix component by index
	std::string SearchNameComponent(int index, ndn::Name name);
	std::string NameToString(ndn::Name name); // convert a name to string

	/*------------------------------------------------------------------*
	 * Process incoming packets: Interest/ContentObject/SynNotification *
	 *------------------------------------------------------------------*/
	//Determine the type of sender of the notification
	std::string GetSenderType(ndn::Name notificationPrefix);
	//Determine the notification type: join/leave/update
	std::string GetNotificationType(ndn::Name notificationPrefix);
	// incoming sync notification sent from client/controller, carrying the fingerprint
	void OnNotification(const Ptr<const ndn::InterestHeader> &syncPrefix, Ptr<Packet> origPacket);

	// incoming content interest
	virtual void
	OnInterest (const Ptr<const ndn::InterestHeader> &interest, Ptr<Packet> origPacket);


	/*------------------------------------------------------------------*
	 * 			   Push new content notification		    	    	*
	 *------------------------------------------------------------------*/

	// generate new notification regarding the content update
	void PushNotification(Ptr<ndn::Name> m_fingerprint, std::string SenderType, std::string notificationType);
	ndn::InterestHeader GenerateSyncNotificationHeader(Ptr<ndn::Name> m_syncPrefix);
	// send the notification to the proxy
	void SendSyncNotification(Ptr<ndn::Name> m_syncPrefix);

	/*------------------------------------------------------------------*
	 * 			Digest Log Operation						    	 	*
	 *------------------------------------------------------------------*/
	void InitialDigestLog();
	//void InsertNewDigset();
	void UpdateDigestLog(std::string senderType, ndn::Name newArrival_FP);
	//void FindDigest();
	int FindDigestIndex(ndn::Name targetName);
	void TraverseLog();
	/*------------------------------------------------------------------*
	 * 		 			Digest Tree Operation						    *
	 *------------------------------------------------------------------*/
	// Initialize DigestTree
	void InitDigestTree();

	// Recompute the digest through the tree
	void UpdateDigestTree();
private:
	// Attribute variables
	std::vector<std::string> 	m_clientIdList; // A list of clientIds that are connected to the proxy
	std::string 				m_clientId;		// a specific clientId
	std::string 				m_controllerId; // the controller that the proxy is connected to
	std::string 				m_proxyId;		// The proxy's Identifier
	ndn::Name 					m_proxyPrefix;	// Proxy's name prefix - fib entry

	// variables for heartbeat messages
	double 						m_frequency;			///< @brief frequency of the interest packets (in Hz)
	bool 						m_start; 				///< @brief indicate the start point of the application
	RandomVariable				*m_random;				///< @brief generate random number
	std::string       			m_randomType;			///< @brief distribution type

	EventId						m_sendHBEvent;			///< @brief EventId of pending "send heartbeat interest packet" event

	// Interest header related variable
	UniformVariable 			m_nonce;				///< @brief nonce generator

	// Fingerprint-related variables
	ndn::Name 					m_currFP;				///< @brief m_currFP is the same as m_contentPrefix in current design

	DigestTree 					m_proxyDigestTree;      ///< @brief m_proxyDigestTree is the local digest tree stored at the proxy

	// ToController Notification struct
	struct ToControllerSyncPrefix
	{
		std::string routing_prefix; // controller Id
		std::string serviceId;		// service Id
		std::string msg_type;		// message type: "0" = Join, "-1" = leave, other positive numbers = chatting message sequence number
		std::string proxyId;		// self proxyId
		std::string dp_pre;			// previous local proxy digest
		std::string dp_curr;		// current local proxy digest
		ndn::Name m_fingerprint;	// producer's fingerprint regarding the updates
	};
	ToControllerSyncPrefix m_toControllerSyncPrefix;

	// ToController Notification struct
	struct ToClientSyncPrefix
	{
		std::string routing_prefix; // client routing prefix
		std::string serviceId;		// service Id
		std::string msg_type;		// message type: "0" = Join, "-1" = leave, other positive numbers = chatting message sequence number
		std::string dr_pre;			// previous proxy digest root
		std::string dr_curr;		// current proxy digest root
		ndn::Name 	m_fingerprint;	// producer's fingerprint regarding the updates
	};
	ToClientSyncPrefix m_toClientSyncPrefix;


	// Digest Log related variable
	struct LogKey
	{
		Digest digest;
		//Ptr<ndn::Name> fingerPrint;
		int index;
	};
	typedef std::vector<LogKey> KeyVector;
	KeyVector m_keyForClient;
	KeyVector m_keyForController;

	std::vector<ndn::Name> m_fpUpdateDB;

	// SystemMutex variable for the shared data structure in the proxy: i.e. digest log, digest tree update
	SystemMutex m_logMutex;
	SystemMutex m_digestTreeMutex;

};

}

#endif // SYNC_DIGEST_H_
