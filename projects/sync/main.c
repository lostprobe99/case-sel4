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

#define write_tls(tcb, vaddr)                                                                       \
    {                                                                                                \
        static char __tls_region__[CONFIG_SEL4RUNTIME_STATIC_TLS] = {};                              \
        uintptr_t __tls__ = sel4runtime_write_tls_image(__tls_region__);                             \
        seL4_IPCBuffer *__ipcbuf__ = (seL4_IPCBuffer *)vaddr;                                        \
        seL4_Error __error__ = sel4runtime_set_tls_variable(__tls__, __sel4_ipc_buffer, __ipcbuf__); \
        ZF_LOGF_IF(__error__, "Failed to set ipc buffer in TLS of new thread");                      \
        __error__ = seL4_TCB_SetTLSBase(tcb, __tls__);                                              \
        ZF_LOGF_IF(__error__, "Failed to set TLS base");                                             \
    }

#define IPCBUF_VADDR_1 0x7000000
#define IPCBUF_VADDR_2 0x8000000

#define THREAD_STACK_SIZE 512
static uint64_t producer_1_stack[THREAD_STACK_SIZE];
static uint64_t producer_2_stack[THREAD_STACK_SIZE];

seL4_CPtr empty, full, producer;
int task, c = 0;
#define MAX_PRODUCT_NUM 50

void thread(void * arg0, void * arg1, void * arg2)
{
    seL4_Word tid = (seL4_Word)arg1;
    printf("producer[%#lx]: hello\n", tid);

    for(; c < MAX_PRODUCT_NUM; c++)
    {
        seL4_Wait(empty, NULL);
        seL4_Wait(producer, NULL);
        task = ++c;
        seL4_Signal(producer);
        printf("producer[%#lx]: awaken\n", tid);
        seL4_Signal(full);  
    }
}

seL4_CPtr mount_frame(seL4_BootInfo * info, seL4_Word vaddr)
{
    seL4_Error error = seL4_NoError;
    // 将物理页映射到虚拟地址
    // 1. 创建一个页表对象
    seL4_CPtr pt = alloc_object(info, seL4_X86_PageTableObject, 0);
    // 2. 将页表对象映射到 VSpace 中
    error = seL4_X86_PageTable_Map(pt, seL4_CapInitThreadVSpace, vaddr, seL4_X86_Default_VMAttributes);
    ZF_LOGF_IF(error, "Failed to map `%p` page table", vaddr);

    // 3. 将物理帧映射到 VSpace
    seL4_CPtr ipc_frame_cap = alloc_object(info, seL4_X86_4K, 0);
    error = seL4_X86_Page_Map(ipc_frame_cap, seL4_CapInitThreadVSpace, vaddr, seL4_AllRights, seL4_X86_Default_VMAttributes);
    ZF_LOGF_IF(error, "Failed to map frame");

    return ipc_frame_cap;
}

seL4_CPtr make_producer(seL4_BootInfo* info, seL4_CPtr ipc_frame_cap, seL4_Word ipc_buf_vaddr, uint64_t* thread_stack)
{
    seL4_Error error = seL4_NoError;
    seL4_CPtr tcb = alloc_object(info, seL4_TCBObject, 0);
    printf("new thread tcb = %#lx\n", tcb);
    error = seL4_TCB_Configure(tcb, seL4_CapNull,
                               seL4_CapInitThreadCNode, 0,
                               seL4_CapInitThreadPD, 0,
                               ipc_buf_vaddr, ipc_frame_cap);
    ZF_LOGF_IF(error, "Failed to configure");

    seL4_UserContext regs = sel4_make_regs(thread, (void*)0, (void *)tcb, 0, thread_stack, THREAD_STACK_SIZE);
    seL4_Word num_regs = sizeof(seL4_UserContext) / sizeof(seL4_Word);

    error = seL4_TCB_WriteRegisters(tcb, 0, 0, num_regs, &regs);
    ZF_LOGF_IF(error, "Failed to write registers");

    write_tls(tcb, ipc_buf_vaddr);

    return tcb;
}

int main(int argc, char ** argv)
{
    seL4_Error error = seL4_NoError;
    seL4_BootInfo *info = platsupport_get_bootinfo();
    seL4_Word ipc_buf_vaddr1 = IPCBUF_VADDR_1;
    seL4_Word ipc_buf_vaddr2 = IPCBUF_VADDR_2;

    seL4_CPtr producer_1 = make_producer(info, mount_frame(info, ipc_buf_vaddr1), ipc_buf_vaddr1, producer_1_stack);
    seL4_CPtr producer_2 = make_producer(info, mount_frame(info, ipc_buf_vaddr2), ipc_buf_vaddr2, producer_2_stack);

    seL4_DebugNameThread(seL4_CapInitThreadTCB, "thread_main");
    seL4_DebugNameThread(producer_1, "producer_1");
    seL4_DebugNameThread(producer_2, "producer_2");

    // 启动线程
    error = seL4_TCB_Resume(producer_1);
    ZF_LOGF_IFERR(error, "Failed to start producer_1.\n");
    error = seL4_TCB_Resume(producer_2);
    ZF_LOGF_IFERR(error, "Failed to start producer_2.\n");

    // 创建端点对象
    empty = alloc_object(info, seL4_NotificationObject, 0);
    full = alloc_object(info, seL4_NotificationObject, 0);
    producer = alloc_object(info, seL4_NotificationObject, 0);
    seL4_Signal(producer);

    while(1)
    {
        printf(EVAL(task, "%d"));
        seL4_Signal(empty);
        printf("consumer: Waiting for produce...\n");
        seL4_Wait(full, NULL);
        printf("consumer: Got task = %d\n", task);
    }

    return 0;
}