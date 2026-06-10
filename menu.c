#include <stdio.h>
#include <string.h>
#include "los_event.h"
#include "los_task.h"
#include "shell.h"
#include "shcmd.h"

#define MENU_EVENT_DISH_1  0x00000001U
#define MENU_EVENT_DISH_2  0x00000002U
#define MENU_EVENT_DISH_3  0x00000004U
#define MENU_EVENT_DISH_4  0x00000008U
#define MENU_EVENT_DISH_5  0x00000010U
#define MENU_EVENT_DISH_6  0x00000020U
#define MENU_EVENT_DISH_7  0x00000040U

#define MENU_CHEF_1_MASK (MENU_EVENT_DISH_1 | MENU_EVENT_DISH_2 | MENU_EVENT_DISH_3 | MENU_EVENT_DISH_4)
#define MENU_CHEF_2_MASK (MENU_EVENT_DISH_5 | MENU_EVENT_DISH_6 | MENU_EVENT_DISH_7)

typedef struct {
    UINT32 flag;
    const CHAR *name;
} MENU_DISH_S;

static const MENU_DISH_S g_menuDishTable[] = {
    {MENU_EVENT_DISH_1, "番茄炒蛋"},
    {MENU_EVENT_DISH_2, "清蒸鱼"},
    {MENU_EVENT_DISH_3, "红烧肉"},
    {MENU_EVENT_DISH_4, "酸辣土豆丝"},
    {MENU_EVENT_DISH_5, "青椒肉丝"},
    {MENU_EVENT_DISH_6, "麻婆豆腐"},
    {MENU_EVENT_DISH_7, "西红柿鸡蛋面"},
};

static EVENT_CB_S g_menuEvent;
static UINT32 g_menuTaskId1;
static UINT32 g_menuTaskId2;
static UINT32 g_menuReady = 0;

static void MenuPrint(void)
{
    UINT32 i;
    printf("\n===== 点菜管理系统 ===== from 2023192004 张铭\n");
    for (i = 0; i < sizeof(g_menuDishTable) / sizeof(g_menuDishTable[0]); i++) {
        printf("  %u. %s\n", i + 1, g_menuDishTable[i].name);
    }
    printf("使用方法: menu <1-7>\n");
    printf("========================\n");
}

static void MenuServeDish(UINT32 chefId, UINT32 event)
{
    UINT32 i;

    for (i = 0; i < sizeof(g_menuDishTable) / sizeof(g_menuDishTable[0]); i++) {
        if ((event & g_menuDishTable[i].flag) != 0U) {
            printf("[厨师%u] 正在上菜: %s\n", chefId, g_menuDishTable[i].name);
            LOS_TaskDelay(10);
            printf("[厨师%u] 已上菜: %s\n", chefId, g_menuDishTable[i].name);
        }
    }
}

static void ChefTask1(void)
{
    UINT32 event;

    while (1) {
       event = LOS_EventRead(
            &g_menuEvent,
            MENU_CHEF_1_MASK,
            LOS_WAITMODE_OR | LOS_WAITMODE_CLR,
            LOS_WAIT_FOREVER);
        if ((event & MENU_CHEF_1_MASK) != 0U) {
            MenuServeDish(1, event & MENU_CHEF_1_MASK);
        }
    }
}

static void ChefTask2(void)
{
    UINT32 event;

    while (1) {
        event = LOS_EventRead(
            &g_menuEvent,
            MENU_CHEF_2_MASK,
            LOS_WAITMODE_OR | LOS_WAITMODE_CLR,
            LOS_WAIT_FOREVER);
        if ((event & MENU_CHEF_2_MASK) != 0U) {
            MenuServeDish(2, event & MENU_CHEF_2_MASK);
        }
    }
}

static UINT32 MenuInit(void)
{
    UINT32 ret;
    TSK_INIT_PARAM_S taskent;

    if (g_menuReady != 0U) {
        return LOS_OK;
    }

    ret = LOS_EventInit(&g_menuEvent);
    if (ret != LOS_OK) {
        printf("LOS_EventInit failed, ret=%u\n", ret);
        return ret;
    }

    memset_s(&taskent, sizeof(TSK_INIT_PARAM_S), 0, sizeof(TSK_INIT_PARAM_S));
    taskent.pfnTaskEntry = (TSK_ENTRY_FUNC)ChefTask1;
    taskent.pcName = "ChefTask1";
    taskent.uwStackSize = 1024 * 4;
    taskent.usTaskPrio = 25;
    ret = LOS_TaskCreate(&g_menuTaskId1, &taskent);
    if (ret != LOS_OK) {
        printf("Create ChefTask1 failed, ret=%u\n", ret);
        return ret;
    }

    taskent.pfnTaskEntry = (TSK_ENTRY_FUNC)ChefTask2;
    taskent.pcName = "ChefTask2";
    taskent.uwStackSize = 1024 * 4;
    taskent.usTaskPrio = 25;
    ret = LOS_TaskCreate(&g_menuTaskId2, &taskent);
    if (ret != LOS_OK) {
        printf("Create ChefTask2 failed, ret=%u\n", ret);
        return ret;
    }

    g_menuReady = 1U;
    printf("点菜管理系统初始化完成，2位厨师已就绪。\n");
    return LOS_OK;
}

UINT32 MenuCmdEntry(UINT32 argc, const CHAR **argv)
{
    UINT32 index;
    UINT32 ret;
    if (argc == 0U) {
    MenuPrint();
    return LOS_OK;
}

    if (argc != 1U) {
    printf("参数错误，请输入一个菜品编号。\n");
    return LOS_NOK;
}

    index = (UINT32)strtoul(argv[0], NULL, 10);
    if ((index < 1U) || (index > 7U)) {
        printf("编号无效，请输入 1~7。\n");
        MenuPrint();
        return LOS_NOK;
    }

    ret = MenuInit();
    if (ret != LOS_OK) {
        printf("菜单系统初始化失败，ret=%u\n", ret);
        return ret;
    }

    ret = LOS_EventWrite(&g_menuEvent, g_menuDishTable[index - 1U].flag);
    if (ret != LOS_OK) {
        printf("点单失败，ret=%u\n", ret);
        return ret;
    }

    printf("已点单: %u. %s\n", index, g_menuDishTable[index - 1U].name);
    printf("厨师正在处理该订单。\n");
    return LOS_OK;
}

int register_user_test_cmd(void)
{
    OsShellInit();
    osCmdReg(CMD_TYPE_EX, "menu", 1, (CMD_CBK_FUNC)MenuCmdEntry);
    Menuprint();
    return LOS_OK;
}
