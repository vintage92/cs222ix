
#include "rm.h"

RelationManager* RelationManager::_rm = 0;

RelationManager* RelationManager::instance()
{
    if(!_rm)
        _rm = new RelationManager();

    return _rm;
}

RelationManager::RelationManager()
{
    
    //TODO: remove these lines
    //for now delete the files
    string fName = "table";
    //remove(fName.c_str());
    fName = "column";
    //remove(fName.c_str());
    
    
    //Initialize rbfm
    rbfm = RecordBasedFileManager::instance();
    
    //Populate the initial table and column record descriptor arrays. These are implemenation dependent
    Attribute attr;
    attr.name = "tableName";
    attr.length = (AttrLength)256;
    attr.type = TypeVarChar;
    
    tableDescriptor.push_back(attr);
    
    /*
    we'll use tableName as fileName for now
    attr.name = "fileName";
    attr.length = (AttrLength)256;
    attr.type = TypeVarChar;

    
    tableDescriptor.push_back(attr);
     */
    
    //0 will be system, 1 will be user table
    attr.name = "type";
    attr.length = (AttrLength)4;
    attr.type = TypeInt;
    
    tableDescriptor.push_back(attr);
    
    attr.name = "numCol";
    attr.length = (AttrLength)4;
    attr.type = TypeInt;
    
    tableDescriptor.push_back(attr);
    
    //Populate the column table descriptor array
    
    attr.name = "tableName";
    attr.length = (AttrLength)256;
    attr.type = TypeVarChar;
    
    columnDescriptor.push_back(attr);
    
    attr.name = "colName";
    attr.length = (AttrLength)256;
    attr.type = TypeVarChar;
    
    columnDescriptor.push_back(attr);
    
    //colType will be 0 for Int, 1 for Real, 2 for varchar
    attr.name = "colType";
    attr.length = (AttrLength)4;
    attr.type = TypeInt;
    
    columnDescriptor.push_back(attr);
    
    attr.name = "colPos";
    attr.length = (AttrLength)4;
    attr.type = TypeInt;
    
    columnDescriptor.push_back(attr);
    
    attr.name = "maxSize";
    attr.length = (AttrLength)4;
    attr.type = TypeInt;
    
    columnDescriptor.push_back(attr);
    
    attrMap["column"] = columnDescriptor;
    attrMap["table"] = tableDescriptor;
    
    /*//0 marks inactive attribute "dropped"
    //1 marks active attribute
    attr.name = "active";
    attr.length = (AttrLength)4;
    attr.type = TypeInt;
    
    columnDescriptor.push_back(attr);
     */
    
    //We will check to see if there exist any table and column catalogs first
   
    if(!FileExists("column")){
        rbfm->createFile("column");
        
        FileHandle fileHandle;
        
        rbfm->openFile("column", fileHandle);
        
        RID rid;
        
        char * data = (char *) malloc (4096);
        unsigned long temp = 0;
        
        unsigned int offset = 0;
       
        //Copy over table's information
        
        
        
        
        for (int i = 0; i<tableDescriptor.size(); i++) {
            //For each column attribute
            
            offset = 0;
            string value = "table";
            temp = value.length();
            memcpy(data, &temp, 4);
            offset += 4;
            memcpy(data + offset, value.c_str(), temp);
            offset += temp;
            
            value = tableDescriptor[i].name;
            temp = value.length();
            memcpy(data + offset, &temp, 4);
            offset += 4;
            memcpy(data + offset, value.c_str(), temp);
            offset += temp;
            
            temp = tableDescriptor[i].type;
            memcpy(data + offset, &temp, 4);
            offset += 4;
            
            temp = i;
            memcpy(data + offset, &temp, 4);
            offset += 4;
            
            temp = tableDescriptor[i].length;
            memcpy(data + offset, &temp, 4);
            char * tempData = (char *) malloc(4096);
            rbfm->insertRecord(fileHandle, columnDescriptor, data, rid);
            rbfm->readRecord(fileHandle, columnDescriptor, rid, tempData);
            rbfm->printRecord(columnDescriptor, tempData);
            free(tempData);
            
           
        }
        
        //Do the same for column's columns
        for (int i = 0; i<columnDescriptor.size(); i++) {
            //For each column attribute
            
            offset = 0;
            string value = "column";
            temp = value.length();
            memcpy(data, &temp, 4);
            offset += 4;
            memcpy(data + offset, value.c_str(), temp);
            offset += temp;
            
            value = columnDescriptor[i].name;
            temp = value.length();
            memcpy(data + offset, &temp, 4);
            offset += 4;
            memcpy(data + offset, value.c_str(), temp);
            offset += temp;
            
            temp = columnDescriptor[i].type;
            memcpy(data + offset, &temp, 4);
            offset += 4;
            
            temp = i;
            memcpy(data + offset, &temp, 4);
            offset += 4;
            
            temp = columnDescriptor[i].length;
            memcpy(data + offset, &temp, 4);
            
            char * tempData = (char *) malloc(4096);
            rbfm->insertRecord(fileHandle, columnDescriptor, data, rid);
            rbfm->readRecord(fileHandle, columnDescriptor, rid, tempData);
            rbfm->printRecord(columnDescriptor, tempData);
            free (tempData);
            
        }
        
        
        
        
        free(data);
        
        
    }
    
    if (!FileExists("table")) {
        //Create it and populate it with initial data
        rbfm->createFile("table");
        
        FileHandle fileHandle;
        RID rid;
        
        rbfm->openFile("table", fileHandle);
        
        char * data = (char *) malloc (1000);
        unsigned int temp = 0;
        unsigned int offset = 0;
        
        
        string name = "table";
        temp = name.length();
        
        memcpy(data, &temp, 4);
        offset += 4;
        memcpy(data + offset, name.c_str(), temp);
        offset += temp;
        
        temp = 0;
        memcpy(data + offset, &temp, 4);
        offset += 4;
        
        temp = 3;
        memcpy(data + offset, &temp, 4);
        
        char * tempData = (char *) malloc(4096);
        
        rbfm->insertRecord(fileHandle, tableDescriptor, data, rid);
        rbfm->readRecord(fileHandle, tableDescriptor, rid, tempData);
        rbfm->printRecord(tableDescriptor, tempData);
        
        //enter column table in table
        
        
        name = "column";
        temp = name.length();
        offset = 0;
        
        memcpy(data, &temp, 4);
        offset += 4;
        memcpy(data + offset, name.c_str(), 6);
        offset += 6;
        
        temp = 0;
        memcpy(data + offset, &temp, 4);
        offset += 4;
        
        temp = 5;
        memcpy(data + offset, &temp, 4);
        
        
        rbfm->insertRecord(fileHandle, tableDescriptor, data, rid);
        rbfm->readRecord(fileHandle, tableDescriptor, rid, tempData);
        rbfm->printRecord(tableDescriptor, tempData);
        free (data);
        free(tempData);
     
    }
    
    parseCatalogs();
    
}

RelationManager::~RelationManager()
{
}

RC RelationManager::createTable(const string &tableName, const vector<Attribute> &attrs)
{
    
    
    RC rc = 0;
    
    //If the table file exists, then return 0
    if (FileExists(tableName)) {
        return 0;
    }
    //Add the table Name, type, and num of columns
    
    //First open the table file
    FileHandle fileHandle;
    
    rc = rbfm->openFile("table", fileHandle);
    
    RID rid;
    
    char * data = (char *) malloc (4096);
    unsigned int temp = 0;
    unsigned int offset = 0;
    
    
    temp = tableName.length();
    
    memcpy(data, &temp, 4);
    offset += 4;
    memcpy(data + offset, tableName.c_str(), temp);
    offset += temp;
    
    //set type to 1 for user flag
    temp = 1;
    memcpy(data + offset, &temp, 4);
    offset += 4;
    
    
    temp = attrs.size();
    memcpy(data + offset, &temp, 4);
    
    
    rc = rbfm->insertRecord(fileHandle, tableDescriptor, data, rid);
    
    //Close table file
    rc = rbfm->closeFile(fileHandle);
    
    //Open columns file
    rc = rbfm->openFile("column", fileHandle);
    
    
    
    //Do the same for column's columns
    for (int i = 0; i<attrs.size(); i++) {
        //For each column attribute
        
        offset = 0;
        string value = tableName;
        temp = value.length();
        memcpy(data + offset, &temp, 4);
        offset += 4;
        memcpy(data + offset, value.c_str(), temp);
        offset += temp;
        
        value = attrs[i].name;
        temp = value.length();
        memcpy(data + offset, &temp, 4);
        offset += 4;
        memcpy(data + offset, value.c_str(), temp);
        offset += temp;
        
        temp = attrs[i].type;
        memcpy(data + offset, &temp, 4);
        offset += 4;
        
        temp = i;
        memcpy(data + offset, &temp, 4);
        offset += 4;
        
        temp = attrs[i].length;
        memcpy(data + offset, &temp, 4);
        
        rc = rbfm->insertRecord(fileHandle, columnDescriptor, data, rid);
       
    }
    
    free(data);
    //TODO: find out what to do if table already exists maybe throw an error?
    
    rbfm->createFile(tableName);
    attrMap[tableName] = attrs;
    
    
    return rc;
}

RC RelationManager::deleteTable(const string &tableName)
{
    //Find the attributes for the tableName
    //Create a file Handle and open the file
    //First delete all entries from the table and column relations
    //TODO: check user flag
    RM_ScanIterator tableIterator;
    RID rid;
    vector<string> attributes;
    attributes.push_back("type");
    char * returnedData = (char *) malloc (256+4);
    char * tableNameValue = (char *) malloc (tableName.size() + 4);
    unsigned int nameSize = tableName.size();
    memcpy(tableNameValue, &nameSize, 4);
    memcpy(tableNameValue + 4, tableName.c_str(), nameSize);
    
    //Scan over each tuple that matches the table name to be deleted
    RC rc = scan("table", "tableName", EQ_OP, tableNameValue, attributes, tableIterator);
    
    vector<RID> tableDelete;
    
    while(tableIterator.getNextTuple(rid, returnedData) != RM_EOF){
        //push onto delete queue
        //Check if the table is a system table or not
        unsigned int tableType = 0;
        memcpy(&tableType, returnedData, 4);
        if(tableType == 0){
            //Exit, user calls should not allow deletion of system catalogs
            return -1;
        }
        tableDelete.push_back(rid);
    
    }
    tableIterator.close();
    
    //Do the same for column table
    RM_ScanIterator columnIterator;
    RID newid;
    vector<string> colValues;
    char * returnedCol = (char *) malloc (1500);
    rc = scan("column", "tableName", EQ_OP, tableNameValue, colValues, columnIterator);
    
    vector<RID> columnDelete;
    while (columnIterator.getNextTuple(newid, returnedCol)) {
        columnDelete.push_back(newid);
        
    }
    columnIterator.close();
    
    //Call delete on all the rids in tableDelete
    for (int i = 0; i<tableDelete.size(); i++) {
        deleteTuple("table", tableDelete[i]);
    }
    //Same on column
    for (int i = 0; i<columnDelete.size(); i++) {
        deleteTuple("column", columnDelete[i]);
    }
    //Call delete tuples...
    deleteTuples(tableName);
    
    //Remove from attrMap
    attrMap.erase(tableName);
    
    return rc;
}

RC RelationManager::getAttributes(const string &tableName, vector<Attribute> &attrs)
{
    
    //the vector will be in the attrMap, just return it
    attrs = attrMap[tableName];
    
    return 0;
}

RC RelationManager::insertTuple(const string &tableName, const void *data, RID &rid)
{
    RC rc;
    FileHandle fileHandle;
    rc = rbfm->openFile(tableName, fileHandle);
    
    //Call the rbfm insert method
    rc = rbfm->insertRecord(fileHandle, attrMap[tableName], data, rid);
    rbfm->closeFile(fileHandle);
    
    
    return rc;
}

RC RelationManager::deleteTuples(const string &tableName)
{ 
    RC rc;
    FileHandle fileHandle;
    rbfm->openFile(tableName, fileHandle);
    
    //Call the rbfm insert method
    rc = rbfm->deleteRecords(fileHandle);
    rbfm->closeFile(fileHandle);
    
    
    return rc;
    
}

RC RelationManager::deleteTuple(const string &tableName, const RID &rid)
{
    RC rc;
    FileHandle fileHandle;
    rbfm->openFile(tableName, fileHandle);
    
    //Call the rbfm insert method
    rc = rbfm->deleteRecord(fileHandle, attrMap[tableName], rid);
    rbfm->closeFile(fileHandle);
    
    
    return rc;
}

RC RelationManager::updateTuple(const string &tableName, const void *data, const RID &rid)
{
    
    
    RC rc;
    FileHandle fileHandle;
    rbfm->openFile(tableName, fileHandle);
    
    //Call the rbfm insert method
    rc = rbfm->updateRecord(fileHandle, attrMap[tableName], data, rid);
    rbfm->closeFile(fileHandle);
    
    
    return rc;
}

RC RelationManager::readTuple(const string &tableName, const RID &rid, void *data)
{
    RC rc;
    
    FileHandle fileHandle;
    rc = rbfm->openFile(tableName, fileHandle);
    
    //Call the rbfm insert method
    rc = rbfm->readRecord(fileHandle, attrMap[tableName], rid, data);
    rbfm->closeFile(fileHandle);
    
    
    return rc;
}

RC RelationManager::readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data)
{
    RC rc;
    FileHandle fileHandle;
    rbfm->openFile(tableName, fileHandle);
    
    //Call the rbfm insert method
    rc = rbfm->readAttribute(fileHandle, attrMap[tableName], rid, attributeName, data);
    rbfm->closeFile(fileHandle);
    
    
    return rc;
}

RC RelationManager::reorganizePage(const string &tableName, const unsigned pageNumber)
{
    RC rc;
    FileHandle fileHandle;
    rbfm->openFile(tableName, fileHandle);
    
    //Call the rbfm insert method
    rc = rbfm->reorganizePage(fileHandle, attrMap[tableName], pageNumber);
    rbfm->closeFile(fileHandle);
    
    
    return rc;
}

RC RM_ScanIterator::getNextTuple(RID &rid, void *data) {
    
    bool match = false;
    //Check if there is another record
    while (hasNext()) {
        RID origID;
        //First resolve the address using resolve address and pass in forwards map
        rbfm->resolveAddress(fileHandle, curid, forwards);
        
        
        //while the curid is listed in the map
        while (forwards[curid.pageNum][curid.slotNum] == true) {
            //Increment slotID
            curid.slotNum++;
            //if hasNext
            if (hasNext()) {
                //loop around
            }
            //else return RM_EOF
            else{
                return RM_EOF;
            }
            //Loop exits when it finds a record that hasn't been seen before or if it hits end of file
            
        }
        //Now resolve address again in case curID was updated
        curid = rbfm->resolveAddress(fileHandle, curid, forwards);
        
        if (compOp == NO_OP) {
            match = true;
        }
        
        else{
            //Read in the attribute
            
            //Allocate enough space...
            char * returnedValue = (char *) malloc(4096);
            
            //If type is int
            if (condDescriptor.type == TypeInt) {
                rbfm->readAttribute(fileHandle, recordDescriptor, curid, condDescriptor.name, returnedValue);
                int checkAgainst;
                memcpy(&checkAgainst, returnedValue, 4);
                if (compOp == EQ_OP) {
                    //
                    if (checkAgainst == compInt) {
                        match = true;
                    }
                }
                else if (compOp == LT_OP){
                    if(checkAgainst < compInt){
                        match = true;
                    }
                    
                }
                else if(compOp == GT_OP){
                    if(checkAgainst > compInt){
                        match = true;
                    }
                    
                }
                else if(compOp == LE_OP){
                    if(checkAgainst <= compInt){
                        match = true;
                    }
                    
                }
                else if (compOp == GE_OP){
                    if(checkAgainst >= compInt){
                        match = true;
                    }
                    
                }
                else if(compOp == NE_OP){
                    if(checkAgainst != compInt){
                        match = true;
                    }
                    
                }
            }
            
            
            //If type is real
            //Compare with compop
            else if (condDescriptor.type == TypeReal) {
                rbfm->readAttribute(fileHandle, recordDescriptor, curid, condDescriptor.name, returnedValue);
                float checkAgainst;
                memcpy(&checkAgainst, returnedValue, 4);
                if (compOp == EQ_OP) {
                    //
                    if (checkAgainst == compFloat) {
                        match = true;
                    }
                }
                else if (compOp == LT_OP){
                    if(checkAgainst < compFloat){
                        match = true;
                    }
                    
                }
                else if(compOp == GT_OP){
                    if(checkAgainst > compFloat){
                        match = true;
                    }
                    
                }
                else if(compOp == LE_OP){
                    if(checkAgainst <= compFloat){
                        match = true;
                    }
                    
                }
                else if (compOp == GE_OP){
                    if(checkAgainst >= compFloat){
                        match = true;
                    }
                    
                }
                else if(compOp == NE_OP){
                    if(checkAgainst != compFloat){
                        match = true;
                    }
                    
                }
            }
            
            //If type is varchar
            //Compare with stringcpr
            else if (condDescriptor.type == TypeVarChar) {
                rbfm->readAttribute(fileHandle, recordDescriptor, curid, condDescriptor.name, returnedValue);
                unsigned int varSize;
                memcpy(&varSize, returnedValue, 4);
                char * checkAgain = (char *) malloc(varSize);
                memcpy(checkAgain, returnedValue + 4, varSize);
                //checkAgain[varSize] = NULL;
                //string checkAgainst (checkAgain);
               // memcpy(&checkAgainst, returnedValue, 4);
                if (compOp == EQ_OP) {
                    //
                    if (strcmp(returnedValue, value) == 0) {
                        match = true;
                    }
                }
                else if (compOp == LT_OP){
                    if (strcmp(returnedValue, value) == 0) {
                        match = true;
                    }
                    
                }
                else if(compOp == GT_OP){
                    if (strcmp(returnedValue, value) == 0) {
                        match = true;
                    }
                    
                }
                else if(compOp == LE_OP){
                    if (strcmp(returnedValue, value) == 0) {
                        match = true;
                    }
                    
                }
                else if (compOp == GE_OP){
                    if (strcmp(returnedValue, value) == 0) {
                        match = true;
                    }
                    
                }
                else if(compOp == NE_OP){
                    if (strcmp(returnedValue, value) == 0) {
                        match = true;
                    }
                    
                }
            }
            
        
        }
        
        
        
        //If match = true
        if(match){
        //Enter preparation of data to send back
            //Go through each attribute and prepare big enough return value collector
            unsigned int size = 0;
            for (int i = 0; i<projAttr.size(); i++) {
                size += projAttr[i].length;
            }
            char * collector = (char *) malloc (4096);
            unsigned int offset = 0;
            //retrive value for each attribute
            for (int i = 0; i<projAttr.size(); i++) {
                char * tempStore = (char *) malloc (projAttr[i].length + 4);
                //char * tempData = (char *) malloc (4096);
                //rbfm->readRecord(fileHandle, recordDescriptor, curid, tempData);
                //rbfm->printRecord(recordDescriptor, tempData);
                rbfm->readAttribute(fileHandle, recordDescriptor, curid, projAttr[i].name, tempStore);
                //Add to collector
                if(projAttr[i].type == TypeInt || projAttr[i].type == TypeReal){
                    memcpy(collector + offset, tempStore, 4);
                    offset += 4;
                }
                else{
                    unsigned int tempStoreSize;
                    memcpy(&tempStoreSize, tempStore, 4);
                    memcpy(collector + offset, tempStore, tempStoreSize + 4); //plus 4 for size of length of string
                    offset += tempStoreSize + 4;
                }
                
                
                free(tempStore);
            }
            memcpy(data, collector, offset);
            
            
            rid.pageNum = curid.pageNum;
            rid.slotNum = curid.slotNum;
            curid.slotNum++;
            return 0;
        //Update the rid with curid
        //increment curid
        //return 0;
        }
        
        //Original id from before the forward resolution
        
        curid.slotNum++;
        //Increment the curid slotnum
    
        
    }
    
    
    
        return RM_EOF;
    

}

RC RelationManager::scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,
      const void *value,
      const vector<string> &attributeNames,
      RM_ScanIterator &rm_ScanIterator)
{
    RC rc;
    rm_ScanIterator.tableName = tableName;
    rm_ScanIterator.compOp = compOp;
    rm_ScanIterator.attributeNames = attributeNames;
    rm_ScanIterator.rbfm = rbfm;
    rm_ScanIterator.recordDescriptor = attrMap[tableName];
    rc = rbfm->openFile(tableName, rm_ScanIterator.fileHandle);
    for (int i = 0; i<attributeNames.size(); i++) {
        for(std::vector<Attribute>::const_iterator it = attrMap[tableName].begin(); it!=attrMap[tableName].end(); it++){
            if(it->name.compare(attributeNames[i])==0){
                //Add this attribute to projAttr vector
                rm_ScanIterator.projAttr.push_back(*it);
                break;
            }
        }
    }
    
    //If compOp != NO_OP
    //Get the attribute type from the map
    //Allocate space in rmsi's value accordingly
    //copy over value
    if (compOp != NO_OP) {
        rm_ScanIterator.valueAllocated = true;
        Attribute descriptor;
            //Check type of attribute by searching recordDescriptor for condition attribute
            for(std::vector<Attribute>::const_iterator it = attrMap[tableName].begin(); it!=attrMap[tableName].end(); it++){
                if (it->name.compare(conditionAttribute) == 0) {
                    //Check what type the attribute is
                    if (it->type == TypeInt) {
                        rm_ScanIterator.value = (char *) malloc (sizeof(int));
                        memcpy(rm_ScanIterator.value, value, sizeof(int));
                        memcpy(&rm_ScanIterator.compInt, value, sizeof(int));
                        rm_ScanIterator.condType = TypeInt;
                        descriptor.name = conditionAttribute;
                        descriptor.type = TypeInt;
                        descriptor.length = 4;
                        rm_ScanIterator.condDescriptor = descriptor;
                        
                        
                        break;
                    }
                    else if(it->type == TypeReal){
                        rm_ScanIterator.value = (char *) malloc (sizeof(int));
                        memcpy(rm_ScanIterator.value, value, sizeof(int));
                        memcpy(&rm_ScanIterator.compFloat, value, sizeof(int));
                        rm_ScanIterator.condType = TypeReal;
                        descriptor.name = conditionAttribute;
                        descriptor.type = TypeReal;
                        descriptor.length = 4;
                        rm_ScanIterator.condDescriptor = descriptor;
                        
                        break;
                    }
                    else{
                        //Get size of value
                        unsigned int sizeVar = 0;
                        memcpy(&sizeVar, value, sizeof(int));
                        //Malloc that much space plus four in condValue
                        rm_ScanIterator.value = (char *) malloc (4 + sizeVar);
                        memcpy(rm_ScanIterator.value, value, sizeVar + 4);
                        char * temp = (char *) malloc (sizeVar + 1);
                        memcpy(temp, value, sizeVar);
                        temp[sizeVar] = NULL;
                        string tempPlace (temp);
                        rm_ScanIterator.compString = tempPlace;
                        rm_ScanIterator.condType = TypeVarChar;
                        descriptor.name = it->name;
                        descriptor.type = it->type;
                        descriptor.length = it->length;
                        rm_ScanIterator.condDescriptor = descriptor;
                        
                        break;
                    }
                    
                }
                
            }
            

        
    }
    
    
    //Read in first Pages
    rbfm->createDPAGE(rm_ScanIterator.fileHandle, 0, rm_ScanIterator.dir);
    //set next Dir num
    rm_ScanIterator.nextDir = rm_ScanIterator.dir.nextDPage;
    rm_ScanIterator.curDir = 0;
    //If the number of pages is 0 set nextPage = 0
    if (rm_ScanIterator.dir.numOfPages == 0) {
        rm_ScanIterator.curPage = 0;
        rm_ScanIterator.pageAllocated = false;
    }
    else{
        rm_ScanIterator.curPage = 1;
        RID id;
        id.pageNum = 1;
        id.slotNum = 0;
        rm_ScanIterator.curid = id;
        //Read in the rec page
        rm_ScanIterator.rec = rbfm->createRPAGE(rm_ScanIterator.fileHandle, 1);
        rm_ScanIterator.pageAllocated = true;
        
    }
    
    
    
    return 0;
}


void RelationManager::parseCatalogs(){
    
    
    RM_ScanIterator tableIterator;
    RID rid;
    vector<string> attributes;
    attributes.push_back("tableName");
    char * returnedData = (char *) malloc (4096);
    
    //Scan over each tuple in table table and get each table's name
    RC rc = scan("table", "", NO_OP, NULL, attributes, tableIterator);
   
    
    while(tableIterator.getNextTuple(rid, returnedData) != RM_EOF){
        unsigned int nameSize;
        memcpy(&nameSize, returnedData, 4);
        char * cntTable = (char *) malloc (nameSize +4);
        memcpy(cntTable, returnedData, nameSize + 4);
        //cntTable[nameSize] = NULL; //write a null terminator at the end
        char * tableToMap = (char *) malloc (nameSize +1);
        memcpy(tableToMap, returnedData + 4, nameSize);
        tableToMap[nameSize] = NULL;
        string cntTableName (tableToMap);
        if (cntTableName.compare("tbl_employee")) {
            int f = 5;
        }
        
        //Scan over column file using condition tablename = one from above
        RM_ScanIterator columnIterator;
        RID newid;
        vector<string> colValues;
        colValues.push_back("colName");
        colValues.push_back("colType");
        colValues.push_back("colPos");
        colValues.push_back("maxSize");
        char * returnedCol = (char *) malloc (4096);
        rc = scan("column", "tableName", EQ_OP, cntTable, colValues, columnIterator);

        vector<Attribute> toReturn;
        while (columnIterator.getNextTuple(newid, returnedCol)!=RM_EOF) {
            //Parse the data into attributes and add to the attrMap
            Attribute attr;
            
            //Read in the Attribute name
            unsigned int offset = 0;
            unsigned int colNameSize = 0;
            memcpy(&colNameSize, returnedCol, 4);
            offset += 4;
            char * colName = (char *) malloc (colNameSize + 1); //Plus one for null terminator
            memcpy(colName, returnedCol + offset, colNameSize);
            colName[colNameSize] = NULL;
            offset += colNameSize;
            
            string place (colName);
            attr.name = place;
            free(colName);
            
            //Read in colType
            unsigned int colType = 0;
            memcpy(&colType, returnedCol + offset, 4);
            offset += 4;
            
            attr.type = (AttrType)colType;
            
            //Read in colPos
            
            unsigned int colPos = 0;
            memcpy(&colPos, returnedCol + offset, 4);
            offset += 4;
            
            //Read in max Size
            unsigned int maxSize = 0;
            memcpy(&maxSize, returnedCol + offset, 4);
            
            attr.length = maxSize;
            
            //Push onto the attrVector in colPos
            
            toReturn.push_back(attr);
          
            
        }
        
        columnIterator.close();
        //TODO: QUESTION why is destructor called automatigically here?!
        //push onto attrMap
        attrMap[cntTableName] = toReturn;
        
        //Free mallocs
        free(cntTable);
        free(tableToMap);
        free(returnedCol);
            
    }
    
    
    
    unsigned int run = 0;
    tableIterator.close();
    
    free(returnedData);
    
    
}


// Extra credit
RC RelationManager::dropAttribute(const string &tableName, const string &attributeName)
{
    return -1;
}

// Extra credit
RC RelationManager::addAttribute(const string &tableName, const Attribute &attr)
{
    return -1;
}

// Extra credit
RC RelationManager::reorganizeTable(const string &tableName)
{
    return -1;
}
bool RelationManager::FileExists(string fileName)
{
    ifstream ifile(fileName.c_str());
    if(ifile)
        return true;
    else return false;
}
