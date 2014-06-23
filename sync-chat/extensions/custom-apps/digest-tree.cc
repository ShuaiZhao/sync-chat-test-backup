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


// digest-tree.cc

#include "digest-tree.h"
#include "digest.h"
#include <vector>

#include <boost/foreach.hpp>


namespace ns3{

// DigestTree class members
DigestTree::DigestTree()
{
	root = 0;
}

// create tree with root
DigestTree::DigestTree(DigestNode *proot)
{
	root = proot;
}

DigestTree::~DigestTree()
{
	//delete root;
}

//check whether the digest tree is empty
bool DigestTree::IsEmpty()
{
	if (root == 0)
		return 1;
	else
		return 0;
}

// Determine whether the tree only has root node
bool DigestTree::IsAlone()
{
	if (!IsEmpty() && root->leftchild == 0 && root->rightsibling == 0)
		return true;
	else
		return false;
}

bool DigestTree::IsLeaf()
{
	if (!IsEmpty() && root->leftchild == 0)
		return true;
	else
		return false;
}


// Get sibling node
DigestTree DigestTree::GetRSibling()
{
	return root->GetRightSibling();
}

// Get child node
DigestTree DigestTree::GetChild()
{
	return root->GetLeftChild();
}

// Get Root node
DigestNode* DigestTree::GetRoot()
{
	return root;
}

void DigestTree::SetRoot(DigestNode* node)
{
	root = node;
}


// Get root's data
Digest* DigestTree::GetData()
{
	return root->GetData();
}

//Build Tree
void DigestTree::BuildTree(std::vector<Digest> nodeDigestVector, std::string parentId)
{
	std::cout<<"BuildTree start"<<std::endl;
	std::string sid;
	std::string sid2;
	Digest dc;


	//for (int i = 0; i < nodeDigestVector.size(); i++)
	for(std::vector<Digest>::iterator it = nodeDigestVector.begin() ; it != nodeDigestVector.end(); ++it)
	{

		//DigestNode *node = new DigestNode(&nodeDigestVector[i]);
		DigestNode *node = new DigestNode(&(*it));
		//std::cout<<"BuildTree: "<<&(node->GetData()->getDigestInfo()->nodeId)<<std::endl;
		std::cout<<"Insert: "<<node->GetDigestUnit()->nodeId<<std::endl;
		InsertNode(node, parentId);

		/*//Debugging
		DigestNode *child = GetChild().GetRoot();
		std::cout<<"child node is "<<child->GetData()<<std::endl;
		std::cout<<"child1: "<<child<<" and child2: "<<child->rightsibling<<std::endl;
		std::cout<<"Inside BuildTree: "
				<<child->GetData()<<" "
				<<child->GetData()->getDigestInfo()<<" "
				<<&(child->GetData()->getDigestInfo()->nodeId)<<" "
				<<*&(child->GetData()->getDigestInfo()->nodeId)<<std::endl;

		DigestNode *sibling = child->rightsibling;
		if (sibling != 0)
		{
			sid = sibling->GetData()->getDigestInfo()->nodeId;
			std::cout<<"Inside BuildTree: "
					<<sibling->GetData()<<" "
					<<sibling->GetData()->getDigestInfo()<<" "
					<<&(sibling->GetData()->getDigestInfo()->nodeId)<<" "
					<<*&(sibling->GetData()->getDigestInfo()->nodeId)<<std::endl;
		}
		if (sibling != 0 && sibling->GetRightSibling() != 0)
		{
			sid2 = sibling->GetRightSibling()->GetData()->getDigestInfo()->nodeId;
			std::cout<<&sid2<<" "<<sid2<<std::endl;
		}*/
	}

}


// Traverse all nodes in the tree
void DigestTree::Traverse()
{
	std::string rootId = root->GetDigestUnit()->nodeId;
	//std::cout<<"Traverse: this tree is "<<rootId<<std::endl;
	std::string nodeId;
	DigestTree Tcursor = this->GetChild();

	if (this->IsEmpty())
		return;
	if (this->IsAlone())
	{
		//std::cout<<"Traverse: the tree "<<rootId<<" is a single node tree"<<std::endl;
		return;
	}
	if (Tcursor.GetRoot()!=NULL)
	{
		nodeId = Tcursor.GetData()->getDigestInfo()->nodeId;
		//std::cout<<"Traverse "<<rootId<<": "<<nodeId<<std::endl;
		Tcursor.Traverse();
		Tcursor = Tcursor.GetRSibling();
		while(Tcursor.GetRoot() != NULL)
		{
			nodeId = Tcursor.GetData()->getDigestInfo()->nodeId;
			//std::cout<<"Traverse "<<rootId<<": "<<nodeId<<std::endl;
			Tcursor.Traverse();
			Tcursor = Tcursor.GetRSibling();
		}
	}
	else
	{
		//std::cout<<"Traverse: "<<rootId<<" has no children"<<std::endl;
		return;
	}
}

// Find a node with targetNodeId in a tree
DigestNode* DigestTree::FindNode(std::string targetNodeId)
{
	std::string rootId = root->GetDigestUnit()->nodeId;
	//std::cout<<"FindNode2: This tree's root is "<<rootId<<std::endl;
	std::string checkNodeId;
	DigestTree Tcursor = this->GetChild();
	DigestNode* find;
	if (this->IsEmpty())
		return NULL;
	if (rootId == targetNodeId)
	{
		//std::cout<<"FindNode2: "<<targetNodeId<<" is found! at root node "<<rootId<<std::endl;
		return root;
	}
	if (Tcursor.GetRoot()!=NULL)
	{
		checkNodeId = Tcursor.GetData()->getDigestInfo()->nodeId;
		//std::cout<<"FindNode2 in: "<<rootId<<"'s first child: "<<checkNodeId<<std::endl;
		find = Tcursor.FindNode(targetNodeId);

		if (find != NULL)
		{
			//std::cout<<"Find "<<targetNodeId<<" in the tree with root "<<checkNodeId<<std::endl;
			return find;
		}
		else
		{
			//std::cout<<"FindNode2 in sibling: "<<Tcursor.GetRSibling().GetData()->getDigestInfo()->nodeId<<std::endl;
			Tcursor = Tcursor.GetRSibling();
			//std::cout<<"FindNode2 in: "<<Tcursor.GetData()->getDigestInfo()->nodeId<<std::endl;
			while(Tcursor.GetRoot() != NULL)
			{
				checkNodeId = Tcursor.GetData()->getDigestInfo()->nodeId;
				//std::cout<<"FindNode2: "<<rootId<<": "<<checkNodeId<<std::endl;
				find = Tcursor.FindNode(targetNodeId);
				if (find != NULL)
				{
					//std::cout<<"Find "<<targetNodeId<<" in the tree with root "<<checkNodeId<<std::endl;
					return find;
				}
				else
				{
					//std::cout<<"FindNode2: Check other children of "<<rootId<<std::endl;
					Tcursor = Tcursor.GetRSibling();
				}
			}
		}
	}
	else
	{
		//std::cout<<"FindNode2: "<<rootId<<" has no children"<<std::endl;
		return NULL;
	}
	return find;
}


// Count the number of children of a parent node
int DigestTree::CountChild(DigestNode* parent)
{
	//std::cout<<"CountChild: Start counting."<<std::endl;
	int count = 0;
	DigestNode* cursor = parent;
	if (parent == 0)
	{
		//std::cout<<"CountChild: This node is empty."<<std::endl;
		return 0;
	}
	else if (parent->leftchild == 0)
	{
		//std::cout<<"CountChild: This node does not have child"<<std::endl;
		return 0;
	}
	else
	{
		cursor = cursor->GetLeftChild();
		std::string nodeID = cursor->GetDigestUnit()->nodeId;
		//std::cout<<"CountChild: the first child is "<<nodeID<<std::endl;
		count = count+1;
		if (cursor->rightsibling == 0)
		{
			std::cout<<"CountChild: This node only has one child."<<std::endl;
		}
		else
		{
			while(cursor->rightsibling!=0)
			{
				count = count + 1;
				cursor = cursor->rightsibling;
				//std::cout<<"CountChild: the number of children is increased by 1"<<std::endl;
			}
		}
	}

	//std::cout<<"CountChild: "<<count<<std::endl;
	return count;
}

/*void DigestTree::InsertNode2(Digest *digest, std::string parentID)
{
	DigestNode* parentNode = FindNode(parentID);
	DigestNode *node = new DigestNode(digest);
	node->rightsibling = parentNode->leftchild;
	parentNode->leftchild = node;

}*/


void DigestTree::InsertNode(DigestNode *node, std::string parentID)
{
	//std::cout<<"InsertNode: InsertNode function starts."<<std::endl;
	Digest::DigestInfo* dnode = node->GetData()->getDigestInfo();
	//std::cout<<"InsertNode: Insert "<<dnode->nodeId<<" to "<<parentID<<std::endl;
	DigestNode* parentNode = FindNode(parentID);


	//std::cout<<"InsertNode: Parent node is "<<parentNode->GetDigestUnit()->nodeId<<std::endl;

	/*if (parentNode->leftchild == 0)
		std::cout<<parentNode->GetDigestUnit()->nodeId<<" has no children! Insert the first child."<<std::endl;
	else
		std::cout<<parentNode->leftchild->GetDigestUnit()->nodeId<<std::endl;*/



	/*node->rightsibling = parentNode->leftchild;
	parentNode->leftchild = node;*/

	node->setRightSibling(parentNode->leftchild);
	parentNode->setLeftChild(node);

	//std::cout<<parentNode->leftchild->GetData()->getDigestInfo()<<std::endl;
	//std::cout<<parentNode->leftchild->GetData()->getDigestInfo()->nodeId<<std::endl;

	//std::cout<<"InsertNode: "<<parentNode->leftchild->GetDigestUnit()->nodeId<<" is the first child node"<<std::endl;
	/*if (node->rightsibling != 0)
	{
		std::cout<<"InsertNode: "<<node->GetDigestUnit()->nodeId
				<<" has sibling "<<node->rightsibling->GetDigestUnit()->nodeId
				<<std::endl;
	}
	else
	{
		std::cout<<"InsertNode: The root only has one child"<<std::endl;
	}*/
}

void DigestTree::RemoveNode(DigestNode *node)
{

}

}
