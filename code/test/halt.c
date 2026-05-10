#include "syscall.h"

int main()
{
    char *msg = "Halt program start...\n";

    /* 参数解释：
       1. 缓冲区地址 (msg)
       2. 字符长度 (22, 包含换行)
       3. 文件句柄 (1 代表 ConsoleOutput，即打印到屏幕)
    */
    Write(msg, 22, 1);

    Halt();

    /* 永远不会执行到这里 */
    return 0;
}