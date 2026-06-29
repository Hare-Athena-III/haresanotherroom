#include <stdio.h>
#include <string.h>
#include "los_interrupt.h"
#include "los_task.h"
#include "shell.h"
#include "shcmd.h"

#define HWI_NUM_TEST 7

STATIC VOID HwiUsrIrq(VOID)
{
    printf("in the func HwiUsrIrq\n");
}

VOID HwiCaseEntry(VOID)
{
    UINT32 ret;
    HWI_PRIOR_T hwiPrio = 3;
    HWI_MODE_T mode = 0;
    HwiIrqParam irqParam;

    (void)memset_s(&irqParam, sizeof(HwiIrqParam), 0, sizeof(HwiIrqParam));
    irqParam.pDevId = 0;

    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, (HWI_PROC_FUNC)HwiUsrIrq, &irqParam);
    if (ret != LOS_OK) {
        printf("Hwi create fail!\n");
        return;
    }
    printf("Hwi create success!\n");

    LOS_TaskDelay(50);

    ret = LOS_HwiDelete(HWI_NUM_TEST, &irqParam);
    if (ret != LOS_OK) {
        printf("Hwi delete fail!\n");
        return;
    }
    printf("Hwi delete success!\n");
    printf("Hwi test success by 2023192004 张铭!\n");
}

int register_user_test_cmd(void)
{
    OsShellInit();
    osCmdReg(CMD_TYPE_EX, "hwi_test", 0, (CMD_CBK_FUNC)HwiCaseEntry);
    return LOS_OK;
}
