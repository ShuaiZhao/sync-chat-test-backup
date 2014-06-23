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

#ifndef DIGEST_H_
#define DIGEST_H_


#include<string>
#include "ns3/uinteger.h"
#include<openssl/evp.h>

namespace ns3{

class DigestNode;


class Digest
{
	friend class DigestNode;

public:

	// constructor
	Digest();

	Digest(std::string nodeId, std::string initDigest);

	// destructor
	~Digest();

	// check if the digest is empty
	bool empty() const;

	// reset digest m_mdctx
	void reset();

	// detect whether the hash has been finalized or not
	bool isZero() const;

	// a hash function for client fingerprint (for debugging)
	size_t hash(const char *clientfp_str);

	// Create Digest
	void createDigestUnit(std::string nodeId, uint32_t seq, std::string digestValue);

	// create new m_mdctx (from openssl library)
	void createDigest();

	// Update Digest Value with hash function
	void updateDigestValue(std::string digest_str);

	// finalize the digest value
	void finalizeDigestValue();

	// get hash value
	std::string getHashValue() const;

	// destroy m_mdctx
	void DestroyDigestValue();

	// parse the digest info from the notification and create digest instance locally
	void setDigest(std::string nodeId, uint32_t seq, size_t value);

	// get the digest value from the digest instance, and append it to the notification
	std::string getDigest();

	// Incremental the trackseq in a digest
	void DigestSeqIncrement();

	// re-construct the digest struct when receiving the notification
	void RetrieveDigest(std::string digest_str);

	// Define DigestInfo Unit, consisting three components
	struct DigestInfo
	{
		std::string nodeId;
		std::string trackSeq;
		std::string digestValue;
	};

	DigestInfo* getDigestInfo();
private:

	DigestInfo digestinfo;
	EVP_MD_CTX *m_mdctx;
	uint8_t *m_digestBuffer;
	uint32_t m_hashLength;


};



} // ns3

#endif //DIGEST_H
