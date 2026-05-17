#include <stdio.h>
#include "los_task.h"
#include "cmsis_os2.h"
#include "shell.h"
#include "shcmd.h"
uint32_t Id1,Id2;

void Thread_Hi(void){
    uint32_t ret;
    printf("Thread_Hi is running\n");

    LOS_TaskDelay(10);
    printf("Thread_Hi run LOS_TaskDelay over\n");

    ret = LOS_TaskSuspend(Id1);
    if(ret != LOS_OK){
        printf("HI LOS_TaskSuspend Failed\n");
        return ;
    }
    printf("Thread_Hi Resume Success\n");
    return ;
}

void Thread_Lo(void){
    uint32_t ret;
    printf("Thread_Lo is run\n");

    LOS_TaskDelay(10);
    printf("Thread_Lo run LOS_TaskDelay over\n");

    ret = LOS_TaskResume(Id1);
    if(ret != LOS_OK){
        printf("HI LOS_TaskResume Failed\n");
        return ;
    }
    printf("LOS_TaskResume Success\n");
    return ;
}

void TaskCaseEntry(void){
    uint32_t ret;
    TSK_INIT_PARAM_S taskent;
    LOS_TaskLock();

    printf("LOS_TaskLock Success\n");
    memset_s(&taskent,sizeof(TSK_INIT_PARAM_S),0,sizeof(TSK_INIT_PARAM_S));
    taskent.pfnTaskEntry = (TSK_ENTRY_FUNC)Thread_Hi;
    taskent.usTaskPrio = 25;
    taskent.pcName = "TaskHi";
    taskent.uwStackSize = 1028*16;

    ret = LOS_TaskCreate(&Id1,&taskent);
    if(ret != LOS_OK){
        LOS_TaskUnlock();
        printf("LOS_TaskCreate create failed!\n");
        return ;
    }
    printf("LOS_TaskCreate HI create Success!\n");
    taskent.pfnTaskEntry = (TSK_ENTRY_FUNC)Thread_Lo;
    taskent.usTaskPrio = 27;
    taskent.pcName = "TaskLo";
    taskent.uwStackSize = 1028*16;

    ret = LOS_TaskCreate(&Id2,&taskent);
    if(ret != LOS_OK){
        LOS_TaskUnlock();
        printf("LOS_TaskCreate create failed!\n");
        return ;
    }
    printf("LOS_TaskCreate LO create Success!\n");
    LOS_TaskUnlock();
    return ;
}
int register_user_test_cmd(){
    OsShellInit();
    osCmdReg(CMD_TYPE_EX,"task_test",0,(CMD_CBK_FUNC)TaskCaseEntry);
}