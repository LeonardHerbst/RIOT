#include <stdio.h>

#include "uvm32_host.h"

#include "blob/program.bin.h"

int main(void) {
    uvm32_instance_t instance = {
        .program = program_bin,
        .program_len = program_bin_len
    };

    uint32_t ret = 0;

    ret = uvm32_instance_init(&instance);
    if (ret != 0) {
        printf("Error initializing vm!\n");
    }

    ret = uvm32_instance_load(&instance);
    if (ret != 0) {
        printf("Error loading the binary!\n");
    }

    ret = uvm32_instance_execute(&instance);
    if (ret == 0) {
        printf("VM returned: %lu\n", (unsigned long int) instance.state.result);
    } else {
        printf("Executed with error!\n");
    }
    return 0;
}
