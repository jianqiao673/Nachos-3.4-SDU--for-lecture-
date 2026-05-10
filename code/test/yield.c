#include"syscall.h"
int main()
{
    int pid = Exec("halt.noff");
    //Join(pid);
    Yield();
    Exit(0);
}