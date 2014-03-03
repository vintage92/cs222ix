//
//  innerNode.h
//  cs222ix
//
//  Created by Zorawar Singh on 3/2/14.
//  Copyright (c) 2014 Zorawar Singh. All rights reserved.
//

#ifndef __node__h
#define __node__h

#include <iostream>
#include <string>
#include <vector>

#include "../rbf/rbfm.h"

using namespace std;

typedef enum { InnerNode = 0, LeafNode } NodeType;

class Superblock{
public:
    int init;
    unsigned int root;
    unsigned int nextPage;
    unsigned int N;
    unsigned int M;
    AttrType keyType;
    unsigned int freeSpace;
    unsigned int freePageCount;
    vector<unsigned int> freePages;
    
    FileHandle * fH;
    
    
    
    Superblock(FileHandle &fileHandle);
    RC writeSuperblock();
};

class Node{

public:
    
    FileHandle* fH;
    
    bool isRoot;
    NodeType type;
    unsigned int pageNum; //Not in mem map
    unsigned int numOfKeys;
    AttrType keyType;
    
    
    vector<float> floatKeys;
    vector<int> intKeys;
    vector<string> varcharKeys;
    
    //Offset recorders to certain key's or pointers
    vector<unsigned int> toKeys;
    vector<unsigned int> toPointers;
    
    //For an inner node
    vector<unsigned int> pointers;
    
    //For a leaf node
    vector<RID> rids;
    
    char * data;
    
    
    
    
    
    Node(FileHandle &fileHandle, unsigned int pageNumber, NodeType theNodeType, AttrType theKeyType);
    
    

    RC writeNode();
    
    RC readNode(unsigned int pageNumber);
    
    
    
    

    
    
    
};



#endif /* defined(__cs222ix__Node__) */
