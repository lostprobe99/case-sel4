#include <sel4/sel4.h>
#include <stdio.h>
#include <sel4platsupport/bootinfo.h>
#include <sel4runtime.h>
#include <sel4runtime/gen_config.h>
#include <stdlib.h>
#include <sel4tutorials/alloc.h>
#include <sel4utils/helpers.h>
#include <sel4/arch/mapping.h>

// #define VADDR_1 0x7000000
#define VADDR_1 0x0001000
#define VADDR_2 0x7001000

char * sel4_mapping_failed_lookup_level_str[40] = {
    [SEL4_MAPPING_LOOKUP_LEVEL] = "SEL4_MAPPING_LOOKUP_LEVEL",
    [SEL4_MAPPING_LOOKUP_NO_PT] = "SEL4_MAPPING_LOOKUP_NO_PT",
    [SEL4_MAPPING_LOOKUP_NO_PD] = "SEL4_MAPPING_LOOKUP_NO_PD",
    [SEL4_MAPPING_LOOKUP_NO_PDPT] = "SEL4_MAPPING_LOOKUP_NO_PDPT",
};

int main()
{
    seL4_Word vaddr = VADDR_1;
    seL4_Error error = seL4_NoError;
    seL4_Word fll;
    seL4_BootInfo * info = platsupport_get_bootinfo();
    printf("initThreadCNode = %d\n", info->initThreadCNodeSizeBits);
    // 将物理页映射到虚拟地址
    // 1. 创建一个页表对象
    seL4_CPtr pt = alloc_object(info, seL4_X86_PageTableObject, 0);
    // 2. 将页表对象映射到 VSpace 中
    error = seL4_X86_PageTable_Map(pt, seL4_CapInitThreadVSpace, vaddr, seL4_X86_Default_VMAttributes);
    ZF_LOGF_IF(error, "Failed to map `%p` page table", vaddr);

    // 3. 将物理帧映射到 VSpace
    seL4_CPtr frame_1 = alloc_object(info, seL4_X86_4K, 0);
    // alloc_object(info, seL4_X86_PDPTObject, 0);
    // alloc_object(info, seL4_X86_PageDirectoryObject, 0);
    error = seL4_ARCH_Page_Map(frame_1, seL4_CapInitThreadVSpace, vaddr, seL4_CanRead, seL4_X86_Default_VMAttributes);
    fll = seL4_MappingFailedLookupLevel();
    printf("fll = %s\n", sel4_mapping_failed_lookup_level_str[fll]);
    ZF_LOGF_IF(error, "Failed to map frame: %d", error);
    error = seL4_ARCH_Page_Map(frame_1, seL4_CapInitThreadVSpace, vaddr, seL4_ReadWrite, seL4_X86_Default_VMAttributes);
    ZF_LOGF_IF(error, "Failed to map frame: %d", error);

    // seL4_CPtr frame_2 = alloc_object(info, seL4_X86_4K, 0);
    // error = seL4_X86_Page_Map(frame_2, seL4_CapInitThreadVSpace, VADDR_2, seL4_AllRights, seL4_X86_Default_VMAttributes);
    // ZF_LOGF_IF(error, "Failed to map frame: %d", error);

    seL4_Word *x = (seL4_Word *)VADDR_1;
    printf("%#lx\n", *x);
    // *x = 5;
    printf("%#lx\n", *x);
    int i = 0;
    int len = BIT(seL4_PageBits) / sizeof(seL4_Word);
    for(i = 0; i < len; i++)
        *(x + i) = i;
    printf("%#lx\n", *x);
    for(i = 0; i < len; i++)
    {
        printf("%8d\t", *(x + i));
        if(i % 5 == 4)
            printf("\n");
    }
    // printf("%8d\t", *(x + i++));
    // printf("%8d\t", *(x + i++));
    len = BIT(seL4_PageBits) / sizeof(char);
    char * buf = (char *)VADDR_1;
    char * s = "Hello, World.";
    i = 0;
    while(*s != 0)
        buf[i++] = *s++;
    buf[i++] = 0;
    printf("%s\n", buf);

    seL4_X86_Page_GetAddress_t paddr = seL4_X86_Page_GetAddress(seL4_CapInitThreadVSpace);
    printf("paddr of init vspace = %#lx\n", paddr.paddr);

    // paddr = seL4_X86_Page_GetAddress(frame_2);
    // printf("paddr of frame_2 = %#lx\n", paddr.paddr);


    error = seL4_X86_Page_Unmap(frame_1);
    ZF_LOGF_IF(error, "Failed to unmap");
    printf("%#lx\n", *x);
    *x = 42;
    printf("%#lx\n", *x);

    return 0;
}