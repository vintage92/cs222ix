#include "pfm.h"

PagedFileManager* PagedFileManager::_pf_manager = 0;

PagedFileManager* PagedFileManager::instance()
{
    if(!_pf_manager)
        _pf_manager = new PagedFileManager();

    return _pf_manager;
}


PagedFileManager::PagedFileManager()
{

    

}


PagedFileManager::~PagedFileManager()
{
}


RC PagedFileManager::createFile(const char *fileName)
{
	//Check if file exists and was created by our structures
	//std::string fName (fileName);
	//if(containsName(fName))
		//{return -1;}//File already exists
	//currentFiles.push_back(fName);
    if (FileExists(fileName)) {
        return -1;
    }
    
    
	char * dirPage = (char *) malloc(PAGE_SIZE);

	//Write values to dirPage
	unsigned int numOfPages = 0;

	unsigned int freeD = 4084;
	unsigned int nextD = 0;

	memcpy(dirPage, &numOfPages, 4);
	memcpy(dirPage+4088, &freeD, 4);
	memcpy(dirPage+4092, &nextD, 4);

	//Create the file
	FILE* fp = fopen(fileName, "w+");
	//write blocks
	fwrite(dirPage, 4096,1, fp);


	if(fclose(fp))
		return -1;
	free(dirPage);




    return 0;
}


RC PagedFileManager::destroyFile(const char *fileName)
{

	//std::string fName (fileName);
	//if(!containsName(fName))
		//return -1;//File does not exist or wasn't created by our file Manager
	//Remove the file
    
    if (!FileExists(fileName)) {
        return -1;
    }
    
	if(remove (fileName))
		return -1;//File wasn't deleted successfully

	//Remove from currentFiles
	//std::vector<std::string>::iterator it = std::find(currentFiles.begin(), currentFiles.end(), fName);
	//currentFiles.erase(it);
	return 0;

}


RC PagedFileManager::openFile(const char *fileName, FileHandle &fileHandle)
{
	std::string fName (fileName);
	//if(!containsName(fName))
		//return -1;//File does not exist or wasn't created by our file Manager
	//Try opening the file
    if (!FileExists(fileName)) {
        return -1;
    }
    
	FILE * pFile;
	pFile = fopen (fileName, "r+");
	if(pFile!=NULL){
		fileHandle.setPage(pFile);
        fileHandle.setFileName(fName);
		return 0;
	}
    return -1;
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
	//Close the file by calling fopen on the target pointer
	if(fclose(fileHandle.target))
		return -1;
	return 0;
}
bool PagedFileManager::containsName(const std::string &target){
	if(std::find(currentFiles.begin(), currentFiles.end(), target) != currentFiles.end()) {
		return true;
} else {
    	return false;
}
}

FileHandle::FileHandle()
{
	target = NULL;
	numOfDir = 1;
}


FileHandle::~FileHandle()
{

}
void FileHandle::setFileName(std::string fN){
    fileName = fN;
    
}

void FileHandle::setPage(FILE * fileTarget){
	target = fileTarget;
}

RC FileHandle::readPage(PageNum pageNum, void *data)
{
	//Seek to the offset of PAGE_SIZE * pageNUM
	if(fseek(target, PAGE_SIZE*pageNum, SEEK_SET))
			return -1;
	//Read the page into data stream and return
	if(fread(data, PAGE_SIZE, 1, target)==1)
			return 0;

    return -1;
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
	if(fseek(target, PAGE_SIZE*pageNum, SEEK_SET))
				return -1;
		//Read the page into data stream and return
		if(fwrite(data, PAGE_SIZE, 1, target)==1)
				return 0;
    return -1;
}


RC FileHandle::appendPage(const void *data)
{
	//Read in an updated directory page
	unsigned int pageNum = getNumberOfPages();
	//Append page to offset at (pageNum+1)*4096
	//Find number of directory pages, add that to total, that will give you true number of pages in file
    unsigned int forward = pageNum + numOfDir;

	fseek(target, forward*PAGE_SIZE, SEEK_SET);
	if(fwrite(data, PAGE_SIZE, 1, target)!=1)
		return -1;

	//update directory to show
	char * dirPage =(char *) malloc (PAGE_SIZE);

	readPage(dirLocations[dirLocations.size()-1], dirPage);

	//Increment counter of that directory page
	unsigned int current;
	memcpy(&current, dirPage, sizeof(int));
	current++;
	//copy it back and write it
	memcpy(dirPage, &current, sizeof(int));
	writePage(dirLocations[dirLocations.size()-1], dirPage);

	free(dirPage);

    return 0;
}


unsigned FileHandle::getNumberOfPages()
{
	//Read in an updated directory page
	//Seek to the beginning
	if(fseek(target,0, SEEK_SET))
			return -1;
	//Allocate memory for page
	char * dirPage = (char *) malloc(PAGE_SIZE);
	//Read in the dirPage
	fread(dirPage, PAGE_SIZE, 1, target);
	unsigned int total  = 0;
	unsigned int pageNum = 0;
	memcpy(&pageNum, dirPage, sizeof(int));
	total = pageNum;

	//Check all other directory pages
	//read in next dir page number
	unsigned int nextD = 0;
	memcpy(&nextD, dirPage + 4092, sizeof(int));
	dirLocations.clear();
	dirLocations.push_back(0);
	unsigned int dirCountUpdater = 1;
	while(nextD!=0)
	{
		//Seek and read in that directory page
		fseek(target, (PAGE_SIZE * nextD), SEEK_SET);
		fread(dirPage, PAGE_SIZE, 1, target);
		//Check how many pages it has
		memcpy(&pageNum, dirPage, sizeof(int));
		//Increment global counter
		total += pageNum;

		//Update quick look up for directory pages
		dirLocations.push_back(nextD);
		//Read in nextD
		memcpy(&nextD, dirPage + 4092, sizeof(int));

		//Loop around again
		dirCountUpdater ++;




	}


	numOfDir = dirCountUpdater;

	free(dirPage);
    return total;
}
bool PagedFileManager::FileExists(const char * fileName)
{
    std::ifstream ifile(fileName);
    if(ifile)
        return true;
    else return false;
}

