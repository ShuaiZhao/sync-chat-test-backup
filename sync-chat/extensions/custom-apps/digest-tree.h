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

// Consider a tree with three levels: root, root's children(proxy level),
// and root's grandchildren (user level)

#ifndef DIGEST_TREE_H_
#define DIGEST_TREE_H_

#include "digest.h"
#include "digest-node.h"
#include <vector>
#include <list>


namespace ns3{


class DigestTree
{
public:
	//constructor
	DigestTree();
	//create tree with root;
	DigestTree(DigestNode *proot);

	~DigestTree();

	//Build Tree level by level
	void BuildTree(std::vector<Digest> nodeDigestVector, std::string parentId);

	// Get sibling node
	DigestTree GetRSibling();

	// Get child node
	DigestTree GetChild();

	// Get root
	DigestNode* GetRoot();

	void SetRoot(DigestNode* node);

	// Get node data: Digest
	Digest* GetData();

	//check whether the digest tree is empty
	bool IsEmpty();

	// check whether the digest tree just has root node
	bool IsAlone();

	// check whether the node is a leaf
	bool IsLeaf();

	//insert new node under a parent node with parentID
	void InsertNode(DigestNode *node, std::string parentID);

	//insert new node under a parent node with parentID (not used)
	void InsertNode2(Digest *digest, std::string parentID);

	//remove a node
	void RemoveNode(DigestNode *node);

	//update the digest tree: recompute the digest value for relevant nodes
	void UpdateTree();

	// Traverse all nodes in the tree
	void Traverse();

	int CountChild(DigestNode* parent);


	// Find a node with targetNodeId in a tree
	DigestNode* FindNode(std::string targetNodeId);



private:
	//the data (digest instance) stored in the node
	DigestNode *root;

};



} //ns3

#endif //DIGEST_TREE_H
