#include "shell.h"
#include "shcmd.h"

INT32 OsShellCmdHello(INT32 argc, const CHAR **argv)
{
    PRINTK("Hello world from 张铭 2023192004\r\n");
    return 1;
}