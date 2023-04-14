#include <sel4/sel4.h>

typedef void (*thread_fn)(void *, void *, void *);

/// @brief 创建一个 sel4 线程
/// @param entry_point 线程的入口点
/// @param arg0 线程第一个参数
/// @param arg1 第二个参数
/// @param arg2 第三个参数
/// @param thread_stack 一个数组，作为线程的运行栈
/// @param stack_size 数组的大小
/// @return 线程的 tcb 的 cap
seL4_CPtr sel4_create_thread(thread_fn entry_point, void * arg0, void * arg1, void * ipc_buf, void * thread_stack, unsigned long stack_size);