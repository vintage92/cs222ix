#ifndef _pfm_h_
#define _pfm_h_

typedef int RC;
typedef unsigned PageNum;


#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <sys/stat.h>
#define PAGE_SIZE 4096


class FileHandle;


class PagedFileManager
{
public:
    static PagedFileManager* instance();                     // Access to the _pf_manager instance

    RC createFile    (const char *fileName);                         // Create a new file
    RC destroyFile   (const char *fileName);                         // Destroy a file
    RC openFile      (const char *fileName, FileHandle &fileHandle); // Open a file
    RC closeFile     (FileHandle &fileHandle);                       // Close a file

protected:
    PagedFileManager();                                   // Constructor
    ~PagedFileManager();                                  // Destructor

private:
    static PagedFileManager *_pf_manager;
    std::vector<std::string> currentFiles;
    bool FileExists(const char * fileName);





    bool containsName(const std::string &target);

};


class FileHandle
{
public:
    FileHandle();                                                    // Default constructor
    ~FileHandle();                                                   // Destructor

    void setPage(FILE * fileTarget);
    void setFileName(std::string fN);

    RC readPage(PageNum pageNum, void *data);                           // Get a specific page
    RC writePage(PageNum pageNum, const void *data);                    // Write a specific page
    RC appendPage(const void *data);                                    // Append a specific page
    unsigned getNumberOfPages(); // Get the number of pages in the file

    FILE * target;
    std::string fileName;
    

    unsigned int numOfDir;
    std::vector<unsigned int> dirLocations;


};

 #endif
