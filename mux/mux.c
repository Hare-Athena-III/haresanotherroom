#include <stdio.h>
#include "los_mux.h"
#include "los_task.h"
#include "cmsis_os2.h"
#include "shell.h"
#include "shcmd.h"

UINT32 g_test_Mux;

void Example_Mux_Task2(void){
    UINT32 ret;
    printf("Example_Mux_Task2: Pend mux...\n");
    ret = LOS_MuxPend(g_test_Mux, LOS_WAIT_FOREVER);
    if(ret != LOS_OK){
        printf("Example_Mux_Task2: Pend failed, ret=%d\n", ret);
        return;
    }
    printf("Example_Mux_Task2: Got mux, doing work...\n");
    LOS_TaskDelay(100);
    LOS_MuxPost(g_test_Mux);
    printf("Example_Mux_Task2: Released mux\n");
    return;
}

void Example_Mux_Task1(void){
    UINT32 ret;
    printf("Example_Mux_Task1: Pend mux...\n");
    ret = LOS_MuxPend(g_test_Mux, LOS_WAIT_FOREVER);
    if(ret != LOS_OK){
        printf("Example_Mux_Task1: Pend failed, ret=%d\n", ret);
        return;
    }
    printf("Example_Mux_Task1: Got mux, working...\n");
    LOS_TaskDelay(50);
    LOS_MuxPost(g_test_Mux);
    printf("Example_Mux_Task1: Released mux\n");
    return;
}

UINT32 Example_EventEntry(void){
    UINT32 ret, task1ID, task2ID;
    TSK_INIT_PARAM_S taskent;

    ret = LOS_MuxCreate(&g_test_Mux);
    if(ret != LOS_OK){
        printf("LOS_MuxCreate failed, ret=%d\n", ret);
        return ret;
    }
    printf("LOS_MuxCreate OK\n");

    memset_s(&taskent, sizeof(TSK_INIT_PARAM_S), 0, sizeof(TSK_INIT_PARAM_S));
    taskent.pfnTaskEntry = (TSK_ENTRY_FUNC)Example_Mux_Task1;
    taskent.pcName = "Task1";
    taskent.uwStackSize = 1024 * 4;
    taskent.usTaskPrio = 25;
    ret = LOS_TaskCreate(&task1ID, &taskent);
    if(ret != LOS_OK){
        printf("Create Task1 failed, ret=%d\n", ret);
        return ret;
    }
    printf("Task1 created OK\n");

    taskent.pfnTaskEntry = (TSK_ENTRY_FUNC)Example_Mux_Task2;
    taskent.pcName = "Task2";
    taskent.usTaskPrio = 24;
    ret = LOS_TaskCreate(&task2ID, &taskent);
    if(ret != LOS_OK){
        printf("Create Task2 failed, ret=%d\n", ret);
        return ret;
    }
    printf("Task2 created OK\n");

    LOS_TaskDelay(200);

    ret = LOS_MuxDelete(g_test_Mux);
    if(ret != LOS_OK){
        printf("Delete mux failed, ret=%d\n", ret);
        return ret;
    }
    printf("LOS_MuxDelete OK\n");

    return LOS_OK;
}

int register_user_test_cmd(){
    OsShellInit();
    osCmdReg(CMD_TYPE_EX,"task_mux",0,(CMD_CBK_FUNC)Example_EventEntry);
}
