#include"syscall.h"
int main()
{
    int pid = Exec("yield.noff");
    Join(pid);
    Exit(0);
}