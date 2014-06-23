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

#ifndef SYNC_CONTROLLER_H_
#define SYNC_CONTROLLER_H_

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

class SyncController : public ndn::App
{
	/*
	 *  This is the sync application for a sync-controller that pushes notification to sync-proxy
	 *  When the sync-controller receives a notification from a sync-proxy, it retrieves
	 *  the fingerprint part, generates a new notification with proxy's prefix and
	 *  sends to the proxies.
	 *
	 *  During the notification processing, the sync-controller also updates its digest tree
	 *  and digest log
	 */

public:
	// register NS-3 type "myChatApp"
	static TypeId
	GetTypeId ();

	// constructor
	SyncController();

	// destructor
	virtual ~SyncController();

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
	std::string NameToString(ndn::Name name);
	std::string FindProxyID(ndn::Name name);
	/*------------------------------------------------------------------*
	 * Process incoming packets: Interest/ContentObject/SynNotification *
	 *------------------------------------------------------------------*/

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
	void PushNotification(Ptr<ndn::Name> m_fingerprint, std::string notificationType);
	ndn::InterestHeader GenerateSyncNotificationHeader(Ptr<ndn::Name> m_syncPrefix);
	// send the notification to the proxy
	void SendSyncNotification(Ptr<ndn::Name> m_syncPrefix);

	/*------------------------------------------------------------------*
	 * 			Digest Log Operation						    	 	*
	 *------------------------------------------------------------------*/
	void InitialDigestLog();
	void InsertNewDigset();
	void UpdateDigestLog(Ptr<ndn::Name> newArrival_FP);
	void FindDigest();
	void TraverseLog();
	/*------------------------------------------------------------------*
	 * 		 			Digest Tree Operation						    *
	 *------------------------------------------------------------------*/
	// Initialize DigestTree
	void InitDigestTree();

	void InitProxyDigestVector(std::vector<std::string> ProxyIdVector);

	// Recompute the digest through the tree
	void UpdateDigestTree();
private:
	// Attribute variables
	std::vector<std::string> 	m_proxyIdList; 		// A list of clientIds that are connected to the proxy
	std::string 				m_proxyId;			// a specific proxyId
	uint32_t					m_numProxy;			// Number of proxies connected to the controllers
	std::string 				m_controllerId;		// The controller's Identifier
	ndn::Name 					m_controllerPrefix;		// Controller's name prefix - fib entry

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

	DigestTree 					m_controllerDigestTree;      ///< @brief m_proxyDigestTree is the local digest tree stored at the proxy

	// ToController Notification struct
	struct ToProxySyncPrefix
	{
		std::string routing_prefix; // controller Id
		std::string serviceId;		// service Id
		std::string msg_type;		// message type: "0" = Join, "-1" = leave, other positive numbers = chatting message sequence number
		std::string proxyId;		// self proxyId
		std::string dc_pre;			// previous controller digest root
		std::string dc_curr;		// current controller digest root
		std::string dp_curr;		// current proxy digest corresponding to the proxyId
		ndn::Name m_fingerprint;	// producer's fingerprint regarding the updates
	};
	ToProxySyncPrefix m_toProxySyncPrefix;


	// Digest Log related variable
	struct LogKey
	{
		Digest digest;
		int index;
	};
	typedef std::vector<LogKey> KeyVector;
	KeyVector m_keyForProxy;


	std::vector<ndn::Name> m_fpUpdateDB;

	std::vector<Digest> m_proxyDigestVector;

	// SystemMutex variable for the shared data structure in the proxy: i.e. digest log, digest tree update
	SystemMutex m_logMutex;
	SystemMutex m_digestTreeMutex;

};

}

#endif // SYNC_DIGEST_H_
