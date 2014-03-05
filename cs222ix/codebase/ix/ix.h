
#ifndef _ix_h_
#define _ix_h_

#include <vector>
#include <string>

#include "../rbf/rbfm.h"
#include "node.h"

# define IX_EOF (-1)  // end of the index scan



class IX_ScanIterator;

class IndexManager {
 public:
  static IndexManager* instance();

  RC createFile(const string &fileName);


  RC destroyFile(const string &fileName);

  RC openFile(const string &fileName, FileHandle &fileHandle);

  RC closeFile(FileHandle &fileHandle);

  // The following two functions are using the following format for the passed key value.
  //  1) data is a concatenation of values of the attributes
  //  2) For int and real: use 4 bytes to store the value;
  //     For varchar: use 4 bytes to store the length of characters, then store the actual characters.
  RC insertEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid);  // Insert new index entry
  RC deleteEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid);  // Delete index entry

  // scan() returns an iterator to allow the caller to go through the results
  // one by one in the range(lowKey, highKey).
  // For the format of "lowKey" and "highKey", please see insertEntry()
  // If lowKeyInclusive (or highKeyInclusive) is true, then lowKey (or highKey)
  // should be included in the scan
  // If lowKey is null, then the range is -infinity to highKey
  // If highKey is null, then the range is lowKey to +infinity
  RC scan(FileHandle &fileHandle,
      const Attribute &attribute,
	  const void        *lowKey,
      const void        *highKey,
      bool        lowKeyInclusive,
      bool        highKeyInclusive,
      IX_ScanIterator &ix_ScanIterator);

 protected:
  IndexManager   ();                            // Constructor
  ~IndexManager  ();                            // Destructor

 private:
  static IndexManager *_index_manager;
    bool FileExists(const char * fileName);
    
    //Update
    void update(FileHandle &fileHandle, vector<unsigned int> trackList, Node &tempNode);
    bool isValid(Node &node);
    
    //Add helpers
    void addToInnerNode(Node &node, int key, unsigned int leftPtr, unsigned int rightPtr);
    void addToInnerNode(Node &node, float key, unsigned int leftPtr, unsigned int rightPtr);
    void addToInnerNode(Node &node, string key, unsigned int leftPtr, unsigned int rightPtr);
    
    //Leaf add helpers
    void addToLeafNode(Node &node, int key, RID rid, unsigned int overflow, bool matchFound, bool copy);
    void addToLeafNode(Node &node, float key, RID rid, unsigned int overflow, bool matchFound, bool copy);
    void addToLeafNode(Node &node, string key, RID rid, unsigned int overflow, bool matchFound, bool copy);
    
    
    
    //Different search methods for each type of key
    SearchResult search(string key);
    SearchResult search(int key, FileHandle &fileHandle);
    SearchResult search(float key);
    SearchResult treeSearch(string key, SearchResult sr);
    SearchResult treeSearch(int key, SearchResult sr);
    SearchResult treeSearch(float key, SearchResult sr);
    
};

class IX_ScanIterator {
 public:
  IX_ScanIterator();  							// Constructor
  ~IX_ScanIterator(); 							// Destructor

  RC getNextEntry(RID &rid, void *key);  		// Get next matching entry
  RC close();             						// Terminate index scan
};

// print out the error message for a given return code
void IX_PrintError (RC rc);


#endif
