
#include "ix.h"

IndexManager* IndexManager::_index_manager = 0;

IndexManager* IndexManager::instance()
{
    if(!_index_manager)
        _index_manager = new IndexManager();

    return _index_manager;
}

IndexManager::IndexManager()
{
}

IndexManager::~IndexManager()
{
}

RC IndexManager::createFile(const string &fileName)
{
    if (FileExists(fileName.c_str())) {
        return -1;
    }
    
    //Create the file
	FILE* fp = fopen(fileName.c_str(), "w+");
    
    
    
    char * superBlock = (char *) malloc(PAGE_SIZE);
    
    //Initialize
    unsigned int offset = 0;
    
    //Values
    int init = 0;
    unsigned int rootPage = 0;
    unsigned int nextPage = 2;
    int innerNodes = 0;//N
    int leafNodes = 0;//M
    unsigned int keyType = 0;
    unsigned int freeSpace = PAGE_SIZE - (4 * 8);
    unsigned int freePageCount = 0;
    
    memcpy(superBlock, &init, 4);
    offset += 4;
    memcpy(superBlock + offset, &rootPage, 4);
    offset += 4;
    memcpy(superBlock + offset, &nextPage, 4);
    offset += 4;
    memcpy(superBlock + offset, &innerNodes, 4);
    offset += 4;
    memcpy(superBlock + offset, &leafNodes, 4);
    offset += 4;
    memcpy(superBlock + offset, &keyType, 4);
    offset += 4;
    memcpy(superBlock + offset, &freeSpace, 4);
    offset += 4;
    memcpy(superBlock + offset, &freePageCount, 4);
    offset += 4;
    
    fwrite(superBlock, PAGE_SIZE, 1, fp);
    
    fseek(fp, PAGE_SIZE, SEEK_SET);
    
    //Write a blank 2nd page to store float and char * min and max key values
    memset(superBlock, 0, 4096);
    fwrite(superBlock, PAGE_SIZE, 1, fp);
    
    free(superBlock);
  
    if(fclose(fp)){
        return -1;
    }
    
	return 0;
}

RC IndexManager::destroyFile(const string &fileName)
{
    if (!FileExists(fileName.c_str())) {
        return -1;
    }
    
	if(remove (fileName.c_str()))
		return -1;//File wasn't deleted successfully
	return 0;
}

RC IndexManager::openFile(const string &fileName, FileHandle &fileHandle)
{
	//if(!containsName(fName))
    //return -1;//File does not exist or wasn't created by our file Manager
	//Try opening the file
    if (!FileExists(fileName.c_str())) {
        return -1;
    }
    
	FILE * pFile;
	pFile = fopen (fileName.c_str(), "r+");
	if(pFile!=NULL){
		fileHandle.setPage(pFile);
        fileHandle.setFileName(fileName);
		return 0;
	}
    
	return -1;
}

RC IndexManager::closeFile(FileHandle &fileHandle)
{
    if(fclose(fileHandle.target))
		return -1;
	return 0;
}

RC IndexManager::insertEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid)
{
    
    
    Superblock sb = Superblock(fileHandle);
    sb.keyType = attribute.type;
    sb.writeSuperblock();
    
    if (!sb.init) {
        //Create new node: root = true,
        Node newRoot = Node(fileHandle, sb.nextPage, LeafNode, attribute.type);
        newRoot.isRoot = true;
        
        //Add key to the node
        if (attribute.type == TypeInt) {
            int target = 0;
            memcpy(&target, key, 4);
            addToLeafNode(newRoot, target, rid, 0, false, false);
            
            sb.N = target;
            sb.M = target;
            
            
        }
        else if(attribute.type == TypeReal){
            float target = 0.0;
            memcpy(&target, key, 4);
            addToLeafNode(newRoot, target, rid, 0, false, false);
            //Read in KeyStore
            KeyStore ks = KeyStore(fileHandle);
            ks.read();
            ks.floatLKey = target;
            ks.floatHKey = target;
            ks.stringHKey = "MX";
            ks.stringLKey = "MX";
            ks.write();
            
            
        }
        else{
            //Read in key
            unsigned int length = 0;
            memcpy(&length, key, 4);
            char * temp = (char *) malloc(length + 5);
            memcpy(temp, key, length + 4);
            char * temp2 = (char *) malloc(length + 1);
            memcpy(temp2, temp + 4, length);
            temp2[length] = NULL;
            string target (temp2);
            addToLeafNode(newRoot, target, rid, 0, false, false);
            
            KeyStore ks = KeyStore(fileHandle);
            ks.read();
            ks.stringLKey = target;
            ks.stringHKey = target;
            ks.write();
            free(temp);
            free(temp2);
         
        }
        //Update superblock
        sb.root = newRoot.pageNum;
        sb.init = true;
        sb.nextPage++;
        //Write nodes
        newRoot.writeNode();
        sb.writeSuperblock();
        
        return 0;
        

    }
    else{
        //Create searchResult object from Search Method
        if (attribute.type == TypeInt) {
            int target = 0;
            memcpy(&target, key, 4);
            
            if (target < sb.N) {
                sb.N = target;
            }
            if (target > sb.M) {
                sb.M = target;
            }
            sb.writeSuperblock();
            
            SearchResult sr = search(target, fileHandle);
            
            Node cntNode = Node(fileHandle, sr.trackList[sr.trackList.size()-1], LeafNode, attribute.type);
            cntNode.readNode();
            //Add to the leaf node
            addToLeafNode(cntNode, target, rid, 0, sr.matchFound, false);
            update(fileHandle, sr.trackList, cntNode);
            return 0;
            
            
            
        }
        else if(attribute.type == TypeReal){
            float target = 0.0;
            memcpy(&target, key, 4);
            
            //Cond Update KeyStore
            KeyStore ks = KeyStore(fileHandle);
            ks.read();
            if (target < ks.floatLKey) {
                ks.floatLKey = target;
            }
            if (target > ks.floatHKey) {
                ks.floatHKey = target;
            }
            ks.write();
            
            SearchResult sr = search(target, fileHandle);
            Node cntNode = Node(fileHandle, sr.trackList[sr.trackList.size()-1], LeafNode, attribute.type);
            cntNode.readNode();
            //Add to this leaf node
            addToLeafNode(cntNode, target, rid, 0, sr.matchFound, false);
            update(fileHandle, sr.trackList, cntNode);
            
            return 0;
        }
        else{
            
            //Read in key
            unsigned int length = 0;
            memcpy(&length, key, 4);
            char * temp = (char *) malloc(length + 5);
            memcpy(temp, key, length + 4);
            char * temp2 = (char *) malloc(length + 1);
            memcpy(temp2, temp + 4, length);
            temp2[length] = NULL;
            string target (temp2);
            
            //Cond Update KeyStore
            KeyStore ks = KeyStore(fileHandle);
            ks.read();
            if (target.compare(ks.stringLKey) < 0) {
                ks.stringLKey = target;
            }
            if (target.compare(ks.stringLKey) > 0) {
                ks.stringHKey = target;
            }
            ks.write();
            
            SearchResult sr = search(target, fileHandle);
            Node cntNode = Node(fileHandle, sr.trackList[sr.trackList.size()-1], LeafNode, attribute.type);
            cntNode.readNode();
            //Add to this leaf node
            addToLeafNode(cntNode, target, rid, 0, sr.matchFound, false);
            update(fileHandle, sr.trackList, cntNode);
            
            return 0;
        
        
        }
        
        
        
    }
    
    
    
    
	return -1;
}

RC IndexManager::deleteEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid)
{
	return -1;
}

RC IndexManager::scan(FileHandle &fileHandle,
    const Attribute &attribute,
    const void      *lowKey,
    const void      *highKey,
    bool			lowKeyInclusive,
    bool        	highKeyInclusive,
    IX_ScanIterator &ix_ScanIterator)
{
    Superblock sb = Superblock(fileHandle);
    if (!sb.init) {
        IX_PrintError(1);
        return 1;
    }
    
    ix_ScanIterator.fH = &fileHandle;
    ix_ScanIterator.onOverFlow = false;
    ix_ScanIterator.cntOver = 0;
    ix_ScanIterator.lAlloc = false;
    ix_ScanIterator.hAlloc = false;
    ix_ScanIterator.all = false;
    ix_ScanIterator.search = true;
    ix_ScanIterator.lowinc = lowKeyInclusive;
    ix_ScanIterator.highinc = highKeyInclusive;
    ix_ScanIterator.attr = attribute;
    
    if (lowKey != NULL) {
        ix_ScanIterator.lAlloc = true;
        if(attribute.type == TypeInt){
            memcpy(&ix_ScanIterator.intLKey, lowKey, 4);
        }
        else if(attribute.type == TypeReal){
            memcpy(&ix_ScanIterator.floatLKey, lowKey, 4);
        }
        else{
            unsigned int length = 0;
            memcpy(&length, lowKey, 4);
            char * temp = (char *) malloc(length + 1);
            temp[length] = NULL;
            string temp2 (temp);
            ix_ScanIterator.stringLKey = temp2;
            free(temp);
        }
    }
    if (highKey != NULL) {
        ix_ScanIterator.hAlloc = true;
        if(attribute.type == TypeInt){
            memcpy(&ix_ScanIterator.intHKey, highKey, 4);
        }
        else if(attribute.type == TypeReal){
            memcpy(&ix_ScanIterator.floatHKey, highKey, 4);
        }
        else{
            unsigned int length = 0;
            memcpy(&length, highKey, 4);
            char * temp = (char *) malloc(length + 1);
            temp[length] = NULL;
            string temp2 (temp);
            ix_ScanIterator.stringHKey = temp2;
            free(temp);
        }
    }
    if (lowKey == NULL) {
        //Set manually
        if(attribute.type == TypeInt){
            ix_ScanIterator.intLKey = sb.N;
          
        }
        else if (attribute.type == TypeReal){
            //TODO: Add search result sr
            KeyStore ks = KeyStore(fileHandle);
            ks.read();
            ix_ScanIterator.floatLKey = ks.floatLKey;
       
        }
        else{
            //TODO: Add search result sr
            KeyStore ks = KeyStore(fileHandle);
            ks.read();
            ix_ScanIterator.stringLKey = ks.stringLKey;
        }
    }
    if (highKey == NULL) {
        //Set manually
        if(attribute.type == TypeInt){
            ix_ScanIterator.intHKey = sb.M;
            
            
        }
        else if (attribute.type == TypeReal){
            //TODO: Add search result sr
            KeyStore ks = KeyStore(fileHandle);
            ks.read();
            ix_ScanIterator.floatHKey = ks.floatHKey;
            
            
            
        }
        else{
            //TODO: Add search result sr
            KeyStore ks = KeyStore(fileHandle);
            ks.read();
            ix_ScanIterator.stringHKey = ks.stringHKey;
        }
    }
    if (ix_ScanIterator.lAlloc == false && ix_ScanIterator.hAlloc == false && lowKeyInclusive) {
        ix_ScanIterator.all = true; //Means return all elements in index
        //Set custom range
        if(attribute.type == TypeInt){
            SearchResult sr = search(sb.N, fileHandle);
            ix_ScanIterator.cntLeaf = sr.pageNumber;
            ix_ScanIterator.cntIndex = 0;
            ix_ScanIterator.intLKey = sb.N;
            ix_ScanIterator.intHKey = sb.M;
            
            
        }
        else if (attribute.type == TypeReal){
            //TODO: Add search result sr
            KeyStore ks = KeyStore(fileHandle);
            ks.read();
            ix_ScanIterator.floatLKey = ks.floatLKey;
            ix_ScanIterator.floatHKey = ks.floatHKey;
            
            
            
        }
        else{
            //TODO: Add search result sr
            KeyStore ks = KeyStore(fileHandle);
            ks.read();
            ix_ScanIterator.stringLKey = ks.stringLKey;
            ix_ScanIterator.stringHKey = ks.stringHKey;
        }
        return 0;
        
    }
        
    //Find starting page number
    if(attribute.type == TypeInt){
        if (lowKeyInclusive) {
            SearchResult sr = search(ix_ScanIterator.intLKey, fileHandle);
            //Iterate until you find the first key bigger than or equal to lowKey
            Node cntNode = Node(fileHandle, sr.pageNumber, LeafNode, TypeInt);
            cntNode.readNode();
            bool hit = false;
            for (int i = 0 ; i < cntNode.intKeys.size(); i++) {
                //If i's key >= ix's low key then hit is true
                //Record this index and pageNumber as starting point
                //Set search to true
                if (cntNode.intKeys[i] >= ix_ScanIterator.intLKey) {
                    ix_ScanIterator.cntLeaf = cntNode.pageNum;
                    ix_ScanIterator.cntIndex = i;
                    ix_ScanIterator.search = true;
                    return 0;
                }
            }
            while (cntNode.nextPage != 0 && hit == false) {
                cntNode = Node(fileHandle, cntNode.nextPage, LeafNode, TypeInt);
                cntNode.readNode();
                for (int i = 0 ; i < cntNode.intKeys.size(); i++) {
                    //If i's key >= ix's low key then hit is true
                    //Record this index and pageNumber as starting point
                    //Set search to true
                    if (cntNode.intKeys[i] >= ix_ScanIterator.intLKey) {
                        ix_ScanIterator.cntLeaf = cntNode.pageNum;
                        ix_ScanIterator.cntIndex = i;
                        ix_ScanIterator.search = true;
                        return 0;
                    }
                }
            
            
            }
            //If it reaches here then set search to false, there is no valid range to search on
            ix_ScanIterator.search = false;
            return 0;
        }
        else{
            //Must be strictly greater than lowKey
            SearchResult sr = search(ix_ScanIterator.intLKey, fileHandle);
            //Iterate until you find the first key bigger than or equal to lowKey
            Node cntNode = Node(fileHandle, sr.pageNumber, LeafNode, TypeInt);
            cntNode.readNode();
            bool hit = false;
            for (int i = 0 ; i < cntNode.intKeys.size(); i++) {
                //If i's key >= ix's low key then hit is true
                //Record this index and pageNumber as starting point
                //Set search to true
                if (cntNode.intKeys[i] > ix_ScanIterator.intLKey) {
                    ix_ScanIterator.cntLeaf = cntNode.pageNum;
                    ix_ScanIterator.cntIndex = i;
                    ix_ScanIterator.search = true;
                    return 0;
                }
            }
            while (cntNode.nextPage != 0 && hit == false) {
                cntNode = Node(fileHandle, cntNode.nextPage, LeafNode, TypeInt);
                cntNode.readNode();
                for (int i = 0 ; i < cntNode.intKeys.size(); i++) {
                    //If i's key >= ix's low key then hit is true
                    //Record this index and pageNumber as starting point
                    //Set search to true
                    if (cntNode.intKeys[i] > ix_ScanIterator.intLKey) {
                        ix_ScanIterator.cntLeaf = cntNode.pageNum;
                        ix_ScanIterator.cntIndex = i;
                        ix_ScanIterator.search = true;
                        return 0;
                    }
                }
                
                
            }
            //If it reaches here then set search to false, there is no valid range to search on
            ix_ScanIterator.search = false;
            return 0;
        
        }
    }
    else if(attribute.type == TypeReal){
        
        if (lowKeyInclusive) {
            SearchResult sr = search(ix_ScanIterator.floatLKey, fileHandle);
            //Iterate until you find the first key bigger than or equal to lowKey
            Node cntNode = Node(fileHandle, sr.pageNumber, LeafNode, TypeReal);
            cntNode.readNode();
            bool hit = false;
            for (int i = 0 ; i < cntNode.floatKeys.size(); i++) {
                //If i's key >= ix's low key then hit is true
                //Record this index and pageNumber as starting point
                //Set search to true
                if (cntNode.floatKeys[i] >= ix_ScanIterator.floatLKey) {
                    ix_ScanIterator.cntLeaf = cntNode.pageNum;
                    ix_ScanIterator.cntIndex = i;
                    ix_ScanIterator.search = true;
                    return 0;
                }
            }
            while (cntNode.nextPage != 0 && hit == false) {
                cntNode = Node(fileHandle, cntNode.nextPage, LeafNode, TypeReal);
                cntNode.readNode();
                for (int i = 0 ; i < cntNode.floatKeys.size(); i++) {
                    //If i's key >= ix's low key then hit is true
                    //Record this index and pageNumber as starting point
                    //Set search to true
                    if (cntNode.floatKeys[i] >= ix_ScanIterator.floatLKey) {
                        ix_ScanIterator.cntLeaf = cntNode.pageNum;
                        ix_ScanIterator.cntIndex = i;
                        ix_ScanIterator.search = true;
                        return 0;
                    }
                }
                
                
            }
            //If it reaches here then set search to false, there is no valid range to search on
            ix_ScanIterator.search = false;
            return 0;
        }
        else{
            //Must be strictly greater than lowKey
            SearchResult sr = search(ix_ScanIterator.floatLKey, fileHandle);
            //Iterate until you find the first key bigger than or equal to lowKey
            Node cntNode = Node(fileHandle, sr.pageNumber, LeafNode, TypeReal);
            cntNode.readNode();
            bool hit = false;
            for (int i = 0 ; i < cntNode.floatKeys.size(); i++) {
                //If i's key >= ix's low key then hit is true
                //Record this index and pageNumber as starting point
                //Set search to true
                if (cntNode.floatKeys[i] > ix_ScanIterator.floatLKey) {
                    ix_ScanIterator.cntLeaf = cntNode.pageNum;
                    ix_ScanIterator.cntIndex = i;
                    ix_ScanIterator.search = true;
                    return 0;
                }
            }
            while (cntNode.nextPage != 0 && hit == false) {
                cntNode = Node(fileHandle, cntNode.nextPage, LeafNode, TypeReal);
                cntNode.readNode();
                for (int i = 0 ; i < cntNode.floatKeys.size(); i++) {
                    //If i's key >= ix's low key then hit is true
                    //Record this index and pageNumber as starting point
                    //Set search to true
                    if (cntNode.floatKeys[i] > ix_ScanIterator.floatLKey) {
                        ix_ScanIterator.cntLeaf = cntNode.pageNum;
                        ix_ScanIterator.cntIndex = i;
                        ix_ScanIterator.search = true;
                        return 0;
                    }
                }
                
                
            }
            //If it reaches here then set search to false, there is no valid range to search on
            ix_ScanIterator.search = false;
            return 0;
            
        }

        
    }
    else{
        
        if (lowKeyInclusive) {
            SearchResult sr = search(ix_ScanIterator.stringLKey, fileHandle);
            //Iterate until you find the first key bigger than or equal to lowKey
            Node cntNode = Node(fileHandle, sr.pageNumber, LeafNode, TypeVarChar);
            cntNode.readNode();
            bool hit = false;
            for (int i = 0 ; i < cntNode.varcharKeys.size(); i++) {
                //If i's key >= ix's low key then hit is true
                //Record this index and pageNumber as starting point
                //Set search to true
                if (cntNode.varcharKeys[i].compare(ix_ScanIterator.stringLKey) >= 0) {
                    ix_ScanIterator.cntLeaf = cntNode.pageNum;
                    ix_ScanIterator.cntIndex = i;
                    ix_ScanIterator.search = true;
                    return 0;
                }
            }
            while (cntNode.nextPage != 0 && hit == false) {
                cntNode = Node(fileHandle, cntNode.nextPage, LeafNode, TypeVarChar);
                cntNode.readNode();
                for (int i = 0 ; i < cntNode.varcharKeys.size(); i++) {
                    //If i's key >= ix's low key then hit is true
                    //Record this index and pageNumber as starting point
                    //Set search to true
                    if (cntNode.varcharKeys[i].compare(ix_ScanIterator.stringLKey) >= 0) {
                        ix_ScanIterator.cntLeaf = cntNode.pageNum;
                        ix_ScanIterator.cntIndex = i;
                        ix_ScanIterator.search = true;
                        return 0;
                    }
                }
                
                
            }
            //If it reaches here then set search to false, there is no valid range to search on
            ix_ScanIterator.search = false;
            return 0;
        }
        else{
            //Must be strictly greater than lowKey
            SearchResult sr = search(ix_ScanIterator.stringLKey, fileHandle);
            //Iterate until you find the first key bigger than or equal to lowKey
            Node cntNode = Node(fileHandle, sr.pageNumber, LeafNode, TypeVarChar);
            cntNode.readNode();
            bool hit = false;
            for (int i = 0 ; i < cntNode.varcharKeys.size(); i++) {
                //If i's key >= ix's low key then hit is true
                //Record this index and pageNumber as starting point
                //Set search to true
                if (cntNode.varcharKeys[i].compare(ix_ScanIterator.stringLKey) > 0) {
                    ix_ScanIterator.cntLeaf = cntNode.pageNum;
                    ix_ScanIterator.cntIndex = i;
                    ix_ScanIterator.search = true;
                    return 0;
                }
            }
            while (cntNode.nextPage != 0 && hit == false) {
                cntNode = Node(fileHandle, cntNode.nextPage, LeafNode, TypeVarChar);
                cntNode.readNode();
                for (int i = 0 ; i < cntNode.varcharKeys.size(); i++) {
                    //If i's key >= ix's low key then hit is true
                    //Record this index and pageNumber as starting point
                    //Set search to true
                    if (cntNode.varcharKeys[i].compare(ix_ScanIterator.stringLKey) > 0) {
                        ix_ScanIterator.cntLeaf = cntNode.pageNum;
                        ix_ScanIterator.cntIndex = i;
                        ix_ScanIterator.search = true;
                        return 0;
                    }
                }
                
                
            }
            //If it reaches here then set search to false, there is no valid range to search on
            ix_ScanIterator.search = false;
            return 0;
            
        }

        
        
    }
    
    	return 0;
}

void IndexManager::update(FileHandle &fileHandle, vector<unsigned int> trackList, Node &tempNode){
    Superblock sb = Superblock(fileHandle);
    
    if (trackList.size() == 0) {
        return;
    }
    else{
        //Read in current Node
        
        
        //If the node isValid then return, we are done updating...Idea is that every node
        //Above this must be valid as well
        if (isValid(tempNode)) {
            tempNode.writeNode();
            return;
        }
        else{//We must split
            //Determine the key to promote
            int newParentKeyIndex = tempNode.numOfKeys/2; //Int division auto returns floor
            if (tempNode.keyType == TypeInt) {
                //Create a newNode empty
                Node newNode = Node(fileHandle, sb.nextPage, tempNode.type, sb.keyType);
                //Connect siblings
                newNode.nextPage = tempNode.nextPage;
                tempNode.nextPage = newNode.pageNum;
                sb.nextPage++;
                
                //For all keys newPKI - end, copy into new Node
                
                    if (tempNode.type == InnerNode) {
                        //Add the first left right pair in
                        addToInnerNode(newNode, tempNode.intKeys[newParentKeyIndex +1], tempNode.pointers[newParentKeyIndex+1], tempNode.pointers[newParentKeyIndex + 2]);
                        for (int i = newParentKeyIndex + 2; i < tempNode.intKeys.size(); i++) {
                        addToInnerNode(newNode, tempNode.intKeys[i], 0, tempNode.pointers[i + 1]);
                        }
                    }
                    else{
                        for (int i = newParentKeyIndex; i < tempNode.intKeys.size(); i++) {
                            addToLeafNode(newNode, tempNode.intKeys[i], tempNode.rids[i], tempNode.overflows[i], true, true);
                            //True because we want to copy over the overflow pages
                        }
                    }
                
                newNode.writeNode();
                
                //Now we need to check if there is an available parent node to put this in
                //That will be true as long as cntNode.root != true;
                if (tempNode.isRoot == true) {
                    
                    //We need to create a new Root Node and add the new key to it
                    //Then reroute the sb's root pointer to this newNode
                    Node newRoot = Node(fileHandle, sb.nextPage, InnerNode, sb.keyType);
                    sb.nextPage++;
                    
                    //Add the parent with it's left pointing to old node
                    addToInnerNode(newRoot, tempNode.intKeys[newParentKeyIndex], tempNode.pageNum, newNode.pageNum);
                    //Add the right pointer pointing to newNode
                    //newRoot.pointers[1] = newNode.pageNum;
                    
                    //Now reroute the root
                    tempNode.isRoot = false;
                    newRoot.isRoot = true;
                    sb.root = newRoot.pageNum;
                    
                    //Delete old keys from tempNode
                    tempNode.intKeys.erase(tempNode.intKeys.begin() + newParentKeyIndex, tempNode.intKeys.begin() + (tempNode.intKeys.size()));
                    tempNode.numOfKeys = tempNode.intKeys.size();
                    
                    //Write back nodes
                    tempNode.writeNode();
                    newRoot.writeNode();
                    sb.writeSuperblock();
                    return;
                    
                    
                }
                else{
                    //It must be either an inner node with a node above it, or a leaf node
                    //With an inner node above it. Either way just add the parent key to the
                    //Node above it and call update on it with tracklist-1
                    trackList.erase(trackList.begin() + (trackList.size()-1));
                    Node parentNode = Node(fileHandle, trackList[trackList.size()-1], InnerNode, sb.keyType);
                    parentNode.readNode();
                    addToInnerNode(parentNode, tempNode.intKeys[newParentKeyIndex], 0, newNode.pageNum);
                    //Make sure this add to innernode adds to pointer index + 1 of the key
                    tempNode.intKeys.erase(tempNode.intKeys.begin() + newParentKeyIndex, tempNode.intKeys.begin() + (tempNode.intKeys.size()));
                    tempNode.numOfKeys = tempNode.intKeys.size();
                    
                    tempNode.writeNode();
                    newNode.writeNode();
                    sb.writeSuperblock();
                    
                    return update(fileHandle, trackList, parentNode);
                  
                    
                }
                
                
            }
            else if (tempNode.keyType == TypeReal){
                //Create a newNode empty
                Node newNode = Node(fileHandle, sb.nextPage, tempNode.type, sb.keyType);
                //Connect siblings
                newNode.nextPage = tempNode.nextPage;
                tempNode.nextPage = newNode.pageNum;
                sb.nextPage++;
                
                //For all keys newPKI - end, copy into new Node
                
                if (tempNode.type == InnerNode) {
                    //Add the first left right pair in
                    addToInnerNode(newNode, tempNode.floatKeys[newParentKeyIndex +1], tempNode.pointers[newParentKeyIndex+1], tempNode.pointers[newParentKeyIndex + 2]);
                    for (int i = newParentKeyIndex + 2; i < tempNode.floatKeys.size(); i++) {
                        addToInnerNode(newNode, tempNode.floatKeys[i], 0, tempNode.pointers[i + 1]);
                    }
                }
                else{
                    for (int i = newParentKeyIndex; i < tempNode.floatKeys.size(); i++) {
                        addToLeafNode(newNode, tempNode.floatKeys[i], tempNode.rids[i], tempNode.overflows[i], true, true);
                        //True because we want to copy over the overflow pages
                    }
                }
                
                newNode.writeNode();
                
                //Now we need to check if there is an available parent node to put this in
                //That will be true as long as cntNode.root != true;
                if (tempNode.isRoot == true) {
                    
                    //We need to create a new Root Node and add the new key to it
                    //Then reroute the sb's root pointer to this newNode
                    Node newRoot = Node(fileHandle, sb.nextPage, InnerNode, sb.keyType);
                    sb.nextPage++;
                    
                    //Add the parent with it's left pointing to old node
                    addToInnerNode(newRoot, tempNode.floatKeys[newParentKeyIndex], tempNode.pageNum, newNode.pageNum);
                    //Add the right pointer pointing to newNode
                    //newRoot.pointers[1] = newNode.pageNum;
                    
                    //Now reroute the root
                    tempNode.isRoot = false;
                    newRoot.isRoot = true;
                    sb.root = newRoot.pageNum;
                    
                    //Delete old keys from tempNode
                    tempNode.floatKeys.erase(tempNode.floatKeys.begin() + newParentKeyIndex, tempNode.floatKeys.begin() + (tempNode.floatKeys.size()));
                    tempNode.numOfKeys = tempNode.floatKeys.size();
                    
                    //Write back nodes
                    tempNode.writeNode();
                    newRoot.writeNode();
                    sb.writeSuperblock();
                    return;
                    
                    
                }
                else{
                    //It must be either an inner node with a node above it, or a leaf node
                    //With an inner node above it. Either way just add the parent key to the
                    //Node above it and call update on it with tracklist-1
                    trackList.erase(trackList.begin() + (trackList.size()-1));
                    Node parentNode = Node(fileHandle, trackList[trackList.size()-1], InnerNode, sb.keyType);
                    parentNode.readNode();
                    addToInnerNode(parentNode, tempNode.floatKeys[newParentKeyIndex], 0, newNode.pageNum);
                    //Make sure this add to innernode adds to pointer index + 1 of the key
                    tempNode.floatKeys.erase(tempNode.floatKeys.begin() + newParentKeyIndex, tempNode.floatKeys.begin() + (tempNode.floatKeys.size()));
                    tempNode.numOfKeys = tempNode.floatKeys.size();
                    
                    tempNode.writeNode();
                    newNode.writeNode();
                    sb.writeSuperblock();
                    
                    return update(fileHandle, trackList, parentNode);
                    
                    
                }

                
            }
            else{
                //Create a newNode empty
                Node newNode = Node(fileHandle, sb.nextPage, tempNode.type, sb.keyType);
                //Connect siblings
                newNode.nextPage = tempNode.nextPage;
                tempNode.nextPage = newNode.pageNum;
                sb.nextPage++;
                
                //For all keys newPKI - end, copy into new Node
                
                if (tempNode.type == InnerNode) {
                    //Add the first left right pair in
                    addToInnerNode(newNode, tempNode.varcharKeys[newParentKeyIndex +1], tempNode.pointers[newParentKeyIndex+1], tempNode.pointers[newParentKeyIndex + 2]);
                    for (int i = newParentKeyIndex + 2; i < tempNode.varcharKeys.size(); i++) {
                        addToInnerNode(newNode, tempNode.varcharKeys[i], 0, tempNode.pointers[i + 1]);
                    }
                }
                else{
                    for (int i = newParentKeyIndex; i < tempNode.varcharKeys.size(); i++) {
                        addToLeafNode(newNode, tempNode.varcharKeys[i], tempNode.rids[i], tempNode.overflows[i], true, true);
                        //True because we want to copy over the overflow pages
                    }
                }
                
                newNode.writeNode();
                
                //Now we need to check if there is an available parent node to put this in
                //That will be true as long as cntNode.root != true;
                if (tempNode.isRoot == true) {
                    
                    //We need to create a new Root Node and add the new key to it
                    //Then reroute the sb's root pointer to this newNode
                    Node newRoot = Node(fileHandle, sb.nextPage, InnerNode, sb.keyType);
                    sb.nextPage++;
                    
                    //Add the parent with it's left pointing to old node
                    addToInnerNode(newRoot, tempNode.varcharKeys[newParentKeyIndex], tempNode.pageNum, newNode.pageNum);
                    //Add the right pointer pointing to newNode
                    //newRoot.pointers[1] = newNode.pageNum;
                    
                    //Now reroute the root
                    tempNode.isRoot = false;
                    newRoot.isRoot = true;
                    sb.root = newRoot.pageNum;
                    
                    //Delete old keys from tempNode
                    tempNode.varcharKeys.erase(tempNode.varcharKeys.begin() + newParentKeyIndex, tempNode.varcharKeys.begin() + (tempNode.varcharKeys.size()));
                    tempNode.numOfKeys = tempNode.varcharKeys.size();
                    
                    //Write back nodes
                    tempNode.writeNode();
                    newRoot.writeNode();
                    sb.writeSuperblock();
                    return;
                    
                    
                }
                else{
                    //It must be either an inner node with a node above it, or a leaf node
                    //With an inner node above it. Either way just add the parent key to the
                    //Node above it and call update on it with tracklist-1
                    trackList.erase(trackList.begin() + (trackList.size()-1));
                    Node parentNode = Node(fileHandle, trackList[trackList.size()-1], InnerNode, sb.keyType);
                    parentNode.readNode();
                    addToInnerNode(parentNode, tempNode.varcharKeys[newParentKeyIndex], 0, newNode.pageNum);
                    //Make sure this add to innernode adds to pointer index + 1 of the key
                    tempNode.varcharKeys.erase(tempNode.varcharKeys.begin() + newParentKeyIndex, tempNode.varcharKeys.begin() + (tempNode.varcharKeys.size()));
                    tempNode.numOfKeys = tempNode.varcharKeys.size();
                    
                    tempNode.writeNode();
                    newNode.writeNode();
                    sb.writeSuperblock();
                    
                    return update(fileHandle, trackList, parentNode);
                    
                    
                }
                
            }
            
            
        }
        
        
        
    }
    
    
}

bool IndexManager::isValid(Node &node){
    unsigned int nodeSize = 24; //Base for a node
    //Calculate size of the node
    
    if (node.type == LeafNode) {
        if (node.keyType == TypeInt || node.keyType == TypeReal) {
            //Mulitply keySize * 3 * 4bytes each
            nodeSize += (node.numOfKeys * 3 * 4);
        }
        else{
            //Calc size of varChar keys
            for (int i = 0; i < node.varcharKeys.size(); i++) {
                unsigned int cntKey = node.varcharKeys[i].size() - 1;
                cntKey += 8; //For RID and overflow page
                nodeSize += cntKey;
            }
        }
    }
    else{
        //Inner Node calc
        if (node.keyType == TypeInt || node.keyType == TypeReal) {
            //Mulitply keySize * 3 * 4bytes each
            nodeSize += (node.numOfKeys * 2 * 4);
            //Add 4 more for last page pointer
            nodeSize += 4;
        }
        else{
            //Calc size of varChar keys
            for (int i = 0; i < node.varcharKeys.size(); i++) {
                unsigned int cntKey = node.varcharKeys[i].size() - 1;
                cntKey += 4; //For page number
                nodeSize += cntKey;
            }
            //Add 4 for last page pointer
            nodeSize += 4;
        }
    }
    
    //TODO: RESETEETSETET THIS!!! when done using
    if (nodeSize > 70) {
        return false;
    }
    
    
    return true;
}

//Add helpers
//Should add to the right of the key i think
void IndexManager::addToInnerNode(Node &node, int key, unsigned int leftPtr, unsigned int rightPtr){
    int index = 0;
    for (int i = 0; i < node.intKeys.size(); i++) {
        if (key > node.intKeys[i]) {
            index++;
        }
        else{
            break;
        }
    }
    
    //If leftPtr != 0, add it to the key's index
    if (leftPtr != 0) {
        node.pointers.insert(node.pointers.begin() + index, leftPtr);
    }
    //Add in the key
    node.intKeys.insert(node.intKeys.begin() + index, key);
    //Add rightPtr regardless to key's index + 1
    node.pointers.insert(node.pointers.begin() + index + 1, rightPtr);
    node.numOfKeys++;
    
}
void IndexManager::addToInnerNode(Node &node, float key, unsigned int leftPtr, unsigned int rightPtr){
    int index = 0;
    for (int i = 0; i < node.floatKeys.size(); i++) {
        if (key > node.floatKeys[i]) {
            index++;
        }
        else{
            break;
        }
    }
    
    //If leftPtr != 0, add it to the key's index
    if (leftPtr != 0) {
        node.pointers.insert(node.pointers.begin() + index, leftPtr);
    }
    //Add in the key
    node.floatKeys.insert(node.floatKeys.begin() + index, key);
    //Add rightPtr regardless to key's index + 1
    node.pointers.insert(node.pointers.begin() + index + 1, rightPtr);
    node.numOfKeys++;
    
}
void IndexManager::addToInnerNode(Node &node, string key, unsigned int leftPtr, unsigned int rightPtr){
    int index = 0;
    for (int i = 0; i < node.intKeys.size(); i++) {
        if (key.compare(node.varcharKeys[i]) > 0) {
            index++;
        }
        else{
            break;
        }
    }
    
    //If leftPtr != 0, add it to the key's index
    if (leftPtr != 0) {
        node.pointers.insert(node.pointers.begin() + index, leftPtr);
    }
    //Add in the key
    node.varcharKeys.insert(node.varcharKeys.begin() + index, key);
    //Add rightPtr regardless to key's index + 1
    node.pointers.insert(node.pointers.begin() + index + 1, rightPtr);
    node.numOfKeys++;
}

//Leaf add helpers
void IndexManager::addToLeafNode(Node &node, int key, RID rid, unsigned int overflow, bool matchFound, bool copy){
    Superblock sb = Superblock(*node.fH);
    //If copy is true, then just copy the overflow in along with other values
    if (copy) {
        int index = 0;
        for (int i = 0; i < node.intKeys.size(); i++) {
            if (key > node.intKeys[i]) {
                index++;
            }
            else{
                break;
            }
        }
        node.intKeys.insert(node.intKeys.begin() + index, key);
        node.rids.insert(node.rids.begin() + index, rid);
        node.overflows.insert(node.overflows.begin() + index, overflow);
        node.numOfKeys++;
        return;

        
    }
    //If matchFound is false then we write 0 to overflow position
    if (!matchFound) {
        //Add the key value, and a 0 for the overflow
        int index = 0;
        for (int i = 0; i < node.intKeys.size(); i++) {
            if (key > node.intKeys[i]) {
                index++;
            }
            else{
                break;
            }
        }
        node.intKeys.insert(node.intKeys.begin() + index, key);
        node.rids.insert(node.rids.begin() + index, rid);
        node.overflows.insert(node.overflows.begin() + index, 0);
        node.numOfKeys++;
        return;
        
        
    }
    //If true, then either copy key to the overflow (as long as its not 0)
    else{
        //Find the index for the key
        int index = 0;
        for (int i = 0; i < node.intKeys.size(); i++) {
            if (key == node.intKeys[i]) {
                break;
            }
            index++;
        }
        //Check what the overflow page number is
        unsigned int oPageNum = node.overflows[index];
        if (oPageNum == 0) {
            //Create a new overflow Node
            Node oPage = Node(*sb.fH, sb.nextPage, OverFlow, node.keyType);
            sb.nextPage++;
            sb.writeSuperblock();
            addToLeafNode(oPage, key, rid, 0, false, false);
            oPage.writeNode();
        }
        else{
            //read in the overflow Node
            Node oPage = Node(*sb.fH, oPageNum, OverFlow, node.keyType);
            oPage.readNode();
            while (oPage.nextPage != 0) {
                //Keep reading in and incrementing the oPages
                oPage = Node(*sb.fH, oPage.nextPage, OverFlow, node.keyType);
                oPage.readNode();
            }
            //Will exit with the last link in oPages
            //Add the element to this oPage
            addToLeafNode(oPage, key, rid, 0, false, false);
            //If node is valid then persist it
            if (isValid(oPage)) {
                oPage.writeNode();
                return;
            }
            else{
                //Create a new oPage and add it to there
                Node newPage = Node(*sb.fH, sb.nextPage, OverFlow, node.keyType);
                sb.nextPage++;
                sb.writeSuperblock();
                addToLeafNode(newPage, key, rid, 0, false, false);
                newPage.writeNode();
                return;
            }
            
            
        }
        
        
    }

    
    
    
}
void IndexManager::addToLeafNode(Node &node, float key, RID rid, unsigned int overflow, bool matchFound, bool copy){
    Superblock sb = Superblock(*node.fH);
    //If copy is true, then just copy the overflow in along with other values
    if (copy) {
        int index = 0;
        for (int i = 0; i < node.floatKeys.size(); i++) {
            if (key > node.floatKeys[i]) {
                index++;
            }
            else{
                break;
            }
        }
        node.floatKeys.insert(node.floatKeys.begin() + index, key);
        node.rids.insert(node.rids.begin() + index, rid);
        node.overflows.insert(node.overflows.begin() + index, overflow);
        node.numOfKeys++;
        return;
        
        
    }
    //If matchFound is false then we write 0 to overflow position
    if (!matchFound) {
        //Add the key value, and a 0 for the overflow
        int index = 0;
        for (int i = 0; i < node.floatKeys.size(); i++) {
            if (key > node.floatKeys[i]) {
                index++;
            }
            else{
                break;
            }
        }
        node.floatKeys.insert(node.floatKeys.begin() + index, key);
        node.rids.insert(node.rids.begin() + index, rid);
        node.overflows.insert(node.overflows.begin() + index, 0);
        node.numOfKeys++;
        return;
        
        
    }
    //If true, then either copy key to the overflow (as long as its not 0)
    else{
        //Find the index for the key
        int index = 0;
        for (int i = 0; i < node.floatKeys.size(); i++) {
            if (key == node.floatKeys[i]) {
                break;
            }
            index++;
        }
        //Check what the overflow page number is
        unsigned int oPageNum = node.overflows[index];
        if (oPageNum == 0) {
            //Create a new overflow Node
            Node oPage = Node(*sb.fH, sb.nextPage, OverFlow, node.keyType);
            sb.nextPage++;
            sb.writeSuperblock();
            addToLeafNode(oPage, key, rid, 0, false, false);
            oPage.writeNode();
        }
        else{
            //read in the overflow Node
            Node oPage = Node(*sb.fH, oPageNum, OverFlow, node.keyType);
            oPage.readNode();
            while (oPage.nextPage != 0) {
                //Keep reading in and incrementing the oPages
                oPage = Node(*sb.fH, oPage.nextPage, OverFlow, node.keyType);
                oPage.readNode();
            }
            //Will exit with the last link in oPages
            //Add the element to this oPage
            addToLeafNode(oPage, key, rid, 0, false, false);
            //If node is valid then persist it
            if (isValid(oPage)) {
                oPage.writeNode();
                return;
            }
            else{
                //Create a new oPage and add it to there
                Node newPage = Node(*sb.fH, sb.nextPage, OverFlow, node.keyType);
                sb.nextPage++;
                sb.writeSuperblock();
                addToLeafNode(newPage, key, rid, 0, false, false);
                newPage.writeNode();
                return;
            }
            
            
        }
        
        
    }
    
}
void IndexManager::addToLeafNode(Node &node, string key, RID rid, unsigned int overflow, bool matchFound, bool copy){
    Superblock sb = Superblock(*node.fH);
    //If copy is true, then just copy the overflow in along with other values
    if (copy) {
        int index = 0;
        for (int i = 0; i < node.varcharKeys.size(); i++) {
            if (key.compare(node.varcharKeys[i])>0) {
                index++;
            }
            else{
                break;
            }
        }
        node.varcharKeys.insert(node.varcharKeys.begin() + index, key);
        node.rids.insert(node.rids.begin() + index, rid);
        node.overflows.insert(node.overflows.begin() + index, overflow);
        node.numOfKeys++;
        return;
        
        
    }
    //If matchFound is false then we write 0 to overflow position
    if (!matchFound) {
        //Add the key value, and a 0 for the overflow
        int index = 0;
        for (int i = 0; i < node.varcharKeys.size(); i++) {
            if (key.compare(node.varcharKeys[i])>0) {
                index++;
            }
            else{
                break;
            }
        }
        node.varcharKeys.insert(node.varcharKeys.begin() + index, key);
        node.rids.insert(node.rids.begin() + index, rid);
        node.overflows.insert(node.overflows.begin() + index, 0);
        node.numOfKeys++;
        return;
        
        
    }
    //If true, then either copy key to the overflow (as long as its not 0)
    else{
        //Find the index for the key
        int index = 0;
        for (int i = 0; i < node.varcharKeys.size(); i++) {
            if (key.compare(node.varcharKeys[i]) == 0) {
                break;
            }
            index++;
        }
        //Check what the overflow page number is
        unsigned int oPageNum = node.overflows[index];
        if (oPageNum == 0) {
            //Create a new overflow Node
            Node oPage = Node(*sb.fH, sb.nextPage, OverFlow, node.keyType);
            sb.nextPage++;
            sb.writeSuperblock();
            addToLeafNode(oPage, key, rid, 0, false, false);
            oPage.writeNode();
        }
        else{
            //read in the overflow Node
            Node oPage = Node(*sb.fH, oPageNum, OverFlow, node.keyType);
            oPage.readNode();
            while (oPage.nextPage != 0) {
                //Keep reading in and incrementing the oPages
                oPage = Node(*sb.fH, oPage.nextPage, OverFlow, node.keyType);
                oPage.readNode();
            }
            //Will exit with the last link in oPages
            //Add the element to this oPage
            addToLeafNode(oPage, key, rid, 0, false, false);
            //If node is valid then persist it
            if (isValid(oPage)) {
                oPage.writeNode();
                return;
            }
            else{
                //Create a new oPage and add it to there
                Node newPage = Node(*sb.fH, sb.nextPage, OverFlow, node.keyType);
                sb.nextPage++;
                sb.writeSuperblock();
                addToLeafNode(newPage, key, rid, 0, false, false);
                newPage.writeNode();
                return;
            }
            
            
        }
        
        
    }
    
}

SearchResult IndexManager::search(string key, FileHandle &fileHandle){
    //Read in the superblock and get root
    Superblock sb = Superblock(fileHandle);
    
    
    SearchResult sr;
    sr.matchFound = false;
    sr.pageNumber = sb.root;
    sr.trackList.clear();
    sr.fH = &fileHandle;
    if (!sb.init) {
        return sr;
    }
    return treeSearch(key, sr);
}
SearchResult IndexManager::search(int key, FileHandle &fileHandle){
    //Read in the superblock and get root
    Superblock sb = Superblock(fileHandle);
    
    
    SearchResult sr;
    sr.matchFound = false;
    sr.pageNumber = sb.root;
    sr.trackList.clear();
    sr.fH = &fileHandle;
    if (!sb.init) {
        return sr;
    }
    return treeSearch(key, sr);
}
SearchResult IndexManager::search(float key, FileHandle &fileHandle){
    //Read in the superblock and get root
    Superblock sb = Superblock(fileHandle);
    
    
    SearchResult sr;
    sr.matchFound = false;
    sr.pageNumber = sb.root;
    sr.trackList.clear();
    sr.fH = &fileHandle;
    if (!sb.init) {
        return sr;
    }
    return treeSearch(key, sr);
}
SearchResult IndexManager::treeSearch(string key, SearchResult sr){
    //Read in the page currently in sr
    SearchResult cntsr = sr;
    Node cntNode = Node(*cntsr.fH, cntsr.pageNumber, InnerNode, TypeVarChar);
    //Read in the cntNode
    cntNode.readNode();
    
    cntsr.matchFound = false;
    
    //Add it to cntsr's tracklist
    cntsr.trackList.push_back(cntsr.pageNumber);
    
    //Check if it's a leaf node
    if (cntNode.type == LeafNode) {
        //Check in the key array if it's present
        for (int i = 0; i<cntNode.varcharKeys.size(); i++) {
            if (cntNode.varcharKeys[i].compare(key) == 0) {
                cntsr.matchFound = true;
                break;
            }
        }
        return cntsr;
    }
    
    //Else if it's an inner node find the page pointer to the left
    //of the key value that is greater than the key
    else{
        
        
        int index = 0;
        while (index < cntNode.varcharKeys.size() && key.compare(cntNode.varcharKeys[index]) >= 0) {
            //Keep incrementing the pointer
            index++;
        }
        //Call treeSearch on whichever page index points to
        cntsr.pageNumber = cntNode.pointers[index];
        return treeSearch(key, cntsr);
        
        
        /*for (int i = 0; i < cntNode.varcharKeys.size(); i++) {
         if (key >= cntNode.varcharKeys[i]) {
         sr.pageNumber = cntNode.pointers[i + 1];
         return treeSearch(key, cntsr);
         }
         else{
         sr.pageNumber = cntNode.pointers[i];
         return treeSearch(key, cntsr);
         }
         }*/
        
    }
    
    cout << "Error in execution of tree search" << endl;
    return sr;
}
SearchResult IndexManager::treeSearch(int key, SearchResult sr){
    //Read in the page currently in sr
    SearchResult cntsr = sr;
    Node cntNode = Node(*cntsr.fH, cntsr.pageNumber, InnerNode, TypeInt);
    //Read in the cntNode
    cntNode.readNode();
    
    
    cntsr.matchFound = false;
    
    //Add it to cntsr's tracklist
    cntsr.trackList.push_back(cntsr.pageNumber);
    
    //Check if it's a leaf node
    if (cntNode.type == LeafNode) {
        //Check in the key array if it's present
        for (int i = 0; i<cntNode.intKeys.size(); i++) {
            if (cntNode.intKeys[i] == key) {
                cntsr.matchFound = true;
                break;
            }
        }
        return cntsr;
    }
    
    //Else if it's an inner node find the page pointer to the left
    //of the key value that is greater than the key
    else{
        
        
        int index = 0;
        while (index < cntNode.intKeys.size() && key >= cntNode.intKeys[index]) {
            //Keep incrementing the pointer
            index++;
        }
        //Call treeSearch on whichever page index points to
        cntsr.pageNumber = cntNode.pointers[index];
        return treeSearch(key, cntsr);
        
        
        /*for (int i = 0; i < cntNode.intKeys.size(); i++) {
            if (key >= cntNode.intKeys[i]) {
                sr.pageNumber = cntNode.pointers[i + 1];
                return treeSearch(key, cntsr);
            }
            else{
                sr.pageNumber = cntNode.pointers[i];
                return treeSearch(key, cntsr);
            }
        }*/
        
    }
    
    cout << "Error in execution of tree search" << endl;
    return sr;
}
SearchResult IndexManager::treeSearch(float key, SearchResult sr){
    //Read in the page currently in sr
    SearchResult cntsr = sr;
    Node cntNode = Node(*cntsr.fH, cntsr.pageNumber, InnerNode, TypeReal);
    //Read in the cntNode
    cntNode.readNode();
    
    cntsr.matchFound = false;
    
    //Add it to cntsr's tracklist
    cntsr.trackList.push_back(cntsr.pageNumber);
    
    //Check if it's a leaf node
    if (cntNode.type == LeafNode) {
        //Check in the key array if it's present
        for (int i = 0; i<cntNode.floatKeys.size(); i++) {
            if (cntNode.floatKeys[i] == key) {
                cntsr.matchFound = true;
                break;
            }
        }
        return cntsr;
    }
    
    //Else if it's an inner node find the page pointer to the left
    //of the key value that is greater than the key
    else{
        
        
        int index = 0;
        while (index < cntNode.floatKeys.size() && key >= cntNode.floatKeys[index]) {
            //Keep incrementing the pointer
            index++;
        }
        //Call treeSearch on whichever page index points to
        cntsr.pageNumber = cntNode.pointers[index];
        return treeSearch(key, cntsr);
        
        
        /*for (int i = 0; i < cntNode.floatKeys.size(); i++) {
         if (key >= cntNode.floatKeys[i]) {
         sr.pageNumber = cntNode.pointers[i + 1];
         return treeSearch(key, cntsr);
         }
         else{
         sr.pageNumber = cntNode.pointers[i];
         return treeSearch(key, cntsr);
         }
         }*/
        
    }
    
    cout << "Error in execution of tree search" << endl;
    return sr;
}



IX_ScanIterator::IX_ScanIterator()
{
}

IX_ScanIterator::~IX_ScanIterator()
{
}

RC IX_ScanIterator::getNextEntry(RID &rid, void *key)
{
    
    if (!search) {
        //Range is invalid, exiting scan
        IX_PrintError(2);
        return -1;
    }
    
        //If onOverFlow
        if (onOverFlow) {
            //Keep scanning overflow elements
            //OverElements have already been checked against conditions,
            //So just keep returning rids and same key
            //Read in current OverFlow node
            Node overNode = Node(*fH, cntOver, LeafNode, attr.type);
            overNode.readNode();
            if (overIndex < overNode.numOfKeys) {
                //Return the next key
                if (attr.type == TypeInt) {
                    memcpy(key, &overNode.intKeys[overIndex], 4);
                }
                else if (attr.type == TypeReal){
                    memcpy(key, &overNode.floatKeys[overIndex], 4);

                }
                else{
                    memcpy(key, overNode.varcharKeys[overIndex].c_str(), overNode.varcharKeys[overIndex].length()-1);

                    
                }
                rid.pageNum = overNode.rids[overIndex].pageNum;
                rid.slotNum = overNode.rids[overIndex].slotNum;
                overIndex++;
                return 0;
            }
            else{
                //If there is another overFlow page read it in and repeat (we assume there is at least one key on the page)
                if (overNode.nextPage != 0) {
                    overNode = Node(*fH, overNode.nextPage, OverFlow, attr.type);
                    overNode.readNode();
                    overIndex = 0; //1 because we're about to read 0 right now
                    cntOver = overNode.pageNum;
                    //Return the next key
                    if (attr.type == TypeInt) {
                        memcpy(key, &overNode.intKeys[overIndex], 4);
                    }
                    else if (attr.type == TypeReal){
                        memcpy(key, &overNode.floatKeys[overIndex], 4);
                        
                    }
                    else{
                        memcpy(key, overNode.varcharKeys[overIndex].c_str(), overNode.varcharKeys[overIndex].length()-1);
                        
                        
                    }
                    rid.pageNum = overNode.rids[overIndex].pageNum;
                    rid.slotNum = overNode.rids[overIndex].slotNum;
                    overIndex++;
                    return 0;

                }
                //Else read in the next leaf value and reset overflow settings
                else{
                    cntIndex++;
                    onOverFlow = false;
                    if (!hasNext()) {
                        return -1;
                    }
                    //Hasnext barrier gurantees the cntLeaf and cntIndex to have a valid node, return it and increment
                    Node cntNode = Node(*fH, cntLeaf, LeafNode, attr.type);
                    cntNode.readNode();
                    //Return the key
                    if (attr.type == TypeInt) {
                        memcpy(key, &cntNode.intKeys[cntIndex], 4);
                    }
                    else if (attr.type == TypeReal){
                        memcpy(key, &cntNode.floatKeys[cntIndex], 4);
                        
                    }
                    else{
                        memcpy(key, cntNode.varcharKeys[cntIndex].c_str(), cntNode.varcharKeys[cntIndex].length()-1);
                        
                        
                    }
                    rid.pageNum = cntNode.rids[cntIndex].pageNum;
                    rid.slotNum = cntNode.rids[cntIndex].slotNum;
                    
                    if (cntNode.overflows[cntIndex] != 0) {
                        //Reset overflow settings and true them
                        overIndex = 0;
                        cntOver = cntNode.overflows[cntIndex];
                        onOverFlow = true;
                        return 0;
                    }
                    overIndex = 0;
                    onOverFlow = false;
                    cntIndex++;
                    return 0;
                }
            }
        }
        else{
            //Keep looking for regular elements
            //Hasnext gurantees there's another entry on the cntLeaf at cntIndex
            if (!hasNext()) {
                return -1;
            }
            Node cntNode = Node(*fH, cntLeaf, LeafNode, attr.type);
            cntNode.readNode();
            //Return the key
            if (attr.type == TypeInt) {
                memcpy(key, &cntNode.intKeys[cntIndex], 4);
            }
            else if (attr.type == TypeReal){
                memcpy(key, &cntNode.floatKeys[cntIndex], 4);
                
            }
            else{
                memcpy(key, cntNode.varcharKeys[cntIndex].c_str(), cntNode.varcharKeys[cntIndex].length()-1);
                
                
            }
            rid.pageNum = cntNode.rids[cntIndex].pageNum;
            rid.slotNum = cntNode.rids[cntIndex].slotNum;
            
            if (cntNode.overflows[cntIndex] != 0) {
                //Reset overflow settings and true them
                overIndex = 0;
                cntOver = cntNode.overflows[cntIndex];
                onOverFlow = true;
                return 0;
            }
            overIndex = 0;
            onOverFlow = false;
            cntIndex++;
            return 0;
            
        }
    
    
    
	return -1;
}
bool IX_ScanIterator::hasNext(){
    //If it's on an overflow page then it will always return true
    if (onOverFlow) {
        return true;
    }
    //Check the next leafNode entry if it satisfy's the condition
    Node cntNode = Node(*fH, cntLeaf, LeafNode, attr.type);
    cntNode.readNode();
    if (cntIndex >= cntNode.numOfKeys && cntNode.nextPage == 0) {
        return false;
    }
    if (cntIndex >= cntNode.numOfKeys) {
        //Read in next Page
        cntLeaf = cntNode.nextPage;
        cntIndex = 0;
        cntNode = Node(*fH, cntLeaf, LeafNode, attr.type);
        cntNode.readNode();
    }
    if (attr.type == TypeInt) {
        if (highinc) {
            if (cntNode.intKeys[cntIndex] <= intHKey) {
                return true;
            }
        }
        else{
            if (cntNode.intKeys[cntIndex] < intHKey) {
                return true;
            }
            
        }
    }
    else if (attr.type == TypeReal){
        if (highinc) {
            if (cntNode.floatKeys[cntIndex] <= floatHKey) {
                return true;
            }
        }
        else{
            if (cntNode.floatKeys[cntIndex] < floatHKey) {
                return true;
            }
            
        }
        
    }
    else{
        if (highinc) {
            if (cntNode.varcharKeys[cntIndex].compare(stringHKey) <= 0) {
                return true;
            }
        }
        else{
            if (cntNode.varcharKeys[cntIndex].compare(stringHKey) < 0) {
                return true;
            }
            
        }
        
    }
    
    return false;
}
RC IX_ScanIterator::close()
{
    //Nothing to deallocate
	return 0;
}

void IX_PrintError (RC rc)
{
    switch (rc) {
        case 1:
            fputs("Trying to Iterate over unintialized Index (NO VALUES)", stderr);
            break;
        case 2:
            fputs("Trying to Iterate over invalid range", stderr);
            break;
        case 3:
            fputs("Searching on Invalid Index", stderr);
            break;
        case 4:
            fputs("Searching on Invalid Index", stderr);
            break;
        case 5:
            fputs("Searching on Invalid Index", stderr);
            break;
        case 6:
            fputs("Searching on Invalid Index", stderr);
            break;
        case 7:
            fputs("Searching on Invalid Index", stderr);
            break;
        case 8:
            fputs("Searching on Invalid Index", stderr);
            break;
        case 9:
            fputs("Searching on Invalid Index", stderr);
            break;
            
        default:
            break;
    }
}

bool IndexManager::FileExists(const char * fileName)
{
    std::ifstream ifile(fileName);
    if(ifile)
        return true;
    else return false;
}
