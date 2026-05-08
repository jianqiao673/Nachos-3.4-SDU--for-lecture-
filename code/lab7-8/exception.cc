// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "openfile.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void  AdvancePC() { 
    machine->WriteRegister(PCReg, machine->ReadRegister(PCReg) + 4); 
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4); 
} 

void StartProcess(int spaceId) 
{ 
    AddrSpace *space = AddrSpaces[spaceId];
    space->InitRegisters(); // set the initial register values 
    space->RestoreState();  // load page table register 
 
    machine->Run();   // jump to the user progam 
    ASSERT(FALSE);  // machine->Run never returns; 
           // the address space exits by doing the syscall "exit" 
}


void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2); 
    if ((which == SyscallException)) { 
        switch(type){ 
            case SC_Halt:{ 
                printf("CurrentThreadId: %d Name: %s, Execute system call of Halt() \n",(currentThread->space)->GetSpaceId(),currentThread->getName());
                DEBUG('a', "Shutdown, initiated by user program.\n"); 
                interrupt->Halt(); 
                break; 
            } 
            case SC_Exec:{  
                printf("CurrentThreadId: %d Name: %s, Execute system call of Exec() \n",(currentThread->space)->GetSpaceId(),currentThread->getName());
                //DEBUG('f',"Execute system call Exec()\n"); 
                //read argument (i.e. filename) of Exec(filename) 
                char filename[128];  
                int addr=machine->ReadRegister(4);   
                int ii=0; 
                //read filename from mainMemory 
                do{ 
                    machine->ReadMem(addr+ii,1,(int *)&filename[ii]); 
                } while (filename[ii++]!='\0');
                    
                //--------------------------------------------------------- 
                // 
                // inner commands --begin 
                // 
                //--------------------------------------------------------- 
                //printf("Call Exec(%s)\n",filename); 
                if (filename[0] == 'l' && filename[1] == 's')   //ls 
                {   
                    printf("File(s) on Nachos DISK:\n"); 
                    fileSystem->List(); 
                    machine->WriteRegister(2,127);   // 
                    AdvancePC();                
                    break;    
                }  
                else if (filename[0] == 'r' && filename[1] == 'm')  //rm file 
                {              
                    char fn[128]; 
                    strncpy(fn,filename,strlen(filename) - 5); 
                    fn[strlen(filename) - 5] = '\0'; 
                    fn[0] = ' '; 
                    fn[1] = ' '; 
                    char *file = EraseStrChar(fn,' '); 
                    if (file != NULL && strlen(file) >0) 
                    { 
                        fileSystem->Remove(file); 
                        machine->WriteRegister(2,127); 
                    } 
                    else 
                    { 
                        printf("Remove: invalid file name.\n"); 
                        machine->WriteRegister(2,-1); 
                    } 
                    AdvancePC();                
                    break;       
                } //rm 
                else if (filename[0] == 'c' && filename[1] == 'a' && filename[2] == 't')  //cat file 
                { 
                    char fn[128]; 
                    strncpy(fn,filename,strlen(filename) - 5); 
                    fn[strlen(filename) - 5] = '\0'; 
                    fn[0] = ' '; 
                    fn[1] = ' '; 
                    fn[2] = ' '; 
                    char *file = EraseStrChar(fn,' '); 
                    //printf("filename=%s, fn=%s, file=%s\n",filename,fn, file); 
                    if (file != NULL && strlen(file) >0) 
                    { 
                        Print(file); 
                        machine->WriteRegister(2,127); 
                    } 
                    else 
                    { 
                        printf("Cat: file not exists.\n"); 
                        machine->WriteRegister(2,-1); 
                    } 
                    AdvancePC();                
                    break;                       
                } 
                else if (filename[0] == 'c' && filename[1] == 'f')  //create a nachos file 
                {  //create file 
                    char fn[128]; 
                    strncpy(fn,filename,strlen(filename) - 5); 
                    fn[strlen(filename) - 5] = '\0'; 
                    //printf("filename=%s, fn=%s\n",filename,fn); 
                    fn[0] = ' '; 
                    fn[1] = ' '; 
                    
                    char *file = EraseStrChar(fn,' '); 
                    if (file != NULL && strlen(file) >0) 
                    { 
                        fileSystem->Create(file,0); 
                        machine->WriteRegister(2,127); 
                    } 
                    else 
                    { 
                        printf("Create: file already exists.\n"); 
                        machine->WriteRegister(2,-1); 
                    } 
                    AdvancePC();                
                    break; 
                } 
                else if (filename[0] == 'c' && filename[1] == 'p')   //cp source dest 
                {
                    char fn[128]; 
                    strncpy(fn,filename,strlen(filename) - 5); 
                    fn[strlen(filename) - 5] = '\0'; 
                    fn[0] = '#'; 
                    fn[1] = '#'; 
                    fn[2] = '#'; 
                    char source[128]; 
                    char dest[128]; 
                    int startPos = 3; 
                    int j = 0; 
                    int k = 0; 
                    for (int i = startPos; i < 128 /*, fn[i] != '\0'*/;i++) 
                        if (fn[i] != ' ') 
                            source[j++] = fn[i]; 
                        else  
                            break; 
                    source[j] = '\0'; 
                    j++;  
                    //printf("j = %d\n",j); 
                    
                    for (int i = startPos + j; i < 128 /*,fn[i] != '\0'*/;i++) 
                        if (fn[i] != ' ') 
                            dest[k++] = fn[i]; 
                        else  
                            break;            
                    dest[k] = '\0'; 
                    
                    if (source == NULL || strlen(source) <= 0) 
                    { 
                        printf("cp: Source file not exists.\n"); 
                        machine->WriteRegister(2,-1); 
                        AdvancePC();                
                        break;        
                    } 
                    if (dest != NULL && strlen(dest) > 0) 
                    { 
                        NAppend(source, dest); 
                        machine->WriteRegister(2,127); 
                    } 
                    else 
                    { 
                        printf("cp: Missing dest file.\n"); 
                        machine->WriteRegister(2,-1); 
                    } 
                    AdvancePC();                
                    break;  
                    }
                //uap source(Unix) dest(nachos) 
                else if ((filename[0] == 'u' && filename[1] == 'a' && filename[2] == 'p')   
                            || (filename[0] == 'u' && filename[1] == 'c' && filename[2] == 'p'))    
                { 
                    char fn[128]; 
                    strncpy(fn,filename,strlen(filename) - 5); 
                    fn[strlen(filename) - 5] = '\0'; 
                    fn[0] = '#'; 
                    fn[1] = '#'; 
                    fn[2] = '#'; 
                    fn[3] = '#'; 
                    char source[128]; 
                    char dest[128]; 
                    int startPos = 4; 
                    int j = 0; 
                    int k = 0; 
                    for (int i = startPos; i < 128/*, fn[i] != '\0'*/;i++) 
                        if (fn[i] != ' ') 
                            source[j++] = fn[i]; 
                        else  
                            break; 
                    source[j] = '\0'; 
                    j++;  
                            
                    for (int i = startPos + j; i < 128/*,fn[i] != '\0'*/;i++) 
                        if (fn[i] != ' ') 
                        dest[k++] = fn[i]; 
                    else  
                        break;            
                    dest[k] = '\0'; 
                    
                    if (source == NULL || strlen(source) <= 0) 
                    { 
                        printf("uap or ucp: Source file not exists.\n"); 
                        machine->WriteRegister(2,-1); 
                        AdvancePC();                
                        break;        
                    } 
                    if (dest != NULL && strlen(dest) > 0) 
                    { 
                        if (filename[0] == 'u' && filename[1] == 'c' && filename[2] == 'p') 
                            Append(source, dest,0); //append dest file at the end of source file 
                        else 
                            Copy(source, dest);    //ucp 
                        machine->WriteRegister(2,127); 
                    } 
                    else 
                    { 
                        printf("uap or ucp: Missing dest file.\n"); 
                        machine->WriteRegister(2,-1); 
                    } 
                    AdvancePC();                
                    break;                       
                } 
                //nap source dest    
                else if (filename[0] == 'n' && filename[1] == 'a' && filename[2] == 'p') 
                { 
                    char fn[128]; 
                    strncpy(fn,filename,strlen(filename) - 5); 
                    fn[strlen(filename) - 5] = '\0'; 
                    fn[0] = '#'; 
                    fn[1] = '#'; 
                    fn[2] = '#'; 
                    fn[3] = '#'; 
                    //char *file = EraseStrSpace(fn, ' '); 
                    char source[128]; 
                    char dest[128]; 
                    int j = 0; 
                    int k = 0; 
                    int startPos = 4; 
                    for (int i = startPos; i < 128/*, fn[i] != '\0'*/;i++) 
                        if (fn[i] != ' ') 
                        source[j++] = fn[i]; 
                        else  
                        break; 
                    source[j] = '\0'; 
                    j++;  
                    //printf("j = %d\n",j); 
                    
                    for (int i = startPos + j; i < 128/*,fn[i] != '\0'*/;i++) 
                        if (fn[i] != ' ') 
                            dest[k++] = fn[i]; 
                        else  
                        break;            
                        dest[k] = '\0'; 
                            
                    if (source == NULL || strlen(source) <= 0) 
                    { 
                        printf("nap: Source file not exists.\n"); 
                        machine->WriteRegister(2,-1); 
                        AdvancePC();                
                        break;        
                    } 
                    if (dest != NULL && strlen(dest) > 0) 
                    { 
                        NAppend(source, dest); 
                        machine->WriteRegister(2,127); 
                    } 
                    else 
                    { 
                        printf("nap: Missing dest file.\n"); 
                        machine->WriteRegister(2,-1); 
                    } 
                    AdvancePC();                
                    break;                       
                } 
                //rename source dest 
                else if (filename[0] == 'r' && filename[1] == 'e' && filename[2] == 'n') 
                { 
                    char fn[128]; 
                    strncpy(fn,filename,strlen(filename) - 5); 
                    fn[strlen(filename) - 5] = '\0'; 
                    fn[0] = '#'; 
                    fn[1] = '#'; 
                    fn[2] = '#'; 
                    fn[3] = '#'; 
                    char source[128]; 
                    char dest[128]; 
                    int j = 0; 
                    int k = 0; 
                    int startPos = 4; 
                    for (int i = startPos; i < 128/*, fn[i] != '\0'*/;i++) 
                    if (fn[i] != ' ') 
                        source[j++] = fn[i]; 
                        else  
                        break; 
                    source[j] = '\0'; 
                    j++;  
                    //printf("j = %d\n",j); 
                    
                    for (int i = startPos + j; i < 128/*,fn[i] != '\0'*/;i++) 
                        if (fn[i] != ' ') 
                        dest[k++] = fn[i]; 
                    else  
                        break;            
                    dest[k] = '\0'; 
                    
                    if (source == NULL || strlen(source) <= 0) 
                    { 
                        printf("rename: Source file not exists.\n"); 
                        machine->WriteRegister(2,-1); 
                        AdvancePC();                
                        break;        
                    } 
                    if (dest != NULL && strlen(dest) > 0) 
                    { 
                        fileSystem->Rename(source, dest); 
                        machine->WriteRegister(2,127); 
                    } 
                    else 
                    { 
                        printf("rename: Missing dest file.\n"); 
                                machine->WriteRegister(2,-1); 
                    } 
                    AdvancePC();                
                    break;                       
                }      
                else if (strstr(filename,"format") != NULL)   //format 
                {   
                    printf("strstr(filename,\"format\"=%s \n",strstr(filename,"format")); 
                    printf("WARNING: Format Nachos DISK will erase all the data on it.\n"); 
                    printf("Do you want to continue (y/n)? "); 
                    char ch; 
                    while (true) 
                    { 
                        ch = getchar(); 
                        if (toupper(ch) == 'Y' || toupper(ch) == 'N') 
                        break; 
                    } //while 
                    if (toupper(ch) == 'N') 
                    { 
                        machine->WriteRegister(2,127);   // 
                        AdvancePC();                
                        break;    
                    } 
                        
                    printf("Format the DISK and create a file system on it.\n"); 
                    fileSystem->FormatDisk(true); 
                    machine->WriteRegister(2,127);   // 
                    AdvancePC();                
                    break;    
                } 
                else if (strstr(filename,"fdisk") != NULL)   //fdisk 
                {                      
                    fileSystem->Print(); 
                    machine->WriteRegister(2,127);   // 
                    AdvancePC();                
                    break;    
                }  
                else if (strstr(filename,"perf") != NULL)   //Performance 
                {                      
                    PerformanceTest(); 
                    machine->WriteRegister(2,127);   // 
                    AdvancePC();                
                    break;    
                }  
                else if (filename[0] == 'p' && filename[1] == 's')   //ps 
                { 
                    scheduler->PrintThreads(); 
                    machine->WriteRegister(2,127);   // 
                    AdvancePC();                
                    break;    
                } 
                else if (strstr(filename,"help") != NULL)   //fdisk 
                { 
                    printf("Commands and Usage:\n"); 
                    printf("ls                : list files on DISK.\n"); 
                    printf("fdisk             : display DISK information.\n"); 
                    printf("format            : format DISK with creating a file system on it.\n");
                    printf("performence       : test DISK performence.\n"); 
                    printf("cf  name          : create a file \"name\" on DISK.\n"); 
                    printf("cp  source dest   : copy Nachos file \"source\" to \"dest\".\n");
                    printf("nap source dest   : append Nachos file \"source\" to \"dest\".\n"); 
                    printf("ucp source dest   : copy Unix file \"source\" to Nachos file \"dest\".\n"); 
                    printf("uap source dest   : append Unix file \"source\" to Nachos file \"dest\".\n"); 
                    printf("cat name          : print content of file \"name\".\n"); 
                    printf("rm  name          : remove file \"name\".\n"); 
                    printf("rename source dest: Rename Nachos file \"source\" to \"dest\".\n"); 
                    printf("ps                : display the system threads.\n");       
                    //----------------------------------------------------------- 
                    machine->WriteRegister(2,127);   
                    AdvancePC();                
                    break;    
                } 
                else  //check if the parameter of exec(file), i.e file, is valid 
                { 
                    if (strchr(filename,' ') != NULL || strstr(filename,".noff") == NULL)   
                    //not an inner command, and not a valid Nachos executable, then return        
                    { 
                        machine->WriteRegister(2,-1);                       
                        AdvancePC();   
                        break; 
                    } 
                }
                //--------------------------------------------------------- 
                // 
                // inner commands --end 
                // 
                //--------------------------------------------------------- 
                //--------------------------------------------------------- 
                // 
                // loading an executable and execute it 
                // 
                //--------------------------------------------------------- 
                //call open() in FILESYS, see filesys.h 
                OpenFile* executable = fileSystem->Open(filename);
                if (executable == NULL) { 
                    //printf("\nUnable to open file %s\n", filename); 
                    DEBUG('f',"\nUnable to open file %s\n", filename); 
                    machine->WriteRegister(2,-1);                    
                    AdvancePC();   
                    break; 
                    //return; 
                }
                 //new address space 
                space = new AddrSpace(executable);   
                delete executable;   // close file 
                DEBUG('H',"Execute system call Exec(\"%s\"), it's SpaceId(pid) = %d \n",filename,space->getSpaceID()); 
                //new and fork thread 
                char *forkedThreadName = filename; 
                
                //------------------------------------------------ 
                char *fname=strrchr(filename,'/'); 
                if (fname != NULL)  // there exists "/" in filename                
                forkedThreadName=strtok(fname,"/");       
                //-----------------------------------------------              
                Thread *thread = new Thread(forkedThreadName); 
                //printf("exec -- new thread pid =%d\n",space->getSpaceID()); 
                thread->Fork(StartProcess, space->getSpaceID()); 
                thread->space = space; 
                space->Print();
                printf("user process \"%s(%d)\" map to kernel thread \" %s \"\n",filename,space->getSpaceID(),forkedThreadName);
                 //return spaceID 
                machine->WriteRegister(2,space->getSpaceID()); 
                //printf("Exec()--space->getSpaceID()=%d\n",space->getSpaceID()); 
                
                //========================================================= 
                //run the new thread, 
                //otherwise, this process will not execute in order to release its memory,  
                //thread "main" may continue to create new processes, 
                //and will not have enough memory for new processes  
                
                currentThread->Yield(); 
                
                //but introduce another problem:
                // when Joiner waits for a Joinee, the joinee maybe finish before Joiner call Join(), 
                //  but Joinee go to sleep after calling Finish() 
                // 
                //============================================================ 
                //advance PC 
                AdvancePC();     
                break;
            }
            case SC_Join: { 
                printf("CurrentThreadId: %d Name: %s, Execute system call of Join() \n",(currentThread->space)->GetSpaceId(),currentThread->getName());
                int SpaceId=machine->ReadRegister(4);  //ie. ThreadId or SpaceId 
                currentThread->Join(SpaceId); 
                //返回 Joinee 的退出码waitProcessExitCode 
                machine->WriteRegister(2, currentThread->waitProcessExitCode); 
                AdvancePC(); 
                break; 
            }
            case SC_Exit: { 
                printf("CurrentThreadId: %d Name: %s, Execute system call of Exit() \n",(currentThread->space)->GetSpaceId(),currentThread->getName());
                int ExitStatus=machine->ReadRegister(4);  
                machine->WriteRegister(2,ExitStatus); 
                currentThread->setExitStatus(ExitStatus); 
                if (ExitStatus == 99)  //parent process exit, delele all terminated threads 
                {  
                List *terminatedList = scheduler->getTerminatedList(); 
                scheduler->emptyList(terminatedList);              
                }   
                delete currentThread->space; 
                currentThread->Finish();     
                AdvancePC();            
                break; 
            }
            case SC_Yield:{ 
                printf("CurrentThreadId: %d Name: %s, Execute system call of Yield() \n",(currentThread->space)->GetSpaceId(),currentThread->getName());
                currentThread->Yield();     
                AdvancePC();            
                break; 
            } 
            /* Create a Nachos file, with "name" */ 
            //void Create(char *name); 
            case SC_Create: { 
                printf("CurrentThreadId: %d Name: %s, Execute system call of Create() \n",(currentThread->space)->GetSpaceId(),currentThread->getName());
                int base=machine->ReadRegister(4); 
                int value; 
                int count=0; 
                char *FileName= new char[128]; 
                do{ 
                    machine->ReadMem(base+count,1,&value); 
                    FileName[count]=*(char*)&value; 
                    count++; 
                } while(*(char*)&value!='\0'&&count<128); 
                int fileDescriptor = OpenForWrite(FileName);     
                if (fileDescriptor == -1)  
                    printf("create file %s failed!\n",FileName); 
                else 
                printf("create file %s succeed!, the file id is %d\n",FileName,fileDescriptor); 
                
                Close(fileDescriptor);          
                    //machine->WriteRegister(2,fileDescriptor); 
                        
                AdvancePC(); 
                break;
            }
            /* Open the Nachos file "name", and return an "OpenFileId" that can  
            * be used to read and write to the file. 
            */ 
            //OpenFileId Open(char *name);  //int OpenFileId
            case SC_Open: { 
                printf("CurrentThreadId: %d Name: %s, Execute system call of Open() \n",(currentThread->space)->GetSpaceId(),currentThread->getName());
                int base=machine->ReadRegister(4); 
                int value; 
                int count=0; 
                char *FileName= new char[128]; 
            
                do{ 
                    machine->ReadMem(base+count,1,&value); 
                    FileName[count]=*(char*)&value; 
                    count++; 
                } while(*(char*)&value!='\0'&&count<128); 
            
                int fileid; 
                //call Open() in FILESYS,see filesys.h,Nachos Open() 
                OpenFile* openfile=fileSystem->Open(FileName);  
                if(openfile == NULL ) {   //file not existes, not found 
                    printf("File \"%s\" not Exists, could not open it.\n",FileName); 
                    fileid = -1; 
                } 
                else {  //file found 
                    //set the opened file id in AddrSpace, which wiil be used in Read() and Write() 
                    fileid = currentThread->space->getFileDescriptor(openfile);  
                    if (fileid < 0) 
                    printf("Too many files opened!\n"); 
                    else                
                    DEBUG('f',"file :%s open secceed!  the file id is %d\n",FileName,fileid);
                        } 
                machine->WriteRegister(2,fileid); 
                AdvancePC(); 
                break;
            }
            /* Write "size" bytes from "buffer" to the open file. */ 
            //void Write(char *buffer, int size, OpenFileId id); 
            case SC_Write: { 
                printf("CurrentThreadId: %d Name: %s, Execute system call of Write() \n",(currentThread->space)->GetSpaceId(),currentThread->getName());
                int base =machine->ReadRegister(4);  //buffer 
                int size=machine->ReadRegister(5);   //bytes written to file  
                int fileId=machine->ReadRegister(6); //fd  
                int value; 
                int count=0; 
            
                // printf("base=%d, size=%d, fileId=%d \n",base,size,fileId ); 
                OpenFile* openfile =new OpenFile (fileId); 
                ASSERT(openfile != NULL); 
                                
                char* buffer= new char[128]; 
                do{ 
                    machine->ReadMem(base+count,1,&value); 
                    buffer[count] = *(char*)&value;                     
                    count++; 
                } while((*(char*)&value!='\0') && (count<size)); 
                buffer[size]='\0'; 
            
                OpenFile*  openfile = currentThread->space->getFileId(fileId);  
                //printf("openfile =%d\n",openfile); 
                if (openfile == NULL) 
                    { 
                    printf("Failed to Open file \"%d\" .\n",fileId);                      
                    AdvancePC(); 
                    break; 
                    }    
                    
                    if (fileId ==1 || fileId ==2) 
                    { 
                    openfile->WriteStdout(buffer,size); 
                    delete [] buffer; 
                AdvancePC(); 
                break; 
                }  
                        
                int WritePosition = openfile->Length(); 
                                
                openfile->Seek(WritePosition);  //append write 
                //openfile->Seek(0);      //write form  begining 
                
                int writtenBytes; 
                //writtenBytes = openfile->AppendWriteAt(buffer,size,WritePosition); 
                writtenBytes = openfile->Write(buffer,size); 
                if((writtenBytes)==0) 
                DEBUG('f',"\nWrite file failed!\n"); 
                else 
                { 
                    if (fileId != 1 && fileId != 2) 
                    { 
                        DEBUG('f',"\n\"%s\" has wrote in file %d succeed!\n",buffer,fileId); 
                        DEBUG('H',"\n\"%s\" has wrote in file %d succeed!\n",buffer,fileId); 
                        DEBUG('J',"\n\"%s\" has wrote in file %d succeed!\n",buffer,fileId); 
                    } 
                //printf("\n\"%s\" has wrote in file %d succeed!\n",buffer,fileId);     
                } 
                
                //delete openfile; 
                delete [] buffer; 
                AdvancePC(); 
                break;
            }
            /* Read "size" bytes from the open file into "buffer".   
            * Return the number of bytes actually read -- if the open file isn't 
            * long enough, or if it is an I/O device, and there aren't enough  
            * characters to read, return whatever is available (for I/O devices,  
            * you should always wait until you can return at least one character). 
            */ 
            //int Read(char *buffer, int size, OpenFileId id); 
            case SC_Read: { 
                printf("CurrentThreadId: %d Name: %s, Execute system call of Read() \n",(currentThread->space)->GetSpaceId(),currentThread->getName());
                int base =machine->ReadRegister(4); 
                int size  = machine->ReadRegister(5); 
                int fileId=machine->ReadRegister(6); 
            
                OpenFile* openfile = currentThread->space->getFileId(fileId);         
                
                char buffer[size]; 
                int readnum=0; 
                if (fileId == 0)  //stdin 
                    readnum = openfile->ReadStdin(buffer,size); 
                else 
                    readnum = openfile->Read(buffer,size); 
                        
                for(int i = 0;i < readnum; i++) 
                    machine->WriteMem(base,1,buffer[i]); 
                buffer[readnum]='\0'; 
                
                for(int i = 0;i < readnum; i++) 
                if (buffer[i] >=0 && buffer[i] <= 9) 
                    buffer[i] = buffer[i] + 0x30;  
                
                char *buf = buffer; 
                if (readnum > 0) 
                { 
                    if (fileId != 0) 
                    { 
                        DEBUG('f',"Read file (%d) succeed! the content is \"%s\" , the length is %d\n",fileId,buf,readnum); 
                        DEBUG('H',"Read file (%d) succeed! the content is \"%s\" , the length is %d\n",fileId,buf,readnum); 
                        DEBUG('J',"Read file (%d) succeed! the content is \"%s\" , the length is %d\n",fileId,buf,readnum); 
                    }                
                } 
                else 
                    printf("\nRead file failed!\n"); 
                        
                machine->WriteRegister(2,readnum); 
                AdvancePC(); 
                break; 
            }
            /* Close the file, we're done reading and writing to it. */ 
            //void Close(OpenFileId id); 
            case SC_Close: { 
                int fileId =machine->ReadRegister(4); 
                OpenFile* openfile = currentThread->space->getFileId(fileId); 
                if (openfile != NULL) 
                { 
                    openfile->WriteBack();  // write file header back to DISK 
                    
                    delete openfile;        // close file            
                    currentThread->space->releaseFileDescriptor(fileId); 
                        
                    DEBUG('f',"File %d  closed succeed!\n",fileId); 
                    DEBUG('H',"File %d  closed succeed!\n",fileId); 
                    DEBUG('J',"File %d  closed succeed!\n",fileId); 
                } 
                else 
                    printf("Failded to Close File %d.\n",fileId);  
                AdvancePC(); 
                break; 
             }  
            /* Create a Nachos file, with "name" */ 
            //void Create(char *name); 
            case SC_Create: { 
                printf("CurrentThreadId: %d Name: %s, Execute system call of Create() \n",(currentThread->space)->GetSpaceId(),currentThread->getName());
                int base=machine->ReadRegister(4); 
                int value; 
                int count=0; 
                char *FileName= new char[128]; 
            
                do{ 
                    machine->ReadMem(base+count,1,&value); 
                    FileName[count]=*(char*)&value; 
                    count++; 
                } while(*(char*)&value!='\0'&&count<128); 
                //when calling Create(),  thread go to sleep, waked up when I/O finish 
                if(!fileSystem->Create(FileName,0))  //call Create() in FILESYS,see filesys.h 
                    printf("create file %s failed!\n",FileName); 
                else 
                    DEBUG('f',"create file %s succeed!\n",FileName);             
                AdvancePC(); 
                break;
            }
            default: 
                printf("Unexpected system call %d\n", type); 
                break;
        }
    }
}
