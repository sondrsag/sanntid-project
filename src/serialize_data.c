#include "serialize_data.h"
#include <string.h>

//add error return to functions and etc
//DISCUSS, potential faults.
//add check for wrong input?

Message_type_t identify_message_type(char* buffer)
{	
	char dummy[10];
	Message_type_t for_return;
	int result=sscanf(buffer,"%s",dummy);
	if(result!=1){for_return = NOT_RECOGNIZED;}
	
	if(dummy[0]=='j'){	for_return=JOB;}
	else if(dummy[0]=='e'){	for_return=ELEVATOR_STATUS;}
	else if(dummy[0]=='o'){	for_return=OUTSIDE_CALLS;}
    else if(dummy[0]=='i'){ for_return=INTERNAL_CALLS;}
	else{for_return=NOT_RECOGNIZED;	}
	
	return for_return;
}

int serialize_job_into_buffer(Job_t job,char* buffer, int buffer_size){
	int chksum=job.floor+job.button+job.finished+job.assignee;
	int result=sprintf(buffer,"job %d %d %d %d %d",chksum,job.floor,job.button,job.finished,job.assignee);
	if(result>buffer_size){return -1;};
	
	return result;
}

int de_serialize_job_from_buffer(char* buffer, Job_t *job){
	int chksum;
	int result=sscanf(buffer,"job %d %d %d %d %d",&chksum,&(job->floor), (int*)&(job->button), (int*)&(job->finished),&(job->assignee));
	//printf("succesfull reads %d\n",result);
	if(result!=5){return -1;}
	int chksum_check=job->floor+job->button+job->finished+job->assignee;
	if(chksum_check!=chksum){return -1;}
	
	//printf("read buffer, the value of check sum is: %d\n",chksum);
	return result;
}

int serialize_ElevatorStatus_into_buffer(ElevatorStatus_t S,char* buffer, int buffer_size){
	int chksum=S.working
		+S.finished
		+S.current_floor
		+S.next_floor
		+S.available
		+S.action
		+S.direction;
	int result=sprintf(buffer,"elevator %d %d %d %d %d %d %d %d",chksum,S.working,S.finished,S.current_floor,
		S.next_floor, S.available,S.action, S.direction);
	if(result>buffer_size){return -1;};
	
	return result;
}

int de_serialize_ElevatorStatus_from_buffer(char* buffer, ElevatorStatus_t *S){
	int chksum;
	int result=sscanf(buffer,"elevator %d %d %d %d %d %d %d %d",
                          &chksum, (int*)&(S->working), (int*)&(S->finished), &(S->current_floor),
		          &(S->next_floor), &(S->available),(int*)&(S->action), &(S->direction));
	//printf("succesfull reads %d\n",result);
	if(result!=8){return -1;}
	int chksum_check=S->working + S->finished + S->current_floor
		+ S->next_floor + S->available + S->action + S->direction;

	if(chksum_check!=chksum){return -1;}
	
	//printf("read buffer, the value of check sum is: %d\n",chksum);
	
	return result;
}

/*static int serializeFloorCallsIntoBuffer(FloorCalls_t floor_calls) {

    return 0;
}*/

int serialize_OutsideCallsList_into_buffer(OutsideCallsList_t calls_list,int size_CallsList,char* buffer, int buffer_size){
	int chksum = 0;
	int result = 0;
	
	if(buffer_size < ( size_CallsList*2 + sizeof("outside")))
	{
            printf(("In serialize_OutsideCallsList_into_buffer, "
                    "size of allocated buffer is too small, required size %lu"),
                    ( size_CallsList*sizeof(FloorCalls_t)*2 + sizeof("Outside")));

            return -1;
        }

        strcpy(buffer, "outside ");
	
        for ( size_t i = 0; i < NUM_FLOORS; ++i) {
            chksum += chksum + calls_list[i].up + calls_list[i].down + calls_list[i].el_id_up
                + calls_list[i].el_id_down;
            result = sprintf(&buffer[strlen(buffer)], "%d %d %d %d ",
                    (int)(calls_list[i].up), (int)(calls_list[i].down), 
                    (int)(calls_list[i].el_id_up), (int)(calls_list[i].el_id_down));
        }
        //printf("%s\n", buffer);
        
	return result;
}

/*
int de_serialize_FloorCalls_from_buffer(char const * buffer, FloorCalls_t * floor_calls) {
    floor_calls->up = buffer[0];
    floor_calls->down = buffer[1];
    floor_calls->el_id_up = buffer[2];
    floor_calls->el_id_down = buffer[3];
    return 1;
}
*/

int de_serialize_OutsideCallsList_from_buffer(char const * buffer, OutsideCallsList_t calls_list){
	//int chksum=0;
	//int index=0;
    //    int result = 0;
        char* buf_ptr = strchr(buffer, ' ') + 1;
        for ( size_t i = 0; i < NUM_FLOORS; ++i ) {
            /*
            result = sscanf(buf_ptr, "%d %d %d %d",
                    (int*)&calls_list[i].up, (int*)&calls_list[i].down,
                    (int*)&calls_list[i].el_id_up, (int*)&calls_list[i].el_id_down);
            
            */
            sscanf(buf_ptr, "%d", (int*)&calls_list[i].up);
            buf_ptr = strchr(buf_ptr, ' ') + 1;
            sscanf(buf_ptr, "%d", (int*)&calls_list[i].down);
            buf_ptr = strchr(buf_ptr, ' ') + 1;
            sscanf(buf_ptr, "%d", (int*)&calls_list[i].el_id_up);
            buf_ptr = strchr(buf_ptr, ' ') + 1;
            sscanf(buf_ptr, "%d", (int*)&calls_list[i].el_id_down);
            buf_ptr = strchr(buf_ptr, ' ') + 1;
        }
    /*
	char trash_buffer[100];
	int result=sscanf(buffer,"Outside");   if(result<0){return -1;}
	index += strlen("Outside"); 
	for( size_t i = 0; i < NUM_FLOORS; i++ )
	{
		result=sscanf(&buffer[index]," %d %d %d %d",(int*)&(CallsList[i].up), (int*)&(CallsList[i].down), (int*)(&(CallsList[i].el_id_up)), (int*)(&(CallsList[i].el_id_down)));   		
		if(result<0){return -1;}
		
		memset(trash_buffer,'\0',sizeof(trash_buffer));
		sprintf(trash_buffer," %d %d %d %d",CallsList[i].up, CallsList[i].down, CallsList[i].el_id_up, CallsList[i].el_id_down);		
		index +=strlen(trash_buffer);
		chksum = chksum + CallsList[i].up + CallsList[i].down + CallsList[i].el_id_up+CallsList[i].el_id_down;	
	}
	//printf("succesfull reads %d\n",result);
	int chksum_read;
	result=sscanf(&buffer[index]," %d",&chksum_read); if(result<0){return -1;}
	memset(trash_buffer,'\0',sizeof(trash_buffer));
	sprintf(trash_buffer," %d",chksum_read);		
	index += strlen(trash_buffer);
		
	if(buffer[index]!='\0'){
		printf("not the whole buffer was read\n");
		return -1;
	}
	
	if(result<0){return -1;}
	if(chksum!=chksum_read){
		printf("check sum values are inconsistent, calculated %d, read %d\n",chksum,chksum_read);
		return -1;}
	return result;
        */
    return 0;
}


int serializeInternalCallsListIntoBuffer(InternalCallsList_t calls_list, char* buffer,
        size_t buffer_size) {
    if (buffer_size < (sizeof(InternalCallsList_t)*2 + sizeof("internal "))) {
        fprintf(stderr, ("Too small buffer size when serializing internal calls list."
                         "Size is %zd"), buffer_size);
        return -1;
    }
    strcpy(buffer, "internal ");
    char* buf_ptr = strchr(buffer, ' ') + 1;
    for(size_t elevator = 0; elevator < NUM_ELEVATORS; ++elevator) {
        for (size_t floor = 0; floor < NUM_FLOORS; ++floor) {
            sprintf(buf_ptr, "%d ", calls_list[elevator][floor]);
            buf_ptr = strchr(buf_ptr, ' ') + 1;
        }
    }
    return 0;
}

int deserializeInternalCallsListFromBuffer(char const * buffer,
        InternalCallsList_t calls_list) {
    //Skip message identifier
    char* buf_ptr = strchr(buffer, ' ') + 1;
    for(size_t elevator = 0; elevator < NUM_ELEVATORS; ++elevator) {
        for (size_t floor = 0; floor < NUM_FLOORS; ++floor) {
            sscanf(buf_ptr, "%d ", (int*)&calls_list[elevator][floor]);
            buf_ptr = strchr(buf_ptr, ' ') + 1;
        }
    }

    return 0;
}


//////////////////
/* Will not be in use*/
/////////////////
/*
int serialize_Floorstate_into_buffer(Floorstate_t floor,char* buffer, int buffer_size){
	int chksum=floor.up+floor.down+floor.el_id;
	int result=sprintf(buffer,"floor %d %d %d %d",chksum,floor.up,floor.down,floor.el_id);
	if(result>buffer_size){return -1;};
	
	return result;
}

int de_serialize_Floorstate_from_buffer(char* buffer, Floorstate_t *floor){
	int chksum;
	int result=sscanf(buffer,"floor %d %d %d %d %d",&chksum,&(floor->up),&(floor->down),&(floor->el_id));
	
	if(result!=5){return -1;}
	int chksum_check=floor->up+floor->down+floor->el_id;
	if(chksum_check!=chksum){return -1;}
	
	return result;
}
*/
