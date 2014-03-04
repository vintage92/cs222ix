
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
    unsigned int nextPage = 1;
    unsigned int innerNodes = 0;//N
    unsigned int leafNodes = 0;//M
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
    if (!sb.init) {
        //Create new node: root = true,
        Node newRoot = Node(fileHandle, sb.nextPage, LeafNode, attribute.type);
        newRoot.isRoot = true;
        
        //Add key to the node
        if (attribute.type == TypeInt) {
            int target = 0;
            memcpy(&target, key, 4);
            addToLeafNode(newRoot, target, rid, 0, false);
        }
        else if(attribute.type == TypeReal){
            
        }
        else{
            
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
            
            SearchResult sr = search(target, fileHandle);
            
            Node cntNode = Node(fileHandle, sr.trackList[sr.trackList.size()-1], LeafNode, attribute.type);
            cntNode.readNode();
            //Add to the leaf node
            addToLeafNode(cntNode, target, rid, 0, sr.matchFound);
            cntNode.writeNode();
            update(fileHandle, sr.trackList);
            return 0;
            
            
            
        }
        else if(attribute.type == TypeReal){
            
        }
        else{
            //Search with Varchar
        
        
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
	return -1;
}

void IndexManager::update(FileHandle &fileHandle, vector<unsigned int> trackList){
    Superblock sb = Superblock(fileHandle);
    
    if (trackList.size() == 0) {
        return;
    }
    else{
        //Read in current Node
        Node cntNode = Node(fileHandle, trackList[trackList.size()-1], InnerNode, TypeInt);
        cntNode.readNode();
        
        //If the node isValid then return, we are done updating...Idea is that every node
        //Above this must be valid as well
        if (isValid(cntNode)) {
            return;
        }
        else{//We must split
            //Determine the key to promote
            int newParentKeyIndex = cntNode.numOfKeys/2; //Int division auto returns floor
            if (cntNode.keyType == TypeInt) {
                //Create a newNode empty
                Node newNode = Node(fileHandle, sb.nextPage, cntNode.type, sb.keyType);
                //Connect siblings
                newNode.nextPage = cntNode.nextPage;
                cntNode.nextPage = newNode.pageNum;
                sb.nextPage++;
                
                //For all keys newPKI - end, copy into new Node
                for (int i = newParentKeyIndex; i < cntNode.intKeys.size(); i++) {
                    if (cntNode.type == InnerNode) {
                        addToInnerNode(newNode, cntNode.intKeys[i], cntNode.pointers[i]);
                    }
                    else{
                        addToLeafNode(newNode, cntNode.intKeys[i], cntNode.rids[i], cntNode.overflows[i], true);
                        //True because we want to copy over the overflow pages
                    }
                }
                newNode.writeNode();
                
                //Now we need to check if there is an available parent node to put this in
                //That will be true as long as cntNode.root != true;
                if (cntNode.isRoot == true) {
                    
                    //We need to create a new Root Node and add the new key to it
                    //Then reroute the sb's root pointer to this newNode
                    Node newRoot = Node(fileHandle, sb.nextPage, InnerNode, sb.keyType);
                    sb.nextPage++;
                    
                    //Add the parent with it's left pointing to old node
                    addToInnerNode(newRoot, cntNode.intKeys[newParentKeyIndex], cntNode.pageNum);
                    //Add the right pointer pointing to newNode
                    newRoot.pointers[1] = newNode.pageNum;
                    
                    //Now reroute the root
                    cntNode.isRoot = false;
                    newRoot.isRoot = true;
                    sb.root = newRoot.pageNum;
                    
                    //Write back nodes
                    cntNode.writeNode();
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
                    addToInnerNode(parentNode, cntNode.intKeys[newParentKeyIndex], newNode.pageNum);
                    //Make sure this add to innernode adds to pointer index + 1 of the key
                    cntNode.intKeys.erase(cntNode.intKeys.begin() + newParentKeyIndex, cntNode.intKeys.begin() + (cntNode.intKeys.size()-1));
                    cntNode.numOfKeys = cntNode.intKeys.size();
                    
                    cntNode.writeNode();
                    newNode.writeNode();
                    sb.writeSuperblock();
                    //Return with call to update with tracklist
                    return update(fileHandle, trackList);
                  
                    
                }
                
                
            }
            else if (cntNode.keyType == TypeReal){
                
            }
            else{
                
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
void IndexManager::addToInnerNode(Node &node, int key, unsigned int pagePtr){
    
}
void IndexManager::addToInnerNode(Node &node, float key, unsigned int pagePtr){
    
}
void IndexManager::addToInnerNode(Node &node, string key, unsigned int pagePtr){
    
}

//Leaf add helpers
void IndexManager::addToLeafNode(Node &node, int key, RID rid, unsigned int overflow, bool matchFound){
    Superblock sb = Superblock(*node.fH);
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
            addToLeafNode(oPage, key, rid, 0, 0);
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
            addToLeafNode(oPage, key, rid, 0, false);
            //If node is valid then persist it
            if (isValid(oPage)) {
                oPage.writeNode();
                return;
            }
            else{
                //Create a new oPage and add it to there
                Node newPage = Node(*sb.fH, sb.nextPage, OverFlow, node.keyType);
                sb.nextPage++;
                addToLeafNode(newPage, key, rid, 0, 0);
                newPage.writeNode();
                return;
            }
            
            
        }
        
        
    }

    
    
    
}
void IndexManager::addToLeafNode(Node &node, float key, RID rid, unsigned int overflow, bool matchFound){
    
}
void IndexManager::addToLeafNode(Node &node, string key, RID rid, unsigned int overflow, bool matchFound){
    
}

SearchResult IndexManager::search(string key){
    SearchResult sr;
    
    
    
    
    
    return sr;
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
SearchResult IndexManager::search(float key){
    SearchResult sr;
    
    
    
    return sr;
}
SearchResult IndexManager::treeSearch(string key, SearchResult sr){
    
    
    
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
        
        for (int i = 0; i < cntNode.intKeys.size(); i++) {
            if (key >= cntNode.intKeys[i]) {
                sr.pageNumber = cntNode.pointers[i + 1];
                return treeSearch(key, cntsr);
            }
            else{
                sr.pageNumber = cntNode.pointers[i];
                return treeSearch(key, cntsr);
            }
        }
        
    }
    
    cout << "Error in execution of tree search" << endl;
    return sr;
}
SearchResult IndexManager::treeSearch(float key, SearchResult sr){
    
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
	return -1;
}

RC IX_ScanIterator::close()
{
	return -1;
}

void IX_PrintError (RC rc)
{
    switch (rc) {
        case 1:
            fputs("Searching on Invalid Index", stderr);
            break;
        case 2:
            fputs("Searching on Invalid Index", stderr);
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
