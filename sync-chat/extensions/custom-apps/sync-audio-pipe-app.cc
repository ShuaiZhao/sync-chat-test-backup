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

#include "sync-audio-pipe-app.h"
#include "sync-client.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/data-rate.h"

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
#include "ns3/onoff-application.h"
#include "ns3/random-variable-stream.h"

NS_LOG_COMPONENT_DEFINE("SyncAudioApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(SyncAudioApp);

// register NS-3 type
TypeId SyncAudioApp::GetTypeId() {
		static TypeId tid =TypeId("SyncAudioApp").SetParent<ndn::App>()
		.AddConstructor<SyncAudioApp>()
		.AddAttribute("Prefix",
							"Prefix, for which producer has the data", StringValue("/"),
							ndn::MakeNameAccessor(&SyncAudioApp::m_nodePrefix),
							ndn::MakeNameChecker())
		.AddAttribute("AccountID",
							"AccoundID for the Application", StringValue("none"),
							MakeStringAccessor(&SyncAudioApp::m_accountId),
							MakeStringChecker())
		.AddAttribute("InterArrival",
							"Packet Inter Arrival Time", StringValue("1.0"),
							MakeDoubleAccessor(&SyncAudioApp::m_interArrival),
							MakeDoubleChecker<double>())
		.AddAttribute("PayloadSize",
							"Virtual payload size per content packets",
							IntegerValue(),
							MakeIntegerAccessor(&SyncAudioApp::m_virtualPayloadSize),
							MakeIntegerChecker<uint32_t>())
		.AddAttribute("StartSeq",
							"Initial sequence number", IntegerValue(0),
							MakeIntegerAccessor(&SyncAudioApp::m_contentSeq),
							MakeIntegerChecker<uint32_t>())
		.AddAttribute(
							"SyncClientID", "The sync proxy ID the client is connected",
							StringValue("none"),
							MakeStringAccessor(&SyncAudioApp::m_syncClientId),
							MakeStringChecker())
		.AddAttribute("LeaveTime", "When User will leave", IntegerValue(3),
							MakeIntegerAccessor(&SyncAudioApp::m_leaveTime),
							MakeIntegerChecker<uint32_t>())
		.AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
							StringValue ("ns3::ConstantRandomVariable[Constant=0.352]"),
							MakePointerAccessor (&SyncAudioApp::m_onTime),
							MakePointerChecker <RandomVariableStream>())
		.AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
							StringValue ("ns3::ConstantRandomVariable[Constant=0.65]"),
							MakePointerAccessor (&SyncAudioApp::m_offTime),
							MakePointerChecker <RandomVariableStream>())
		.AddAttribute("DataRate", "the data reate is on state", DataRateValue(DataRate("500kb/s")),
							MakeDataRateAccessor(&SyncAudioApp::m_cbrRate),
							MakeDataRateChecker());
		return tid;
}

// constructor
SyncAudioApp::SyncAudioApp() :
		m_frequency(), m_start(true), m_random() {
	NS_LOG_FUNCTION_NOARGS ();
	m_audioContentSeq = 0;
	m_onSequence = 0;
	m_offSequence = 0;
}

// destructor
SyncAudioApp::~SyncAudioApp() {
	if (m_random)
		delete m_random;
}

//////////////////////////////////////////////////////////////////////////
//                    Start or Stop the application            			//
//////////////////////////////////////////////////////////////////////////

// Processing upon start of the application
void SyncAudioApp::StartApplication() {
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

	NS_LOG_DEBUG("This audio node's prefix "<<m_nodePrefix);
	ScheduleStartEvent();
}


void SyncAudioApp::ScheduleStartEvent(){
	// Schedules the event to start sending data (switch to the "On" state)
	//NS_LOG_INFO(this);
	//if(m_interArrival != 0){
	if(m_leaveTime != 1000){
		Time offInterval = Seconds(m_offTime->GetValue());

		//ExponentialVariable exp(m_offTime->GetValue());
		//NS_LOG_INFO("exp(0.7)=" <<Seconds(exp.GetValue()));
		NS_LOG_INFO(m_nodeName << ": Speaker" << ", will start in: " << offInterval );
		m_startStopEvent = Simulator::Schedule (offInterval, &SyncAudioApp::StartSending, this);
	}else{
		NS_LOG_INFO("Listener");
/*		Time offInterval = Seconds(m_offTime->GetValue());
		m_startStopEvent = Simulator::Schedule (Seconds(m_leaveTime), &SyncAudioApp::StartSending, this);*/
	}
}

void SyncAudioApp::StartSending(){
	NS_LOG_INFO(m_nodeName <<": Start Sending");
	m_lastStartTime = Simulator::Now();
	//ScheduleNextContent();
	ScheduleOnContent();
	ScheduleStopEvent();
}

void SyncAudioApp::CancelEvents(){
	NS_LOG_INFO(m_nodeName <<": Canceling Events");
	m_audioContentSeq = m_lastAudioContentSeq;
	//Simulator::Cancel(m_sendOnContentEvent);
	//Simulator::Cancel(m_sendOffContentEvent);
	Simulator::Cancel(m_startStopEvent);  // send on/off events.
	Simulator::Cancel(m_sendOnContentEvent);	//send on events
	Simulator::Cancel(m_sendOffContentEvent);	// send off events
	Simulator::Cancel(m_sendContentEvent);		// send audio event
	Simulator::Cancel(m_contentStartStopEvent);  //start/stop  audio event
	Simulator::Cancel(m_sendInterestEvent);  // send interest event
	Simulator::Cancel(m_startPipeStopEvent); //piping interest event
}

void SyncAudioApp::ScheduleStopEvent(){

	Time onInterval = Seconds (m_onTime->GetValue());
	ExponentialVariable exp(m_onTime->GetValue());
	//NS_LOG_INFO("exp(0.3)=" <<Seconds(exp.GetValue()));
	ScheduleOffContent();
	NS_LOG_INFO(m_nodeName << ": Schedule Stop Events" << ", will be off in " << exp.GetValue());
	m_startStopEvent = Simulator::Schedule(onInterval, &SyncAudioApp::StopSending, this);
}

void SyncAudioApp::StopSending(){
	NS_LOG_INFO(m_nodeName << ": Stop Sending");
	CancelEvents();
	ScheduleStartEvent();
}

//////////////////////////////////////////////////////////////////////////
//                    Start or Stop the  audio content generation application            			//
//////////////////////////////////////////////////////////////////////////

void SyncAudioApp::ScheduleContentStartEvent(){
	// Schedules the event to start sending data (switch to the "On" state)
	//NS_LOG_INFO(this);
	Time onInterval = Seconds (m_onTime->GetValue());
	ExponentialVariable exp(m_onTime->GetValue());
	//m_contentStartStopEvent = Simulator::Schedule (Seconds(m_interArrival), &SyncAudioApp::StartContent, this);
	m_contentStartStopEvent = Simulator::Schedule(Seconds(exp.GetValue()), &SyncAudioApp::StartContent, this);
}

void SyncAudioApp::StartContent(){
	NS_LOG_INFO(m_nodeName <<": Start Content");
	m_lastStartTime = Simulator::Now();
	//ScheduleNextContent();
	ScheduleNextContent();
	ScheduleContentStopEvent();
}


void SyncAudioApp::ScheduleContentStopEvent(){

	Time onInterval = Seconds (m_onTime->GetValue());
	ExponentialVariable exp(m_onTime->GetValue());
	//NS_LOG_INFO("exp(0.3)=" <<Seconds(exp.GetValue()));
	NS_LOG_INFO(m_nodeName << ": Schedule Stop content Events" );
	m_contentStartStopEvent = Simulator::Schedule(Seconds(exp.GetValue()), &SyncAudioApp::StopContent, this);
}

void SyncAudioApp::StopContent(){
	NS_LOG_INFO(m_nodeName << ": Stop content");
	CancelContentEvents();
	ScheduleContentStartEvent();
}

void SyncAudioApp::CancelContentEvents(){
	NS_LOG_INFO(m_nodeName <<": Canceling Events");
	m_audioContentSeq = m_lastAudioContentSeq;
	//Simulator::Cancel(m_sendOnContentEvent);
	//Simulator::Cancel(m_sendOffContentEvent);
	Simulator::Cancel(m_contentStartStopEvent);
	Simulator::Cancel(m_sendContentEvent);
}

// publish content locally, not to other remote service node
void SyncAudioApp::ScheduleNextContent() {
	if (m_start) {
		m_audioContentSeq = 0;
		m_sendContentEvent = Simulator::Schedule(Seconds(m_interArrival), &SyncAudioApp::generateContent, this);
		NS_LOG_INFO(m_nodeName<<": Schedule the first audio content generating");
		//NS_LOG_INFO(m_nodeName<<": ontime:" << m_onTime->GetValue() << " offtime: " << m_offTime->GetValue());
		m_start = false;

	} else if (!m_sendContentEvent.IsRunning()) {
		m_lastAudioContentSeq =m_audioContentSeq;
		m_audioContentSeq++;

		Time nextTime = (Seconds(m_interArrival ));
		m_sendContentEvent = Simulator::Schedule(nextTime, &SyncAudioApp::generateContent, this);
		NS_LOG_INFO(m_nodeName <<": ScheduleNextContent");
	}

}

//  Generate Content <contentPrefix, contentSize>
void SyncAudioApp::generateContent() {
	// convert uint32_t to string;
	std::stringstream ss2;
	ss2 << m_audioContentSeq;
	std::string str_audioContentSeq;
	ss2 >> str_audioContentSeq;

	std::stringstream ss3;
	ss3 << m_onSequence;
	std::string str_onContentSeq;
	ss3 >> str_onContentSeq;
	str_onContentSeq = "on-" + str_onContentSeq;

	Ptr<ndn::Name> m_audioContentName = Create<ndn::Name>(m_nodePrefix);
	//NS_LOG_DEBUG(m_nodeName<<": New-Audio Content prefix is "<<*m_audioContentName);
	m_audioContentName->Add("chatroom-cona");
	m_audioContentName->Add(m_accountId);
	m_audioContentName->Add("audio");
	m_audioContentName->Add(str_onContentSeq);
	m_audioContentName->Add(str_audioContentSeq);

	m_audioContentSize = m_virtualPayloadSize;

	//NS_LOG_INFO(m_nodeName<<": Audio prefix is "<<*m_audioContentName<<" and its size is "<<m_audioContentSize);
	m_contentDict[*m_audioContentName] = m_audioContentSize;

	// Get FIB object
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();

	Ptr<ndn::fib::Entry> fibEntry2 = fib->Add(m_audioContentName, m_face, 0);
	fibEntry2->UpdateStatus(m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);

	NS_LOG_INFO(m_nodeName<<": Audio Content sequence number is "<<m_audioContentSeq << " prefix: "
				<< *m_audioContentName << " size: "  << m_audioContentSize );
	/*
	//GenerateNotificationPrefix(m_videoContentName);
	GenerateNotificationPrefix(m_audioContentName);

	//if(Simulator::Now().GetSeconds() >= m_leaveTime)
	//	SyncAudioApp::StopApplication();
	*/
	ScheduleNextContent();
}

//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Content for On/Off       			//
//////////////////////////////////////////////////////////////////////////
void SyncAudioApp::ScheduleOnContent() {
	m_onSequence++;
	m_sendOnContentEvent = Simulator::Schedule(Seconds(0.0), &SyncAudioApp::generateOnContent, this);
	//ScheduleContentStartEvent();
	ScheduleNextContent();

}

void SyncAudioApp::ScheduleOffContent() {
	m_offSequence++;
	m_sendOffContentEvent = Simulator::Schedule(Seconds(0.0), &SyncAudioApp::generateOffContent, this);
	NS_LOG_INFO(m_nodeName<<": Schedule the audio Off content generating " << m_offSequence );

}

void SyncAudioApp::generateOnContent() {
	// convert uint32_t to string;
	std::stringstream ss2;
	ss2 << m_onSequence;
	std::string str_onContentSeq;
	ss2 >> str_onContentSeq;
	str_onContentSeq = "on-" + str_onContentSeq;

	Ptr<ndn::Name> m_audioContentName = Create<ndn::Name>(m_nodePrefix);
	//NS_LOG_DEBUG(m_nodeName<<": New-Audio Content prefix is "<<*m_audioContentName);
	m_audioContentName->Add("chatroom-cona");
	m_audioContentName->Add(m_accountId);
	m_audioContentName->Add("audio");
	//m_audioContentName->Add(str_audioContentSeq);
	m_audioContentName->Add(str_onContentSeq);

	m_audioContentSize = m_virtualPayloadSize;

	//NS_LOG_INFO(m_nodeName<<": Audio prefix is "<<*m_audioContentName<<" and its size is "<<m_audioContentSize);
	m_contentDict[*m_audioContentName] = m_audioContentSize;

	// Get FIB object
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();

	Ptr<ndn::fib::Entry> fibEntry2 = fib->Add(m_audioContentName, m_face, 0);
	fibEntry2->UpdateStatus(m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);

	NS_LOG_INFO(m_nodeName<<": Sending Audio On Interest" );
	/*	NS_LOG_INFO(m_nodeName<<": Audio Content sequence number is "<<m_audioContentSeq << " prefix: "
				<< *m_audioContentName << " size: "  << m_audioContentSize );*/

	/*GenerateNotificationPrefix(m_videoContentName);*/
	GenerateNotificationPrefix(m_audioContentName);

	/*if(Simulator::Now().GetSeconds() >= m_leaveTime)
		SyncAudioApp::StopApplication();*/

}

void SyncAudioApp::generateOffContent() {
	// convert uint32_t to string;
	std::stringstream ss2;
	ss2 << m_offSequence;
	std::string str_offContentSeq;
	ss2 >> str_offContentSeq;
	str_offContentSeq = "off-" + str_offContentSeq;

	Ptr<ndn::Name> m_audioContentName = Create<ndn::Name>(m_nodePrefix);
	//NS_LOG_DEBUG(m_nodeName<<": New-Audio Content prefix is "<<*m_audioContentName);
	m_audioContentName->Add("chatroom-cona");
	m_audioContentName->Add(m_accountId);
	m_audioContentName->Add("audio");
	//m_audioContentName->Add(str_audioContentSeq);
	m_audioContentName->Add(str_offContentSeq);

	m_audioContentSize = m_virtualPayloadSize;

	//NS_LOG_INFO(m_nodeName<<": Audio prefix is "<<*m_audioContentName<<" and its size is "<<m_audioContentSize);
	m_contentDict[*m_audioContentName] = m_audioContentSize;

	// Get FIB object
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();

	Ptr<ndn::fib::Entry> fibEntry2 = fib->Add(m_audioContentName, m_face, 0);
	fibEntry2->UpdateStatus(m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);

	NS_LOG_INFO(m_nodeName<<": Sending Audio Off Interest" );
/*	NS_LOG_INFO(m_nodeName<<": Audio Content sequence number is "<<m_audioContentSeq << " prefix: "
				<< *m_audioContentName << " size: "  << m_audioContentSize );*/

	/*GenerateNotificationPrefix(m_videoContentName);*/
	GenerateNotificationPrefix(m_audioContentName);
/*
	if(Simulator::Now().GetSeconds() >= m_leaveTime)
		SyncAudioApp::StopApplication();*/
}


//////////////////////////////////////////////////////////////////////////
//                    Start or Stop the  pipe application            			//
//////////////////////////////////////////////////////////////////////////

void SyncAudioApp::SchedulePipeStartEvent(){

	//Time offInterval = Seconds(m_offTime->GetValue());
	//ExponentialVariable exp(m_offTime->GetValue());
	NS_LOG_INFO(m_nodeName << ": Schedule start Pipe Events" );
	m_startPipeStopEvent = Simulator::Schedule (Seconds(m_interArrival), &SyncAudioApp::StartPipe, this);
}

void SyncAudioApp::StartPipe(){
	NS_LOG_INFO(m_nodeName <<": Start Piping");
	m_lastStartTime = Simulator::Now();
	//ScheduleNextContent();
	ScheduleNextInterest();
	SchedulePipeStopEvent();
}


void SyncAudioApp::SchedulePipeStopEvent(){

	Time onInterval = Seconds (m_onTime->GetValue());
	ExponentialVariable exp(m_onTime->GetValue());
	//NS_LOG_INFO("exp(0.3)=" <<Seconds(exp.GetValue()));
	NS_LOG_INFO(m_nodeName << ": Schedule Stop Pipe Events" );
	m_startPipeStopEvent = Simulator::Schedule(onInterval, &SyncAudioApp::StopPipe, this);

}

void SyncAudioApp::StopPipe(){
	NS_LOG_INFO(m_nodeName << ": Stop piping");
	CancelPipeEvents();
	//SchedulePipeStartEvent();
}

void SyncAudioApp::CancelPipeEvents(){
	NS_LOG_INFO(m_nodeName <<": Canceling Events");
	m_audioContentSeq = m_lastAudioContentSeq;
	Simulator::Cancel(m_sendInterestEvent);
	Simulator::Cancel(m_startPipeStopEvent);
	//Simulator::Cancel(m_startStopEvent);
}

// Processing when application is stopped
void SyncAudioApp::StopApplication() {
	NS_LOG_FUNCTION_NOARGS ();
	NS_ASSERT(GetNode()->GetObject<ndn::Fib>() != 0);
	// cancel periodic packet generation
	Simulator::Cancel(m_startStopEvent);
	//Simulator::Cancel(m_sendContentEvent);

	// cleanup ndn::App
	ndn::App::StopApplication();
}



//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Interest		           			//
//////////////////////////////////////////////////////////////////////////

void SyncAudioApp::ScheduleNextInterest() {
	m_interestName_global = m_interestName_global_save;

	if (m_start) {
		m_audioContentSeq=0;

		std::stringstream ss2;
		ss2 << m_audioContentSeq;
		std::string str_audioContentSeq;
		ss2 >> str_audioContentSeq;
		m_interestName_global.Add(str_audioContentSeq);

		m_sendInterestEvent = Simulator::Schedule(Seconds(m_interArrival), &SyncAudioApp::SendPipeInterest, this, m_interestName_global);
		NS_LOG_INFO(m_nodeName<<": Schedule the first audio Interest");
		//NS_LOG_INFO(m_nodeName<<": ontime:" << m_onTime->GetValue() << " offtime: " << m_offTime->GetValue());
		m_start = false;

	} else if (!m_sendContentEvent.IsRunning()) {
		m_lastAudioContentSeq =m_audioContentSeq;
		m_audioContentSeq++;
		std::stringstream ss2;
		ss2 << m_audioContentSeq;
		std::string str_audioContentSeq;
		ss2 >> str_audioContentSeq;
		m_interestName_global.Add(str_audioContentSeq);

		Time nextTime = (Seconds(m_interArrival ));
		m_sendInterestEvent = Simulator::Schedule(nextTime,  &SyncAudioApp::SendPipeInterest, this, m_interestName_global);
		NS_LOG_INFO(m_nodeName <<": ScheduleNextInterest:" << m_interestName_global);
	}
}

ndn::InterestHeader SyncAudioApp::GenerateInterestHeader(
		ndn::Name m_interestName) {
	// Create and configure ndn::InterestHeader
	ndn::InterestHeader interestHeader;
	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
	interestHeader.SetNonce(rand.GetValue());
	interestHeader.SetName(m_interestName);
	interestHeader.SetInterestLifetime(Seconds(1.0));
	//NS_LOG_INFO(m_nodeName<<": InterestHeader " << interestHeader);
	return interestHeader;
}

//Sending one Interest packet out
void SyncAudioApp::SendInterest(ndn::Name m_interestName) {
	//NS_LOG_INFO(m_nodeName<<": Start sending interest! " << m_interestName);
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

//Sending one Interest packet out
void SyncAudioApp::SendPipeInterest(ndn::Name m_interestName) {
	//NS_LOG_INFO(m_nodeName<<": Start sending interest! " << m_interestName);
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

	ScheduleNextInterest();
}

//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Notification		           			//
//////////////////////////////////////////////////////////////////////////
void SyncAudioApp::GenerateNotificationPrefix(Ptr<ndn::Name> m_contentName) {
	//NS_LOG_INFO(m_nodeName<<": Start generating notification prefix");
/*
	m_outSyncNotificationPrefix.routing_prefix = m_syncClientId; // proxy01 on service router 01
	m_outSyncNotificationPrefix.serviceId = "chatroom-cona";
	m_outSyncNotificationPrefix.m_fingerprint = *m_contentName;
*/
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

	NS_LOG_INFO(m_nodeName<<": Now: The notification prefix is "<<*m_syncPrefix);
	SendSyncNotification(m_syncPrefix);

}

ndn::InterestHeader SyncAudioApp::GenerateSyncNotificationHeader(
		Ptr<ndn::Name> m_syncPrefix) {
	NS_LOG_INFO(m_nodeName<<": Generate the notification header");
	//NS_LOG_DEBUG(m_nodeName<<": Sync Notification Prefix is "<<*m_syncPrefix);
	ndn::InterestHeader syncNotificationHeader;
	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
	syncNotificationHeader.SetNonce(rand.GetValue());
	syncNotificationHeader.SetName(m_syncPrefix);
	syncNotificationHeader.SetInterestLifetime(Seconds(1.0));
	//NS_LOG_INFO(m_nodeName<<": notification header " << syncNotificationHeader );
	return syncNotificationHeader;
}

void SyncAudioApp::SendSyncNotification(Ptr<ndn::Name> m_syncPrefix) {

	ndn::InterestHeader syncNotificationHeader;
	syncNotificationHeader = GenerateSyncNotificationHeader(m_syncPrefix);

	//NS_LOG_DEBUG(m_nodeName<<": Sending audio notification to sync client " << *m_syncPrefix);
	//NS_LOG_DEBUG(m_nodeName<<": Create the sync notification packet");
	// Create packet and add ndn::syncNotificationHeader
	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(syncNotificationHeader);

	// Forward packet to lower (network) layer
	m_protocolHandler(packet);

	// Call trace (for logging purposes)
	m_transmittedInterests(&syncNotificationHeader, this, m_face);
	m_lastStartTime = Simulator::Now();
	ScheduleNextContent();
}
 


////////////////////////////////////////////////////////////////////////
//          			Process incoming packets       			      //
////////////////////////////////////////////////////////////////////////


// Callback that will be called when Interest arrives
void SyncAudioApp::OnInterest(const Ptr<const ndn::InterestHeader> &interest, Ptr<Packet> origPacket) {
	ndn::Name interestName = interest->GetName();
	std::list<std::string> subPrefixList = interestName.GetComponents();

	std::stringstream ss2, temp;
	ss2 << interestName;
	std::string str_audioContentName;
	ss2 >> str_audioContentName;
	std::string delimiter = "/";

	std::string s;
	s=str_audioContentName;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
	    token = s.substr(0, pos);

	    //std::cout << token << std::endl;
	    s.erase(0, pos + delimiter.length());
	}
	//std::cout << "medie type: " << token <<  std::endl;
	//std::cout << str_audioContentName << std::endl;
	if(token == "video" || token == "chat")
		return;

	bool isNotification = false;


	//NS_LOG_INFO (m_nodeName<<": Received Interest packet  " << interest->GetName ());

	int prefixSize = subPrefixList.size();

	if (prefixSize == 3) // notification sent by sync-client has three components
	{
		isNotification = true;
		NS_LOG_INFO(m_nodeName<<": Received a sync notification from sync-client");
		OnNotification(interest, origPacket);
		return;
	}

	if (!isNotification) { //interest from other UEs
		if (m_contentDict.find(interestName) == m_contentDict.end()) {
			// not found
			NS_LOG_INFO(m_nodeName<<": There is no content matching the interest with name "<< interestName);
		} else {
			NS_LOG_INFO (m_nodeName<<": Received content request Interest packet " << interestName);
			ndn::ContentObjectHeader contentData;
			contentData.SetName(Create<ndn::Name>(interestName)); // data will have the same name as Interests

			ndn::ContentObjectTail trailer; // doesn't require any configuration

			// Create packet and add header and trailer
			int packet_size =m_virtualPayloadSize;
			Ptr<Packet> packet = Create<Packet>(packet_size);
			packet->AddHeader(contentData);
			packet->AddTrailer(trailer);

			//NS_LOG_INFO (m_nodeName<<": Sending ContentObject packet for " << contentData.GetName ());
			//NS_LOG_INFO (m_nodeName<<": packet content " << contentData);

			// Forward packet to lower (network) layer
			m_protocolHandler(packet);

			// Call trace (for logging purposes)
			m_transmittedContentObjects(&contentData, packet, this, m_face);
		}
	}
}


void SyncAudioApp::OnNotification(const Ptr<const ndn::InterestHeader> &interest,
		Ptr<Packet> origPacket) {
	ndn::Name remote_FP;
	remote_FP = interest->GetName().GetLastComponent();
	NS_LOG_INFO(m_nodeName<<": Received Notification for "<<interest->GetName());
	//NS_LOG_DEBUG(m_nodeName<<": The content name is: "<< remote_FP);
	//ndn::Name new_interestPrefix;
	ndn::Name m_interestName = remote_FP;

	// get prefix components
	std::string delimiter = "/";
	std::stringstream ss2;
	ss2 << interest->GetName();
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

	//get last token
	last_token = s.substr(0, pos);
	//NS_LOG_INFO(m_nodeName<<": token " << last_token);

	if(media_typeToken == "video" || media_typeToken == "chat")
		return;

	std::size_t found;
	found = last_token.find("on-");
	if (found!=std::string::npos){
		NS_LOG_INFO(m_nodeName<<": ONNNNNNNNNNNNNNNNNNNNNNNNNNNNNN ");

		//m_interestName.Add(str_audioContentSeq);
		m_interestName_global = m_interestName;
		m_interestName_global_save = m_interestName_global;
		//SendInterest(m_interestName);  // send new interest for content
		SchedulePipeStartEvent();
		//ScheduleNextInterest();
	}


	found = last_token.find("off-");
	if (found!=std::string::npos){
		NS_LOG_INFO(m_nodeName<<": OFFFFFFFFFFFFFFFFFFFFF ");
		//SendInterest(m_interestName);  // send new interest for content
		SchedulePipeStopEvent();
	}


}

// Callback that will be called when Data arrives
void SyncAudioApp::OnContentObject(const Ptr<const ndn::ContentObjectHeader> &contentObject,
		Ptr<Packet> payload) {

	ndn::Name received_prefix = contentObject->GetName();
	NS_LOG_INFO (m_nodeName<<": Receiving Audio ContentObject packet for " << received_prefix);

}



//////////////////////////////////////
//  	Others				 		//
//////////////////////////////////////



/**
 * @brief Set type of frequency randomization
 * @param value Either 'none', 'uniform', or 'exponential'
 */
void SyncAudioApp::SetRandomize(const std::string &value) {
	if (m_random)
		delete m_random;
	if (value == "uniform") {
		m_random = new UniformVariable(0.0, 2 * 1.0 / m_frequency);
	} else if (value == "exponential") {
		m_random = new ExponentialVariable( m_frequency, 33);
	} else
		m_random = 0;

	m_randomType = value;
}

/**
 * @brief Get type of frequency randomization
 * @returns either 'none', 'uniform', or 'exponential'
 */
std::string SyncAudioApp::GetRandomize() const {
	return m_randomType;
}


/*
 * A test function
 */
int  SyncAudioApp::AudioPacketSize(){
	//RandomVariable*  var = new LogNormalVariable(log(143502), log(132));
	//NS_LOG_INFO("From test function: value=" << var->GetInteger());
	//return var->GetValue();
/*

	int g771_packet_size = 136;
	//int g726_packet_size = 104;
	//int g729_packet_size = 70;

*/
	return m_virtualPayloadSize;
}


// Write to CSV file
int SyncAudioApp::WriteFile(std::string fname,
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
std::map<ndn::Name, std::string> SyncAudioApp::ReadFile(std::string fname) {
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
