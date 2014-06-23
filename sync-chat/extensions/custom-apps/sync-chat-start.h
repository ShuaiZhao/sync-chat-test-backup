/*
 * sync-chat-start.h
 *
 *  Created on: Apr 22, 2014
 *      Author: shuai
 */

#ifndef SYNC_CHAT_START_H_
#define SYNC_CHAT_START_H_


#include "ns3/ndn-app.h"
#include "ns3/ndn-name.h"
#include "ns3/random-variable.h"
#include "digest.h"
#include <map>
#include <vector>


namespace ns3 {

/**
 * @brief A simple custom application communicate with sync-client
 *
 * This applications demonstrates how to send Interests and respond with ContentObjects to incoming interests
 *
 * When application starts it "sets interest filter" (install FIB entry) for /prefix/sub, as well as
 * sends Interest for this prefix
 *
 * When an Interest is received, it is replied with a ContentObject with 1024-byte fake payload
 */
class SyncStartApp : public ndn::App
{
public:
  // register NS-3 type "SyncStartApp"
  static TypeId
  GetTypeId ();

  // constructor
  SyncStartApp();
  // destructor
  virtual ~SyncStartApp();

  // (overridden from ndn::App) Processing upon start of the application
  virtual void
  StartApplication ();

  // (overridden from ndn::App) Processing when application is stopped
  virtual void
  StopApplication ();



  /*------------------------------------------------------------------*
   * Process incoming packets: Interest/ContentObject/SynNotification *
   *------------------------------------------------------------------*/
  // incoming content interest
  virtual void
  OnInterest (const Ptr<const ndn::InterestHeader> &interest, Ptr<Packet> origPacket);

  // incoming content object
  virtual void
  OnContentObject (const Ptr<const ndn::ContentObjectHeader> &contentObject,
                   Ptr<Packet> payload);

  // incoming sync notification sent from proxy, carring the fingerprint
  void OnNotification(const Ptr<const ndn::InterestHeader> &syncPrefix, Ptr<Packet> origPacket);


  /*------------------------------------------------------------------*
   * Process incoming packets: Interest/ContentObject/SynNotification *
   *------------------------------------------------------------------*/
  // Write to CSV file
  int WriteFile(std::string fname,
		  std::map<ndn::Name, uint32_t> m_contentDict);
  // Read CSV file
  std::map<ndn::Name, std::string> ReadFile(std::string fname);

  /*------------------------------------------------------------------*
   * 						Publish Content					    	  *
   *------------------------------------------------------------------*/
  // generate new content prefix
  void generateContent();
  // generate new hello content prefix
  void generateHelloContent();
  //generate leave content
  void generateLeaveContent();
  // publish new content object
  void PublishContentObject();
  // schedule next new content
  void ScheduleNextContent();


  /*------------------------------------------------------------------*
   * 			Push new content notification				    	  *
   *------------------------------------------------------------------*/

  // generate new notification regarding the content update
  void GenerateNotificationPrefix(Ptr<ndn::Name> m_contentName);
  ndn::InterestHeader GenerateSyncNotificationHeader(Ptr<ndn::Name> m_syncPrefix);
  // send the notification to the proxy
  void SendSyncNotification(Ptr<ndn::Name> m_syncPrefix);
  // schedule next notification
  //void ScheduleNextNotification(); // when a new content is published


  /*------------------------------------------------------------------*
   * 			Send content interest 						    	  *
   *------------------------------------------------------------------*/
  // generateInterest based on the interest name
  ndn::InterestHeader GenerateInterestHeader(ndn::Name m_interestName);
  // when receives a new FP from sync service client
  void ScheduleNextInterest();
  // consumer's behavior
  void SendInterest (ndn::Name m_interestName);



  /**
  * @brief Set type of frequency randomization
  * @param value Either 'none', 'uniform', or 'exponential'
  */
  void
  SetRandomize (const std::string &value);

  /**
  * @brief Get type of frequency randomization
  * @returns either 'none', 'uniform', or 'exponential'
  */
  std::string
  GetRandomize () const;

  /**
   *  A test function for testing anything
   */
  int HelloPacketSize();


private:
  //Attributes information
  std::string 		m_syncClientId;   		// The routing prefix for the sync-client, should match m_clientPrefix in sync-client
  std::string  		m_accountId;
  std::string 		m_nodeName;
  ndn::Name 		m_nodePrefix;
  // variables
  double 			m_frequency;			///< @brief frequency of the interest packets (in Hz)
  bool 				m_start; 				///< @brief indicate the start point of the application
  RandomVariable	*m_random;				///< @brief generate random number
  std::string       m_randomType;			///< @brief distribution type

  EventId			m_sendContentEvent;		///< @brief EventId of pending "send content packet" event
  EventId			m_sendInterestEvent;	///< @brief EventId of pending "send interest packet" event

  // Content related variables
  uint32_t 			m_contentSeq; 			///< @brief currently sequence of generated content

  // added by shuai
  // video, audio packet sequence number
  uint32_t 		m_timeTrack;					//track current simulation time
  uint32_t          hello_time_interval;  //3s
  uint32_t 		 m_leaveTime;					// user leave system time s
  uint32_t 			m_ContentSeq;		///< @brief currently sequence of generated audio content
  uint32_t			m_contentSize;			///< @brief the size of the content generated
  uint32_t 			m_virtualPayloadSize;	///< @brief payload size of the content object
  std::map <ndn::Name, uint32_t> m_contentDict;	/// < @brief a dictionary for generated content <contentPrefix, contentSize>

  // Interest related variables
  UniformVariable 	m_nonce;				///< @brief nonce generator
  //uint32_t 			m_signature; // 0, means no signature, any other value application dependent (not a real signature)

  // Fingerprint related variables
  ndn::Name 		m_currFP;				///< @brief m_currFP is the same as m_contentPrefix in current design

  // Define the notification struct

  struct outSyncNotificationPrefix
  {
	  std::string routing_prefix;
	  std::string serviceId;
	  ndn::Name m_fingerprint;
  };
  outSyncNotificationPrefix m_outSyncNotificationPrefix;



};

} // namespace ns3

#endif /* SYNC_CHAT_START_H_ */
