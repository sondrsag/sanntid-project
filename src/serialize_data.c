#include "serialize_data.h"
#include <string.h>

//add error return to functions and etc
//DISCUSS, potential faults.
//add check for wrong input?

message_type_t identify_message_type(char* buffer)
{	
	char dummy[10];
	message_type_t for_return;
	int result=sscanf(buffer,"%s",dummy);
	if(result!=1){return -1;}
	
	if(dummy[0]=='j'){	for_return=JOB;}
	else if(dummy[0]=='e'){	for_return=ELEVATOR_STATUS;}
	else if(dummy[0]=='O'){	for_return=OUTSIDE_CALLS;}
	else{for_return=NOT_RECOGNIZED;	}
	
	return for_return;
}

int serialize_job_into_buffer(job_t job,char* buffer, int buffer_size){
	int chksum=job.floor+job.button+job.finished+job.assignee;
	int result=sprintf(buffer,"job %d %d %d %d %d",chksum,job.floor,job.button,job.finished,job.assignee);
	if(result>buffer_size){return -1;};
	
	return result;
}

int de_serialize_job_from_buffer(char* buffer, job_t *job){
	int chksum;
	int result=sscanf(buffer,"job %d %d %d %d %d",&chksum,&(job->floor),&(job->button),&(job->finished),&(job->assignee));
	//printf("succesfull reads %d\n",result);
	if(result!=5){return -1;}
	int chksum_check=job->floor+job->button+job->finished+job->assignee;
	if(chksum_check!=chksum){return -1;}
	
	//printf("read buffer, the value of check sum is: %d\n",chksum);
	return result;
}

int serialize_ElevatorStatus_into_buffer(ElevatorStatus S,char* buffer, int buffer_size){
	int chksum=S.working
		+S.finished
		+S.current_floor
		+S.next_floor
		+S.action
		+S.direction;
	int result=sprintf(buffer,"elevator %d %d %d %d %d %d %d",chksum,S.working,S.finished,S.current_floor,
		S.next_floor,S.action,S.direction);
	if(result>buffer_size){return -1;};
	
	return result;
}

int de_serialize_ElevatorStatus_from_buffer(char* buffer, ElevatorStatus *S){
	int chksum;
	int result=sscanf(buffer,"elevator %d %d %d %d %d %d %d",&chksum, &(S->working), &(S->finished), &(S->current_floor),
		&(S->next_floor), &(S->action), &(S->direction));
	//printf("succesfull reads %d\n",result);
	if(result!=7){return -1;}
	int chksum_check=S->working + S->finished + S->current_floor
		+ S->next_floor + S->action + S->direction;
	if(chksum_check!=chksum){return -1;}
	
	//printf("read buffer, the value of check sum is: %d\n",chksum);
	
	return result;
}


int serialize_OutsideCallsList_into_buffer(floorstate_t *CallsList,int size_CallsList,char* buffer, int buffer_size){
	int chksum=0;
	int result;
	
	if(buffer_size < ( size_CallsList*2 + sizeof("Outside")))
	{
		printf("In serialize_OutsideCallsList_into_buffer, size of allocated buffer is too small, required size %d",
		( size_CallsList*sizeof(floorstate_t)*2 + sizeof("Outside")));
		return -1;
		}
	
	result=result+sprintf(buffer,"Outside");
	if(result<0){return -1;}
	for(int i=0;i<size_CallsList/sizeof(CallsList[0]);i++)
		{
		chksum = chksum + CallsList[i].up + CallsList[i].down + CallsList[i].el_id_up+CallsList[i].el_id_down;
		result=sprintf(&buffer[strlen(buffer)]," %d %d %d %d",CallsList[i].up, CallsList[i].down, CallsList[i].el_id_up, CallsList[i].el_id_down);
		if(result<0){return -1;}
		}
	result=sprintf(&buffer[strlen(buffer)]," %d",chksum);
	if(result<0){return -1;}
	
	return result;
}

int de_serialize_OutsideCallsList_into_buffer(char* buffer, floorstate_t *CallsList,int size_CallsList){
	int chksum=0;
	int index=0;
	char trash_buffer[100];
	int result=sscanf(buffer,"Outside");   if(result<0){return -1;}
	index += strlen("Outside"); 
	for(int i=0;i<size_CallsList/sizeof(CallsList[0]);i++)
	{
		result=sscanf(&buffer[index]," %d %d %d %d",&(CallsList[i].up), &(CallsList[i].down), &(CallsList[i].el_id_up), &(CallsList[i].el_id_down));   		
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
}



//////////////////
/* Will not be in use*/
/////////////////
/*
int serialize_floorstate_into_buffer(floorstate_t floor,char* buffer, int buffer_size){
	int chksum=floor.up+floor.down+floor.el_id;
	int result=sprintf(buffer,"floor %d %d %d %d",chksum,floor.up,floor.down,floor.el_id);
	if(result>buffer_size){return -1;};
	
	return result;
}

int de_serialize_floorstate_from_buffer(char* buffer, floorstate_t *floor){
	int chksum;
	int result=sscanf(buffer,"floor %d %d %d %d %d",&chksum,&(floor->up),&(floor->down),&(floor->el_id));
	
	if(result!=5){return -1;}
	int chksum_check=floor->up+floor->down+floor->el_id;
	if(chksum_check!=chksum){return -1;}
	
	return result;
}
*/