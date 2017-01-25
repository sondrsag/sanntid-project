#include <unistd.h>
#include "control.h"
#include "elevatorcontrol.h"

int main() {
    ectr_start(&ctr_updateElevStatus, &ctr_receiveJob);
    ctr_start(&ectr_handleJob);

    while (1) {
        usleep(1000);
    }

    return 0;
}
