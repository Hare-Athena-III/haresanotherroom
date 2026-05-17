#include <stdio.h>
#include "los_task.h"
#include "cmsis_os2.h"
#include "los_event.h"

UINT32 g_losTask_ID;

EVENT_CB_S g_exampleEvent;

#define EVENT_WAIT 0x0001

void Example_Event(void){
    UINT32 event;
    printf("Event write event\n");
    event = LOS_EventWrite(&g_exampleEvent,EVENT_WAIT);
    if(event != LOS_OK){
        printf("LOS_EventWrite failed\n");
    }
    printf("Event write ok\n");
}


void Example_task(void){
    UINT32 event;
    printf("Example_task event\n");
    event = LOS_EventRead(&g_exampleEvent,EVENT_WAIT,LOS_WAITMODE_AND,LOS_WAIT_FOREVER);
    if(event = EVENT_WAIT){
        printf("LOS_EventRead failed\n");
    }
    printf("Event read ok\n");
}

UINT32 Example_EventEntry(void){
    UINT32 ret;
    TSK_INIT_PARAM_S taskent;

    ret = LOS_EventInit(&g_exampleEvent);
    if(ret != LOS_OK){
        printf("LOS_EventInit failed\n");
        return -1;
    }
    printf("g_exampleEvent OK\n");
    memset_s(&taskent,sizeof(TSK_INIT_PARAM_S),0,sizeof(TSK_INIT_PARAM_S));
    taskent.pfnTaskEntry = (TSK_ENTRY_FUNC)Example_Event;
    taskent.pcName = "EvenTask1";
    taskent.uwStackSize = 1024 * 4;
    taskent.usTaskPrio = 25;
    ret = LOS_TaskCreate(&g_losTask_ID,&taskent);
    if(ret != LOS_OK){
        printf("LOS_TaskCreate Example_Event failed\n");
        return -1;
    }
    printf("Example_Event OK\n");
    taskent.pfnTaskEntry = (TSK_ENTRY_FUNC)Example_Event;
    taskent.pcName = "EvenTask2";
    taskent.uwStackSize = 1024 * 4;
    taskent.usTaskPrio = 25;
    ret = LOS_TaskCreate(&g_losTask_ID,&taskent);
    if(ret != LOS_OK){
        printf("LOS_TaskCreate Example_Event failed\n");
        return -1;
    }
    printf("Example_Event OK\n");
}
