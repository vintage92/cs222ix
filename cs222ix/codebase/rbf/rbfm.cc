
#include "rbfm.h"

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;

RecordBasedFileManager* RecordBasedFileManager::instance()
{
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
	//Initialize the pfm and get an instance
	pfm = PagedFileManager::instance();


}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

RC RecordBasedFileManager::createFile(const string &fileName) {
	if(!pfm->createFile(fileName.c_str())){
		return 0;
	}
    return -1;
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {
	if(!pfm->destroyFile(fileName.c_str())){
			return 0;
		}
    return -1;
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
	//Call pfm instance to open the file with the given handle
	if(!pfm->openFile(fileName.c_str(), fileHandle)){
		return 0;
	}
    return -1;
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
	if(!pfm->closeFile(fileHandle)){
			return 0;
		}
    return -1;
}

RC RBFM_ScanIterator::getNextRecord(RID &rid, void *data) {
    
    //TODO: take care of addresses that have been forwarded
    //add functionality to resolve ids to create a
    
    //Instantiate a forwarded record array. This will keep track of which rid's have already been scanned
    //via forwards
    vector<vector<unsigned int>> forwards;
    
    //for as many directory pages, fileHandle should have a count after getPageNumber
        //For as many pages on each directory
            //for as many slots on each page
                //create the RID
                //if the slot offset > 4096 then go to next iteration, this slot has been
                //marked for garbage collection
                //check if it is in the forwards vector
                    //if it is then continue to next iteration this means it's already been retrieved
    
                    //else resolve the id
                        //resolver will make sure to update forwards array if the record id is a forwarded address
                        //if(compOp)read the attribute and check it against condition
                        //if satifies
                        //else read the newID with readRecord and data
                            //read and concatenate the values from readAttribute on each attribute in attributeNames
    
    
    
    
    
    
    
    
    return RBFM_EOF;
}

void RBFM_ScanIterator::setVar(FileHandle *fH, vector<Attribute> rD, string cA, CompOp cO, char * value, vector<string> aN){
    fileHandle = fH;
    recordDescriptor = rD;
    conditionAttribute = cA;
    compOp = cO;
    attributeNames = aN;
    
    //For value
    //If compOp is no op then we don't need value
    if (compOp == NO_OP) {
        //Do nothing
    }
    else{
        //Check type of attribute by searching recordDescriptor for condition attribute
        for(std::vector<Attribute>::const_iterator it = recordDescriptor.begin(); it!=recordDescriptor.end(); it++){
            if (it->name.compare(conditionAttribute) == 0) {
                //Check what type the attribute is
                if (it->type == TypeInt) {
                    condValue = (char *) malloc (sizeof(int));
                    memcpy(condValue, value, sizeof(int));
                    condType = TypeInt;
                    return;
                }
                else if(it->type == TypeReal){
                    condValue = (char *) malloc (sizeof(int));
                    memcpy(condValue, value, sizeof(int));
                    condType = TypeReal;
                    return;
                }
                else{
                    //Get size of value
                    unsigned int sizeVar = 0;
                    memcpy(&sizeVar, value, sizeof(int));
                    //Malloc that much space plus four in condValue
                    condValue = (char *) malloc (4 + sizeVar);
                    memcpy(condValue, value, sizeVar + 4);
                    return;
                }
            }
            
            
        }
    }
    
    
}

RC RecordBasedFileManager::scan(FileHandle &fileHandle,
        const vector<Attribute> &recordDescriptor,
        const string &conditionAttribute,
        const CompOp compOp,                  // comparision type such as "<" and "="
        const void *value,                    // used in the comparison
        const vector<string> &attributeNames, // a list of projected attributes
        RBFM_ScanIterator &rbfm_ScanIterator){
    
    rbfm_ScanIterator.setVar(&fileHandle, recordDescriptor, conditionAttribute, compOp, (char*) value, attributeNames);
    
    
    
    
    
    return 0;
}

//  Format of the data passed into the function is the following:
 //  1) data is a concatenation of values of the attributes
 //  2) For int and real: use 4 bytes to store the value;
 //     For varchar: use 4 bytes to store the length of characters, then store the actual characters.
 //  !!!The same format is used for updateRecord(), the returned data of readRecord(), and readAttribute()
RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {


	//import data into char * array
	const char * rawData = (char * ) data;
	//First we calculate how much space we need to store this record
	//Initialize sizeReq with bookkeeping overhead in page: 4 bytes for slot offset ptr in bookkeeping, 1 for Type R if record,  4 for size of total
	//record stored in sizeReq
	

	unsigned int sizeReq = 9;
	unsigned long arrayOffsetLen = recordDescriptor.size();
	//now increment the values that will be actually stored by the array
	sizeReq += (arrayOffsetLen*4);
	//We will store the offset in bytes in this array
	vector<unsigned int> arrayOffset;
	//arrayOffset will use the values updated in perAttribute Size to compute offsets
	vector<unsigned int> perAttributeSize;

	//arrayOffset[0] = 4*(arrayOffsetLen-1);
	//Now iterate through the recordDiscriptor and count sizes

	//Current Attribute Reading Offset
	unsigned int curAttribute = 0;
	for(std::vector<Attribute>::const_iterator it = recordDescriptor.begin(); it!=recordDescriptor.end(); it++){
		unsigned int attributeSize = 0;
		if(it->type == TypeInt || it->type == TypeReal){
			//Increment by 4 for real or int
			attributeSize +=4;

		}
		else{
			//Increment by 4 bytes to store length of char, then store actual data

			attributeSize +=4;
			//Read in the amount of varchars
			unsigned int varLength = 0;
			memcpy(&varLength, rawData + curAttribute, sizeof(int));

			attributeSize += varLength;

		}
		//Now push it onto the vector of per attribute size and increment the pointer inside the data (curAttribute)
		//Use curAttribute to add to arrayOffset too

		curAttribute +=attributeSize;
		perAttributeSize.push_back(attributeSize);


	}//end for loop
	//Once all attribute sizes are computed, calculate the offset to their starting from that array position
	for(unsigned int i = 0; i<arrayOffsetLen ; i++){ //For each arrayOffset[i]
		//And for that specific perAttributeSize[i]
		//the offset will be calculated-->
			//sum of all attributes in perAttributeSize from 0-(i-1) into previous
			//the ((Length of arrayOffset - i)*4)+previous
			//the offset will be relative to the starting position which is located at that offset's beginning
			//storage byte (which in turn is (i*4)+4 from the beggining of the record)
		unsigned int previous = 0;

		//cout << arrayOffset[i] << endl;
		for(unsigned int j = 0; j<(i); j++){
			previous += perAttributeSize[j];
		}
		unsigned int theOffset = ((arrayOffsetLen - i )*4) + previous;
		arrayOffset.push_back(theOffset);

	}//end for
	//Now for all attribute Sizes, add to sizeReq
	for(unsigned int i =0; i<perAttributeSize.size(); i++){
		sizeReq += perAttributeSize[i];
	}
    
	//now we have our complete Attribute size
	//cout << "Attribute Size is: " << sizeReq << endl;
    
    
    //If the sizeReq > 4066 bytes return an error or don't add it to dB it is larger than a single file
    if (sizeReq > 4066) {
        return -1;
    }
	unsigned int targetDirPageNum = 0;
	//FIND THE PAGE TO PLACE
	//Read in the Directory Page
	DPAGE dir;
	createDPAGE(fileHandle, 0, dir);
	//Find a page with enough free space (note this is the same as a pages conFreeSpace, not total free space)
	unsigned int targetPageNumber = 0;
	//Read in first page number from dir page mem




		bool keepChecking = true;
		for(unsigned int i = 1; i<(510+1); i++ ){//increment numOfPages by one because pages start at 1 not 0
			//Check how much space there is on each page i in dir.freeMap
			//cout << "Free Map" << i << " is : " << dir.freeMap[i] << endl;
			if(sizeReq <=dir.freeMap[i]){
				//Set tPN to i
				targetPageNumber = i;
				keepChecking = false;
				break;//break out of the loop
			}
		}

		unsigned int nextDir = 0;
		nextDir = dir.nextDPage;
		//If it reaches inside this loop it means that there are other directories, and this one should
		//not be used anymore or updated
		while(keepChecking && nextDir != 0){
			//Free old dir
			free (dir.data);
			targetDirPageNum = nextDir;
            
			createDPAGE(fileHandle, nextDir, dir);
			for(unsigned int i = nextDir+1; i<(nextDir+510); i++ ){//510 pages per directory
						//Check how much space there is on each page i in dir.freeMap
						if(sizeReq <=dir.freeMap[i]){
							//Set tPN to i
							targetPageNumber = i;
							keepChecking = false;

							break;//break out of the loop
						}
			}//end for
            nextDir = dir.nextDPage;




		}//end while

//If a page still hasn't been found that means all the current directories are full of full pages
if(targetPageNumber ==0){

        unsigned int thisfirst = targetDirPageNum;
    
		//memcpy(&thisfirst, dir.data + 4, sizeof(int));
		unsigned int newDNum = thisfirst + 511;
		//Update this dir Page with new dpage no (firstPageNumber + 510)
		memcpy(dir.data + 4092, &newDNum, sizeof(int));
		//write current dpage back
		fileHandle.writePage(thisfirst, dir.data);
		targetDirPageNum = newDNum;


		//CREATE A NEW page, add its.... page number = 0+numofofiles + 2) (free - sizeREq)
		char * dirPage = (char *) malloc(PAGE_SIZE);

			//Write values to dirPage
			unsigned int numOfPages = 0;
			unsigned int firstPage = newDNum + 1;
			//Free space on every new page will be 4096 bytes-12
			unsigned int freeD = 4076;
			//unsigned int freeR = 4096;
			unsigned int nextD = 0;

			memcpy(dirPage, &numOfPages, sizeof(int));
			//memcpy(dirPage+4, &firstPage, sizeof(int));
			//memcpy(dirPage+8, &freeR, sizeof(int));
			memcpy(dirPage+4088, &freeD, sizeof(int));
			memcpy(dirPage+4092, &nextD, sizeof(int));
			//Write the d page in
			fileHandle.writePage(newDNum, dirPage);
			
            free(dirPage);
			targetPageNumber = firstPage;


}//end dirupdate if
//TODO check the page being created, if it's free space is 4096, that means you must create a new page
	//By now we have a targetPageNumber, read it in, edit it and write it back
	//Dir has already by updated

	free(dir.data);
	createDPAGE(fileHandle, targetDirPageNum, dir);
	unsigned int freeSpace = dir.freeMap[targetPageNumber];
	//If it is an empty page, create a new record page and update its directories page count


	if(freeSpace==4096){
		dir.numOfPages++;
		memcpy(dir.data, &dir.numOfPages, sizeof(int));
		char * newRPage = (char *) malloc(PAGE_SIZE);
		unsigned int pageNum = targetPageNumber;
		unsigned int slots = 0;
		unsigned int FragFree = 4076;
		unsigned int ConFree = 4076;
		unsigned int freeOffset = 4;
		memcpy(newRPage, &pageNum, sizeof(int));
		memcpy(newRPage+4080, &slots, sizeof(int));
		memcpy(newRPage+4084, &FragFree, sizeof(int));
		memcpy(newRPage+4088, &ConFree, sizeof(int));
		memcpy(newRPage+4092, &freeOffset, sizeof(int));

		fileHandle.writePage(targetPageNumber, newRPage);
		free (newRPage);

	}

	//Now read in the page, whether just created or preexistent
    
	RPAGE targetPage = createRPAGE(fileHandle, targetPageNumber);
    
    
    //Check if the freeOffsetPointer plus sizeReq stays within bounds of the page. If it exceeds this means
    //that the page should be reorganized
    
        //TODO: check if this should be numslots or numslots -1, because we are adding a new new node as well
    
        unsigned int threshold = 4076-(4*(targetPage.numSlots - 1));
        //If the freeOffsetPointer + sizeReq exceeds the threshold, that means the page needs to be regorganzied
        //after reorganization the record will fit at the end of the page
    //TODO: check this logic
        if(targetPage.freePointerOffset + sizeReq >= threshold){
            //Free targetPage
            free(targetPage.data);
           reorganizePage(fileHandle, recordDescriptor, targetPageNumber);
            
            //Read the updated page back into targetPage
            targetPage = createRPAGE(fileHandle, targetPageNumber);
            
        }
    
    
			unsigned int temp;
    
            unsigned int activeSlot = 0;
            bool slotActive = false;
   
            //Check for any recyclable slots
            for(unsigned int i = 0; i < targetPage.numSlots; i++ ){
                
                if (targetPage.arraySlotOffset[i] == 0){
                    //set this slot as active slot, because it can be reused
                    activeSlot = i;
                    slotActive = true;
                    targetPage.arraySlotOffset[i] = targetPage.freePointerOffset;
                    targetPage.conFreeSpace -= (sizeReq -4); //-4 because we're recycling 4 bits for slot
                    targetPage.totFreeSpace -= (sizeReq -4); //-4 because we're recycling 4 bits for slot
                    break;
                }
            }
    
            if (slotActive == false) {
                //increment the slots N
                    targetPage.numSlots++;
                    temp = targetPage.numSlots;
                    memcpy(targetPage.data+4080, &temp, sizeof(int));
                    targetPage.arraySlotOffset.push_back(targetPage.freePointerOffset);
                    targetPage.conFreeSpace -= sizeReq;
                    targetPage.totFreeSpace -= sizeReq;
                    activeSlot = targetPage.numSlots - 1;
            }
    
            //write  freespaces of page
			
			
			memcpy(targetPage.data+4088, &targetPage.conFreeSpace, sizeof(int));
			
            memcpy(targetPage.data+4084, &targetPage.totFreeSpace, sizeof(int));

    
			
        
            
			
            //Change the free pointer offset to be the new records beginning offset pointer
			//targetPage.arraySlotOffset[targetPage.numSlots] = targetPage.freePointerOffset; //this points to beginning of attribute array
            //cout << "Adding to aSoffset: " << targetPage.freePointerOffset << endl;
			
			targetPage.freePointerOffset += (sizeReq-4);
			//cout << targetPage.freePointerOffset << endl;
			temp = targetPage.freePointerOffset;
			//Now write the new values into targetData's data pointer
			memcpy(targetPage.data+4092, &temp, sizeof(int));
    
			//Write the newest block on the arrayOffset from arraySlotOffset
			unsigned int forward = 4076-(4*(activeSlot));
			temp = targetPage.arraySlotOffset[activeSlot];
			memcpy(targetPage.data+forward, &temp, sizeof(int));

			//Copy over the attribute array
			temp = targetPage.arraySlotOffset[activeSlot];
    
            //Begin copying the record in
            //Copy in total size of the record to the beggining of the record
            char recType = 'R';
            memcpy(targetPage.data + temp, &recType, sizeof(char));
    
            temp += 1;
            unsigned int sizeOfRecData = sizeReq - 4;
    
			memcpy(targetPage.data + temp, &sizeOfRecData, sizeof(int));
			temp += 4;
			for(unsigned int i = 0; i<arrayOffset.size(); i++){
				//memcopy the array
				memcpy(targetPage.data + temp, &arrayOffset[i], sizeof(int));
				temp+=4;
			}
			unsigned int dataSize = 0;
			for(unsigned int i =0; i<perAttributeSize.size(); i++){
					dataSize += perAttributeSize[i];
				}
    
			memcpy(targetPage.data+temp, rawData, dataSize);

	fileHandle.writePage(targetPageNumber, targetPage.data);
    
    


	rid.pageNum = targetPageNumber;
	rid.slotNum = activeSlot;

	free (targetPage.data);
    

	//update the directory with conFreeSpace value and write in the pages values to the directory
	unsigned int toPage = dir.pageToOffset[targetPageNumber];
	memcpy(dir.data + toPage, &targetPageNumber, sizeof(int));
	toPage +=4;
	memcpy(dir.data + toPage, &targetPage.conFreeSpace, sizeof(int));
	fileHandle.writePage(targetDirPageNum, dir.data);
    
	free (dir.data);

	return 0;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    
    //Read in the dir page and make sure we are within page limits
    float dirPageNumber = ceilf(rid.pageNum/510.0) - 1 ;
    unsigned int theDNum = (unsigned int)dirPageNumber;
    unsigned int ActualDnum = (511 * theDNum);
    
    //TODO: fix page range finder below
    //Page number must be less than or equal to dirPageNumber + numOfPages
    
    
    DPAGE tempDir;
    createDPAGE(fileHandle, ActualDnum, tempDir);
    unsigned int range = ActualDnum + tempDir.numOfPages;
    
    if (rid.pageNum > range) {
        return -1;
    }
    
	//Read in that page
    cout << "Record Id: " << rid.pageNum << " " << rid.slotNum << endl;
	//Find the offset to beginning of pointer
    RID newid = resolveAddress(fileHandle, rid);
    
	unsigned int numOfAtt = recordDescriptor.size();
	RPAGE targetPage = createRPAGE(fileHandle, newid.pageNum);
	unsigned int sizeRet = 0;

	//Calculate size of data to be returned
	//Read in data for attribute array
	vector<unsigned int> arrayOffset;
	unsigned int forward = targetPage.arraySlotOffset[newid.slotNum];
    /*char recType;
    memcpy(&recType, targetPage.data + forward, sizeof(char));
    cout << recType << endl;
     */
    
    //If the array offset is greater than 4096, that means this record has been deleted
    //Return -1
    if (forward > 4096) {
        return -1;
    }
	unsigned int cnt = forward + sizeof(int) + sizeof(char); //+ recType, recSize
	for(unsigned int i = 0; i<recordDescriptor.size(); i++){
		unsigned int temp;
		memcpy(&temp, targetPage.data + cnt, sizeof(int));
		arrayOffset.push_back(temp);
		cnt += 4;
	}
    unsigned int lastOffset = arrayOffset[numOfAtt-1];
	//TODO print out arrayOffsets here to debug
    if(lastOffset > 4096){
        cout <<"Problem here";
    }
	//Find beginning of values

	//Find the offset plus length of the last attribute
	 
	if(recordDescriptor[numOfAtt-1].type == TypeReal || recordDescriptor[numOfAtt-1].type == TypeInt )
	{
		lastOffset +=4;
	}
	else{
		//Read in length of chars
		unsigned int addon = 0;
		memcpy(&addon, targetPage.data + forward + lastOffset, 4);
		//Increment last offset by 4 for size of length of characters
		lastOffset += 4;
		lastOffset += addon;
	}

	sizeRet =  (lastOffset - 4);
	unsigned int begincopy = forward + arrayOffset[0] + 4 + 1; //plus 4 cause size of array in beginning and recType
    //TODO: Last offset here is the problem
	memcpy(data, targetPage.data + begincopy, sizeRet);

	free (targetPage.data);




    return 0;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {

	unsigned int curAttribute = 0;
	const char * rawData = (char *) data;
	cout << "Record--->" << endl;
		for(std::vector<Attribute>::const_iterator it = recordDescriptor.begin(); it!=recordDescriptor.end(); it++){
            if(it->name.compare("attr29")==0){
                cout << "Here" << endl;
            }
			cout << it->name << ": ";
			if(it->type==TypeInt){
				//Read in an int and print it out
				int temp;
				memcpy(&temp, rawData + curAttribute, sizeof(int));
				//Increment data pointer
				curAttribute += 4;
				cout << temp << endl;
			}
			else if(it->type==TypeReal){
				float temp = 0;
				memcpy(&temp, rawData + curAttribute, sizeof(int));
				curAttribute += 4;
				cout << temp << endl;
			}
			else if(it->type==TypeVarChar){
				//Read in length of char string
				unsigned int length = 0;
				memcpy(&length, rawData + curAttribute, sizeof(int));
				curAttribute += 4;
				//Now read in length size from data
				char * value = (char *)malloc(length + 1);
				memcpy(value, rawData + curAttribute, length);
				curAttribute += length;
				value[length] = NULL;
				string theValue = value;
				cout << theValue << endl;
			}

		}

		cout << "--End Record--" << endl;
    return 0;
}
RC RecordBasedFileManager::deleteRecords(FileHandle &fileHandle){
    
    //Delete the file
    if(destroyFile(fileHandle.fileName)){
        return -1;
    }
    
    //Create a new empty file with the same name
    if(createFile(fileHandle.fileName)){
        return -1;
    }
    return 0;

}
RC RecordBasedFileManager::deleteRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid){
    //Check if the record has a forwarding address we will need to delete it as well
    RID newID = resolveAddress(fileHandle, rid);
    if(newID.pageNum != rid.pageNum || newID.slotNum != rid.slotNum){
       
        //If so set the first byte of the forwarding address to 'D'
        RPAGE firstPage = createRPAGE(fileHandle, rid.pageNum);
        unsigned int offset = firstPage.arraySlotOffset[rid.slotNum];
        char value = 'X';
        memcpy(firstPage.data + offset, &value, sizeof(char));
        
        //Write the page back in and update free space. Offset will be reclaimed in reorganizePage
        firstPage.conFreeSpace += 9;
        firstPage.totFreeSpace += 9;
        
        memcpy(firstPage.data + 4084, &firstPage.conFreeSpace, sizeof(int));
        memcpy(firstPage.data + 4088, &firstPage.totFreeSpace, sizeof(int));
        
        //Update arrayslot offset by 9000, this will signify deleted forwarding address
        firstPage.arraySlotOffset[rid.slotNum] = offset + 9000;
        unsigned int writeTo = 4076-(4*rid.slotNum);
        memcpy(firstPage.data + writeTo, &firstPage.arraySlotOffset[rid.slotNum], sizeof(int));
        
        fileHandle.writePage(rid.pageNum, firstPage.data);
        updateDPAGE(fileHandle, rid.pageNum, firstPage.conFreeSpace);
        
        free(firstPage.data);
        
        
    }
    
    //Read in the page
    RPAGE targetPage = createRPAGE(fileHandle, newID.pageNum);
    //Navigate to that record and read in size
    unsigned int forward = targetPage.arraySlotOffset[newID.slotNum];
    unsigned int recSize;
    memcpy(&recSize, targetPage.data + forward + 1, sizeof(int));
    //Add size to freespace on Page
    char delMarker = 'D';
    memcpy(targetPage.data + forward, &delMarker, 1);
    targetPage.conFreeSpace += recSize;
    targetPage.totFreeSpace += recSize;
    
    //update arrayslotoffset with the offset + 4096
    targetPage.arraySlotOffset[newID.slotNum] = forward + 4096;
    
    //Update data on targetPage
    //Update bookkeeping for new page
    
    memcpy(targetPage.data + 4084, &targetPage.totFreeSpace, sizeof(int));
    memcpy(targetPage.data + 4088, &targetPage.conFreeSpace, sizeof(int));
    
        forward = 4076-(4*newID.slotNum);
        memcpy(targetPage.data + forward, &targetPage.arraySlotOffset[newID.slotNum], sizeof(int));
    
    //Persist data
    fileHandle.writePage(newID.pageNum, targetPage.data);
    updateDPAGE(fileHandle, newID.pageNum, targetPage.conFreeSpace);
    
    //Free targetPage
    free (targetPage.data);
        
    
    
    
    return 0;
}

RC RecordBasedFileManager::updateRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, const RID &rid){

    //Check if the RID has a forwarding address already
    RID newID = resolveAddress(fileHandle, rid);
    if(newID.pageNum != rid.pageNum || newID.slotNum != rid.slotNum){
        //If so delete it and recliam space
        deleteRecord(fileHandle, recordDescriptor, newID);
        
        //Reinsert the record into the file
        insertRecord(fileHandle, recordDescriptor, data, newID);
        //Update page with forwarding address from newID
        
        RPAGE targetPage = createRPAGE(fileHandle, rid.pageNum);
        
        unsigned int offset = targetPage.arraySlotOffset[rid.slotNum];
        memcpy(targetPage.data + offset + 1, &newID.pageNum, sizeof(int));
        memcpy(targetPage.data + offset + 5, &newID.slotNum, sizeof(int));
        
        //update the rid with new one
        
        
        //write the page back
        fileHandle.writePage(rid.pageNum, targetPage.data);
        
        return 0;
        
        
        
        
    }
    
    //else delete the record and reorganize the page
    else{
        
        //Insert the updated record
        insertRecord(fileHandle, recordDescriptor, data, newID);
        
        //Keeping track of the slot num, we will add the forwarding address to the end of the page
        //and assign the offset to the slot num. There will be enough space
        //reorganizePage(fileHandle, recordDescriptor, rid.pageNum);
        RPAGE targetPage = createRPAGE(fileHandle, rid.pageNum);
       
        //Get size of the record
        
        unsigned int offset = targetPage.arraySlotOffset[rid.slotNum];
        unsigned int recSize = 0;
        memcpy(&recSize, targetPage.data + offset + 1, sizeof(int));
        
        
        //Update page to reflect added forwarding address
        
        char head = 'F';
        memcpy(targetPage.data + offset, &head, 1);
        targetPage.conFreeSpace += (recSize - 9);
        targetPage.totFreeSpace += (recSize - 9);
        updateDPAGE(fileHandle, rid.pageNum, targetPage.conFreeSpace);
        
        
        
        unsigned int forward = 4076 - (4 * (rid.slotNum));
        memcpy(targetPage.data + 4092, &targetPage.freePointerOffset, sizeof(int));
        memcpy(targetPage.data + 4088, &targetPage.conFreeSpace, sizeof(int));
        memcpy(targetPage.data + 4084, &targetPage.totFreeSpace, sizeof(int));
        //memcpy(targetPage.data + 4080, &targetPage.numSlots, sizeof(int));
        memcpy(targetPage.data + forward, &offset, sizeof(int));
        fileHandle.writePage(rid.pageNum, targetPage.data);
        free (targetPage.data);
        
        
        
        //Read in original page and add forwarding address
        
        targetPage = createRPAGE(fileHandle, rid.pageNum);
        
        
        memcpy(targetPage.data + offset + 1, &newID.pageNum, sizeof(int));
        memcpy(targetPage.data + offset + 5, &newID.slotNum, sizeof(int));
        
        //Write the page back
        fileHandle.writePage(rid.pageNum, targetPage.data);
        
        printf("Record %u %u , points to %u %u\n", rid.pageNum, rid.slotNum, newID.pageNum, newID.slotNum);
        
        free (targetPage.data);
        
        return 0;
    }
    return -1;
}

RC RecordBasedFileManager::readAttribute(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, const string attributeName, void *data){
    
    
    //Get absolute address
    RID newID = resolveAddress(fileHandle, rid);
    
    
    //Read in the page
    RPAGE targetPage = createRPAGE(fileHandle, newID.pageNum);
    
    //Find the attribute position in the recordDescriptor
    unsigned int position = 0;
    
    for(std::vector<Attribute>::const_iterator it = recordDescriptor.begin(); it!=recordDescriptor.end(); it++){
        //If name is found then break
        if(it->name.compare(attributeName) == 0){
            break;
        }
        //Otherwise incremement position
        position++;
        
    }
    
    
    //Calculate offset to its array position on file
    unsigned int forward = targetPage.arraySlotOffset[newID.slotNum];
    forward += 1+4;
    forward += 4 * position;
    
    //Read in the offset to the attribute
    unsigned int offset = 0;
    memcpy(&offset, targetPage.data + forward, sizeof(int));
    offset += forward;
    
    
       /* char * testData = (char *) malloc (2000);
        readRecord(fileHandle, recordDescriptor, rid, testData);
        printRecord(recordDescriptor, testData);*/
    
    
    //If its an int or real just read in the four bytes
    
    if(recordDescriptor[position].type == TypeInt || recordDescriptor[position].type == TypeReal){
        memcpy(data, targetPage.data + offset, 4);
        unsigned int tester = 0;
        memcpy(&tester, data, 4);
        return 0;
        
    }
    else{
    
        //Read in the size of the attribute
        unsigned int varSize;
        memcpy(&varSize, targetPage.data + offset, sizeof(int));
       
        //char * tester = (char *) malloc (4);
        
        //copy in rest of attribute
        memcpy(data, targetPage.data + offset, varSize +4);
        //memcpy(tester, (char*)data + 4, varSize);
    
        return 0;
    }
    return -1;
}

RC RecordBasedFileManager::reorganizePage(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const unsigned pageNumber){
    //Allocate newData PageSize
    char * newData = (char *) malloc (PAGE_SIZE);
    //start newData offset at 4
    unsigned int writeOffset = 4;
    unsigned int lastUsedSlot = 0;
    
    //Read in the page
    RPAGE targetPage = createRPAGE(fileHandle, pageNumber);
    
    //For as many slots in the page...
    for(unsigned int i = 0; i < targetPage.numSlots; i++){
    
    //Read slot Offset Pointer
        unsigned int cntRecord = targetPage.arraySlotOffset[i];
        
        if (cntRecord == 0) {
            //Continue
        }
        else if(cntRecord > 9000){
            //It's a deleted forwarding address
            targetPage.arraySlotOffset[i] = 0;
            
        }
        
        else if(cntRecord > 4096){
            //If the cntRecord offset points to greater than 4096 that means that it is location of a tombstone
            //subtract it by 4096 to get the offset of this tombstone
            //We need to garbage collect it
            //Read in the size of the record from cntRecord + 1
            //Set the arrayOffsetSlot Pointer to 0 this will tell
            //insertRecord method to reuse this slot
            
            targetPage.arraySlotOffset[i] = 0;
        
        }
        
        else{
        //Read in recType from beginning of record
            char * recType = (char *) malloc(8);
            memcpy(recType, targetPage.data + cntRecord, 7);
            //Now recType[0] == 'R' or add null terminator and strcmp with FoRwArD
            if(recType[0] == 'R'){
                //Read in size of record
                unsigned int recSize;
                memcpy(&recSize, targetPage.data + cntRecord + 1, sizeof(int));
                //Add to record to newData
                memcpy(newData + writeOffset, targetPage.data + cntRecord, recSize);
                //Update the slot offset
                targetPage.arraySlotOffset[i] = writeOffset;
                writeOffset += recSize;
                lastUsedSlot = i;
                
                
                
            }
            else{
                //It must be a forward
                
                    //Just copy the 9 byte address over to newData
                    //Update the arraySlotOffset
                    memcpy(newData + writeOffset, targetPage.data + cntRecord, 9);
                    targetPage.arraySlotOffset[i] = writeOffset;
                    writeOffset += 9;
                    lastUsedSlot = i;
                
            }
            
            free (recType);
            
            
        }
        
    }
    //write Offset will now be pointing to the end of the newFree Space, set it to freeOffsetPointer
    targetPage.freePointerOffset = writeOffset;
    //Update number of valid slots, it will be last used slot + 1 the rest after it can be discarded
    targetPage.numSlots = lastUsedSlot + 1;
    
    //Update bookkeeping for new page
    memcpy(newData, &pageNumber, sizeof(int));
    memcpy(newData + 4080, &targetPage.numSlots, sizeof(int));
    memcpy(newData + 4092, &targetPage.freePointerOffset, sizeof(int));
    
    //Insert the slot values
    for (unsigned int i = 0; i<targetPage.numSlots; i++) {
        unsigned int forward = 4076-(4*i);
        memcpy(newData + forward, &targetPage.arraySlotOffset[i], sizeof(int));
        
    }
    
    //Persist newData
    fileHandle.writePage(pageNumber, newData);
    //updateDPAGE(fileHandle, pageNumber, targetPage.)
    
    
    
    //free newData
    //free targetPage.data
    
    free(newData);
    free(targetPage.data);
    
    return 0;
}

RID RecordBasedFileManager::resolveAddress(FileHandle &fileHandle, RID rid, map<unsigned int, map<unsigned int, bool>> &forwards){
   /*
    //Read in the first Page
    RID newID;
    RPAGE targetPage = createRPAGE(fileHandle, rid.pageNum);
    //Set forwarding = true;
    //bool forwarding = true;
    //Read in first byte of record if it is R then return this page
    unsigned int offset = targetPage.arraySlotOffset[rid.slotNum];
    char * recType = (char *) malloc(1);
    memcpy(recType, targetPage.data + offset, sizeof(char));
    //Now recType[0] == 'R'
    if(recType[0] == 'R'){
        newID.pageNum = rid.pageNum;
        newID.slotNum = rid.slotNum;
        free(targetPage.data);
        free (recType);
        forwards[newID.pageNum][newID.slotNum] = false;
        return newID;
        
    }
    //If not then set newID and return
    memcpy(&newID.pageNum, targetPage.data + offset, sizeof(int));
    memcpy(&newID.slotNum, targetPage.data + offset + 4, sizeof(int));
    //Add to list of forwards
    forwards[newID.pageNum][newID.slotNum] = true;
    
    
    free(targetPage.data);
    
    free (recType);
    return newID;
    */
    
    //Read in the first Page
    RID newID;
    RPAGE targetPage = createRPAGE(fileHandle, rid.pageNum);
    //Set forwarding = true;
    //bool forwarding = true;
    //Read in first byte of record if it is R then return this page
    unsigned int offset = targetPage.arraySlotOffset[rid.slotNum];
    char * recType = (char *) malloc(1);
    memcpy(recType, targetPage.data + offset, sizeof(char));
    //Now recType[0] == 'R'
    if(recType[0] == 'R'){
        newID.pageNum = rid.pageNum;
        newID.slotNum = rid.slotNum;
        free (recType);
        free(targetPage.data);
        forwards[newID.pageNum][newID.slotNum] = false;
        //AnewID.valid = true;
        return newID;
        
    }
    else if(recType[0] == 'D'){
        //Means its a deleted forwarding address
        //Return -1's for the record id
        //AnewID.valid = false;
        free(recType);
        free(targetPage.data);
        return newID;
    }
    //TODO: now we have an F in front of forwarded addresses
    //If not then set newID and return
    memcpy(&newID.pageNum, targetPage.data + offset + 1, sizeof(int));
    memcpy(&newID.slotNum, targetPage.data + offset + 5, sizeof(int));
    forwards[newID.pageNum][newID.slotNum] = true;
    
    //AnewID.valid = true;
    
    free(targetPage.data);
    free (recType);
    return newID;
    
    
}

RID RecordBasedFileManager::resolveAddress(FileHandle &fileHandle, RID rid){
    //Read in the first Page
    RID newID;
    RPAGE targetPage = createRPAGE(fileHandle, rid.pageNum);
    //Set forwarding = true;
    //bool forwarding = true;
    //Read in first byte of record if it is R then return this page
    unsigned int offset = targetPage.arraySlotOffset[rid.slotNum];
    if (offset > 4096) {
        return rid;
    }
    char * recType = (char *) malloc(1);
    memcpy(recType, targetPage.data + offset, sizeof(char));
    //Now recType[0] == 'R'
    if(recType[0] == 'R'){
        newID.pageNum = rid.pageNum;
        newID.slotNum = rid.slotNum;
        free (recType);
        free(targetPage.data);
        //AnewID.valid = true;
        return newID;
        
    }
    else if(recType[0] == 'D'){
        //Means its a deleted forwarding address
        //Return -1's for the record id
        //AnewID.valid = false;
        free(recType);
        free(targetPage.data);
        return newID;
    }
    //TODO: now we have an F in front of forwarded addresses
    //If not then set newID and return
    memcpy(&newID.pageNum, targetPage.data + offset + 1, sizeof(int));
    memcpy(&newID.slotNum, targetPage.data + offset + 5, sizeof(int));
    //AnewID.valid = true;
   
    free(targetPage.data);
    free (recType);
    return newID;
    
}

void  RecordBasedFileManager::createDPAGE(FileHandle &fileHandle, unsigned int pageNumber, DPAGE &directory){


		//Allocate memory for page
		char * dirPage = (char *) malloc(PAGE_SIZE);
		//Read in the dirPage
		fileHandle.readPage(pageNumber, dirPage);
		//fread(dirPage, PAGE_SIZE, 1, fileHandle.target);

		//memcpy(&pageNum, dirPage, sizeof(4));
		//Create a new DPAGE struct

		memcpy(&directory.numOfPages, dirPage, sizeof(int));
		memcpy(&directory.freeDSpace, dirPage+4088, sizeof(int));
		memcpy(&directory.nextDPage, dirPage+4092, sizeof(int));

		//For the num of pages, copy in the page number and free space associated with it
		int offset = 4;
		unsigned int pageno = pageNumber + 1;
		for(unsigned int i = 0; i<directory.numOfPages; i++){

			unsigned int freespace;

			memcpy(&pageno, dirPage+offset, sizeof(int));
			offset +=4;
			memcpy(&freespace, dirPage + offset, sizeof(int));
			offset +=4;
			directory.freeMap[pageno] = freespace;
			directory.pageToOffset[pageno] = (offset -8);
			//directory.freeMap.insert (std::pair<unsigned int,unsigned int>(pageno,freespace));
			//directory.pageToOffset.insert(std::pair<unsigned int, unsigned int>(pageno, (offset-8)));

		}//end for

		//Initialize free pages
		//now for rest of pages, from (pageNumber + numofPages + 1) to total possible pages on file 510

		for(unsigned int i = (pageNumber + directory.numOfPages + 1); i<=(pageNumber+510) ; i++){
			directory.freeMap[i] = 4096;
			directory.pageToOffset[i] = (offset);
			//cout << "Free Map inserted " << i << " is : " << directory.freeMap[pageno] << endl;
			offset +=8;
		}

		directory.data = dirPage;



}
RPAGE RecordBasedFileManager::createRPAGE(FileHandle &fileHandle, unsigned int pageNumber){
			//fseek(fileHandle.target,pageNumber*4096, SEEK_SET);
			//Allocate memory for page
			char * recPage = (char *) malloc(PAGE_SIZE);
			//Read in the dirPage
			//fread(recPage, PAGE_SIZE, 1, fileHandle.target);
			fileHandle.readPage(pageNumber, recPage);
			RPAGE rec;
			rec.pageNum = pageNumber;
			rec.freePointerOffset = 0;
			rec.conFreeSpace = 0;
			rec.totFreeSpace = 0;
			rec.numSlots = 0;
			if(pageNumber == 1){
				//TODO find out why page one values are getting corrupt, put a breakpoint here
				//and resume to it after record 11 is inserted. See if it is ever called as a
				//RPAGE.
				//cout << pageNumber << "Being called again:" << endl;
			}
			memcpy(&rec.pageNum, recPage, sizeof(int));
			memcpy(&rec.freePointerOffset, recPage + 4092, sizeof(int));
			memcpy(&rec.conFreeSpace, recPage + 4088, sizeof(int));
			memcpy(&rec.totFreeSpace, recPage + 4084, sizeof(int));
			memcpy(&rec.numSlots, recPage + 4080, sizeof(int));
            rec.arraySlotOffset.clear();
			for(unsigned int i = 0; i<rec.numSlots; i++){
				//memcopy the offset for the particular slot number and add it to array in rec
				unsigned int recOffset;
				unsigned int forward = 4076-(4*i);
				memcpy(&recOffset, recPage + forward , sizeof(int));
				rec.arraySlotOffset.push_back(recOffset);

			}//end for
			//copy over entire data
			rec.data = recPage;
			//Remember to call when you are done with a page free(recPage);
			return rec;

}

void RecordBasedFileManager::updateDPAGE(FileHandle &fileHandle, unsigned int pageNum, unsigned int freeSpace){
    float dirPageNumber = ceilf(pageNum/510.0) - 1 ;
    unsigned int theDNum = (unsigned int)dirPageNumber;
    unsigned int ActualDnum = (511 * theDNum);
    
    //Read in the dir Page
    DPAGE aDir;
    createDPAGE(fileHandle, ActualDnum, aDir);
    //Write new free space at the appropriate page
    unsigned int offsetToPage = aDir.pageToOffset[pageNum];
    memcpy(aDir.data + offsetToPage + 4, &freeSpace, 4);
    
    fileHandle.writePage(ActualDnum, aDir.data);
    
    free(aDir.data);
    
}

//




//
