
#ifndef _rm_h_
#define _rm_h_

#include <string>
#include <vector>
#include <map>

#include "../rbf/rbfm.h"

using namespace std;


# define RM_EOF (-1)  // end of a scan operator

// RM_ScanIterator is an iteratr to go through tuples
// The way to use it is like the following:
//  RM_ScanIterator rmScanIterator;
//  rm.open(..., rmScanIterator);
//  while (rmScanIterator(rid, data) != RM_EOF) {
//    process the data;
//  }
//  rmScanIterator.close();

class RM_ScanIterator {
public:
  RM_ScanIterator() {
      forwards.clear();
  };
  ~RM_ScanIterator() {
      /*free (dir.data);
      free (rec.data);
      if(valueAllocated){
      free (value);
      }*/
  };
    bool valueAllocated = false;
    FileHandle fileHandle;
     string tableName;
    vector<Attribute> recordDescriptor;
     CompOp compOp;
    unsigned int compInt;
    float compFloat;
    string compString;
    char * value;
    vector<string> attributeNames;
    RecordBasedFileManager *rbfm;
    AttrType condType;
    Attribute condDescriptor;
    vector<Attribute> projAttr;
    
    bool pageAllocated;
    DPAGE dir;
    RPAGE rec;
    unsigned int curDir;
    unsigned int curPage;
    unsigned int nextDir;
    unsigned int nextPage;
    RID curid;
  
  // "data" follows the same format as RelationManager::insertTuple()
  RC getNextTuple(RID &rid, void *data);

  RC close() {
      
      free(dir.data);
      
      if(pageAllocated){
          free(rec.data);
      }
      rbfm->closeFile(fileHandle);
      
      return 0; };

private:
    //vector<vector<int>> forwards;//This will keep track of forwarded records
    map<unsigned int, map<unsigned int, bool>> forwards;
    
    bool hasNext(){
        //If the curPage is 0 means there are no pages, return false
        if(curPage == 0){
            return false;
        }
        bool searching = true;
        
        //Check the current ID to see if its valid, forwards are ok
        //1. check if the slot is valid
        while(curid.slotNum < rec.numSlots){
            
            if(rec.arraySlotOffset[curid.slotNum] < 4096 && rec.arraySlotOffset[curid.slotNum] != 0){
                //Check if the record is a valid address, just make sure it doesn't have 'D' in first byte
                char * check = (char *) malloc (1);
                memcpy(check, rec.data + rec.arraySlotOffset[curid.slotNum], 1);
                if(check[0] != 'D' && check[0] != 'X'){
                    //The curid points to a valid address, getnext will pick it up
                    return true;
                }
                
            }
            //Increment the slot
            curid.slotNum++;
           
           
        }
        //If it reaches here that means there are no more slots on this page
        //Start checking other pages on the directory
        //increment the pageNum and reset the slot
        curid.pageNum++;
        curid.slotNum = 0;
        
        while (curid.pageNum <= dir.numOfPages ) {
            //Read in the newPage
            free(rec.data);
            rec = rbfm->createRPAGE(fileHandle, curid.pageNum);
            while(curid.slotNum < rec.numSlots){
                
                if(rec.arraySlotOffset[curid.slotNum] < 4096 && rec.arraySlotOffset[curid.slotNum] != 0){
                    //Check if the record is a valid address, just make sure it doesn't have 'D' in first byte
                    char * check = (char *) malloc (1);
                    memcpy(check, rec.data + rec.arraySlotOffset[curid.slotNum], 1);
                    if(check[0] != 'D' && check[0] != 'X'){
                        //The curid points to a valid address, getnext will pick it up
                        return true;
                    }
                    
                }
                //Increment the slot
                curid.slotNum++;
            }
            //Increment the page number and reset slot
            curid.pageNum++;
            curid.slotNum = 0;
            
        }
        
        //If it reaches here that means no pages on this directory have a valid slot
        //Check the other directory pages
        while (nextDir != 0) {
            //Free this dir and load the next one
            free (dir.data);
            rbfm->createDPAGE(fileHandle, nextDir, dir);
            //Save this directory's pageno
            unsigned int start = nextDir;
            nextDir = dir.nextDPage;
            //Reset slot and page counters
            if(dir.numOfPages != 0)
            {
                //Import first page and start searching, reset slot and page number
                curid.pageNum = start + 1;
                curid.slotNum = 0;
                while (curid.pageNum <= start + dir.numOfPages ) {
                    //Read in the newPage
                    free(rec.data);
                    rec = rbfm->createRPAGE(fileHandle, curid.pageNum);
                    while(curid.slotNum < rec.numSlots){
                        
                        if(rec.arraySlotOffset[curid.slotNum] < 4096 && rec.arraySlotOffset[curid.slotNum] != 0){
                            //Check if the record is a valid address, just make sure it doesn't have 'D' in first byte
                            char * check = (char *) malloc (1);
                            memcpy(check, rec.data + rec.arraySlotOffset[curid.slotNum], 1);
                            if(check[0] != 'D' && check[0] != 'X'){
                                //The curid points to a valid address, getnext will pick it up
                                return true;
                            }
                            
                        }
                        //Increment the slot
                        curid.slotNum++;
                    }
                }
            }
            
        }
        //If the code above didn't find a record then there are none left
        return false;
 
    }
   
    };


// Relation Manager
class RelationManager
{
public:
  static RelationManager* instance();

  RC createTable(const string &tableName, const vector<Attribute> &attrs);

  RC deleteTable(const string &tableName);

  RC getAttributes(const string &tableName, vector<Attribute> &attrs);

  RC insertTuple(const string &tableName, const void *data, RID &rid);

  RC deleteTuples(const string &tableName);

  RC deleteTuple(const string &tableName, const RID &rid);

  // Assume the rid does not change after update
  RC updateTuple(const string &tableName, const void *data, const RID &rid);

  RC readTuple(const string &tableName, const RID &rid, void *data);

  RC readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data);

  RC reorganizePage(const string &tableName, const unsigned pageNumber);

  // scan returns an iterator to allow the caller to go through the results one by one. 
  RC scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,                  // comparision type such as "<" and "=="
      const void *value,                    // used in the comparison
      const vector<string> &attributeNames, // a list of projected attributes
      RM_ScanIterator &rm_ScanIterator);


// Extra credit
public:
  RC dropAttribute(const string &tableName, const string &attributeName);

  RC addAttribute(const string &tableName, const Attribute &attr);

  RC reorganizeTable(const string &tableName);



protected:
  RelationManager();
  ~RelationManager();

private:
    RecordBasedFileManager *rbfm;
    //RC getFileHandle(tableName, fileHandle);
  static RelationManager *_rm;
    bool FileExists(string fileName);
    std::vector<Attribute> tableDescriptor;
    std::vector<Attribute> columnDescriptor;
    void parseCatalogs();
    map <string, vector<Attribute>> attrMap;
    
};

#endif
