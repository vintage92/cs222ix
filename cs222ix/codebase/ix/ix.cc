
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
    unsigned int rootPage = 1;
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
}

bool IndexManager::FileExists(const char * fileName)
{
    std::ifstream ifile(fileName);
    if(ifile)
        return true;
    else return false;
}
