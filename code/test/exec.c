#include"syscall.h"
int main()
{
    int pid = Exec("../test/halt.noff");
    Join(pid);
    Exit();
}