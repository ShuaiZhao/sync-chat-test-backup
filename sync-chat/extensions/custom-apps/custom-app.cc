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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

// custom-app.cc

#include "custom-app.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"

#include "ns3/core-module.h"
#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-content-object.h"
#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"

#include "ns3/ndn-fib.h"
#include "ns3/random-variable.h"
#include <string>
#include <sstream>
#include <iostream>
#include "digest.h"
//#include "digest-node.h"
#include "digest-tree.h"
#include <vector>
#include <boost/foreach.hpp>
#include <memory>
#include <boost/property_map/vector_property_map.hpp>

#include <openssl/evp.h>



NS_LOG_COMPONENT_DEFINE ("CustomApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CustomApp);

std::string toString(int num)
{
    std::string str;
    std::stringstream ss;
    ss<<num;
    ss>>str;
    return str;
}

std::string getNodeId(std::string prefix, int num)
{
    std::string nodeId;
    std::string seq_str = toString(num);
    nodeId = prefix + seq_str;
    return nodeId;
}

std::vector<std::string> getNodeVector(std::string nodePrefix, int node_num)
{
    std::vector<std::string> nodeVector;
    //int node_index = 0;
    for (int node_index=0; node_index<node_num; node_index++)
    {
        std::string node_Id = getNodeId(nodePrefix, node_index);
        nodeVector.push_back(node_Id);
    }
    return nodeVector;
}

void printNodeId(std::vector<std::string> nodeVector)
{
    int size = nodeVector.size();
    for(int i = 0; i<size;i++)
    {
        std::cout<<"The "<<i<<"th node is "<<nodeVector[i]<<"\n";
    }

}

Digest createDigest(std::string nodeId, uint32_t trackSeq, std::string digestValue)
{
	std::cout<<"createDigest is called."<<std::endl;
	Digest nodedigest;
	//nodedigest.setDigest(nodeId, trackSeq, digestValue);
	nodedigest.createDigestUnit(nodeId, trackSeq, digestValue);
	std::string digest_str;
	digest_str = nodedigest.getDigest();
	//std::cout<<"CreateDigest: Digest is "<<digest_str<<" at: "<<nodedigest.getDigestInfo()<<std::endl;
	std::cout<<"CreateDigest: Digest is "<<digest_str<<std::endl;
	return nodedigest;
}

std::vector<Digest> getNodeDigestVector(std::vector<std::string> nodeVector,
		uint32_t trackSeq, std::string digestValue)
{
	std::vector<Digest> nodeDigestVector;

	//nodeDigestVector.push_back(Digest());

	for(int i = 0; i < nodeVector.size(); i++)
	{

		//Digest *tempdigest = new Digest();

		std::cout<<i<<std::endl;
		//std::cout<<"tempdigest: "<<tempdigest->getDigestInfo()->digestValue<<std::endl;
		std::string tempNodeId = nodeVector[i];
		Digest tempdigest = createDigest(tempNodeId, trackSeq, digestValue);


		nodeDigestVector.push_back(tempdigest);
		std::cout<<tempdigest.getDigestInfo()->nodeId<<std::endl;

		tempdigest.DestroyDigestValue();
	}
	return nodeDigestVector;
}


std::vector<DigestNode> fun(std::vector<Digest> nodeDigestVector)
{
	 std::vector<DigestNode> nodevector;
	 for (std::vector<Digest>::iterator it = nodeDigestVector.begin(); it != nodeDigestVector.end(); ++it)
		 {
		 	 DigestNode *node = new DigestNode(&(*it));
		 	 nodevector.push_back(*node);

		 }
	 //DigestNode n = nodevector[0];
	 std::cout<<nodevector[0].GetData()->getDigestInfo()->nodeId<<std::endl;
	 return nodevector;
}




void test()
{
	// Initialization
	 std::vector<Digest> clientDigestVector;
	 std::vector<Digest> proxyDigestVector;

	 std::vector<Digest> controllerDigestVector;
	 std::string proxy_prefix = "proxy";
	 std::string controller_prefix = "controller";
	 std::string client_prefix = "client";

	 int num_controller = 4;
	 int num_proxy = 9;
	 int num_client = 5;

	    // Generates sequence number, by default it's given as 0
	 uint32_t trackSeq = 1;
	    // Generates digest Value, by default it's given as 100;
	 std::string digestValue = "100";

	    // Generates Node Names
	 std::string rootId = "root";
	 std::vector<std::string> clientVector = getNodeVector(client_prefix, num_client);
	 std::vector<std::string> proxyVector = getNodeVector(proxy_prefix, num_proxy);
	 std::vector<std::string> controllerVector = getNodeVector(controller_prefix, num_controller);
	 printNodeId(clientVector);
	 printNodeId(proxyVector);
	 printNodeId(controllerVector);


	 Digest rootdigest = createDigest(rootId, trackSeq, digestValue);
	 std::cout<<"Root Digest: "<<rootdigest.getDigestInfo()->digestValue<<std::endl;

	 // Generate client, proxy, controller digest pool
	 clientDigestVector = getNodeDigestVector(clientVector, trackSeq, digestValue);

	 proxyDigestVector = getNodeDigestVector(proxyVector, trackSeq, digestValue);
	 controllerDigestVector = getNodeDigestVector(controllerVector, trackSeq, digestValue);

	 for (int i = 0;i<controllerDigestVector.size();i++)
	 {
		Digest dc = controllerDigestVector[i];
		std::cout<<controllerDigestVector[i].getDigestInfo()<<" "<<dc.getDigestInfo()->nodeId<<std::endl;
	 }

	 std::vector<DigestNode> nodelist = fun(controllerDigestVector);
	 std::cout<<nodelist[0].GetData()->getDigestInfo()->nodeId<<std::endl;
	 std::cout<<nodelist[1].GetData()->getDigestInfo()->nodeId<<std::endl;
	 std::cout<<nodelist[2].GetData()->getDigestInfo()->nodeId<<std::endl;
	 std::cout<<nodelist[3].GetData()->getDigestInfo()->nodeId<<std::endl;


	 // Construct tree
	 DigestNode root = DigestNode(&rootdigest);
	 DigestTree dtree = DigestTree(&root);

	 DigestNode root1 = DigestNode(&rootdigest);
	 DigestTree dtree1 = DigestTree(&root1);

	 DigestNode node1 = nodelist[0];
	 dtree.InsertNode(&node1, "root");

/*	 std::cout<<"Start creating digest tree"<<std::endl;
	 int count = 0;
	 count = dtree.CountChild(&root);
	 std::cout<<"Test1: root has "<< count<<" children."<<std::endl;


	 Digest::DigestInfo* dr = dtree.GetRoot()->GetData()->getDigestInfo();
	 std::string dr_str = dr->nodeId;
	 std::cout<<"Get root digest: "<<dr_str<<std::endl;
	 std::cout<<"Root Digest: "<<dr<<std::endl;





// Build tree using loop
	 std::cout<<"Build tree using loop"<<std::endl;
	 dtree.BuildTree(controllerDigestVector, "root");

	 // Debugging
	 std::cout<<"Outside BuildTree: "
			 <<dtree.GetChild().GetRoot()->GetData()<<" "
			 <<dtree.GetChild().GetRoot()->GetData()->getDigestInfo()<<" "
			 <<&(dtree.GetChild().GetRoot()->GetData()->getDigestInfo()->nodeId)<<" "
			 <<*&(dtree.GetChild().GetRoot()->GetData()->getDigestInfo()->nodeId)
			 <<std::endl;
	 std::cout<<"Outside BuildTree: "
	 			 <<dtree.GetChild().GetRSibling().GetRoot()->GetData()<<" "
	 			 <<dtree.GetChild().GetRSibling().GetRoot()->GetData()->getDigestInfo()<<" "
	 			 <<&(dtree.GetChild().GetRSibling().GetRoot()->GetData()->getDigestInfo()->nodeId)<<" "
	 			 <<*&(dtree.GetChild().GetRSibling().GetRoot()->GetData()->getDigestInfo()->nodeId)
	 			 <<std::endl;

	 Digest::DigestInfo *dc1 = dtree.GetChild().GetRoot()->GetData()->getDigestInfo();
	 Digest::DigestInfo *dc0 = dtree.GetChild().GetRSibling().GetRoot()->GetData()->getDigestInfo();
	 std::cout<<"WITH LOOP: "<<&(dc1->nodeId)<<" "<<*&(dc1->nodeId)<<std::endl;
	 std::cout<<"WITH LOOP: "<<&(dc0->nodeId)<<" "<<*&(dc0->nodeId)<<std::endl;



	 dtree.BuildTree(proxyDigestVector, "controller1");

	 dtree.BuildTree(clientDigestVector, "proxy4");
	 dtree.Traverse();
*/




/*
	 std::vector<DigestNode> nodelist = fun(controllerDigestVector);
	 DigestNode n1 = nodelist[0];
	 std::string nid = n1.GetData()->getDigestInfo()->nodeId;
	 std::cout<<nid<<std::endl;*/





}
// register NS-3 type
TypeId
CustomApp::GetTypeId ()
{
  static TypeId tid = TypeId ("CustomApp")
    .SetParent<ndn::App> ()
    .AddConstructor<CustomApp> ()
    ;
  return tid;
}

// Processing upon start of the application
void
CustomApp::StartApplication ()
{
  // initialize ndn::App
  ndn::App::StartApplication ();
  test();
  // Create a name components object for name ``/prefix/sub``
  Ptr<ndn::NameComponents> prefix = Create<ndn::NameComponents> (); // now prefix contains ``/``
  prefix->Add ("prefix"); // now prefix contains ``/prefix``
  prefix->Add ("sub/service"); // now prefix contains ``/prefix/sub``
  prefix->Add ("name/game");
  std::string my_prefix;
  std::stringstream ss;

  Ptr<ndn::Name> newName = prefix;
  newName->Add("newPrefix");
  std::cout<<*prefix<<" "<<*newName<<std::endl;

  std::cout<<"get name components"<<std::endl;
  int count = 1;
  int index = 3;
  BOOST_FOREACH(const std::string &component, prefix->GetComponents())
  {
	  std::cout<<"count: "<<count<<std::endl;
	  std::cout<<component<<std::endl;
	  if (index == count)
	  {
		  std::cout<<"find it "<<component<<std::endl;
		  break;
	  }
	  count++;

  }



  std::string last_comp = prefix->GetLastComponent();
  std::cout<<last_comp<<std::endl;

  ss<<*prefix;
  ss>>my_prefix;
  std::cout<<my_prefix<<std::endl;
  my_prefix.erase(0,1);
  std::cout<<my_prefix<<std::endl;
  /////////////////////////////////////////////////////////////////////////////
  // Creating FIB entry that ensures that we will receive incoming Interests //
  /////////////////////////////////////////////////////////////////////////////

  // Get Node Name
  Ptr<Node> node = GetNode();
  std::string node_name;
  node_name = Names::FindName(node);
  NS_LOG_DEBUG("this node's name is "<<node_name);

  // Get FIB object
  Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();

  // Add entry to FIB
  // Note that ``m_face`` is cretaed by ndn::App
  Ptr<ndn::fib::Entry> fibEntry = fib->Add (*prefix, m_face, 0);

  Simulator::Schedule (Seconds (0.0), &CustomApp::SendInterest, this);
}

// Processing when application is stopped
void
CustomApp::StopApplication ()
{
  // cleanup ndn::App
  ndn::App::StopApplication ();
}

size_t hash(const char *str)
{
    size_t h = 0;
    while (*str)
       h = h << 1 ^ *str++;
    return h;
}

void
CustomApp::SendInterest ()
{
  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////
  
  // testing my_prefix type
  //ndn::Name my_prefix;
  std::vector<std::string> nameVector;


  my_prefix.Add("prefix");
  nameVector.push_back(my_prefix.GetLastComponent());
  my_prefix.Add("sub");
  nameVector.push_back(my_prefix.GetLastComponent());
  my_prefix.Add("sub2");
  nameVector.push_back(my_prefix.GetLastComponent());
  std::cout<<my_prefix<<std::endl;
  std::string SenderId = *my_prefix.GetComponents().begin();
  std::cout<<"First component: "<<SenderId<<std::endl;

  std::string SenderId1 = "hello1";
  size_t i = hash(SenderId1.c_str());
  std::cout<<"To integer: "<<SenderId1<<" "<<i<<std::endl;

  // find sub2 in the vector
  std::vector<std::string>::iterator it;
  it = std::find(nameVector.begin(), nameVector.end(), "sub2");
  //++it;
  if (it == nameVector.end())
  {
	  std::cout<<"Don't find it!"<<std::endl;
  }
  else
  {
	  std::cout<<"Find name string: "<<*it<<" "<<&(*it)<<std::endl;

  }


  // -- Test Hashing
  std::string str1 = "hello";
  std::string str2 = "how";
  std::string str3 = "are";
  std::string str4 = "you";

  size_t a1 = 1;
  size_t a2 = 2;
  size_t a3 = 3;

  EVP_MD_CTX *mdctx;
  const EVP_MD *md;


  OpenSSL_add_all_digests();

  md = EVP_get_digestbyname("md5");
  mdctx = EVP_MD_CTX_create();
  std::cout<<"first mdctx: "<<mdctx<<std::endl;
  EVP_DigestInit_ex(mdctx, md, NULL);

  uint8_t md_value[EVP_MAX_MD_SIZE];
  unsigned int md_len, j;

  EVP_DigestUpdate(mdctx, str1.c_str(), strlen(str1.c_str()));

  EVP_DigestUpdate(mdctx, str2.c_str(), strlen(str2.c_str()));
  //EVP_DigestUpdate(mdctx, str3.c_str(), strlen(str3.c_str()));
  EVP_DigestFinal_ex(mdctx, md_value, &md_len);
  EVP_MD_CTX_destroy(mdctx);
  mdctx = EVP_MD_CTX_create();
  std::cout<<"Second mdctx: "<<mdctx<<std::endl;

  printf("Digest is: ");
  for(j = 0; j < md_len; j++)
  {
	  printf("%02x", md_value[j]);
  }
  printf("\n");


  size_t res = *(reinterpret_cast<std::size_t*> (&md_value));
  std::cout<<res<<std::endl;

  // ENd test hashing function


  Ptr<ndn::NameComponents> prefix = Create<ndn::NameComponents> ("/prefix/sub"); // another way to create name

  // Create and configure ndn::InterestHeader
  ndn::InterestHeader interestHeader;
  UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
  interestHeader.SetNonce            (rand.GetValue ());
  interestHeader.SetName             (my_prefix);
  interestHeader.SetInterestLifetime (Seconds (1.0));

  // Create packet and add ndn::InterestHeader
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (interestHeader);

  NS_LOG_DEBUG ("Sending Interest packet for " << my_prefix);
  
  ndn::FwHopCountTag hopCountTag;
  packet->AddPacketTag (hopCountTag);
  NS_LOG_DEBUG("hopCountTag is "<<hopCountTag.Get());
  // Forward packet to lower (network) layer
  m_protocolHandler (packet);

  // Call trace (for logging purposes)
  m_transmittedInterests (&interestHeader, this, m_face);
}


////////////////////////////////////////////////////////////////////////
//          			Process incoming packets       			      //
////////////////////////////////////////////////////////////////////////

// Callback that will be called when Interest arrives
void
CustomApp::OnInterest (const Ptr<const ndn::InterestHeader> &interest, Ptr<Packet> origPacket)
{
  NS_LOG_DEBUG ("Received Interest packet for " << interest->GetName ());

  // Create and configure ndn::ContentObjectHeader and ndn::ContentObjectTail
  // (header is added in front of the packet, tail is added at the end of the packet)

  // Note that Interests send out by the app will not be sent back to the app !
  
  ndn::ContentObjectHeader data;
  data.SetName (Create<ndn::NameComponents> (interest->GetName ())); // data will have the same name as Interests

  ndn::ContentObjectTail trailer; // doesn't require any configuration

  // Create packet and add header and trailer
  Ptr<Packet> packet = Create<Packet> (1024);
  packet->AddHeader (data);
  packet->AddTrailer (trailer);

  NS_LOG_DEBUG ("Sending ContentObject packet for " << data.GetName ());

  // Forward packet to lower (network) layer
  m_protocolHandler (packet);

  // Call trace (for logging purposes)
  m_transmittedContentObjects (&data, packet, this, m_face);
}

// Callback that will be called when Data arrives
void
CustomApp::OnContentObject (const Ptr<const ndn::ContentObjectHeader> &contentObject,
                            Ptr<Packet> payload)
{
  NS_LOG_DEBUG ("Receiving ContentObject packet for " << contentObject->GetName ());

  std::cout << "DATA received for name " << contentObject->GetName () << std::endl;
  int hopCount = -1;
  ndn::FwHopCountTag hopCountTag;
  std::cout <<payload->RemovePacketTag (hopCountTag)<<std::endl;
  if (payload->RemovePacketTag (hopCountTag))
  {
      hopCount = hopCountTag.Get ();
      NS_LOG_DEBUG( "hopCount is " << hopCount );
  }

}


} // namespace ns3
