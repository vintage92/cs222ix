//
//  innerNode.cpp
//  cs222ix
//
//  Created by Zorawar Singh on 3/2/14.
//  Copyright (c) 2014 Zorawar Singh. All rights reserved.
//

#include "node.h"

Superblock::Superblock(FileHandle &fileHandle){
    //Read in values from SuperBlock page '0'
    
    fH = &fileHandle;
    
    char * data = (char *) malloc(PAGE_SIZE);
    fileHandle.readPage(0, data);
    
    unsigned int offset = 0;
    memcpy(&init, data, 4);
    offset += 4;
    memcpy(&root, data + offset, 4);
    offset += 4;
    memcpy(&nextPage, data + offset, 4);
    offset += 4;
    memcpy(&N, data + offset, 4);
    offset += 4;
    memcpy(&M, data + offset, 4);
    offset += 4;
    memcpy(&keyType, data + offset, 4);
    offset += 4;
    memcpy(&freeSpace, data + offset, 4);
    offset += 4;
    memcpy(&freePageCount, data + offset, 4);
    offset += 4;
    
    for (int i = 0; i < freePageCount; i++) {
        //Read in freePageNumber
        unsigned int cnt = 0;
        memcpy(&cnt, data + offset, 4);
        offset += 4;
        
        freePages.push_back(cnt);
    }
    
    free (data);
    
}

RC Superblock::writeSuperblock(){

    char * data = (char *) malloc(PAGE_SIZE);
    
    //Set free page to array size * 4
    freeSpace = freePageCount * 4;
    
    //Write values to data
    unsigned int offset = 0;
    memcpy(data, &init, 4);
    offset += 4;
    memcpy(data + offset, &root, 4);
    offset += 4;
    memcpy(data + offset, &nextPage, 4);
    offset += 4;
    memcpy(data + offset, &N, 4);
    offset += 4;
    memcpy(data + offset, &M, 4);
    offset += 4;
    memcpy(data + offset, &keyType, 4);
    offset += 4;
    memcpy(data + offset, &freeSpace, 4);
    offset += 4;
    memcpy(data + offset, &freePageCount, 4);
    offset += 4;
    
    //Copy array values
    for (int i = 0; i < freePages.size(); i++) {
        memcpy(data + offset, &freePages[i], 4);
        offset += 4;
    }
    
    RC rc = fH->writePage(0, data);
    
    free(data);
  
    return rc;
}

Node::Node(FileHandle &fileHandle, unsigned int pageNumber, NodeType theNodeType, AttrType theKeyType){

    fH = &fileHandle;
    isRoot = false;
    type = theNodeType;
    pageNum = pageNumber;
    
    //Initialize values
    numOfKeys = 0;
    
}

RC Node::readNode(unsigned int pageNumber){
    
    //Set page num
    pageNum = pageNumber;
    
    //Allocate page Memory
    data = (char *) malloc(PAGE_SIZE);
    fH->readPage(pageNumber, data);
    
    //Read in values
    unsigned offset = 0;
    memcpy(&isRoot, data, sizeof(int));
    offset += 4;
    memcpy(&type, data + offset, sizeof(int));
    offset += 4;
    memcpy(&numOfKeys, data + offset, sizeof(int));
    offset += 4;
    memcpy(&keyType, data + offset, sizeof(int));
    offset += 4;
    
    
    
    
    //If its an inner node, load the key's and page numbers in
    if (type == InnerNode) {
        
        for (int i = 0; i<numOfKeys; i++) {
            //Read in first page number pointer
            unsigned int cntPtr;
            memcpy(&cntPtr, data + offset, sizeof(int));
            offset += 4;
            pointers.push_back(cntPtr);
            
            if (keyType == TypeInt){
                int cntKey;
                memcpy(&cntKey, data + offset, sizeof(int));
                offset += 4;
                intKeys.push_back(cntKey);
                
                
            }
            else if(keyType == TypeReal) {
                float cntKey;
                memcpy(&cntKey, data + offset, sizeof(int));
                offset += 4;
                floatKeys.push_back(cntKey);
                
                
            }
            else{//keyType is varchar
                //Read in size of key
                unsigned int keySize;
                memcpy(&keySize, data + offset, sizeof(int));
                offset += 4;
                char * temp = (char *) malloc(keySize + 1);
                memcpy(temp, data + offset, keySize);
                offset += keySize;
                
                //TODO: check this
                temp[keySize] = NULL;
                string cntKey (temp);
                varcharKeys.push_back(cntKey);
                
            }
        }//End key readin for loop
        //Read in last page number ptr
        unsigned int cntPtr;
        memcpy(&cntPtr, data + offset, sizeof(int));
        offset += 4;
        pointers.push_back(cntPtr);
        
    }
    
    //Else load in key value pairs from leaf node
    else{
        for (int i = 0; i<numOfKeys; i++) {
            
            //Read in key
            if (keyType == TypeInt){
                int cntKey;
                memcpy(&cntKey, data + offset, sizeof(int));
                offset += 4;
                intKeys.push_back(cntKey);
                
                
            }
            else if(keyType == TypeReal) {
                float cntKey;
                memcpy(&cntKey, data + offset, sizeof(int));
                offset += 4;
                floatKeys.push_back(cntKey);
                
                
            }
            else{//keyType is varchar
                //Read in size of key
                unsigned int keySize;
                memcpy(&keySize, data + offset, sizeof(int));
                offset += 4;
                char * temp = (char *) malloc(keySize + 1);
                memcpy(temp, data + offset, keySize);
                offset += keySize;
                
                //TODO: check this
                temp[keySize] = NULL;
                string cntKey (temp);
                varcharKeys.push_back(cntKey);
                
            }
            
            //Read in RID
            RID cntID;
            memcpy(&cntID.pageNum, data + offset, sizeof(int));
            offset += 4;
            memcpy(&cntID.slotNum, data + offset, sizeof(int));
            offset += 4;
            rids.push_back(cntID);
            
        }//End key,value readin for loop
        
    }
    
    //Free data
    free(data);
    
    return 0;
    
    
}

RC Node::writeNode(){
    
    //Allocate page Memory
    data = (char *) malloc(PAGE_SIZE);
    
    unsigned int offset = 0;
    if (type == InnerNode) {
        
        for (int i = 0; i<numOfKeys; i++) {
            //Write in first page number pointer
            
            memcpy(data + offset, &pointers[i], sizeof(int));
            offset += 4;
            
            if (keyType == TypeInt){
                
                memcpy(data + offset, &intKeys[i], sizeof(int));
                offset += 4;
            }
            else if(keyType == TypeReal) {
                memcpy(data + offset, &floatKeys[i], sizeof(int));
                offset += 4;
            }
            else{//keyType is varchar
                //Subtract one from length because of Null terminator at the end
                memcpy(data + offset, varcharKeys[i].c_str(), varcharKeys[i].length() - 1);
                offset += varcharKeys[i].length() - 1;
                
            }
        }//End key readin for loop
        //Write in last page number ptr
        memcpy(data + offset, &pointers[numOfKeys - 1], sizeof(int));
        offset += 4;
        
    }
    
    //Else load in key value pairs from leaf node
    else{
        for (int i = 0; i<numOfKeys; i++) {
            
            //Write in key
            if (keyType == TypeInt){
                memcpy(data + offset, &intKeys[i], sizeof(int));
                offset += 4;
                
                
            }
            else if(keyType == TypeReal) {
                memcpy(data + offset, &floatKeys[i], sizeof(int));
                offset += 4;
                
                
            }
            else{//keyType is varchar
                //Read in size of key
                memcpy(data + offset, varcharKeys[i].c_str(), varcharKeys[i].length() - 1);
                offset += varcharKeys[i].length() - 1;
                
            }
            
            //Write in RID
            memcpy(data + offset, &rids[i].pageNum, sizeof(int));
            offset += 4;
            memcpy(data + offset, &rids[i].slotNum, sizeof(int));
            offset += 4;
            
            
        }//End key,value readin for loop
        
    }
    
    //Persist Page
    fH->writePage(pageNum, data);

    //Free data
    free(data);
    
    
    return 0;
}





