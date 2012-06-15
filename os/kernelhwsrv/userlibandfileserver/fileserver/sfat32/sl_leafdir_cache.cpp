// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// f32\sfat\sl_leafdir_cache.cpp
// 
//

#include "sl_std.h"
#include "sl_leafdir_cache.h"




/**
Get the lru list count

@return the count of lru list
*/
TInt CLeafDirTree::LruCount() const 
	{
	return iLruList.Count();
	}

/**
Count currently cached items

@return the number of currently cached items
*/
TInt CLeafDirCache::CacheCount() const 
	{
	return iTree->LruCount();
	}

//---------------------------------------------------------------------------------------------------------------------------------
/**
Default constructor of TDirPosition, a data structure that represents a location of directory
*/
TLeafDirData::TLeafDirData()
             :iClusterNum(0),iMRUPos(0,0)
	{
	}

/**
Constructor of TDirPosition, a data structure that represents a location of directory

@param  aClusterNum		the cluster number of the directory stores   
*/
TLeafDirData::TLeafDirData(TUint aClusterNum)
             :iClusterNum(aClusterNum),iMRUPos(0,0)
	{
	}

/**
Constructor of TDirPosition, a data structure that represents a location of directory

@param  aClusterNum		the cluster number of the directory stores   
*/
TLeafDirData::TLeafDirData(TUint aClusterNum, const TEntryPos& aMRUPos)
             :iClusterNum(aClusterNum),iMRUPos(aMRUPos.Cluster(), aMRUPos.Pos())
	{
	}



/**
Factory fucntion of tree nodes

@param  aOwnerTree	a pointer of the tree that owns this node   
@param  aPathName	the directory path this node represents
@param  aDirPos		the location of the directory this node represents   
@param  aType		the type of the node   
*/
CLeafDirTreeNode* CLeafDirTreeNode::NewL(CLeafDirTree* aOwnerTree, const TDesC& aPathName, const TLeafDirData& aDirPos, TLeafDirTreeNodeType aType)
	{
	CLeafDirTreeNode* self = new(ELeave) CLeafDirTreeNode(aDirPos, aType);
	CleanupStack::PushL(self);
	self->ConstructL(aOwnerTree, aPathName);
	CleanupStack::Pop();
	return self;
	}

/**
Constructor of tree nodes

@param  aDirPos		the location of the directory this node represents   
@param  aType		the type of the node   
*/
CLeafDirTreeNode::CLeafDirTreeNode(const TLeafDirData& aDirPos, TLeafDirTreeNodeType aType)
                  :iParent(NULL), iLeafDirData(aDirPos), iNodeType(aType)
	{
	}

/**
2nd phase constructor of tree nodes

@param  aOwnerTree	a pointer of the tree that owns this node   
@param  aPathName	the directory path this node represents
*/
void CLeafDirTreeNode::ConstructL(CLeafDirTree* aOwnerTree, const TDesC& aPath)
	{
	if (aOwnerTree == NULL)
		{
		ASSERT(0);
		User::Leave(KErrArgument);
		}
	iOwnerTree = aOwnerTree;
	iPath.CreateL(aPath);
#ifdef _DEBUG
	iOwnerTree->AddToObjectContainerL(this);
#endif //_DEBUG
	}

/**
Destructor of tree nodes

@pre	The node should already be removed from its parent before being deleted
*/
CLeafDirTreeNode::~CLeafDirTreeNode()
	{
#ifdef _DEBUG
	TRAPD(err, iOwnerTree->RemoveFromObjectContainerL(this));
	ASSERT(err == KErrNone);
#endif // _DEBUG
	iPath.Close();
	iChildren.Close();
	}

/**
Set type of the node

@param  aType	the type to be set
*/
void CLeafDirTreeNode::SetType(const CLeafDirTreeNode::TLeafDirTreeNodeType aType)
	{
	// Root node can not be reset type
	if (iNodeType == CLeafDirTreeNode::ERoot)
		return;
	iNodeType = aType;
	}

/**
Set path of the directory the node represents

@param  aPath	the path to be set   
*/
void CLeafDirTreeNode::SetPathL(const TDesC& aPath)
	{
	ASSERT(aPath.Length() > 0);
	if (iPath.Length() < aPath.Length())
		{
		TInt err = iPath.ReAlloc(aPath.Length());
		ASSERT(err==KErrNone);
		User::LeaveIfError(err);
		}
    iPath = aPath;
	}

/**
Removes from the children list, sets aNode's parent NULL, does not delete aNode

@param  aNode	the node to be removed   
*/
TInt CLeafDirTreeNode::RemoveChild(CLeafDirTreeNode* aNode)
	{
	ASSERT(aNode);
	if (aNode->IsRoot())
		{
		ASSERT(0);
		return KErrArgument;
		}
	
	if (iChildren.Count() > 0)
		{
		for (TInt i = iChildren.Count() - 1; i >= 0; i--)
			{
			if (iChildren[i] == aNode)
				{
				iChildren.Remove(i);
				aNode->SetParent(NULL);
				return KErrNone;
				}
			}
		}
	return KErrNotFound;
	}

/**
Add a new child node to self

@pre	aNode should have been removed from its original parent
@param  aNode	the node to be added   
*/
void CLeafDirTreeNode::MakeItChildL(CLeafDirTreeNode* aNode)
	{
	ASSERT(aNode->Parent() == NULL);
	if (aNode->IsRoot())
		{
		ASSERT(0);
		User::Leave(KErrArgument);
		}
	iChildren.AppendL(aNode);
	aNode->SetParent(this);
	}


/**
Factory function of CLeafDirTree

@param  aLimit	the maximum number of 'leaf' nodes allowed of the tree   
*/
CLeafDirTree* CLeafDirTree::NewL(TUint32 aSize)
	{
	CLeafDirTree* self = new(ELeave) CLeafDirTree(aSize);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

/**
Constructor of CLeafDirTree

@param  aLimit	the maximum number of 'leaf' nodes allowed of the tree   
*/
CLeafDirTree::CLeafDirTree(TUint32 aSize)
:iSize(aSize)
	{
	}

_LIT(KRootDirPath, "\\");
/**
2nd phase constructor of CLeafDirTree
*/
void CLeafDirTree::ConstructL()
	{
	TLeafDirData rootDirPos(0);
	CLeafDirTreeNode* root = CLeafDirTreeNode::NewL(this, KRootDirPath, rootDirPos, CLeafDirTreeNode::ERoot);
	iRoot = root;
	iRoot->SetType(CLeafDirTreeNode::ERoot);
	}

/**
Destructor of CLeafDirTree
*/
CLeafDirTree::~CLeafDirTree()
	{
	Reset();
	delete iRoot;
	iLruList.Close();

#ifdef _DEBUG
	iContainer.Close();
#endif //_DEBUG
	}

/**
Free all the nodes from the tree except root node
*/
void CLeafDirTree::Reset()
	{
	TInt err = KErrNone;
	TRAP(err, DeleteSubTreeL(iRoot));
	ASSERT(err == KErrNone);
	}

/**
Search for a node by directory path

@param	aPath		the path as the key to search in the tree
@param	aNodeFound	in return, the node found 
@param	aDirPos		the location of the directory
@return	KErrNone 	if a node found
		KErrNotFound if no node is found
*/
TInt CLeafDirTree::Search(const TDesC& aPath, CLeafDirTreeNode*& aNodeFound, TLeafDirData& aDirPos)
	{
	return (DoSearch(aPath, iRoot, aNodeFound, aDirPos));
	}

/**
Search for a node by directory path, start from children of aNodeToStart but do not include aNodeToStart.

@param	aPath			the path as the key to search in the tree
@param	aNodeToStart	the node whose children to start with 
@param	aNodeFound		in return, the node found 
@param	aDirPos			the location of the directory
@return	KErrNone 		if a node found
		KErrNotFound 	if no node is found
*/
TInt CLeafDirTree::DoSearch(const TDesC& aPath, CLeafDirTreeNode* aNodeToStart, CLeafDirTreeNode*& aNodeFound, TLeafDirData& aLeafDirData)
	{
	RPointerArray<CLeafDirTreeNode> currentLevel = aNodeToStart->Children();
	TInt currentPos = currentLevel.Count() - 1;
	// Current path in search
	TPtrC currentPath;
	currentPath.Set(aPath);
	while (currentLevel.Count() > 0 && currentPos >= 0)
		{
		CLeafDirTreeNode* currentNode = currentLevel[currentPos];
		TPtrC currentNodePath;
		currentNodePath.Set(currentNode->Path());
		TInt foundPos = currentPath.FindF(currentNodePath);
		// If current child's path is part of the searching path, 
		// 	go to next level
		// 	E.g.: current child's path = "1\2\3\", searching path = "1\2\3\5\".
		if (foundPos == 0 && currentNodePath.Length() < currentPath.Length())
			{
			currentPath.Set(currentPath.Mid(currentNodePath.Length()));
			currentLevel = currentNode->Children();
			currentPos = currentLevel.Count() - 1;
			continue;
			}
		// If current child's path matches current searching path,
		// 	check the node type.
		else if (foundPos == 0 && currentNodePath.Length() == currentPath.Length())
			{
			if (currentNode->IsPureIntermediary())
			// If found is 'pure intermediary', it is not cached. 
				{
				return KErrNotFound;
				}
			// Otherwise, we have got a cache hit!
			MakeMostRecentlyUsed(currentNode);
			aNodeFound = currentNode;
			aLeafDirData = currentNode->LeafDirData();
			return KErrNone;
			}
		// else, go through current level
		currentPos--;
		}
	// If there is no child or we have not found any matching node,
	//	return KErrNotFound
	return KErrNotFound;
	}

/**
Find the longest common 'path' between two paths.
Note: not the longest common 'string'.

@param	aPathA	path A
@param	aPathB	path B 
@return		the length of the longest common path found
			KErrNotFound 	if no node is found
*/
TInt FindLongestCommonPath(const TDesC& aPathA, const TDesC& aPathB)
	{
	const TInt compareLength = Min(aPathA.Length(), aPathB.Length());
	if (compareLength <= 0)
		{
		return KErrArgument;
		}
	TInt i = 0;
	TInt lastPathDelimiterPos = KErrNotFound;
	while (i < compareLength && aPathA[i] == aPathB[i])
		{
		if (aPathA[i] == '\\')
			{
			lastPathDelimiterPos = i;
			}
		i++;
		}
	
	if (i == 0)
		{
		return KErrNotFound;
		}
	return lastPathDelimiterPos;
	}

/**
Insert a new node to the tree according to the path 

@param	aPath			the path of the new node to be inserted
@param	aDirPos 		the position of the new node to be inserted
@param	aNodeInserted 	in return, the node that has been successfully inserted
*/
void CLeafDirTree::InsertL(const TDesC& aPath, const TLeafDirData& aLeafDirData, CLeafDirTreeNode*& aNodeInserted)
	{
	ASSERT(aPath.Length() > 0);
	// aPath should always start and end with a '\\'.
	if (aPath[0] == '\\' && aPath[aPath.Length() - 1] =='\\')
		{
		if (aPath.Length() > 1)
			{
			TPtrC path;
			path.Set(aPath.Mid(1));
			DoInsertL(iRoot, path, aLeafDirData, aNodeInserted);
			}
		}
	else
		{
		ASSERT(0);
		User::Leave(KErrBadName);
		}
	}

/**
Implementation of the insertion algorithm 

@param	aNodeToStart	the node whose children to start with
@param	aPath			the path of the new node to be inserted
@param	aDirPos 		the position of the new node to be inserted
@param	aNodeInserted 	in return, the node that has been successfully inserted
*/
void CLeafDirTree::DoInsertL(CLeafDirTreeNode* aNodeToStart, const TDesC& aPath, const TLeafDirData& aLeafDirData, CLeafDirTreeNode*& aNodeInserted)
	{
	CLeafDirTreeNode* currentParent = aNodeToStart;
	TInt foundPos = 0;
	RPointerArray<CLeafDirTreeNode> currentLevel = aNodeToStart->Children();
	TInt currentPos = currentLevel.Count() - 1;
	TPtrC currentPath;
	currentPath.Set(aPath);
	while (currentLevel.Count() > 0 && currentPos >= 0)
		{
		CLeafDirTreeNode* currentNode = currentLevel[currentPos];
		TPtrC currentNodePath;
		currentNodePath.Set(currentNode->Path());

		// If current node is contained by aPath.
		// 	E.g.: current node = "1\2\3\", currentPath = "1\2\3\5\"
		//	In this case, we need to go to next level,
		//	discard logged position (currentPos) in this level as we don't need to come back.
		foundPos = currentPath.FindF(currentNodePath);
		if (foundPos == 0 && currentNodePath.Length() < currentPath.Length())
			{
			currentParent = currentNode;
			currentLevel = currentNode->Children();
			currentPos = currentLevel.Count() - 1;
			currentPath.Set(currentPath.Mid(currentNodePath.Length()));
			continue;
			}

		// If current node's path contains aPath 
		// 	E.g.: current node = "1\2\3\4\", currentPath = "1\2\3\"
		//	We need to split current node to two nodes and return.
		foundPos = currentNodePath.FindF(currentPath);
		if (foundPos == 0 && currentNodePath.Length() > currentPath.Length())
			{
			CLeafDirTreeNode* newNode = CLeafDirTreeNode::NewL(this, currentPath, aLeafDirData, CLeafDirTreeNode::ELeafIntermediary);
			currentParent->MakeItChildL(newNode);
			
			TPtrC restPath;
			restPath.Set(currentNodePath.Mid(currentPath.Length()));
			currentNode->SetPathL(restPath);
			currentParent->RemoveChild(currentNode);
			
			newNode->MakeItChildL(currentNode);
			AddOntoLruL(newNode);
			aNodeInserted = newNode;
			return;
			}

		// If current node's path equals aPath,
		//	change the node type if it is necessary
		if (foundPos == 0 && currentNodePath.Length() == currentPath.Length())
			{
			// Check node type, if already cached, update Lru list and return.
			if (currentNode->IsLeaf() || currentNode->IsLeafIntermediary())
				{
				currentNode->SetLeafDirData(aLeafDirData);
				aNodeInserted = currentNode;
				MakeMostRecentlyUsed(currentNode);
				return;
				}
			// If it has not been cached yet, i.e., it is a 'pure intermediary' node,
			//	cache the node and put it onto Lru list
			else if(currentNode->IsPureIntermediary())
				{
				currentNode->SetLeafDirData(aLeafDirData);
				currentNode->SetType(CLeafDirTreeNode::ELeafIntermediary);
				AddOntoLruL(currentNode);
				aNodeInserted = currentNode;
				return;
				}
			}
		
		// If none of above is the case (i.e. haven't found exact match or paths 
		//	are not contained by each other), we need to find the first common part 
		//	between each child and aPath to share path data.
		foundPos = FindLongestCommonPath(currentNodePath, currentPath);
		// If a common part of path is found, we need to create a pure intermediary node to share
		//	the common part of path data, and create a new leaf node for the target path.
		if (foundPos > 0)
			{
			TPtrC commonPath;
			commonPath.Set(currentNodePath.Left(foundPos + 1));

			currentNodePath.Set(currentNodePath.Mid(foundPos + 1));
			TPtrC newLeafPath;
			newLeafPath.Set(currentPath.Mid(foundPos + 1));

			// Add new pureintermediary node, set it as child of current parent
			TLeafDirData dummyPos(0);
			CLeafDirTreeNode* newPureIntermediaryNode = CLeafDirTreeNode::NewL(this, commonPath, dummyPos, CLeafDirTreeNode::EPureIntermediary);
			currentParent->MakeItChildL(newPureIntermediaryNode);

			// Remove current child from aNodeToStart, do not need to change
			//	node type of aNodeToStart
			currentParent->RemoveChild(currentNode);

			// Modify current pathData, make it child of new node
			newPureIntermediaryNode->MakeItChildL(currentNode);
			currentNode->SetPathL(currentNodePath);

			// Add new leaf node as a child of the new pure intermediary node
			CLeafDirTreeNode* newLeafNode = CLeafDirTreeNode::NewL(this, newLeafPath, aLeafDirData, CLeafDirTreeNode::ELeaf);
			newPureIntermediaryNode->MakeItChildL(newLeafNode);
			aNodeInserted = newLeafNode;
			AddOntoLruL(newLeafNode);
			return;
			}

		// Otherwise, move on within this level.
		currentPos--;
		}
	
	// No match case found, add a new node straight on at current level
	CLeafDirTreeNode* newNode = CLeafDirTreeNode::NewL(this, currentPath, aLeafDirData, CLeafDirTreeNode::ELeaf);

	if (currentParent->IsLeaf())		// might be the root node
		{
		currentParent->SetType(CLeafDirTreeNode::ELeafIntermediary);
		}
	currentParent->MakeItChildL(newNode);
	aNodeInserted = newNode;
	AddOntoLruL(newNode);
	}

/**
Remove nodes with a specific position from the tree  
Note: multiple nodes may have the same position value, as directories can be accessed
	by both long names and short names:
E.g.: 	"\\LongDirName01\\LongDirName02\\LongDirName03\\"
		"\\LongDirName01\\LongDirName02\\LONGDI~1\\"
		"\\LongDirName01\\LONGDI~1\\LongDirName03\\"
		"\\LONGDI~1\\LongDirName02\\LongDirName03\\"

@param	aDirPos 	the position of the nodes to be removed
*/
void CLeafDirTree::RemoveDirL(const TLeafDirData& aDirPos)
	{
	// remove alias nodes in cache
	for (TInt i = iLruList.Count() - 1; i >= 0; i--)
		{
		if (iLruList[i]->StartClusterNum() == aDirPos.iClusterNum)
			{
			RemoveFromCacheL(iLruList[i]);
			}
		}
	}


/**
Update the MRU entry position of the tree nodes.
@param	aLeafDirData	contains the index of the cache node and the new MRU entry position 
*/
void CLeafDirTree::UpdateMRUPos(const TLeafDirData& aLeafDirData)
	{
	// update alias nodes in cache
	for (TInt i = iLruList.Count() - 1; i >= 0; i--)
		{
		if (iLruList[i]->StartClusterNum() == aLeafDirData.iClusterNum)
			{
			iLruList[i]->SetLeafDirData(aLeafDirData);
			}
		}
	}

/**
Remove a 'leaf' node, i.e. a leaf node or leaf intermediary node.

@param	aNodeTodelete the node to be removed
*/
void CLeafDirTree::RemoveFromCacheL(CLeafDirTreeNode* aNodeToDelete)
	{
	ASSERT(aNodeToDelete->IsLeaf() || aNodeToDelete->IsLeafIntermediary());
	CLeafDirTreeNode* parent = aNodeToDelete->Parent(); 
	// Deleting 'leaf intermediary' nodes:
	if (aNodeToDelete->IsLeafIntermediary())
		{
		// If there is no child, error! The 'tree' is corrupted.
		if (aNodeToDelete->Children().Count() == 0)
			{
			ASSERT(0);
			User::Leave(KErrCorrupt);
			}
		// If there is only one child, 'promote' the child, delete self
		else if (aNodeToDelete->Children().Count() == 1)
			{
			CLeafDirTreeNode* child = (aNodeToDelete->Children())[0];
			TFileName newPath = aNodeToDelete->Path();
			newPath.Append(child->Path());
			child->SetPathL(newPath);
			aNodeToDelete->RemoveChild(child);
			parent->MakeItChildL(child);

			parent->RemoveChild(aNodeToDelete);
			RemoveFromLru(aNodeToDelete);
			delete aNodeToDelete;
			return;
			}
		// If there are more than one child, just change node type to 'pure intermediary',
		//	but remove self from Lru list.
		else
			{
			aNodeToDelete->SetType(CLeafDirTreeNode::EPureIntermediary);
			RemoveFromLru(aNodeToDelete);
			return;
			}
		}
	// Deleting 'leaf' nodes:
	else
		{
		// If 'parent' is a 'leaf intermediary' node
		if (parent->IsLeafIntermediary())
			{
			// If there is no other sibling, change parent's node type to 'leaf',
			//  otherwise, leave parent's type as 'leaf intermediary' 
			if (parent->Children().Count() == 1)
				{
				parent->SetType(CLeafDirTreeNode::ELeaf);
				}
			parent->RemoveChild(aNodeToDelete);
			RemoveFromLru(aNodeToDelete);
			delete aNodeToDelete;
			return;
			}
		// If 'parent' is 'pure intermediary'
		else if (parent->IsPureIntermediary())
			{
			// If there is no sibling nodes, the tree is corrupted,
			//	as 'pure intermediary' node should always have more than one child.
			if (parent->Children().Count() <= 1)
				{
				ASSERT(0);
				User::Leave(KErrCorrupt);
				}
			// If there is only one sibling node, we need to merge the sibling node
			//	to 'parent'.
			else if (parent->Children().Count() == 2)
				{
				// Promote the sibling node, delete both parent and self
				CLeafDirTreeNode* sibling = (parent->Children())[0] ;
				if (sibling == aNodeToDelete)
					{
					sibling = (parent->Children())[1];
					}
				TFileName newPath = aNodeToDelete->Parent()->Path();
				newPath.Append(sibling->Path());
				sibling->SetPathL(newPath);
				parent->RemoveChild(sibling);
				parent->Parent()->MakeItChildL(sibling);
				
				parent->RemoveChild(aNodeToDelete);
				RemoveFromLru(aNodeToDelete);
				delete aNodeToDelete;
				aNodeToDelete = NULL;

				parent->Parent()->RemoveChild(parent);
				delete parent;
				return;
				}
			// Else if there are more than 2 sibling nodes, simply delete self.
			else
				{
				parent->RemoveChild(aNodeToDelete);
				RemoveFromLru(aNodeToDelete);
				delete aNodeToDelete;
				aNodeToDelete = NULL;
				return;
				}
			}
		// If 'parent' is root node, delete self straightaway
		else if (aNodeToDelete->Parent()->IsRoot())
			{
			aNodeToDelete->Parent()->RemoveChild(aNodeToDelete);
			RemoveFromLru(aNodeToDelete);
			delete aNodeToDelete;
			aNodeToDelete = NULL;
			return;
			}
		// If 'parent' is 'leaf', the tree is corrupted. 
		else if (aNodeToDelete->Parent()->IsLeaf())
			{
			ASSERT(0);
			User::Leave(KErrCorrupt);
			}
		}
	}

/**
Find the leftest node
Note: the leftest node must be a 'leaf' node

@param	aNodeToStart	a node whose children to start with
@return the leftest node
*/
CLeafDirTreeNode* CLeafDirTree::FindLeftestLeafNode(CLeafDirTreeNode* aNodeToStart) const
	{
	CLeafDirTreeNode* current = aNodeToStart;
	while (current->Children().Count() > 0)
		{
		current = (current->Children())[0];
		}
	return current;
	}

/**
Delete all nodes derived from aNodeToStart, except itself.

@param	aNodeToStart	a node whose children to start with
*/
void CLeafDirTree::DeleteSubTreeL(CLeafDirTreeNode* aNodeToStart)
	{
	while(aNodeToStart->Children().Count() > 0)
		{
		CLeafDirTreeNode* aLeafNode = FindLeftestLeafNode(aNodeToStart);
		RemoveFromCacheL(aLeafNode);
		}
	}

/**
Make the a node most recent used in LRU list

@param	aNodeUsed	the node to be made MRU
*/
TInt CLeafDirTree::MakeMostRecentlyUsed(CLeafDirTreeNode* aNodeUsed)
	{
	for(TInt i = 0; i < iLruList.Count(); i++)
		{
		if (aNodeUsed == iLruList[i])
			{
			if (i == 0)
				{
				return KErrNone;
				}
			else
				{
				iLruList.Remove(i);
				iLruList.Insert(aNodeUsed, 0);
				return KErrNone;
				}
			}
		}
	return KErrNotFound;
	}

/**
Check cache limit, remove least-used cached item when necessary.
*/
void CLeafDirTree::CheckLimitL()
	{
	const TInt cacheSize = iSize;
	while (iLruList.Count() > cacheSize)
		{
		CLeafDirTreeNode* lruNode = LruNode();
		RemoveFromCacheL(lruNode);
		}
	return;
	}

/**
Add new node onto cache list

@param	aNodeToAdd	the new node to be added onto cache list
*/
void CLeafDirTree::AddOntoLruL(CLeafDirTreeNode* aNodeToAdd)
	{
	if (aNodeToAdd == NULL)
		{
		ASSERT(0);
		User::Leave(KErrArgument);
		}
	
	TInt r = iLruList.Insert(aNodeToAdd, 0);
	if (r != KErrNone)
		{
		ASSERT(0);
		User::Leave(KErrArgument);
		}
	CheckLimitL();
	}

/**
Remove a node from cached list.

@param	aNodeToRemove	the node to be removed from the cache list
*/
TInt CLeafDirTree::RemoveFromLru(CLeafDirTreeNode* aNodeToRemove)
	{
	for (TInt i = 0; i < iLruList.Count(); i++)
		{
		if (aNodeToRemove == iLruList[i])
			{
			iLruList.Remove(i);
			return KErrNone;
			}
		}
	return KErrNotFound;
	}

/**
Return the least-recent-used node.

@return	the least recent used node on cache
*/
CLeafDirTreeNode* CLeafDirTree::LruNode()
	{
	if (iLruList.Count() > 0)
		{
		return iLruList[iLruList.Count() - 1];
		}
	return NULL;
	}

/*
Factory function of CLeafDirCache

@param	aLimit	the cache size 
*/
CLeafDirCache* CLeafDirCache::NewL(TUint32 aLimit)
	{
	CLeafDirCache* self = new(ELeave) CLeafDirCache(aLimit);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/*
2nd phase constructor of CLeafDirCache
*/
void CLeafDirCache::ConstructL()
	{
	CLeafDirTree* tree = CLeafDirTree::NewL(iSize);
	iTree = tree;
	}

/*
Destructor of CLeafDirCache
*/
CLeafDirCache::~CLeafDirCache()
	{
	delete iTree;
	}

/*
Constructor of CLeafDirCache

@param aLimit the cache size
*/
CLeafDirCache::CLeafDirCache(TUint32 aSize)
              :iSize(aSize)
	{
	}

/*
Reset cache, delete all memory allocated
*/
void CLeafDirCache::Reset()
	{
	iTree->Reset();
	}

/*
Cache interface for searching operations.

@param	aPath	the path of the directory to search for
@param	aDirPos	the location of the direcotry found
@return	KErrNone if a cached direcotry is found,
		KErrBadName if the path is incorrect, otherwise 
		other system wide error code
*/
TInt CLeafDirCache::FindInCache(const TDesC& aPath, TLeafDirData& aLeafDirData) const 
	{
	if (aPath[0] == '\\')
		{
		TPtrC path;
		path.Set(aPath.Mid(1));
		CLeafDirTreeNode* dummy = NULL;
		return (iTree->Search(path, dummy, aLeafDirData));
		}
	else
		{
		return KErrBadName;
		}
	}

/*
Cache interface for insertion operations.

@param	aPath	the path of the directory to be added
@param	aDirPos	the location of the direcotry to be added
*/
void CLeafDirCache::AddToCacheL(const TDesC& aPath, const TLeafDirData& aDirPos)
	{
	if (aPath.Length() == 1 && aPath[0] == '\\')
		return;

	CLeafDirTreeNode* dummy = NULL;
	iTree->InsertL(aPath, aDirPos, dummy);
	}

/*
Cache interface for deletion oeprations.
Remove all the cached directories with the same specfied position

@param	aDirPos	the location of the direcotry to be removed
*/
void CLeafDirCache::RemoveDirL(const TLeafDirData& aDirPos)
	{
	iTree->RemoveDirL(aDirPos);
	}

/**
Update the MRU entry position of the cached leaf dir.
@param	aLeafDirData	contains a cluster number as the index of the leaf dir and the new MRU entry position 
*/
void CLeafDirCache::UpdateMRUPos(const TLeafDirData& aLeafDirData)
	{
	iTree->UpdateMRUPos(aLeafDirData);
	}
/*
 * Helper functions of CLeafDirTree for debugging & testing use
 */
#ifdef _DEBUG
/*
All node created will be added to the container of its owner tree, so that we can calculate
	the number of objects created.

@param	aNodeToAdd	the newly created node to be add to object container 
*/
void CLeafDirTree::AddToObjectContainerL(CLeafDirTreeNode* aNodeToAdd)
	{
	iContainer.AppendL(aNodeToAdd);
	}

/*
A node is removed from object container if it is deleted.

@param	aNodeToRemove	the node to be deleted 
*/
void CLeafDirTree::RemoveFromObjectContainerL(CLeafDirTreeNode* aNodeToRemove)
	{
	for (TInt i = 0; i < iContainer.Count(); i++)
		{
		if (aNodeToRemove == iContainer[i])
			{
			iContainer.Remove(i);
			return;
			}
		}
	ASSERT(0);
	User::Leave(KErrNotFound);
	}

/*
Print out current tree content
*/
void CLeafDirTree::DumpTreeContentL() const
	{
	RPointerArray<CLeafDirTreeNode>* nodeStack = new(ELeave) RPointerArray<CLeafDirTreeNode>(4);
	RFs fs;
	fs.Connect();
	const TUint32 debugRegister = DebugRegister();
	fs.SetDebugRegister(debugRegister|KFSYS);
	if (iRoot != NULL)
		{
		nodeStack->Insert(iRoot, 0);
		while(nodeStack->Count() > 0)
			{
			CLeafDirTreeNode* current = (*nodeStack)[0];
			if (current->Parent() != NULL)
				{
				__PRINT3(_L("(\"%S\") -> \"%S\" : (%d)\n"), &current->Parent()->Path(), &current->Path(), current->StartClusterNum());
				}
			else
				{
				__PRINT2(_L("\"%S\" : (%d)\n"), &current->Path(), current->StartClusterNum());				
				}

			nodeStack->Remove(0);
			
			TInt currentCount = current->Children().Count();
			if (currentCount > 0)
				{
				RPointerArray<CLeafDirTreeNode> children = current->Children();
				for (TInt i = 0; i < currentCount; i++)
					{
					nodeStack->Insert(children[i], 0);
					}
				}
			}
		}

	fs.SetDebugRegister(debugRegister);
	fs.Close();
	nodeStack->Close();
	delete nodeStack;
	}

/*
Print out current cache content
*/
void CLeafDirCache::DumpCacheContentL() const
	{
	iTree->DumpTreeContentL();
	}

/*
Count of all the nodes
*/
TInt CLeafDirCache::NodeCount() const
	{
	return iTree->ObjectCount();
	}
#endif // _DEBUG

