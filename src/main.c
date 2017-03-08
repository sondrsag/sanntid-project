#include <unistd.h>
#include "work_distribution.h"
#include "elevatorcontrol.h"
#include "utils.h"
int main(int argc, char* argv) {
	
	int id_of_this_elevator;
	
	///* to_be_inserted
	if(argc == 2)
	{ 
		id_of_this_elevator = str2int(argv[1]); 
	}
	else
	{
		printf("Elevator id in the range from 0 to %d should be given\n",NUM_ELEVATORS);
		return -1;
	}
	//*//
	
	ectr_start(&wd_updateLocalElevStatus, &wd_receiveJob_from_local_elevator);
    work_distribution_start(&ectr_handleJob, id_of_this_elevator);
	
	//should better write a function which would use config_file
	
	///* to_be_inserted
	
	char * ips_and_ports[NUM_ELEVATORS];
	ips_and_ports[0] = "address1:2013\0";
	ips_and_ports[1] = "0\0";
	ips_and_ports[2] = "address1:2013\0";
	ips_and_ports[3] = "1\0";
	ips_and_ports[4] = "address2:2013\0";
	ips_and_ports[5] = "2\0";
	
	elcom_init(ips_and_ports);
    //*/
	
    while (1) {
        usleep(1000);
    }

    return 0;
}
