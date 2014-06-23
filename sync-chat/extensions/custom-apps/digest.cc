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
 * Author: Xuan Liu <xuan.liu@mail.umkc.edu>
 */

// digest.cc

#include "digest.h"
#include <string>
#include <cstring>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include <openssl/evp.h>


#define HASH_FUNCTION EVP_md5
#define DIGEST_NAME "md5"


namespace ns3{


Digest::Digest()
	:m_digestBuffer(0)
	,m_hashLength(0)
{
	digestinfo.nodeId = "none";
	digestinfo.trackSeq = "0";
	digestinfo.digestValue = "0000";

	// Initialize m_mdctx
	OpenSSL_add_all_digests();
	m_mdctx = EVP_MD_CTX_create();
	reset();
}

Digest::Digest(std::string nodeId, std::string initDigest)
	:m_digestBuffer(0)
	,m_hashLength(0)
{
	// Initialize digestinfo
	digestinfo.nodeId = nodeId;
	digestinfo.trackSeq = "0";
	digestinfo.digestValue = initDigest;

	// Initialize m_mdctx
	OpenSSL_add_all_digests();
	m_mdctx = EVP_MD_CTX_create();
	reset();
}

//distruct
Digest::~Digest()
{

}


bool Digest::empty() const
{
	return digestinfo.digestValue == "0000";
}

void Digest::reset()
{
	if (m_digestBuffer != 0)
	{
		delete [] m_digestBuffer;
		m_digestBuffer = 0;
	}

	int ok = EVP_DigestInit_ex(m_mdctx, HASH_FUNCTION (), 0);
	if (!ok)
	{
		std::cout<<"EVP_DigestInit_ex returned error "<<ok<<std::endl;
	}
}

// Incremental the trackseq in a digest
void Digest::DigestSeqIncrement()
{
	int newSeq;
	std::string newSeq_str;
	newSeq = atoi(digestinfo.trackSeq.c_str())+1;
	std::stringstream ss;
	ss<<newSeq;
	ss>>newSeq_str;
	digestinfo.trackSeq=newSeq_str;
}


bool Digest::isZero() const
{
	if (m_digestBuffer == 0)
		std::cout<<"Digest has not been yet finalized"<<std::endl;

	return (m_hashLength == 1 && m_digestBuffer[0] == 0);
}
// set the digest instance
void Digest::setDigest(std::string nodeId, uint32_t seq, size_t value)
{
	std::stringstream ss1, ss2;
	digestinfo.nodeId = nodeId;
	ss1 << seq;
	ss1 >> digestinfo.trackSeq;
	ss2 << value;
	ss2 >> digestinfo.digestValue;

}

void Digest::createDigestUnit(std::string nodeId, uint32_t seq, std::string digestValue)
{
	std::stringstream ss1;
	digestinfo.nodeId = nodeId;
	ss1 << seq;
	ss1 >> digestinfo.trackSeq;
	digestinfo.digestValue = digestValue;
}

void Digest::createDigest()
{
	OpenSSL_add_all_digests();
	m_mdctx = EVP_MD_CTX_create();
}

// Update Digest Value with hash function
void Digest::updateDigestValue(std::string digest_str)
{
	EVP_DigestUpdate(m_mdctx, digest_str.c_str(), std::strlen(digest_str.c_str()));
}

void Digest::finalizeDigestValue()
{
	if (m_digestBuffer != 0)
		return;
	m_digestBuffer = new uint8_t[EVP_MAX_MD_SIZE];
	int ok = EVP_DigestFinal_ex(m_mdctx, m_digestBuffer, &m_hashLength);
	if (!ok)
	{
		std::cout<<"EVP_DigestFinal_ex returned error. "<<ok<<std::endl;
	}
}

std::string Digest::getHashValue() const
{
	if (isZero()) return 0;

	if (sizeof(std::size_t) > m_hashLength)
	{
		std::cout<<"Hash is not zero and length is less than size_t"<<std::endl;
	}
	std::stringstream ss;
	std::string digest_str;
	size_t digestValue = *(reinterpret_cast<std::size_t*> (m_digestBuffer));
	ss<<digestValue;
	ss>>digest_str;
	return digest_str;
}

void Digest::DestroyDigestValue()
{
	//std::cout<<"m_mdctx: "<<m_mdctx<<std::endl;
	EVP_MD_CTX_destroy(m_mdctx);
}


size_t Digest::hash(const char *clientfp_str)
{
    size_t h = 0;
    while (*clientfp_str)
       h = h << 1 ^ *clientfp_str++;
    return h;
}

// combine three elements of the digest struct as one string, which is used in notification
std::string Digest::getDigest()
{
	std::string digest_str;
	digest_str = digestinfo.nodeId + "-"
				+ digestinfo.trackSeq + "-"
				+ digestinfo.digestValue;
	return digest_str;
}

// re-construct the digest struct when receiving the notification
void Digest::RetrieveDigest(std::string digest_str)
{
	std::vector<std::string> key_value;
	std::stringstream strstr(digest_str);
	std::string word;
	while(getline(strstr, word, '-'))
	{
		key_value.push_back(word);
	}
	digestinfo.nodeId = key_value[0];
	digestinfo.trackSeq = key_value[1];
	digestinfo.digestValue = key_value[2];
}

Digest::DigestInfo* Digest::getDigestInfo()
{
	return &digestinfo;
}


}
