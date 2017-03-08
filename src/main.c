#include <unistd.h>
#include "work_distribution.h"
#include "elevatorcontrol.h"
#include "elcom.h"
//#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	
	int id_of_this_elevator;
	
	printf("her 0\n");
	///* to_be_inserted
	if(argc == 2)
	{ 
		printf("he %d\n",argc);
		
		id_of_this_elevator = atoi(argv[1]);
				
		
		printf("her 1\n");
	}
	else
	{
		printf("Elevator id in the range from 0 to %d should be given\n",NUM_ELEVATORS);
		printf("her 2");
		return -1;
	}
	//*//
	
	
	ectr_start(&wd_updateLocalElevStatus, &wd_receiveJob_from_local_elevator);
    work_distribution_start(&ectr_handleJob, id_of_this_elevator);
	
	//should better write a function which would use config_file
	
	char * ips_and_ports[NUM_ELEVATORS*2];
	ips_and_ports[0] = "192.168.38.105:20013\0";
	ips_and_ports[1] = "0\0";
	ips_and_ports[2] = "192.168.38.145:20013\0";
	ips_and_ports[3] = "1\0";
	ips_and_ports[4] = "192.168.38.155:20013\0";
	ips_and_ports[5] = "2\0";
	
	elcom_init(ips_and_ports);
	
    while (1) {
        usleep(1000);
    }

    return 0;
}
