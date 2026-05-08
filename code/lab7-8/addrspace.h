// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch 
    void Print();
    int GetspaceID();
    #ifdef USER_PROGRAM
    OpenFile* fileDescriptor[10]; //每个地址空间最多打开10个文件，fileDescriptor[i]为第i个文件的OpenFile指针
    int getFileDescriptor(OpenFile * openfile); //返回openfile在当前地址空间的文件描述符，如果没有则返回-1
    OpenFile* getFileId(int fd); //返回当前地址空间中fd对应的OpenFile指针，如果没有则返回NULL
    void releaseFileDescriptor(int fd);//释放当前地址空间中fd对应的文件描述符
    #endif
  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
    int spaceID;                // Address space ID
    
};

#endif // ADDRSPACE_H
