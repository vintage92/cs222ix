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

typedef struct{
	bool matchFound;
	unsigned int pageNumber; //Of the leaf node that the match is found on, or to be inserted on
	vector<unsigned int> trackList;
    
    FileHandle * fH;
    
} SearchResult;

class KeyStore {
    
    
public:
    FileHandle * fH;
    float floatLKey;
    float floatHKey;
    string stringLKey;
    string stringHKey;
    
    KeyStore(FileHandle &fileHandle);
    void read();
    void write();
};

typedef enum { InnerNode = 0, LeafNode, OverFlow } NodeType;

class Superblock{
public:
    int init;
    unsigned int root;
    unsigned int nextPage;
    int N; //Minimum int key
    int M; //Maximum int key
    AttrType keyType;
    unsigned int freeSpace;
    unsigned int freePageCount;
    vector<unsigned int> freePages;
    
    FileHandle * fH;
    
    bool FileExists(const char * fileName);
    
    
    Superblock(FileHandle &fileHandle);
    RC writeSuperblock();
};

class Node{

public:
    
    FileHandle* fH;
    
    unsigned int isRoot; //0 means it isnt, 1 means it is
    NodeType type;
    unsigned int pageNum; //Not in mem map
    unsigned int numOfKeys;
    AttrType keyType;
    
    unsigned int freeSpace;
    
    
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
    vector<unsigned int> overflows; //Keeps overflow pages for any duplicate keys
    
    char * data;
    
    unsigned int nextPage;
    
    
    
    
    //Initializes a skeleton Node
    Node(FileHandle &fileHandle, unsigned int pageNumber, NodeType theNodeType, AttrType theKeyType);
    
    
    //Persists node data to disk
    RC writeNode();
    
    //Fills node with data
    RC readNode();
    
  
    
    
};



#endif /* defined(__cs222ix__Node__) */
