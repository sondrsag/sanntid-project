#include <unistd.h>
#include "work_distribution.h"
#include "elevatorcontrol.h"
#include "elcom.h"
//#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {

	unsigned int id_of_this_elevator;

	///* to_be_inserted
	if(argc == 2)
	{
		id_of_this_elevator = atoi(argv[1]);
		printf("elev id: %d",id_of_this_elevator);
	}
	else
	{
		printf("Elevator id in the range from 0 to %d should be given, the value shoudl correspond to the value in network_config.conf file\n",NUM_ELEVATORS);
		return -1;
	}
	//*//


	ectr_start(&wd_updateLocalElevStatus, &wd_receiveJob_from_local_elevator);
    work_distribution_start(&ectr_handleJob,
							&ectr_updateFinishedJob,
							id_of_this_elevator);

	elcom_init(id_of_this_elevator);

    while (1) {
        usleep(1000);
    }

    return 0;
}
