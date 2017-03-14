#include "work_distribution.h"
#include "elcom.h"
#include "globals.h"
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void                  (*handleJob)(Job_t); // elevatorcontrol module callback

static pthread_mutex_t wd_mtx;
static int local_assignee_id;  

ElevatorStatus_t All_elevators[NUM_ELEVATORS]; //is it needed to use static here or not?
OutsideCallsList_t OutsideCallsList;
InternalCallsList_t InternalCalls;


void* wd_WorkDistributionLoop();
void AssignElevators(OutsideCallsList_t OutsideCallsList,ElevatorStatus_t *All_elevators);
int FindClosestElevator(ElevatorStatus_t *All_elevators, elev_motor_direction_t call_direction, int call_floor);
void Handle_jobs_assigned();


void work_distribution_start(HandleJobCallback_t jobCallback, int IdLocalElevator)
{
    local_assignee_id = IdLocalElevator;

    handleJob = jobCallback;
    pthread_t WorkDistribution_thrd;
    pthread_create(&WorkDistribution_thrd, NULL, wd_WorkDistributionLoop, NULL);
}

void init_global_variables()
{

    pthread_mutex_lock(&wd_mtx);
    for(int i=0;i<NUM_ELEVATORS;i++)
    {
        All_elevators[i].available=false;
		if(i == local_assignee_id){All_elevators[i].available = true;}
    }
    for(int i=0;i<NUM_FLOORS;i++)
    {
        OutsideCallsList[i].up         = false;

        OutsideCallsList[i].el_id_up   = NoneElevator_assigned;
        OutsideCallsList[i].down       = false;
        OutsideCallsList[i].el_id_down = NoneElevator_assigned;
    }
    for(int i=0; i<NUM_ELEVATORS;i++)
    {
        for(int j=0;j<NUM_FLOORS;j++)
        {
            InternalCalls[i][j] = false;
        }
    }	
    pthread_mutex_unlock(&wd_mtx);
}

void wd_receiveCallsListFromPrimary(OutsideCallsList_t newOutsideCallsList)
{
    pthread_mutex_lock(&wd_mtx);
    for(int i=0;i<NUM_FLOORS;i++)
    {
        OutsideCallsList[i]=newOutsideCallsList[i];
    }
    pthread_mutex_unlock(&wd_mtx);
    //Handle_jobs_assigned(); // jobs are handled in work_distributionLoop
}



void wd_HandleInternalCallsAfterRestart(InternalCallsList_t newInternalCalls)
{
    Job_t dummy_job;

    for(int i=0; i<NUM_FLOORS; i++)
    {
        if( newInternalCalls[local_assignee_id][i] )
        {
            pthread_mutex_lock(&wd_mtx);
            InternalCalls[local_assignee_id][i] = true;
            pthread_mutex_unlock(&wd_mtx);

            dummy_job.floor=i;
            dummy_job.button=BUTTON_COMMAND;
            dummy_job.finished=false;
            dummy_job.assignee=local_assignee_id;

            handleJob(dummy_job);
        }
    }
}


void* wd_WorkDistributionLoop() {
    int ret = pthread_mutex_init(&wd_mtx, NULL);
    if(ret !=0)
    {
        printf("Initialisation of work_distirbution_mutex failed\n");
        return -1;
    }

    init_global_variables();
    usleep(2000000); //Wait for the start of communication module

    while(true) {
        usleep(100000);

        AssignElevators(OutsideCallsList,All_elevators); //Cost function
        //printOutsideCallsList(OutsideCallsList);
        ///*to_be_inserted
		elcom_broadcastElevatorStatus(All_elevators[local_assignee_id]);
		usleep(20000);
        elcom_broadcastOutsideCallsList(OutsideCallsList); 
		usleep(20000);
        elcom_broadcastInternalCallsList(InternalCalls);
        //*/
        usleep(100000);

        Handle_jobs_assigned();
        //wd_receiveCallsListFromPrimary(OutsideCallsList); // JUST FOR THE MOMENT, THIS function should be called by communication module

    }
    return NULL;
}


void Handle_jobs_assigned()
{
    Job_t dummy_job;

    for(int i=0;i<NUM_FLOORS;i++)
    {
        if(OutsideCallsList[i].up==true && OutsideCallsList[i].el_id_up==local_assignee_id) 
        {	
            dummy_job.floor=i;
            dummy_job.button=BUTTON_CALL_UP;
            dummy_job.finished=false;
            dummy_job.assignee=local_assignee_id;
            handleJob(dummy_job);
        }
        if(OutsideCallsList[i].down==true && OutsideCallsList[i].el_id_down==local_assignee_id)
        {
            dummy_job.floor=i;
            dummy_job.button=BUTTON_CALL_DOWN;
            dummy_job.finished=false;
            dummy_job.assignee=local_assignee_id;
            handleJob(dummy_job);
        }
    }		
}// void Handle_jobs_assigned()

//This is the cost function which calculates which elevator should do what job
//and updates the state
//In order to validate the state we must ensure that all elevators
//which have jobs assigned to them are available.
//If they are not, then all jobs must be reassigned
void AssignElevators(OutsideCallsList_t OutsideCallsList,ElevatorStatus_t *All_elevators) {
    pthread_mutex_lock(&wd_mtx);
    for(int i_f=0; i_f<NUM_FLOORS;i_f++)
    {
        if(OutsideCallsList[i_f].up == true && OutsideCallsList[i_f].el_id_up == NoneElevator_assigned){
            OutsideCallsList[i_f].el_id_up=FindClosestElevator(All_elevators, DIRN_UP,i_f);
        }
        if(OutsideCallsList[i_f].down == true && OutsideCallsList[i_f].el_id_down == NoneElevator_assigned){
            OutsideCallsList[i_f].el_id_down=FindClosestElevator(All_elevators, DIRN_DOWN,i_f);
        }		
    }	/*Anton: should the return be to another array of OutsideCallsList? As we plan to use only one comming from Primary elevator?ANSWER: functions fine now*/
    pthread_mutex_unlock(&wd_mtx);
}
/*helper function for AssignElevators*/
int FindClosestElevator(ElevatorStatus_t *All_elevators, elev_motor_direction_t call_direction, int call_floor)
{
    /*
    printf("This elevator status\n");
    printElevatorStatus(All_elevators[0]);
    */

    for(int i_e = 0; i_e < NUM_ELEVATORS; ++i_e){   //First find idle elevator
        if(All_elevators[i_e].available && (All_elevators[i_e].action == IDLE)){
            printf("Assigning job to elevator %d\n", i_e);
            return i_e;
        }
    }

    //If no idle, find elevator in same direction which has not reached this floor yet
    for(int i_e=0;i_e<NUM_ELEVATORS;i_e++){ 
        /*
        if( All_elevators[i_e].available == true &&
            All_elevators[i_e].direction == call_direction && 
            All_elevators[i_e].direction*(call_floor-All_elevators[i_e].current_floor) >= 0 ) {
            return i_e;
        }
        */
    }
    return NoneElevator_assigned;//If neither, leave job for next time
}

// if on elevator dyes all outside jobs assigned to it should be reassigned. in two steps. first remove elevator id from the outside calls, next can

void wd_updateLocalElevStatus(ElevatorStatus_t new_status)
{

    elcom_broadcastElevatorStatus(new_status);

    putc('\n', stdout);

    pthread_mutex_lock(&wd_mtx);
    All_elevators[local_assignee_id] = new_status;
    pthread_mutex_unlock(&wd_mtx);
    /*
    printf("Local status\n");
    printElevatorStatus(new_status);
    putc('\n', stdout);
    */
}

void wd_updateElevStatus(ElevatorStatus_t new_status, int assignee_id)
{
    
	pthread_mutex_lock(&wd_mtx);
    All_elevators[assignee_id] = new_status;
    if(new_status.available == false) //remove all tasks assigned to this elevator
    {
        for(int i_f=0; i_f<NUM_FLOORS;i_f++)
        {
            if(OutsideCallsList[i_f].el_id_up == assignee_id){
                OutsideCallsList[i_f].el_id_up = NoneElevator_assigned;
            }
            if(OutsideCallsList[i_f].el_id_down == assignee_id){
                OutsideCallsList[i_f].el_id_down = NoneElevator_assigned;
            }		
        }
    }
    pthread_mutex_unlock(&wd_mtx);
}

void wd_receiveJob_from_local_elevator(Job_t job)
{
    //internal, i.e., cabin jobs
    if(job.button==BUTTON_COMMAND) {
        job.assignee = local_assignee_id;
    }

    job.assignee = NoneElevator_assigned;
    elcom_broadcastJob(job);
    //*/

    pthread_mutex_lock(&wd_mtx);
    if(job.button==BUTTON_COMMAND) //internal, i.e., cabin jobs
    {//at the moment will not function as internal commands are not sent from elevatorcontrol, but required for back up at restart of elevator after crush
        if(job.finished==true)
        {
            InternalCalls[local_assignee_id][job.floor]=false; //removes call from the list
        }
        else
        {
            InternalCalls[local_assignee_id][job.floor]=true;
        }
    }
    else // external, hall jobs
    {
        if(job.finished==true) //removes call from the list
        {
            if(job.button==BUTTON_CALL_UP)
            {
                OutsideCallsList[job.floor].up = false; 
                OutsideCallsList[job.floor].el_id_up = NoneElevator_assigned;
            }			
            else
            {
                OutsideCallsList[job.floor].down = false; 
                OutsideCallsList[job.floor].el_id_down = NoneElevator_assigned;
            }
        }
        else //set jobs to be active for this floor
        {
            if(job.button==BUTTON_CALL_UP)
            {
                OutsideCallsList[job.floor].up=true; 
            }			
            else
            {
                OutsideCallsList[job.floor].down=true; 
            }
        }					
    }   
    pthread_mutex_unlock(&wd_mtx);

}

void wd_receiveJob_from_elcom(Job_t job)
{
    pthread_mutex_lock(&wd_mtx);
    if(job.button==BUTTON_COMMAND) //internal, i.e., cabin jobs
    {//at the moment will not function as internal commands are not sent from elevatorcontrol, but required for back up at restart of elevator after crush
        if(job.finished==true)
        {
            InternalCalls[job.assignee][job.floor]=false; //removes call from the list
        }
        else
        {
            InternalCalls[job.assignee][job.floor]=true;
        }
    }
    else // external, hall jobs
    {
        if(job.finished==true) //removes call from the list
        {
            if(job.button==BUTTON_CALL_UP)
            {
                OutsideCallsList[job.floor].up = false; 
                OutsideCallsList[job.floor].el_id_up = NoneElevator_assigned;
            }			
            else
            {
                OutsideCallsList[job.floor].down = false; 
                OutsideCallsList[job.floor].el_id_down = NoneElevator_assigned;
            }
        }
        else //set jobs to be active for this floor
        {
            if(job.button==BUTTON_CALL_UP)
            {
                OutsideCallsList[job.floor].up=true; 
            }			
            else
            {
                OutsideCallsList[job.floor].down=true; 
            }
        }					
    }   
    //printOutsideCallsList(OutsideCallsList);
    pthread_mutex_unlock(&wd_mtx);

}

