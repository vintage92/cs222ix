
#ifndef _rbfm_h_
#define _rbfm_h_

#include <string>
#include <vector>
#include <map>
#include <math.h>

#include "../rbf/pfm.h"

using namespace std;


// Record ID
typedef struct
{
  unsigned pageNum;
  unsigned slotNum;
    //bool valid;
} RID;

typedef struct
{
	unsigned int numOfPages;
	map <unsigned int, unsigned int> freeMap;
	map <unsigned int, unsigned int> pageToOffset;
	unsigned int freeDSpace;
	unsigned int nextDPage;
	char * data;

}DPAGE;
typedef struct
{
	unsigned int pageNum;

	unsigned int freePointerOffset;
	unsigned int conFreeSpace;//The continous free space after the freePointerOffset
	unsigned int totFreeSpace;//Includes fragmented free space after attribute,rec deletion
	unsigned int numSlots;
	vector<unsigned int> arraySlotOffset;
	char * data;
}RPAGE;


// Attribute
typedef enum { TypeInt = 0, TypeReal, TypeVarChar } AttrType;

typedef unsigned AttrLength;

struct Attribute {
    string   name;     // attribute name
    AttrType type;     // attribute type
    AttrLength length; // attribute length
};

// Comparison Operator (NOT needed for part 1 of the project)
typedef enum { EQ_OP = 0,  // =
           LT_OP,      // <
           GT_OP,      // >
           LE_OP,      // <=
           GE_OP,      // >=
           NE_OP,      // !=
           NO_OP       // no condition
} CompOp;



/****************************************************************************
The scan iterator is NOT required to be implemented for part 1 of the project 
*****************************************************************************/

# define RBFM_EOF (-1)  // end of a scan operator

// RBFM_ScanIterator is an iteratr to go through records
// The way to use it is like the following:
//  RBFM_ScanIterator rbfmScanIterator;
//  rbfm.scan(..., rbfmScanIterator);
//  while (rbfmScanIterator(rid, data) != RBFM_EOF) {
//    process the data;
//  }
//  rbfmScanIterator.close();


class RBFM_ScanIterator {
public:
  RBFM_ScanIterator() {
      
      
  
  };
    ~RBFM_ScanIterator() {
        
        free (currentPage.data);
        free (currentDir.data);
    };

  // "data" follows the same format as RecordBasedFileManager::insertRecord()
  RC getNextRecord(RID &rid, void *data);
  RC close() { return -1; };
    void setVar(FileHandle *fH, vector<Attribute> rD, string cA, CompOp cO, char * value, vector<string> aN);
    
private:
    FileHandle *fileHandle;
    vector<Attribute> recordDescriptor;
    string conditionAttribute;
    CompOp compOp;
    char * condValue;
    vector<string> attributeNames;
    AttrType condType;
    RPAGE currentPage;
    DPAGE currentDir;
    //RecordBasedFileManager* rbfm;
    
    
};


class RecordBasedFileManager
{
public:
  static RecordBasedFileManager* instance();

  RC createFile(const string &fileName);
  
  RC destroyFile(const string &fileName);
  
  RC openFile(const string &fileName, FileHandle &fileHandle);
  
  RC closeFile(FileHandle &fileHandle);

  //  Format of the data passed into the function is the following:
  //  1) data is a concatenation of values of the attributes
  //  2) For int and real: use 4 bytes to store the value;
  //     For varchar: use 4 bytes to store the length of characters, then store the actual characters.
  //  !!!The same format is used for updateRecord(), the returned data of readRecord(), and readAttribute()
  RC insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid);

  RC readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data);
  
  // This method will be mainly used for debugging/testing
  RC printRecord(const vector<Attribute> &recordDescriptor, const void *data);

/**************************************************************************************************************************************************************
***************************************************************************************************************************************************************
IMPORTANT, PLEASE READ: All methods below this comment (other than the constructor and destructor) are NOT required to be implemented for part 1 of the project
***************************************************************************************************************************************************************
***************************************************************************************************************************************************************/
    //done
  RC deleteRecords(FileHandle &fileHandle);

  RC deleteRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid);

  // Assume the rid does not change after update
  RC updateRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, const RID &rid);

  RC readAttribute(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, const string attributeName, void *data);

  RC reorganizePage(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const unsigned pageNumber);

  // scan returns an iterator to allow the caller to go through the results one by one. 
  RC scan(FileHandle &fileHandle,
      const vector<Attribute> &recordDescriptor,
      const string &conditionAttribute,
      const CompOp compOp,                  // comparision type such as "<" and "="
      const void *value,                    // used in the comparison
      const vector<string> &attributeNames, // a list of projected attributes
      RBFM_ScanIterator &rbfm_ScanIterator);


// Extra credit for part 2 of the project, please ignore for part 1 of the project
public:

  RC reorganizeFile(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor);
    void createDPAGE(FileHandle &fileHandle, unsigned int pageNumber, DPAGE &directory);
    RPAGE createRPAGE(FileHandle &fileHandle, unsigned int pageNumber);
    RID resolveAddress(FileHandle &fileHandle, RID rid);
    RID resolveAddress(FileHandle &fileHandle, RID rid, map<unsigned int, map<unsigned int, bool>> &forwards);
    void updateDPAGE(FileHandle &fileHandle, unsigned int pageNum, unsigned int freeSpace);
protected:
  RecordBasedFileManager();
  ~RecordBasedFileManager();

private:
  static RecordBasedFileManager *_rbf_manager;
  PagedFileManager *pfm;
  //create private methods for reading and writing page structs
  
    


};

#endif
