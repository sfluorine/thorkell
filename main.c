#include <stdio.h>
#include <stdint.h>

#include "vm.h"

int main(void) {
    uint8_t instructions[] = {
        10, INS_IPUSH, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        3, INS_POP, 0x00,
        2, INS_HALT,
    };

    VM* vm = vm_init(instructions);

    vm_execute(vm);
    vm_deinit(vm);

    return 0;
}
