#include <sel4/sel4.h>

typedef void (*thread_fn)(void *, void *, void *);

/// @brief 创建一个 seL4_UserContext
/// @param entry_point 子线程的入口点
/// @param arg0 子线程参数 1
/// @param arg1 子线程参数 2
/// @param arg2 子线程参数 3
/// @param thread_stack 作为子线程运行栈的数组首地址 
/// @param stack_size 栈数组大小，用于计算栈顶地址
/// @return 
seL4_UserContext sel4_make_regs(thread_fn entry_point, void * arg0, void * arg1, void * arg2, void * thread_stack, unsigned long stack_size);