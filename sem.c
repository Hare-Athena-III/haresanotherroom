#include <stdio.h>
#include "los_task.h"
#include "los_sem.h"
#include "cmsis_os2.h"
#include "shell.h"
#include "shcmd.h"

UINT32 sem_hand;
void Task1(void){
    UINT32 ret = 0;
    int num=0;
    printf("task1 start ok\n");
        ret = LOS_SemPend(sem_hand,10);
        if (ret != LOS_OK){
            printf("Task1 LOS_SemPend fail %d\n",ret);
            ret = LOS_SemPend(sem_hand,LOS_WAIT_FOREVER);
            if (ret != LOS_OK){
                printf("task1 LOS_SemPend fail %d\n",ret);
            }
            printf("task1 LOS_SemPend ok \n");
        }
}

void Task2(void){
    UINT32 ret = 0;
    printf("task1 start ok\n");
        ret = LOS_SemPend(sem_hand,LOS_WAIT_FOREVER);
        if (ret == LOS_OK){
            printf("task2 LOS_SemPend ok\n");
        }
        printf("task2 LOS_SemPend ok \n");
        LOS_TaskDelay(20);
        LOS_SemPost(sem_hand);
        printf("task2 LOS_SemPost ok\n");
    
}

void Task3(void){
    UINT32 ret = 0;
    int num=0;
    printf("task3 start ok \n");
    ret = LOS_SemPend(sem_hand,10);
    if(ret != LOS_OK){
        printf("task3 LOS_SemPend fail %d\n",ret);
        ret = LOS_SemPend(sem_hand,LOS_WAIT_FOREVER);
        if(ret != LOS_OK){
            printf("task3 LOS_SemPend fail%d\n",ret);
        }
        printf("task3 LOS_SemPend OK\n");
    }
    LOS_TaskDelay(2000);
    LOS_SemPost(sem_hand);
    printf("task3 LOS_SemPost OK\n");
}

void task_sem_demo(){
    uint32_t ret,Id1,Id2;
    TSK_INIT_PARAM_S initParam;

    ret = LOS_SemCreate(2,&sem_hand);
    if(ret != LOS_OK){
        printf("LOS_SemCreate fail %d\n",ret);
        return ;
    }
    ret = LOS_SemPend(sem_hand,10);
        if (ret == LOS_OK){
            printf("task_sem_demo LOS_SemPend \n");
        }
    LOS_TaskLock();

    printf("LOS_TaskLock Success\n");
    memset_s(&initParam,sizeof(TSK_INIT_PARAM_S),0,sizeof(TSK_INIT_PARAM_S));
    initParam.pfnTaskEntry = (TSK_ENTRY_FUNC)Task1;
    initParam.usTaskPrio = 25;
    initParam.pcName = "Task1";
    initParam.uwStackSize = 1028*16;

    ret = LOS_TaskCreate(&Id1,&initParam);
    if(ret != LOS_OK){
        printf("LOS_TaskCreate create failed!\n");
        return ;
    }
    printf("LOS_TaskCreate Task1 create Success!\n");
    initParam.pfnTaskEntry = (TSK_ENTRY_FUNC)Task2;
    initParam.usTaskPrio = 20;
    initParam.pcName = "Task2";
    initParam.uwStackSize = 1028*16;

    ret = LOS_TaskCreate(&Id2,&initParam);
    if(ret != LOS_OK){
        LOS_TaskUnlock();
        printf("LOS_TaskCreate create failed!\n");
        return ;
    }
    printf("LOS_TaskCreate Task2 create Success!\n");
    initParam.pfnTaskEntry = (TSK_ENTRY_FUNC)Task3;
    initParam.usTaskPrio = 20;
    initParam.pcName = "Task3";
    initParam.uwStackSize = 1028*16;

    ret = LOS_TaskCreate(&Id2,&initParam);
    if(ret != LOS_OK){
        LOS_TaskUnlock();
        printf("LOS_TaskCreate create failed!\n");
        return ;
    }
    printf("LOS_TaskCreate Task3 create Success!\n");
    LOS_TaskUnlock();
    LOS_TaskDelay(400);
    ret = LOS_SemDelete(sem_hand);
    if(ret != LOS_OK){
        LOS_TaskDelay(2000);
        printf("LOS_SemDelete fail!\n");
        ret = LOS_SemDelete(sem_hand);
        if(ret != LOS_OK){
            printf("LOS_SemDelete fail2!\n");
            return ;
        }
    }
    printf("LOS_SemDelete Success!\n");
    return ;
}
int register_user_test_cmd(){
    OsShellInit();
    osCmdReg(CMD_TYPE_EX,"sem_test",0,(CMD_CBK_FUNC)task_sem_demo);
}
