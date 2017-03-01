#include <unistd.h>
#include "work_distribution.h"
#include "elevatorcontrol.h"

int main() {
    ectr_start(&wd_updateLocalElevStatus, &wd_receiveJob);
    work_distribution_start(&ectr_handleJob);

    while (1) {
        usleep(1000);
    }

    return 0;
}
