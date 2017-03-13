#include <unistd.h>
#include "work_distribution.h"
#include "elevatorcontrol.h"

int main() {
	
	ectr_start(&wd_updateLocalElevStatus, &wd_receiveJob);
    work_distribution_start(&ectr_handleJob);
	
	//should better write a function which would use config_file
	/* to_be_inserted
	char * ips_and_ports[NUM_ELEVATORS];
	ips_and_ports[0] = "address1:2015" 
	ips_and_ports[1] = "address2:2015"
	ips_and_ports[2] = "address3:2015"
	
	elcom_init(ips_and_ports)
    */
	
    while (1) {
        usleep(1000);
    }

    return 0;
}
