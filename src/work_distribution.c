#include "work_distribution.h"
#include "network.h"
#include "elcom.h"
#include "globals.h"
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void (*handleJob)(Job_t); // elevatorcontrol module callback

static pthread_mutex_t wd_mtx;
static int local_assignee_id;

ElevatorStatus_t All_elevators[NUM_ELEVATORS]; 
OutsideCallsList_t OutsideCallsList;
InternalCallsList_t InternalCalls;
AlertJobFinished_t AlertJobFinished;

/*helper function for AssignElevators*/
static int FindIdleElevator(ElevatorStatus_t *All_elevators)
{
	for(int i_e = 0; i_e < NUM_ELEVATORS; ++i_e){   
        if(All_elevators[i_e].available && (All_elevators[i_e].action == IDLE)){
            printf("Assigning job to elevator %d\n", i_e);
            return i_e;
        }
    }

    return NoneElevator_assigned;
}

//This is the cost function which calculates which elevator should do each job
static void AssignElevators(OutsideCallsList_t OutsideCallsList,ElevatorStatus_t *All_elevators) 
{
    pthread_mutex_lock(&wd_mtx);
    for(int i_f=0; i_f<NUM_FLOORS;i_f++)
    {
        if(OutsideCallsList[i_f].up == true && OutsideCallsList[i_f].el_id_up == NoneElevator_assigned){
            OutsideCallsList[i_f].el_id_up = FindIdleElevator(All_elevators);
        }
        if(OutsideCallsList[i_f].down == true && OutsideCallsList[i_f].el_id_down == NoneElevator_assigned){
            OutsideCallsList[i_f].el_id_down = FindIdleElevator(All_elevators);
        }
    }	
    pthread_mutex_unlock(&wd_mtx);
}

void Handle_jobs_assigned()
{
    

    for(int i=0;i<NUM_FLOORS;i++)
    {
        int perform_job = 0;
		Job_t dummy_job;
		
		pthread_mutex_lock(&wd_mtx);
		
		if(OutsideCallsList[i].up == true && OutsideCallsList[i].el_id_up == local_assignee_id)
        {
            dummy_job.floor=i;
            dummy_job.button=BUTTON_CALL_UP;
            dummy_job.finished=false;
            dummy_job.assignee=local_assignee_id;
            perform_job = 1;
        }
		pthread_mutex_unlock(&wd_mtx);
		if(perform_job == 1)
		{
			handleJob(dummy_job);
			perform_job = 0;
		}
		
		pthread_mutex_lock(&wd_mtx);
        if(OutsideCallsList[i].down == true && OutsideCallsList[i].el_id_down == local_assignee_id)
        {
            dummy_job.floor=i;
            dummy_job.button=BUTTON_CALL_DOWN;
            dummy_job.finished=false;
            dummy_job.assignee=local_assignee_id;
            perform_job = 1;
        }
		pthread_mutex_unlock(&wd_mtx);
		
		if(perform_job == 1)
		{
			handleJob(dummy_job);
			perform_job = 0;
		}
		
    }
}

static void init_global_variables()
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
	Handle_jobs_assigned();
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

void* wd_WorkDistributionLoop() 
{
    int ret = pthread_mutex_init(&wd_mtx, NULL);
    if(ret !=0)
    {
        printf("Initialisation of work_distirbution_mutex failed\n");
        return NULL;
    }

    init_global_variables();
    usleep(1000000); //Wait for the start of communication module

    while(true) {
        
        AssignElevators(OutsideCallsList,All_elevators); //Cost function
        
		elcom_broadcastElevatorStatus(All_elevators[local_assignee_id]);
		usleep(20000);
        elcom_broadcastOutsideCallsList(OutsideCallsList);
		usleep(20000);
        elcom_broadcastInternalCallsList(InternalCalls);
        usleep(20000);
		if( local_assignee_id == net_getMasterId())
		{
			Handle_jobs_assigned();
		}
        
     
    }
    return NULL;
}

void work_distribution_start(HandleJobCallback_t jobCallback,
                             AlertJobFinished_t alertCallback,
                             int IdLocalElevator)
{
    local_assignee_id = IdLocalElevator;

    AlertJobFinished = alertCallback;
    handleJob = jobCallback;
    pthread_t WorkDistribution_thrd;
    pthread_create(&WorkDistribution_thrd, NULL, wd_WorkDistributionLoop, NULL);
}

void wd_updateLocalElevStatus(ElevatorStatus_t new_status)
{

    elcom_broadcastElevatorStatus(new_status);

    pthread_mutex_lock(&wd_mtx);
    All_elevators[local_assignee_id] = new_status;

	pthread_mutex_unlock(&wd_mtx);

}

void wd_updateElevStatus(ElevatorStatus_t new_status, int assignee_id)
{
    pthread_mutex_lock(&wd_mtx);
    All_elevators[assignee_id] = new_status;
    if(new_status.available == false) 
    {
        //remove all tasks assigned to this elevator which is dead now
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
    job.assignee = NoneElevator_assigned;
	
    if(job.button==BUTTON_COMMAND) {
        job.assignee = local_assignee_id;
    }

    elcom_broadcastJob(job);
    
    pthread_mutex_lock(&wd_mtx);
    if(job.button==BUTTON_COMMAND)
    {
        if(job.finished==true)
        {
            InternalCalls[local_assignee_id][job.floor]=false; 
        }
        else
        {
            InternalCalls[local_assignee_id][job.floor]=true;
        }
    }
    else // external, hall jobs
    {
        if(job.finished==true) 
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
        else 
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
    if(job.button==BUTTON_COMMAND) 
    {
        if(job.finished==true)
        {
            InternalCalls[job.assignee][job.floor]=false; 
            AlertJobFinished(job);
        }
        else
        {
            InternalCalls[job.assignee][job.floor]=true;
        }
    }
    else // external, hall jobs
    {
        if(job.finished==true) 
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
            AlertJobFinished(job);
        }
        else 
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
