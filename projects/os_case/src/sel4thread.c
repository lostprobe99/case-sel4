#include "osutil/sel4thread.h"
#include <sel4platsupport/bootinfo.h>   // for platsupport_get_bootinfo
#include <sel4tutorials/alloc.h>    // for alloc_object
#include <utils/zf_log_if.h>    // for ZF_LOGF_IF
#include <sel4utils/helpers.h>

typedef unsigned long uintptr_t;

seL4_CPtr sel4_create_thread(thread_fn entry_point, void * arg0, void * arg1, void * arg2, void * thread_stack, unsigned long stack_size)
{
    seL4_Error error = seL4_NoError;
    seL4_BootInfo *info = platsupport_get_bootinfo();
    seL4_CPtr tcb = alloc_object(info, seL4_TCBObject, seL4_TCBBits);

    error = seL4_TCB_Configure(tcb, seL4_CapNull,
                               seL4_CapInitThreadCNode, seL4_NilData,
                               seL4_CapInitThreadVSpace, seL4_NilData,
                               0, 0);
    ZF_LOGF_IF(error != seL4_NoError, "Failed to configure tcb");

    error = seL4_TCB_SetPriority(tcb, seL4_CapInitThreadTCB, 255);
    ZF_LOGF_IF(error != seL4_NoError, "Failed to set priority");

    seL4_UserContext regs = {0};
    seL4_Word num_regs = sizeof(seL4_UserContext) / sizeof(seL4_Word);
    // 检查栈是否16字节对齐
    const int stack_alignment_requirement = sizeof(seL4_Word) * 2;
    uintptr_t thread_stack_top = (uintptr_t)thread_stack + stack_size;
    ZF_LOGF_IF(thread_stack_top % (stack_alignment_requirement) != 0,
               "Stack top isn't aligned correctly to a %dB boundary.\n",
               stack_alignment_requirement);

    sel4utils_arch_init_local_context(entry_point,	// 线程入口点
					arg0, arg1, arg2,   //  三个参数
					thread_stack_top, &regs); // 栈顶指针和寄存器

    error = seL4_TCB_WriteRegisters(tcb, 0, 0, num_regs, &regs);
    ZF_LOGF_IF(error != seL4_NoError, "Failed to write registers");

    error = seL4_TCB_Resume(tcb);
    ZF_LOGF_IF(error != seL4_NoError, "Failed to resume tcb");

    return tcb;
}