#include "globals.h"
#include "serialize_data.h"
#include <stdio.h>
#include <string.h>

//gcc -std=gnu99 -g -o serialize serialize_data.c main_test.c

int main() {
	
	// TEST USE serialization of JOB_T
	elev_button_type_t test_button;
	test_button=BUTTON_CALL_DOWN;
	job_t test_case,test_case_return;	
	test_case.floor=1;
	test_case.button=test_button;
	test_case.finished=false;
	test_case.assignee=123;
	
	char buffer[100],buffer_return[100];
	memset(buffer,'\0',sizeof(buffer));
	int a;
	
	a=serialize_job_into_buffer(test_case,buffer,100);
	
	printf("line 25 message is: %d\n",identify_message_type(buffer));
	
	printf("symbols written: %d\n",a);
	puts(buffer);
	printf(buffer);
	printf("\n");
	printf("buffer size %d\n",sizeof(buffer));
	
	int b=15;
	sprintf(&buffer[b],"hui");
	puts(buffer);
	//buffer[15]='1';
	a=de_serialize_job_from_buffer(buffer, &test_case_return);
	serialize_job_into_buffer(test_case_return,buffer_return,100);
	
	printf("line 40 message is: %d\n",identify_message_type(buffer));
	
	printf("%d\n",a);
	printf("%s\n",buffer_return);
	
	//memset(buffer,'\0',sizeof(buffer));
	a=sscanf(buffer_return,"%s",buffer);
	puts(buffer);
	// TEST USE of serialization of FLOOR ##########################################
	int N=2;

	floorstate_t CallsList[N];
	for(int i; i<N;i++){
		CallsList[i].up=false;
		CallsList[i].down=true;
		CallsList[i].el_id_up=-1;
		CallsList[i].el_id_down=123+i;
	}
	
	a=serialize_OutsideCallsList_into_buffer(CallsList,sizeof(CallsList),buffer,100);
	puts(buffer);
	
	printf("line 62 message is: %d\n",identify_message_type(buffer));
	
	floorstate_t CallsList_2[3];
	
	//sprintf(&buffer[23],"hui");
	a=de_serialize_OutsideCallsList_into_buffer(buffer, CallsList_2,sizeof(CallsList_2));
	if(a==-1){printf("error in de_serialization\n");}
	for(int i=0;i<3;i++)
	{
		printf("read OutsideCalls %d %d %d %d\n",CallsList_2[i].up, CallsList_2[i].down, CallsList_2[i].el_id_up, CallsList_2[i].el_id_down);
	}
	
	return 0;
}
