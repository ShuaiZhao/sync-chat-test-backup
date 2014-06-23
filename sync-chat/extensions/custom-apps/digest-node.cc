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

//digest-node.cc

#include "digest.h"
#include "digest-node.h"


namespace ns3{

// DigestNode class members
DigestNode::DigestNode(Digest *d)
	:digest(*d)
	,leftchild(0)
	,rightsibling(0)
{
	// empty body
}

DigestNode::DigestNode(Digest *d, DigestNode *lchild, DigestNode *rsibling)
	:digest(*d)
	,leftchild(lchild)
	,rightsibling(rsibling)
{
	// empty body
}

DigestNode::~DigestNode()
{

}


Digest* DigestNode::GetData()
{
	return &digest;
}

Digest::DigestInfo* DigestNode::GetDigestUnit()
{
	return digest.getDigestInfo();
}

DigestNode* DigestNode::GetLeftChild()
{
	return leftchild;
}

DigestNode* DigestNode::GetRightSibling()
{
	return rightsibling;
}

void DigestNode::setLeftChild(DigestNode *node)
{
	leftchild = node;
}

void DigestNode::setRightSibling(DigestNode *node)
{
	rightsibling = node;
}

} // ns3
