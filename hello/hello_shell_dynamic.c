#include "shell.h"
#include "shcmd.h"
int cmd_test(void)
{
    printf("hello cmd test!\n");
}

int register_user_test_cmd()
{
    OsShellInit();
    osCmdReg(CMD_TYPE_EX,"test",0,(CMD_CBK_FUNC)cmd_test);
}