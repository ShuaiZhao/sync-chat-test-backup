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

#include "sync-video-pipe-app.h"
#include "../custom-apps/sync-client.h"
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>

NS_LOG_COMPONENT_DEFINE("SyncVideoApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(SyncVideoApp);

// register NS-3 type
TypeId SyncVideoApp::GetTypeId() {
		static TypeId tid =TypeId("SyncVideoApp").SetParent<ndn::App>()
		.AddConstructor<SyncVideoApp>()
		.AddAttribute("Prefix",
							"Prefix, for which producer has the data", StringValue("/"),
							ndn::MakeNameAccessor(&SyncVideoApp::m_nodePrefix),
							ndn::MakeNameChecker())
		.AddAttribute("AccountID",
							"AccoundID for the Application", StringValue("none"),
							MakeStringAccessor(&SyncVideoApp::m_accountId),
							MakeStringChecker())
		.AddAttribute("InterArrival",
							"Packet Inter Arrival Time", StringValue("1.0"),
							MakeDoubleAccessor(&SyncVideoApp::m_interArrival),
							MakeDoubleChecker<double>())
		.AddAttribute("PayloadSize",
							"Virtual payload size per content packets",
							IntegerValue(),
							MakeIntegerAccessor(&SyncVideoApp::m_virtualPayloadSize),
							MakeIntegerChecker<uint32_t>())
		.AddAttribute("StartSeq",
							"Initial sequence number", IntegerValue(0),
							MakeIntegerAccessor(&SyncVideoApp::m_contentSeq),
							MakeIntegerChecker<uint32_t>())
		.AddAttribute(
							"SyncClientID", "The sync proxy ID the client is connected",
							StringValue("none"),
							MakeStringAccessor(&SyncVideoApp::m_syncClientId),
							MakeStringChecker())
		.AddAttribute("LeaveTime", "When User will leave", IntegerValue(3),
							MakeIntegerAccessor(&SyncVideoApp::m_leaveTime),
							MakeIntegerChecker<uint32_t>())
		.AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
							StringValue ("ns3::ConstantRandomVariable[Constant=0.4]"),
							MakePointerAccessor (&SyncVideoApp::m_onTime),
							MakePointerChecker <RandomVariableStream>())
		.AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
							StringValue ("ns3::ConstantRandomVariable[Constant=0.001]"),
							MakePointerAccessor (&SyncVideoApp::m_offTime),
							MakePointerChecker <RandomVariableStream>())
		.AddAttribute("DataRate", "the data reate is on state", DataRateValue(DataRate("500kb/s")),
							MakeDataRateAccessor(&SyncVideoApp::m_cbrRate),
							MakeDataRateChecker());
		return tid;
}

// constructor
SyncVideoApp::SyncVideoApp() :
		m_frequency(), m_start(true), m_random() {
	NS_LOG_FUNCTION_NOARGS ();
	m_gopPosition = 1;  //mark initial gop poistion
	GetRandomize(); // initial m_randomNumber for access gopsize
	m_randomNumber=100;
	m_fetchInterArrival = 0.0;
}

// destructor
SyncVideoApp::~SyncVideoApp() {
	if (m_random)
		delete m_random;
}

//////////////////////////////////////////////////////////////////////////
//                    Start or Stop the application            			//
//////////////////////////////////////////////////////////////////////////

// Processing upon start of the application
void SyncVideoApp::StartApplication() {
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

	NS_LOG_DEBUG("This video node's prefix "<<m_nodePrefix);

	ScheduleStartEvent();
}

void SyncVideoApp::ScheduleStartEvent(){
	// Schedules the event to start sending data (switch to the "On" state)
	//NS_LOG_INFO(this);
	if(m_virtualPayloadSize != 0){
		NS_LOG_INFO("Video Source");	
		GetGopSize(1);	
		//GetRandomize();
		Time offInterval = Seconds(m_offTime->GetValue());
		gop_size =  m_gopSizeArray[m_randomNumber];
		m_interArrival = m_onTime->GetValue() / gop_size;
		NS_LOG_INFO(m_nodeName << ": **GOP** Position: " << m_gopPosition 
			<< " GOP Size: " << gop_size <<" Packet sending Interval:" << m_interArrival);
		//ExponentialVariable exp(m_offTime->GetValue());
		//NS_LOG_INFO("exp(0.7)=" <<Seconds(exp.GetValue()));
		m_startStopEvent = Simulator::Schedule (offInterval, &SyncVideoApp::StartSending, this);
	}else{
		NS_LOG_INFO("Video Receiver");
		//Time offInterval = Seconds(m_offTime->GetValue());
		//ExponentialVariable exp(m_offTime->GetValue());
		//NS_LOG_INFO("exp(0.7)=" <<Seconds(exp.GetValue());
		//m_startStopEvent = Simulator::Schedule (offInterval, &SyncVideoApp::StartSending, this);
	}
}

void SyncVideoApp::StartSending(){
	NS_LOG_INFO(m_nodeName <<": Start Sending");
	m_lastStartTime = Simulator::Now();
	//ScheduleNextContent();
	ScheduleOnContent();
	ScheduleStopEvent();
}

void SyncVideoApp::CancelEvents(){
	//NS_LOG_INFO(this);
	m_videoContentSeq = m_lastVideoContentSeq;
	Simulator::Cancel(m_startStopEvent);  // send on/off events.
	Simulator::Cancel(m_sendOnContentEvent);	//send on events
	Simulator::Cancel(m_sendOffContentEvent);	// send off events
	Simulator::Cancel(m_sendContentEvent);		// send audio event
	Simulator::Cancel(m_contentStartStopEvent);  //start/stop  audio event
	Simulator::Cancel(m_sendInterestEvent);  // send interest event
	Simulator::Cancel(m_startPipeStopEvent); //piping interest event
}

void SyncVideoApp::ScheduleStopEvent(){
	//NS_LOG_INFO(this);
	Time onInterval = Seconds (m_onTime->GetValue());
	ExponentialVariable exp(m_onTime->GetValue());
	//NS_LOG_INFO("exp(0.3)=" <<Seconds(exp.GetValue()));
	ScheduleOffContent();
	NS_LOG_INFO(m_nodeName << ": Schedule Stop Events" << ", will be off in " << exp.GetValue());
	m_startStopEvent = Simulator::Schedule(onInterval, &SyncVideoApp::StopSending, this);
}

void SyncVideoApp::StopSending(){
	//NS_LOG_INFO(this);

	// change to next gop size
	m_gopPosition++;
	m_randomNumber++;
	//starting from next gop size
	CancelEvents();
	ScheduleStartEvent();
}

void SyncVideoApp::ScheduleContentStartEvent(){
	// Schedules the event to start sending data (switch to the "On" state)
	//NS_LOG_INFO(this);
	Time onInterval = Seconds (m_onTime->GetValue());
	ExponentialVariable exp(m_onTime->GetValue());
	m_videoContentSeq = 0;
	//m_contentStartStopEvent = Simulator::Schedule (Seconds(m_interArrival), &SyncVideoApp::StartContent, this);
	m_contentStartStopEvent = Simulator::Schedule(Seconds(exp.GetValue()), &SyncVideoApp::StartContent, this);
}

void SyncVideoApp::StartContent(){
	NS_LOG_INFO(m_nodeName <<": Start Content");
	
	m_lastStartTime = Simulator::Now();
	//ScheduleNextContent();
	ScheduleNextContent();
	ScheduleContentStopEvent();
}

void SyncVideoApp::ScheduleContentStopEvent(){

	Time onInterval = Seconds (m_onTime->GetValue());
	ExponentialVariable exp(m_onTime->GetValue());
	//NS_LOG_INFO("exp(0.3)=" <<Seconds(exp.GetValue()));
	NS_LOG_INFO(m_nodeName << ": Schedule Stop content Events" );
	m_contentStartStopEvent = Simulator::Schedule(Seconds(exp.GetValue()), &SyncVideoApp::StopContent, this);
}

void SyncVideoApp::StopContent(){
	NS_LOG_INFO(m_nodeName << ": Stop content");
	
	CancelContentEvents();
	ScheduleContentStartEvent();
}

void SyncVideoApp::CancelContentEvents(){
	NS_LOG_INFO(m_nodeName <<": Canceling Events");
	//m_videoContentSeq = m_lastVideoContentSeq;

	Simulator::Cancel(m_contentStartStopEvent);
	Simulator::Cancel(m_sendContentEvent);
}

// Processing when application is stopped
void SyncVideoApp::StopApplication() {
	NS_LOG_FUNCTION_NOARGS ();
	NS_ASSERT(GetNode()->GetObject<ndn::Fib>() != 0);
	// cancel periodic packet generation
	Simulator::Cancel(m_startStopEvent);
	//Simulator::Cancel(m_sendContentEvent);

	// cleanup ndn::App
	ndn::App::StopApplication();
}

void SyncVideoApp::GetGopSize(uint32_t startFrom){
	//get gop size from result/log/gop_size.txt and store int into a array
	std::string gop_size_filename="/home/cona/workspace/ndnsim/sync-chat/results/log/gop_size.txt";
	std::ifstream file(gop_size_filename.c_str());
	if(file.is_open()){
		for(int  i =0; i<10000;  ++i){
				std::string temp;
				file >> temp;
				m_gopSizeArray[i] = atoi(temp.c_str());
				//std::cout<<m_gopSizeArray[i];
				//NS_LOG_INFO("GOP size is: " << m_gopSizeArray[i]);
		}
	}

	// print test GOP size
	//for (int i=0;i<=5;i++)
		//NS_LOG_INFO("GOP size is: " << m_gopSizeArray[i]);
}

//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Content		           			//
//////////////////////////////////////////////////////////////////////////

// publish content locally, not to other remote service node
void SyncVideoApp::ScheduleNextContent() {
	if (m_start) {
		m_videoContentSeq = 0;
		m_sendContentEvent = Simulator::Schedule(Seconds(m_interArrival), &SyncVideoApp::generateContent, this);
		NS_LOG_INFO(m_nodeName<<": Schedule the first video content generating");
		//NS_LOG_INFO(m_nodeName<<": ontime:" << m_onTime->GetValue() << " offtime: " << m_offTime->GetValue());
		m_start = false;

	} else if (!m_sendContentEvent.IsRunning()) {
		//m_lastVideoContentSeq =m_videoContentSeq;
		m_videoContentSeq++;

		Time nextTime = (Seconds(m_interArrival ));
		m_sendContentEvent = Simulator::Schedule(nextTime, &SyncVideoApp::generateContent, this);

	}
}

//  Generate Content <contentPrefix, contentSize>
void SyncVideoApp::generateContent() {
	// convert uint32_t to string;


	std::stringstream ss1;
	ss1 << gop_size;
	std::string str_gopSize;
	ss1 >> str_gopSize;

	std::stringstream ss2;
	ss2 << m_videoContentSeq;
	std::string str_videoContentSeq;
	ss2 >> str_videoContentSeq;


	std::stringstream ss3;
	ss3 << m_onSequence;
	std::string str_onContentSeq;
	ss3 >> str_onContentSeq;
	str_onContentSeq = "on-" + str_onContentSeq + '-' + str_gopSize;


	Ptr<ndn::Name> m_videoContentName = Create<ndn::Name>(m_nodePrefix);
	//NS_LOG_DEBUG(m_nodeName<<": New-Audio Content prefix is "<<*m_videoContentName);
	m_videoContentName->Add("chatroom-cona");
	m_videoContentName->Add(m_accountId);
	m_videoContentName->Add("video");
	m_videoContentName->Add(str_onContentSeq);
	m_videoContentName->Add(str_videoContentSeq);

	m_videoContentSize = m_virtualPayloadSize;

	/*NS_LOG_INFO(m_nodeName<<": Audio prefix is "
	 * <<*m_videoContentName<<" and its size is "<<m_audioContentSize);
	*/
	m_contentDict[*m_videoContentName] = m_videoContentSize;

	// Get FIB object
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();

	Ptr<ndn::fib::Entry> fibEntry2 = fib->Add(m_videoContentName, m_face, 0);
	fibEntry2->UpdateStatus(m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);

	NS_LOG_INFO(m_nodeName<<": Video Content sequence number is "<<m_videoContentSeq << " prefix: "
			<< *m_videoContentName << " size: "  << m_videoContentSize );
	//NS_LOG_INFO(m_nodeName<<": Audio FIB Entry "<< *m_videoContentName);

	/*GenerateNotificationPrefix(m_videoContentName);*/
	//GenerateNotificationPrefix(m_videoContentName);

	/*if(Simulator::Now().GetSeconds() >= m_leaveTime)
		SyncVideoApp::StopApplication();*/
	ScheduleNextContent();
}
//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Interest		           			//
//////////////////////////////////////////////////////////////////////////

void SyncVideoApp::ScheduleOnContent() {
	m_onSequence++;
	m_sendOnContentEvent = Simulator::Schedule(Seconds(0.0), &SyncVideoApp::generateOnContent, this);
	//ScheduleContentStartEvent();
	ScheduleNextContent();

}

void SyncVideoApp::ScheduleOffContent() {
	m_offSequence++;
	m_sendOffContentEvent = Simulator::Schedule(Seconds(0.0), &SyncVideoApp::generateOffContent, this);
	NS_LOG_INFO(m_nodeName<<": Schedule the video Off content generating " << m_offSequence );

}

void SyncVideoApp::generateOnContent() {
	// convert uint32_t to string;

	std::stringstream ss3;
	ss3 << gop_size;
	std::string str_gopSize;
	ss3 >> str_gopSize;
	//NS_LOG_INFO(m_nodeName<<": #### " << str_gopSize );

	std::stringstream ss2;
	ss2 << m_onSequence;
	std::string str_onContentSeq;
	ss2 >> str_onContentSeq;
	str_onContentSeq = "on-" + str_onContentSeq + '-' +str_gopSize;

	Ptr<ndn::Name> m_videoContentName = Create<ndn::Name>(m_nodePrefix);
	//NS_LOG_DEBUG(m_nodeName<<": New-Audio Content prefix is "<<*m_videoContentName);
	m_videoContentName->Add("chatroom-cona");
	m_videoContentName->Add(m_accountId);
	m_videoContentName->Add("video");
	//m_videoContentName->Add(str_audioContentSeq);
	m_videoContentName->Add(str_onContentSeq);

	m_videoContentSize = m_virtualPayloadSize;

	//NS_LOG_INFO(m_nodeName<<": Audio prefix is "<<*m_videoContentName<<" and its size is "<<m_audioContentSize);
	m_contentDict[*m_videoContentName] = m_videoContentSize;

	// Get FIB object
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();

	Ptr<ndn::fib::Entry> fibEntry2 = fib->Add(m_videoContentName, m_face, 0);
	fibEntry2->UpdateStatus(m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);

	NS_LOG_INFO(m_nodeName<<": Sending Video On Interest" );
	/*	NS_LOG_INFO(m_nodeName<<": Audio Content sequence number is "<<m_videoContentSeq << " prefix: "
				<< *m_videoContentName << " size: "  << m_audioContentSize );*/

	/*GenerateNotificationPrefix(m_videoContentName);*/
	GenerateNotificationPrefix(m_videoContentName);

	/*if(Simulator::Now().GetSeconds() >= m_leaveTime)
		SyncVideoApp::StopApplication();*/

}

void SyncVideoApp::generateOffContent() {
	// convert uint32_t to string;
	std::stringstream ss2;
	ss2 << m_offSequence;
	std::string str_offContentSeq;
	ss2 >> str_offContentSeq;
	str_offContentSeq = "off-" + str_offContentSeq;

	Ptr<ndn::Name> m_videoContentName = Create<ndn::Name>(m_nodePrefix);
	//NS_LOG_DEBUG(m_nodeName<<": New-Audio Content prefix is "<<*m_videoContentName);
	m_videoContentName->Add("chatroom-cona");
	m_videoContentName->Add(m_accountId);
	m_videoContentName->Add("video");
	//m_videoContentName->Add(str_audioContentSeq);
	m_videoContentName->Add(str_offContentSeq);

	m_videoContentSize = m_virtualPayloadSize;

	//NS_LOG_INFO(m_nodeName<<": Audio prefix is "<<*m_videoContentName<<" and its size is "<<m_audioContentSize);
	m_contentDict[*m_videoContentName] = m_videoContentSize;

	// Get FIB object
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();

	Ptr<ndn::fib::Entry> fibEntry2 = fib->Add(m_videoContentName, m_face, 0);
	fibEntry2->UpdateStatus(m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);

	NS_LOG_INFO(m_nodeName<<": Sending video Off Interest" );
/*	NS_LOG_INFO(m_nodeName<<": Audio Content sequence number is "<<m_audioContentSeq << " prefix: "
				<< *m_videoContentName << " size: "  << m_audioContentSize );*/

	/*GenerateNotificationPrefix(m_videoContentName);*/
	GenerateNotificationPrefix(m_videoContentName);
/*
	if(Simulator::Now().GetSeconds() >= m_leaveTime)
		SyncVideoApp::StopApplication();*/
}

//////////////////////////////////////////////////////////////////////////
//                    Start or Stop the  pipe application            			//
//////////////////////////////////////////////////////////////////////////

void SyncVideoApp::SchedulePipeStartEvent(){

	//Time offInterval = Seconds(m_offTime->GetValue());
	//ExponentialVariable exp(m_offTime->GetValue());
	//GetGopSize(1);	
		//GetRandomize();
	Time offInterval = Seconds(m_offTime->GetValue());
	//uint32_t gop_size =  m_gopSizeArray[m_randomNumber];
	m_fetchInterArrival = m_onTime->GetValue() / gop_size;
	NS_LOG_INFO(m_nodeName << ": Schedule start Pipe Events" << " GOP Size: " << gop_size <<" Packet sending Interval:" << m_fetchInterArrival);
	m_startPipeStopEvent = Simulator::Schedule (Seconds(m_fetchInterArrival), &SyncVideoApp::StartPipe, this);
}

void SyncVideoApp::StartPipe(){
	NS_LOG_INFO(m_nodeName <<": Start Piping");
	m_lastStartTime = Simulator::Now();
	m_videoContentSeq=0;

	//ScheduleNextContent();
	ScheduleNextInterest();
	SchedulePipeStopEvent();
}


void SyncVideoApp::SchedulePipeStopEvent(){

	Time onInterval = Seconds (m_onTime->GetValue());
	ExponentialVariable exp(m_onTime->GetValue());
	//NS_LOG_INFO("exp(0.3)=" <<Seconds(exp.GetValue()));
	NS_LOG_INFO(m_nodeName << ": Schedule Stop Pipe Events" );
	m_startPipeStopEvent = Simulator::Schedule(onInterval, &SyncVideoApp::StopPipe, this);

}

void SyncVideoApp::StopPipe(){
	NS_LOG_INFO(m_nodeName << ": Stop piping");
	m_gopPosition++;
	m_randomNumber++;
	CancelPipeEvents();
	//SchedulePipeStartEvent();
}

void SyncVideoApp::CancelPipeEvents(){
	NS_LOG_INFO(m_nodeName <<": Canceling Events");
	m_videoContentSeq = m_lastVideoContentSeq;
	Simulator::Cancel(m_sendInterestEvent);
	Simulator::Cancel(m_startPipeStopEvent);
	//Simulator::Cancel(m_startStopEvent);
}



//////////////////////////////////////////////////////////////////////////
//                    Generate & Send Interest		           			//
//////////////////////////////////////////////////////////////////////////

void SyncVideoApp::ScheduleNextInterest() {
	m_interestName_global = m_interestName_global_save;

	if (m_start) {
		m_videoContentSeq=0;

		std::stringstream ss2;
		ss2 << m_videoContentSeq;
		std::string str_videoContentSeq;
		ss2 >> str_videoContentSeq;
		m_interestName_global.Add(str_videoContentSeq);

		m_sendInterestEvent = Simulator::Schedule(Seconds(m_fetchInterArrival), &SyncVideoApp::SendPipeInterest, this, m_interestName_global);
		NS_LOG_INFO(m_nodeName<<": Schedule the first video Interest" << " with InterArrival:" << m_fetchInterArrival);
		//NS_LOG_INFO(m_nodeName<<": ontime:" << m_onTime->GetValue() << " offtime: " << m_offTime->GetValue());
		m_start = false;

	} else if (!m_sendContentEvent.IsRunning()) {
		m_lastVideoContentSeq =m_videoContentSeq;
		m_videoContentSeq++;
		std::stringstream ss2;
		ss2 << m_videoContentSeq;
		std::string str_videoContentSeq;
		ss2 >> str_videoContentSeq;
		m_interestName_global.Add(str_videoContentSeq);

		Time nextTime = (Seconds(m_interArrival ));
		m_sendInterestEvent = Simulator::Schedule(Seconds(m_fetchInterArrival),  &SyncVideoApp::SendPipeInterest, this, m_interestName_global);
		NS_LOG_INFO(m_nodeName <<": ScheduleNextInterest:" << m_interestName_global << " with InterArrival:" << m_fetchInterArrival);
	}
}


ndn::InterestHeader SyncVideoApp::GenerateInterestHeader(
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
void SyncVideoApp::SendInterest(ndn::Name m_interestName) {
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
void SyncVideoApp::SendPipeInterest(ndn::Name m_interestName) {
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
void SyncVideoApp::GenerateNotificationPrefix(Ptr<ndn::Name> m_contentName) {
	/*NS_LOG_INFO(m_nodeName<<": Start generating notification prefix:");
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

ndn::InterestHeader SyncVideoApp::GenerateSyncNotificationHeader(
		Ptr<ndn::Name> m_syncPrefix) {
	//NS_LOG_INFO(m_nodeName<<": Generate the notification header");
	//NS_LOG_DEBUG(m_nodeName<<": Sync Notification Prefix is "<<*m_syncPrefix);
	ndn::InterestHeader syncNotificationHeader;
	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
	syncNotificationHeader.SetNonce(rand.GetValue());
	syncNotificationHeader.SetName(m_syncPrefix);
	syncNotificationHeader.SetInterestLifetime(Seconds(1.0));
	//NS_LOG_INFO(m_nodeName<<": notification header " << syncNotificationHeader );
	return syncNotificationHeader;
}

void SyncVideoApp::SendSyncNotification(Ptr<ndn::Name> m_syncPrefix) {

	ndn::InterestHeader syncNotificationHeader;
	syncNotificationHeader = GenerateSyncNotificationHeader(m_syncPrefix);

	NS_LOG_DEBUG(m_nodeName<<": Sending video notification to sync client " << *m_syncPrefix);
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
void SyncVideoApp::OnInterest(const Ptr<const ndn::InterestHeader> &interest, Ptr<Packet> origPacket) {
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
		if(token == "audio" || token == "chat")
			return;

	bool isNotification = false;


	//NS_LOG_INFO (m_nodeName<<": Received Interest packet  " << interest->GetName ());

	int prefixSize = subPrefixList.size();

	if (prefixSize == 3) // notification sent by sync-client has three components
	{
		isNotification = true;
		//NS_LOG_INFO(m_nodeName<<": Received a sync notification from sync-client");
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


void SyncVideoApp::OnNotification(const Ptr<const ndn::InterestHeader> &interest,
		Ptr<Packet> origPacket) {
	ndn::Name remote_FP;
	remote_FP = interest->GetName().GetLastComponent();
	NS_LOG_INFO(m_nodeName<<": Received Notification for "<<interest->GetName());
	//NS_LOG_DEBUG(m_nodeName<<": The content name is: "<< remote_FP);
	//ndn::Name new_interestPrefix;
	ndn::Name m_interestName = remote_FP;

	// get prefix components
	
	std::stringstream ss2;
	ss2 << interest->GetName();
	std::string str_prefixComponents;
	ss2 >> str_prefixComponents;

	std::string delimiter = "/";
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
	NS_LOG_INFO(m_nodeName<<": token " << last_token);

	if(media_typeToken == "audio" || media_typeToken == "chat")
		return;

	std::size_t found;
	found = last_token.find("on-");
	if (found!=std::string::npos){
		NS_LOG_INFO(m_nodeName<<": ONNNNNNNNNNNNNNNNNNNNNNNNNNNNNN ");

	std::string delimiter = "-";
	std::string s;
	s=last_token;
	size_t pos = 0;
	std::string media_typeToken;
	std::string received_gopsize;
	while ((pos = s.find(delimiter)) != std::string::npos) {
			media_typeToken = s.substr(0, pos);
		    //NS_LOG_INFO(m_nodeName<<": token " << media_typeToken);
		    s.erase(0, pos + delimiter.length());
		}

		//get last token
		received_gopsize = s.substr(0, pos);
		NS_LOG_INFO(m_nodeName<<": received_gopsize " << received_gopsize);
		gop_size = atoi(received_gopsize.c_str());
		//m_interestName.Add(str_videoContentSeq);
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
void SyncVideoApp::OnContentObject(const Ptr<const ndn::ContentObjectHeader> &contentObject,
		Ptr<Packet> payload) {

	ndn::Name received_prefix = contentObject->GetName();
	NS_LOG_INFO (m_nodeName<<": Receiving Video ContentObject packet for " << received_prefix);

}

/**
 * @brief Set type of frequency randomization
 * @param value Either 'none', 'uniform', or 'exponential'
 */
void SyncVideoApp::SetRandomize(const std::string &value) {
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
void SyncVideoApp::GetRandomize(){
	std::srand(time(NULL));
	m_randomNumber = std::rand()%1000 + 500;	
}


/*
 * A test function
 */
int  SyncVideoApp::VideoPacketSize(){
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

//////////////////////////////////////
//  	CSV File Read & Write  		//
//////////////////////////////////////

// Write to CSV file
int SyncVideoApp::WriteFile(std::string fname,
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
std::map<ndn::Name, std::string> SyncVideoApp::ReadFile(std::string fname) {
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
