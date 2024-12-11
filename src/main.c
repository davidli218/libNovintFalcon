#include <stdio.h>

#include "falcon_comm.h"

int main(void) {
    uint32_t device_count;

    falcon_comm_get_device_count(&device_count);

    printf("falcon_device_count: %d\n", device_count);
}
