#include "syscall.h"

int main()
{
    OpenFileId fileId;
    char buffer[20];
    int size;
    Create("testfile");
    fileId = Open("testfile");
    Write("Hello, world!\n", 14, fileId);
    size = Read(buffer, 20, fileId);
    Close(fileId);
    Exit(0);
}