#include <stdio.h>
#include <sel4/sel4.h>
#include <sel4platsupport/bootinfo.h>
#include <sel4tutorials/alloc.h>
#include <sel4utils/irq_server.h>
#include <usb/usb.h>

int main()
{
    seL4_Error error = seL4_NoError;
    seL4_BootInfo * info = platsupport_get_bootinfo();

    usb_t host;
    ps_io_ops_t psops;
    ps_mutex_ops_t sync;
    usb_init(USB_HOST_DEFAULT, &psops, &sync, &host);

    // seL4_CPtr kbd = alloc_slot(info);
    // error = seL4_IRQControl_Get(seL4_CapIRQControl, 0x16, seL4_CapInitThreadCNode, kbd, seL4_WordBits);
    // ZF_LOGF_IF(error, "irq control get failed");

    return 0;
}