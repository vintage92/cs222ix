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

#include "../rbf/pfm.h"

using namespace std;

typedef enum { InnerNode = 0, LeafNode } NodeType;

class Node{

public:
    
    unsigned int pageNum;
    bool isRoot;
    NodeType
    
    
    
    
    Node(FileHandle &fileHandle, unsigned int pageNumber);

    ~Node();
    
    
    
};



#endif /* defined(__cs222ix__Node__) */
