
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
        //Add key to the node
        //Update superblock
        //Write nodes
        //Return 0

    }
    else{
        //Create searchResult object from Search Method
        if (attribute.type == TypeInt) {
            //Search with int method
            /*//If a match is found{
                //Then add as an overflow insert (have separate method)
             
             
             }
             
            
            //else{
                add key to returned node number
                call update node
             
             }
             
             
             */
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
                
                //For all keys newPKI - end, copy into new Node
                for (int i = newParentKeyIndex; i < cntNode.intKeys.size(); i++) {
                    if (cntNode.type == InnerNode) {
                        addToInnerNode(newNode, cntNode.intKeys[i], cntNode.pointers[i]);
                    }
                    else{
                        addToLeafNode(newNode, cntNode.intKeys[i], cntNode.rids[i], cntNode.overflows[i]);
                    }
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
    
    if (nodeSize > 4096) {
        return false;
    }
    
    
    return true;
}

//Add helpers
void IndexManager::addToInnerNode(Node &node, int key, unsigned int pagePtr){
    
}
void IndexManager::addToInnerNode(Node &node, float key, unsigned int pagePtr){
    
}
void IndexManager::addToInnerNode(Node &node, string key, unsigned int pagePtr){
    
}

//Leaf add helpers
void IndexManager::addToLeafNode(Node &node, int key, RID rid, unsigned int overflow){
    
}
void IndexManager::addToLeafNode(Node &node, float key, RID rid, unsigned int overflow){
    
}
void IndexManager::addToLeafNode(Node &node, string key, RID rid, unsigned int overflow){
    
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
