#include <stdio.h>
#include "los_task.h"
#include "los_queue.h"
#include "cmsis_os2.h"
#include "shell.h"
#include "shcmd.h"

UINT32 g_queue;

void sendqueue_task(void){
    UINT32 ret = 0;
    CHAR abuf[10] = {};
    UINT32 len = sizeof(abuf);
    int num = 0;
    while(1){
        sprintf_s(abuf,10,"%d",num++);
        printf("send :%s\n",abuf);
        len = sizeof(abuf);
        ret = LOS_QueueWriteCopy(g_queue,abuf,len,LOS_WAIT_FOREVER);
        if(ret != LOS_OK){
            printf("sendqueue_task LOS_QueueWriteCopy fail %d\n",ret);
        }
        LOS_Msleep(1000);

    }
}

void readqueue_task(void){
    UINT32 ret = 0;
    CHAR abuf[10] = {};
    UINT32 len = sizeof(abuf);

    while(1){
        ret = LOS_QueueReadCopy(g_queue,abuf,&len,LOS_WAIT_FOREVER);
        if(ret == LOS_OK){
            printf("LOS_QueueReadCopy:%s\n",abuf);
        }
    }
}


UINT32 Example_msgEntry(void){
    UINT32 ret,g_losTask_ID;
    TSK_INIT_PARAM_S taskent;

    ret = LOS_QueueCreate("Q1",8,&g_queue,0,10);
    if(ret != LOS_OK){
        printf("LOS_QueueCreate failed %d\n,ret");
        return -1;
    }
    printf("LOS_QueueCreate OK\n");
    memset_s(&taskent,sizeof(TSK_INIT_PARAM_S),0,sizeof(TSK_INIT_PARAM_S));
    taskent.pfnTaskEntry = (TSK_ENTRY_FUNC)sendqueue_task;
    taskent.pcName = "Task1";
    taskent.uwStackSize = 1024 * 4;
    taskent.usTaskPrio = 25;
    ret = LOS_TaskCreate(&g_losTask_ID,&taskent);
    if(ret != LOS_OK){
        printf("LOS_TaskCreate Example_msg failed\n");
        return -1;
    }
    printf("Example_msg1 OK\n");
    taskent.pfnTaskEntry = (TSK_ENTRY_FUNC)readqueue_task;
    taskent.pcName = "Task2";
    taskent.uwStackSize = 1024 * 4;
    taskent.usTaskPrio = 25;
    ret = LOS_TaskCreate(&g_losTask_ID,&taskent);
    if(ret != LOS_OK){
        printf("LOS_TaskCreate readqueue_task failed\n");
        return -1;
    }
    printf("readqueue_task OK\n");
}

int register_user_test_cmd(){
    OsShellInit();
    osCmdReg(CMD_TYPE_EX,"test_msg",0,(CMD_CBK_FUNC)Example_msgEntry);
}
