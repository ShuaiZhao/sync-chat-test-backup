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


#include "digest.h"


namespace ns3{

class DigestTree;

class DigestNode
{
	friend class DigestTree;
public:
	// constructor
	DigestNode(Digest *d);
	DigestNode(Digest *d, DigestNode *lchild, DigestNode *rsibling);
	~DigestNode();
	Digest::DigestInfo* GetDigestUnit();
	Digest* GetData();
	DigestNode* GetLeftChild();
	DigestNode* GetRightSibling();
	void setLeftChild(DigestNode* node);
	void setRightSibling(DigestNode* node);


private:
	Digest digest; // data stored at the node, which is a digest structure
	DigestNode *leftchild; // pointer to the child node
	DigestNode *rightsibling; // pointer to the sibling node that under the same parent node
};


}
