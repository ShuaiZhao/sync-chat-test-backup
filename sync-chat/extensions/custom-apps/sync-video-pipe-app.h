// sync-video-app.h

#ifndef MY_VIDEO_APP_H_
#define MY_VIDEO_APP_H_

#include "ns3/ndn-app.h"
#include "ns3/ndn-name.h"
#include "ns3/random-variable.h"
#include "../custom-apps/digest.h"
#include "ns3/data-rate.h"
#include <map>
#include <vector>
#include "ns3/random-variable-stream.h"


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
class SyncVideoApp : public ndn::App
{
public:
  // register NS-3 type "SyncVideoApp"
  static TypeId
  GetTypeId ();

  // constructor
  SyncVideoApp();
  // destructor
  virtual ~SyncVideoApp();

  // (overridden from ndn::App) Processing upon start of the application
  virtual void
  StartApplication ();

  void ScheduleStartEvent ();
  void ScheduleStopEvent ();
  void StartSending();
  void StopSending();
  void CancelEvents ();

  void ScheduleContentStartEvent();
  void ScheduleContentStopEvent ();
  void StartContent();
  void StopContent();

  void CancelPipeEvents ();
  void CancelContentEvents ();

  void SchedulePipeStartEvent ();
  void SchedulePipeStopEvent ();
  void StartPipe();
  void StopPipe();


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
  // publish new content object
  void PublishContentObject();
  // schedule next new content
  void ScheduleNextContent();


  // schedule next On/Off  content

  void ScheduleOnContent();
  void ScheduleOffContent();

  // generate new content prefix
  void generateOnContent();
  void generateOffContent();


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


  void SendPipeInterest (ndn::Name m_interestName);

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
  void
  GetRandomize ();

  /**
   *  A test function for testing anything
   */
  int VideoPacketSize();

  void
  GetGopSize(uint32_t startFrom);


private:
  //Attributes information
  ndn::Name m_interestName_global;
  ndn::Name m_interestName_global_save;
  uint32_t      m_contentSeq;       ///< @brief currently sequence of generated content
  uint32_t      m_onSequence;
  uint32_t      m_offSequence;

  std::string 		m_syncClientId;   		// The routing prefix for the sync-client, should match m_clientPrefix in sync-client
  std::string  		m_accountId;
  std::string 		m_nodeName;
  ndn::Name 		m_nodePrefix;
  // variables
  uint32_t    m_randomNumber;      // random start # for gop size
  uint32_t 			m_frequency;			///< @brief frequency of the interest packets (in Hz)
  uint32_t			m_gopSizeArray[10000];
  uint32_t    gop_size;
  uint32_t 		m_gopPosition;
  bool 				m_start; 				///< @brief indicate the start point of the application
  RandomVariable	*m_random;				///< @brief generate random number
  std::string       m_randomType;			///< @brief distribution type

  EventId 			m_startPipeStopEvent; // Event id for next start or stop event
 EventId 			m_contentStartStopEvent; // Event id for next start or stop event

EventId     m_sendOnContentEvent;   ///< @brief EventId of pending "send content packet" event
EventId     m_sendOffContentEvent;   ///< @brief EventId of pending "send content packet" event

EventId     m_sendOnInterestEvent;  ///< @brief EventId of pending "send interest packet" event

  EventId			m_sendContentEvent;		///< @brief EventId of pending "send content packet" event
  EventId			m_sendInterestEvent;	///< @brief EventId of pending "send interest packet" event
  EventId 			m_startStopEvent; // Event id for next start or stop event
  Time					m_lastStartTime; // Time last packet sent
  DataRate 		m_cbrRate; // Rate that data is generated

  // Content related variables


  // added by shuai
  // video, video packet sequence number
  Ptr<RandomVariableStream>  m_onTime;       // rng for On Time
  Ptr<RandomVariableStream>  m_offTime;      // rng for Off Time
  double 				m_interArrival;			//packet inter arrival time
  double        m_fetchInterArrival;     //packet fetch arrival time

  uint32_t 			m_leaveTime;					// user leave system time s
  uint32_t 			m_lastVideoContentSeq;		// keep track of last seq number of unfinished video content
  uint32_t 			m_videoContentSeq;		///< @brief currently sequence of generated video content
  uint32_t				m_videoContentSize;		///< @brief the size of the video content generated
  uint32_t				m_contentSize;			///< @brief the size of the content generated
  uint32_t 			m_virtualPayloadSize;	///< @brief payload size of the content object
  uint32_t 			m_residualBits; // Number of generated, but not sent, bits
  uint32_t 			m_maxBytes; // Limit total number of bytes sent
  uint32_t 			m_totBytes; // Total bytes sent so far

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

#endif // MY_VIDEO_APP_H_
