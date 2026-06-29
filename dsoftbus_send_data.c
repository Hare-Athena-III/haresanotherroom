#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "securec.h"
#include "shell.h"
#include "shcmd.h"
#include "los_task.h"

#define STUDENT_ID "2023192004 张铭"

#define TYPE_SEND_BYTE        15
#define TYPE_SEND_MESSAGE     16
#define SLEEP_TIME            15
#define TRANS_UINIT_SIZE      1024
#define TRANS_SIZE_NUM        2
#define NETWORKIDSIZE         100

typedef enum {
    TYPE_BYTES = 1,
    TYPE_MESSAGE = 2
} DataType;

typedef struct {
    int dataType;
} SessionAttribute;

typedef struct {
    char networkId[NETWORKIDSIZE];
    int deviceTypeId;
    char deviceName[64];
} NodeBasicInfo;

typedef struct {
    int events;
    void (*onNodeOnline)(const NodeBasicInfo *info);
    void (*onNodeOffline)(const NodeBasicInfo *info);
} INodeStateCb;

#define EVENT_NODE_STATE_ONLINE   0x01
#define EVENT_NODE_STATE_OFFLINE  0x02

typedef enum {
    CONNECTION_ADDR_ETH = 1,
} ConnectionAddrType;

typedef struct {
    int type;
} ConnectionAddr;

typedef struct {
    int (*OnSessionOpened)(int sessionId, int result);
    void (*OnSessionClosed)(int sessionId);
    void (*OnBytesReceived)(int sessionId, const void *data, unsigned int len);
    void (*OnMessageReceived)(int sessionId, const void *data, unsigned int len);
} ISessionListener;

#define SOFTBUS_OK   0
#define SOFTBUS_ERR  -1
#define EOK          0

static void Start(void);
static void Wait(void);

static void OnNodeOnline(const NodeBasicInfo *info);
static void OnNodeOffline(const NodeBasicInfo *info);

static void OnJoinLNNDone(int retCode, const char *networkId, void *data);
static void OnLeaveLNNDone(int retCode, void *data);

static int RegNodeDeviceStateCb(const char *pkgName, INodeStateCb *callback);
static int JoinLNN(const char *pkgName, ConnectionAddr *addr,
                   void (*onJoinLNNDone)(int, const char *, void *), void *data);
static int JoinNetwork(void);
static int GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info);
static int UnregNodeDeviceStateCb(INodeStateCb *callback);
static int LeaveLNN(const char *networkId,
                    void (*onLeaveLNNDone)(int, void *), void *data);
static int LeaveNetwork(void);

static int OnSessionOpened(int sessionId, int result);
static void OnSessionClosed(int sessionId);
static void OnBytesReceived(int sessionId, const void *data, unsigned int len);
static void OnMessageReceived(int sessionId, const void *data, unsigned int len);

static int CreateSessionServer(const char *pkgName, const char *sessionName,
                                ISessionListener *listener);
static int OpenSession(const char *mySessionName, const char *peerSessionName,
                       const char *peerNetworkId, const char *groupId,
                       SessionAttribute *attr);
static int CreateSsAndOpenSession(const char *peerNetworkId);
static int RemoveSessionServer(const char *pkgName, const char *sessionName);
static int RemoveSession(void);

static int SendBytes(int sessionId, const void *data, unsigned int len);
static int SendMessage(int sessionId, const void *data, unsigned int len);
static int DataSend(int size, int type);

static int SoftbusDemoCmd(int argc, char **argv);

static const char *g_pkgName = "com.huawei.communication.demo";
static char g_networkId[NETWORKIDSIZE] = "local_001";
static int g_sessionId = 0;
static char *g_contcx = NULL;
static int g_testCount = 0;
const char *g_sessionName = "com.huawei.ctrlbustest.JtCreateSessionServerLimit";
const char *g_groupId = "TEST_GROUP_ID";
static bool g_state = true;

static SessionAttribute g_sessionAttr = {
    .dataType = TYPE_BYTES
};

static void Start(void)
{
    g_state = true;
}

static void Wait(void)
{
    printf("[%s] Wait enter\n", STUDENT_ID);
SLEEP:
    LOS_TaskDelay(100);
    if (g_state == false) {
        goto SLEEP;
    }
    printf("[%s] Wait end\n", STUDENT_ID);
    g_state = false;
}

static void OnNodeOnline(const NodeBasicInfo *info)
{
    printf("[%s] Node is On Line\n", STUDENT_ID);
    return;
}

static void OnNodeOffline(const NodeBasicInfo *info)
{
    printf("[%s] Node is Off Line\n", STUDENT_ID);
    return;
}

static INodeStateCb g_nodeStateCallback = {
    .events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE,
    .onNodeOnline = OnNodeOnline,
    .onNodeOffline = OnNodeOffline,
};

static void OnJoinLNNDone(int retCode, const char *networkId, void *data)
{
    printf("[%s] Join LNN done, retCode: %d, networkId: %s\n",
           STUDENT_ID, retCode, networkId);
    if (retCode == 0 && networkId != NULL) {
        strncpy_s(g_networkId, NETWORKIDSIZE, networkId, NETWORKIDSIZE - 1);
    }
    Start();
    return;
}

static void OnLeaveLNNDone(int retCode, void *data)
{
    printf("[%s] Leave LNN done, retCode: %d\n", STUDENT_ID, retCode);
    Start();
    return;
}

static int RegNodeDeviceStateCb(const char *pkgName, INodeStateCb *callback)
{
    printf("[%s] RegNodeDeviceStateCb: register node state callback\n", STUDENT_ID);
    return SOFTBUS_OK;
}

static int JoinLNN(const char *pkgName, ConnectionAddr *addr,
                   void (*onJoinLNNDone)(int, const char *, void *), void *data)
{
    printf("[%s] JoinLNN: join LAN network via ETH\n", STUDENT_ID);
    LOS_TaskDelay(50);
    onJoinLNNDone(0, "demo_network_123456", data);
    return SOFTBUS_OK;
}

static int JoinNetwork(void)
{
    Wait();
    printf("[%s] [test] enter JoinNetwork\n", STUDENT_ID);

    if (RegNodeDeviceStateCb(g_pkgName, &g_nodeStateCallback) != 0) {
        printf("[%s] [test] RegNodeDeviceStateCb error!\n", STUDENT_ID);
        return -1;
    }

    ConnectionAddr addr = {
        .type = CONNECTION_ADDR_ETH,
    };
    if (JoinLNN(g_pkgName, &addr, OnJoinLNNDone, NULL) != 0) {
        printf("[%s] [test] JoinLNN error!\n", STUDENT_ID);
        return -1;
    }
    return 0;
}

static int GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info)
{
    strncpy_s(info->networkId, NETWORKIDSIZE, g_networkId, NETWORKIDSIZE - 1);
    info->deviceTypeId = 100;
    strncpy_s(info->deviceName, 64, "QEMU_Demo_Device", 63);
    return SOFTBUS_OK;
}

static int UnregNodeDeviceStateCb(INodeStateCb *callback)
{
    printf("[%s] UnregNodeDeviceStateCb: unregister callback\n", STUDENT_ID);
    return SOFTBUS_OK;
}

static int LeaveLNN(const char *networkId,
                    void (*onLeaveLNNDone)(int, void *), void *data)
{
    printf("[%s] LeaveLNN: leave LAN network\n", STUDENT_ID);
    LOS_TaskDelay(50);
    onLeaveLNNDone(0, data);
    return SOFTBUS_OK;
}

static int LeaveNetwork(void)
{
    Wait();
    NodeBasicInfo info1;
    int ret = GetLocalNodeDeviceInfo(g_pkgName, &info1);
    if (ret != 0) {
        printf("[%s] GetLocalNodeDeviceInfo error!\n", STUDENT_ID);
        return -1;
    }
    printf("[%s] Local device: networkId=%s, type=%d, name=%s\n",
           STUDENT_ID, info1.networkId, info1.deviceTypeId, info1.deviceName);

    if (UnregNodeDeviceStateCb(&g_nodeStateCallback) != 0) {
        printf("[%s] UnregNodeDeviceStateCb error!\n", STUDENT_ID);
        return -1;
    }

    if (LeaveLNN(info1.networkId, OnLeaveLNNDone, NULL) != 0) {
        printf("[%s] LeaveLNN error!\n", STUDENT_ID);
        return -1;
    }
    return 0;
}

static int OnSessionOpened(int sessionId, int result)
{
    printf("[%s] session opened, session id = %d, result = %d\n",
           STUDENT_ID, sessionId, result);
    g_sessionId = sessionId;
    Start();
    return 0;
}

static void OnSessionClosed(int sessionId)
{
    printf("[%s] session closed, session id = %d\n", STUDENT_ID, sessionId);
}

static void OnBytesReceived(int sessionId, const void *data, unsigned int len)
{
    printf("[%s] bytes received, session id = %d, data: %s\n",
           STUDENT_ID, sessionId, (char *)data);
    Start();
}

static void OnMessageReceived(int sessionId, const void *data, unsigned int len)
{
    printf("[%s] message received, session id = %d\n", STUDENT_ID, sessionId);
    Start();
}

static ISessionListener g_sessionListener = {
    .OnSessionOpened = OnSessionOpened,
    .OnSessionClosed = OnSessionClosed,
    .OnBytesReceived = OnBytesReceived,
    .OnMessageReceived = OnMessageReceived,
};

static int CreateSessionServer(const char *pkgName, const char *sessionName,
                                ISessionListener *listener)
{
    printf("[%s] CreateSessionServer: create session server %s\n", STUDENT_ID, sessionName);
    return SOFTBUS_OK;
}

static int OpenSession(const char *mySessionName, const char *peerSessionName,
                       const char *peerNetworkId, const char *groupId,
                       SessionAttribute *attr)
{
    printf("[%s] OpenSession: connect to peer %s\n", STUDENT_ID, peerNetworkId);
    LOS_TaskDelay(50);
    OnSessionOpened(1, 0);
    return SOFTBUS_OK;
}

static int CreateSsAndOpenSession(const char *peerNetworkId)
{
    int ret;
    Wait();
    g_testCount = 0;
    printf("[%s] enter CreateSessionServer\n", STUDENT_ID);

    ret = CreateSessionServer(g_pkgName, g_sessionName, &g_sessionListener);
    if (ret != 0) {
        printf("[%s] CreateSessionServer failed, ret=%d\n", STUDENT_ID, ret);
        return ret;
    }

    printf("[%s] OpenSession to peer: %s\n", STUDENT_ID, peerNetworkId);
    ret = OpenSession(g_sessionName, g_sessionName, peerNetworkId, g_groupId, &g_sessionAttr);
    if (ret != 0) {
        printf("[%s] OpenSession failed, ret=%d\n", STUDENT_ID, ret);
        RemoveSessionServer(g_pkgName, g_sessionName);
        Start();
    }
    return ret;
}

static int RemoveSessionServer(const char *pkgName, const char *sessionName)
{
    printf("[%s] RemoveSessionServer: remove session %s\n", STUDENT_ID, sessionName);
    return SOFTBUS_OK;
}

static int RemoveSession(void)
{
    int ret;
    Wait();
    ret = RemoveSessionServer(g_pkgName, g_sessionName);
    printf("[%s] RemoveSessionServer done, ret=%d\n", STUDENT_ID, ret);
    Start();
    return ret;
}

static int SendBytes(int sessionId, const void *data, unsigned int len)
{
    printf("[%s] SendBytes: send %u bytes via session %d\n", STUDENT_ID, len, sessionId);
    return SOFTBUS_OK;
}

static int SendMessage(int sessionId, const void *data, unsigned int len)
{
    printf("[%s] SendMessage: send %u bytes message via session %d\n", STUDENT_ID, len, sessionId);
    return SOFTBUS_OK;
}

static int DataSend(int size, int type)
{
    int ret;
    g_contcx = (char *)calloc(1, size * sizeof(char));
    if (g_contcx == NULL) {
        printf("[%s] calloc memory failed\n", STUDENT_ID);
        return SOFTBUS_ERR;
    }
    if (memset_s(g_contcx, size, 'h', size) != EOK) {
        free(g_contcx);
        return SOFTBUS_ERR;
    }

    Wait();
    g_testCount++;
    if (type == TYPE_SEND_BYTE) {
        ret = SendBytes(g_sessionId, g_contcx, size);
        printf("[%s] SendBytes done, size=%d, ret=%d\n", STUDENT_ID, size, ret);
    } else if (type == TYPE_SEND_MESSAGE) {
        ret = SendMessage(g_sessionId, g_contcx, size);
        printf("[%s] SendMessage done, size=%d, ret=%d\n", STUDENT_ID, size, ret);
    } else {
        ret = -1;
    }

    free(g_contcx);
    g_contcx = NULL;
    return ret;
}

static int SoftbusDemoCmd(int argc, char **argv)
{
    int ret;
    const char *peerNetworkId = "peer_demo_654321";

    if (argc >= 2) {
        peerNetworkId = argv[1];
    }

    printf("\n[%s] ===== Distributed Softbus Demo Start =====\n", STUDENT_ID);
    printf("[%s] Target peer networkId: %s\n", STUDENT_ID, peerNetworkId);

    printf("[%s] SetUp begin\n", STUDENT_ID);
    ret = JoinNetwork();
    if (ret != 0) {
        printf("[%s] Join network failed\n", STUDENT_ID);
        return -1;
    }
    printf("[%s] SetUp end\n", STUDENT_ID);

    ret = CreateSsAndOpenSession(peerNetworkId);
    if (ret != 0) {
        printf("[%s] Create session failed, exit\n", STUDENT_ID);
        LeaveNetwork();
        return -1;
    }

    int dataSize = TRANS_SIZE_NUM * TRANS_UINIT_SIZE;
    DataSend(dataSize, TYPE_SEND_BYTE);
    Start();

    DataSend(1, TYPE_SEND_MESSAGE);
    Start();

    RemoveSession();

    printf("[%s] TearDown begin\n", STUDENT_ID);
    LeaveNetwork();
    printf("[%s] TearDown end\n", STUDENT_ID);

    return 0;
}

void SoftbusDemoCmdRegister(void)
{   
    OsShellInit();
    osCmdReg(CMD_TYPE_EX, "softbus_demo", 0, (CMD_CBK_FUNC)SoftbusDemoCmd);
}