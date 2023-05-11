#include <stdio.h>
#include <sel4/sel4.h>
#include <sel4platsupport/bootinfo.h>
#include <sel4runtime.h>
#include <sel4runtime/gen_config.h>
#include <sel4utils/helpers.h>
#include <sel4tutorials/alloc.h>
#include <osutil/sel4thread.h>

#define str(x) #x
#define EVAL(x, s) str(x) ": " s "\n", x

#define BADGE 1
#define MSG_DATA 0x61
#define IPCBUF_VADDR 0x7000000

#define THREAD_2_STACK_SIZE 512
static uint64_t thread_2_stack[THREAD_2_STACK_SIZE];

static char tls_region[CONFIG_SEL4RUNTIME_STATIC_TLS] = {};

void check_badge(int badge)
{
    printf("thread_2: badge = %d\n", badge);
    switch(badge) {
        case 0:
            printf("thread_2: msg from `ep_object`\n");
            break;
        case 1:
            printf("thread_2: msg from `ep_cap`\n");
            break;
        case 2:
            printf("thread_2: msg from `ep_cap1`\n");
            break;
    }
}

void thread_2(void * arg0, void * arg1, void * arg2)
{
    printf("thread_2: hello\n");
    printf(EVAL(&__sel4_ipc_buffer, "%p"));
    printf(EVAL(sel4runtime_get_tls_base(), "%p"));
    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 0);
    seL4_Word sender_badge;
    seL4_Word msg;
    seL4_CPtr ep_object = (seL4_CPtr)arg0;
    // 将 IPCBUF_VADDR 强转到 seL4_IPCBuffer * 并访问其 msg 成员
    seL4_IPCBuffer* ipcbuf = (seL4_IPCBuffer*)IPCBUF_VADDR;

    // 1. ep 和 badge 测试
    tag = seL4_Recv(ep_object, &sender_badge);
    // printf(EVAL(sender_badge, "%d"));
    check_badge(sender_badge);

    tag = seL4_Recv(ep_object, &sender_badge);
    // printf(EVAL(sender_badge, "%d"));
    check_badge(sender_badge);

    tag = seL4_Recv(ep_object, &sender_badge);
    // printf(EVAL(sender_badge, "%d"));
    check_badge(sender_badge);

    // 2. seL4_Call + seL4_ReplyRecv
    tag = seL4_Recv(ep_object, &sender_badge);
    // printf("thread_2: sender_badge = %lu\n", sender_badge);
    ZF_LOGF_IF(sender_badge != BADGE, "BADGE is not expected");
    ZF_LOGF_IF(seL4_MessageInfo_get_length(tag) != 1, "length is not expected");

    msg = seL4_GetMR(0);
    printf("thread_2: got a message %#lx\n", msg);
    ZF_LOGF_IF(msg != MSG_DATA, "msg is not expected");

    // 响应 seL4_Call
    msg = ~msg;
    seL4_SetMR(0, msg);
    tag = seL4_ReplyRecv(ep_object, tag, &sender_badge);
    // seL4_Reply(tag);
    // printf("thread_2: sender_badge = %lu\n", sender_badge);
    ZF_LOGF_IF(sender_badge != BADGE, "BADGE is not expected");
    ZF_LOGF_IF(seL4_MessageInfo_get_length(tag) != 1, "length is not expected");

    msg = seL4_GetMR(0);
    printf("thread_2: got a message %#lx\n", msg);
    ZF_LOGF_IF(msg != MSG_DATA + 1, "msg is not expected");

    // 3. seL4_Call + seL4_Reply
    tag = seL4_Recv(ep_object, &sender_badge);
    // printf("thread_2[%d]: sender_badge = %lu\n", __LINE__, sender_badge);
    ZF_LOGF_IF(sender_badge != BADGE, "BADGE is not expected");
    ZF_LOGF_IF(seL4_MessageInfo_get_length(tag) != 1, "length is not expected");

    msg = seL4_GetMR(0);
    printf("thread_2: got a message %#lx\n", msg);
    ZF_LOGF_IF(msg != ~MSG_DATA, "msg is not expected");

    msg = ~msg;
    seL4_SetMR(0, msg);
    seL4_Reply(tag);

    msg = seL4_GetMR(0);
    printf("thread_2[%d]: got a message %#lx\n", __LINE__, msg);
    ZF_LOGF_IF(msg != MSG_DATA, "msg is not expected");

    // 4. 长消息测试 1
    char buf[seL4_MsgMaxLength] = {0};
    tag = seL4_Recv(ep_object, &sender_badge);
    int len = seL4_MessageInfo_get_length(tag);
    printf("thread_2: got msg len = %d\n", len);
    for(int i = 0; i < len; i++)
        buf[i] = seL4_GetMR(i);
    printf("thread_2: got: `%s`\n", buf);
    seL4_SetMR(0, 'W');
    seL4_SetMR(1, 'o');
    seL4_SetMR(2, 'r');
    seL4_SetMR(3, 'l');
    seL4_SetMR(4, 'd');
    tag = seL4_MessageInfo_new(0, 0, 0, 5);
    seL4_Send(ep_object, tag);
}

int main(int argc, char ** argv)
{
    seL4_Error error = seL4_NoError;
    seL4_BootInfo *info = platsupport_get_bootinfo();
    seL4_Word ipc_buf_vaddr = IPCBUF_VADDR;
    #if 1
    // 将物理页映射到虚拟地址
    // 1. 创建一个页表对象
    seL4_CPtr pt = alloc_object(info, seL4_X86_PageTableObject, 0);
    // 2. 将页表对象映射到 VSpace 中
    error = seL4_X86_PageTable_Map(pt, seL4_CapInitThreadVSpace, ipc_buf_vaddr, seL4_X86_Default_VMAttributes);
    ZF_LOGF_IF(error, "Failed to map page table");

    // 3. 将物理帧映射到 VSpace
    seL4_CPtr ipc_frame_cap = alloc_object(info, seL4_X86_4K, seL4_PageBits);
    error = seL4_X86_Page_Map(ipc_frame_cap, seL4_CapInitThreadVSpace, ipc_buf_vaddr, seL4_AllRights, seL4_X86_Default_VMAttributes);
    ZF_LOGF_IF(error, "Failed to map frame");
    #endif

    // 创建 EndPoint 对象
    seL4_CPtr ep_object = alloc_object(info, seL4_EndpointObject, seL4_EndpointBits);
    // 将 badge mint 到 EndPoint 对象
    seL4_CPtr ep_cap = alloc_slot(info);
    error = seL4_CNode_Mint(seL4_CapInitThreadCNode, ep_cap, seL4_WordBits, 
                            seL4_CapInitThreadCNode, ep_object, seL4_WordBits,
                            seL4_AllRights, BADGE);
    ZF_LOGF_IF(error, "Failed to mint");

    seL4_CPtr ep_cap1 = alloc_slot(info);
    error = seL4_CNode_Mint(seL4_CapInitThreadCNode, ep_cap1, seL4_WordBits, 
                            seL4_CapInitThreadCNode, ep_object, seL4_WordBits,
                            seL4_AllRights, 2);
    ZF_LOGF_IF(error, "Failed to mint");
    // 后续通过 ep_cap 调用 send/recv 函数

    seL4_CPtr tcb = alloc_object(info, seL4_TCBObject, 0);
    printf("new thread tcb = %#lx\n", tcb);
    error = seL4_TCB_Configure(tcb, seL4_CapNull,
                               seL4_CapInitThreadCNode, 0,
                               seL4_CapInitThreadPD, 0,
                               ipc_buf_vaddr, ipc_frame_cap);
                            //    0, 0);
    ZF_LOGF_IF(error, "Failed to configure");

    seL4_DebugNameThread(seL4_CapInitThreadTCB, "thread_main");
    seL4_DebugNameThread(tcb, "thread_2");

    seL4_UserContext regs = sel4_make_regs(thread_2, (void*)ep_object, 0, 0, thread_2_stack, THREAD_2_STACK_SIZE);
    seL4_Word num_regs = sizeof(seL4_UserContext) / sizeof(seL4_Word);

    error = seL4_TCB_WriteRegisters(tcb, 0, 0, num_regs, &regs);
    ZF_LOGF_IF(error, "Failed to write registers");

    #if 1
   /* create a thread local storage (TLS) region for the new thread to store the
      ipc buffer pointer */
    uintptr_t tls = sel4runtime_write_tls_image(tls_region);
    seL4_IPCBuffer *ipcbuf = (seL4_IPCBuffer*)ipc_buf_vaddr;
    // 将 ipcbuf 写入子线程的 线程局部存储区域
    error = sel4runtime_set_tls_variable(tls, __sel4_ipc_buffer, ipcbuf);
    ZF_LOGF_IF(error, "Failed to set ipc buffer in TLS of new thread");
    // 把 tls 设置为子线程的 tls_base
    error = seL4_TCB_SetTLSBase(tcb, tls);
    ZF_LOGF_IF(error, "Failed to set TLS base");
    printf(EVAL(tls, "%p"));
    printf(EVAL(__sel4_ipc_buffer, "%p"));
    uint64_t* it = tls_region;
    int llen = CONFIG_SEL4RUNTIME_STATIC_TLS / sizeof(uint64_t);
    printf(EVAL(it, "%p"));
    for(int i = 0; i < llen; i++)
        if(it[i] == ipc_buf_vaddr)
            printf("tls_region[%p]: %#lx\n", it + i, *(it + i));
    #endif

    error = seL4_TCB_Resume(tcb);
    ZF_LOGF_IFERR(error, "Failed to start new thread.\n");

    // 准备工作完成，开始通信
    seL4_Word msg = 0;
    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 1);
    // 1. ep 和 badge 测试
    // 这是三个不同的 cap，也就是三个不同的 EndPoint 对象
    printf(EVAL(ep_object, "%#lx"));
    printf(EVAL(ep_cap, "%#lx"));
    printf(EVAL(ep_cap1, "%#lx"));
    seL4_Send(ep_object, tag);
    seL4_Send(ep_cap, tag);
    seL4_Send(ep_cap1, tag);

    // 2. seL4_Call + seL4_ReplyRecv
    tag = seL4_MessageInfo_new(0, 0, 0, 1);
    seL4_SetMR(0, MSG_DATA);

    tag = seL4_Call(ep_cap, tag);

    msg = seL4_GetMR(0);
    ZF_LOGF_IF(seL4_MessageInfo_get_length(tag) != 1, "length is not expected");
    ZF_LOGF_IF(msg != ~MSG_DATA, "msg is not expected");

    printf("thread_main: got a reply %#lx\n", msg);

    seL4_SetMR(0, MSG_DATA + 1);
    seL4_Send(ep_cap, tag);

    // 3. seL4_Call + seL4_Reply
    seL4_SetMR(0, msg);

    tag = seL4_Call(ep_cap, tag);
    msg = seL4_GetMR(0);
    ZF_LOGF_IF(seL4_MessageInfo_get_length(tag) != 1, "length is not expected");
    ZF_LOGF_IF(msg != MSG_DATA, "msg is not expected");
    printf("thread_main: got a reply %#lx\n", msg);


    // 4. 长消息（长度大于4）测试 1
    seL4_SetMR(0, 0x48);
    seL4_SetMR(1, 0x65);
    seL4_SetMR(2, 0x6c);
    seL4_SetMR(3, 0x6c);
    seL4_SetMR(4, 0x6f);
    // msg = "Hello"

    tag = seL4_MessageInfo_new(0, 0, 0, 5);
    seL4_Send(ep_cap, tag);

    char buf[seL4_MsgMaxLength] = {0};
    seL4_Word sender_badge;
    tag = seL4_Recv(ep_cap, NULL);
    int len = seL4_MessageInfo_get_length(tag);
    printf("thread_main: got msg len = %d\n", len);
    for(int i = 0; i < len; i++)
        buf[i] = seL4_GetMR(i);
    printf("thread_main: got: `%s`\n", buf);


    printf("\nEnd\n");
    while(1);
    return 0;
}