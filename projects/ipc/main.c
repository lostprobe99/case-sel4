#include <stdio.h>
#include <sel4/sel4.h>
#include <sel4platsupport/bootinfo.h>
#include <sel4utils/helpers.h>
#include <sel4tutorials/alloc.h>
#include <osutil/sel4thread.h>

#define BADGE 1

#define THREAD_2_STACK_SIZE 512
static uint64_t thread_2_stack[THREAD_2_STACK_SIZE];

void thread_2(void * arg0, void * arg1, void * arg2)
{
    printf("thread: hello\n");
}

int main(int argc, char ** argv)
{
    seL4_CPtr ep = alloc_object(info, seL4_EndpointObject, seL4_EndpointBits);

    seL4_CPtr tcb = sel4_create_thread(thread_2, 0, 0, 0, thread_2_stack, sizeof(thread_2_stack));
    printf("new thread tcb = %#x\n", tcb);

    return 0;
}