#include "sched.h"

#include <inc/assert.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/trap.h>
#include <kern/mem/kheap.h>
#include <kern/mem/memory_manager.h>
#include <kern/tests/utilities.h>
#include <kern/cmd/command_prompt.h>


uint32 isSchedMethodRR(){if(scheduler_method == SCH_RR) return 1; return 0;}
uint32 isSchedMethodMLFQ(){if(scheduler_method == SCH_MLFQ) return 1; return 0;}
uint32 isSchedMethodBSD(){if(scheduler_method == SCH_BSD) return 1; return 0;}

//===================================================================================//
//============================ SCHEDULER FUNCTIONS ==================================//
//===================================================================================//

//===================================
// [1] Default Scheduler Initializer:
//===================================
void sched_init()
{
	old_pf_counter = 0;

	sched_init_RR(INIT_QUANTUM_IN_MS);

	init_queue(&env_new_queue);
	init_queue(&env_exit_queue);
	scheduler_status = SCH_STOPPED;
}

//=========================
// [2] Main FOS Scheduler:
//=========================
void
fos_scheduler(void)
{
	//	cprintf("inside scheduler\n");

	chk1();
	scheduler_status = SCH_STARTED;

	//This variable should be set to the next environment to be run (if any)
	struct Env* next_env = NULL;

	if (scheduler_method == SCH_RR)
	{
		// Implement simple round-robin scheduling.
		// Pick next environment from the ready queue,
		// and switch to such environment if found.
		// It's OK to choose the previously running env if no other env
		// is runnable.

		//If the curenv is still exist, then insert it again in the ready queue
		if (curenv != NULL)
		{
			enqueue(&(env_ready_queues[0]), curenv);
		}

		//Pick the next environment from the ready queue
		next_env = dequeue(&(env_ready_queues[0]));

		//Reset the quantum
		//2017: Reset the value of CNT0 for the next clock interval
		cprintf("q -->%d \n",quantums[0]);
		kclock_set_quantum(quantums[0]);
		//uint16 cnt0 = kclock_read_cnt0_latch() ;
		//cprintf("CLOCK INTERRUPT AFTER RESET: Counter0 Value = %d\n", cnt0 );

	}
	else if (scheduler_method == SCH_MLFQ)
	{
		next_env = fos_scheduler_MLFQ();
	}
	else if (scheduler_method == SCH_BSD)
	{
		next_env = fos_scheduler_BSD();
	}
	//temporarily set the curenv by the next env JUST for checking the scheduler
	//Then: reset it again
	struct Env* old_curenv = curenv;
	curenv = next_env ;
	chk2(next_env) ;
	curenv = old_curenv;

	//sched_print_all();

	if(next_env != NULL)
	{
		//		cprintf("\nScheduler select program '%s' [%d]... counter = %d\n", next_env->prog_name, next_env->env_id, kclock_read_cnt0());
		//		cprintf("Q0 = %d, Q1 = %d, Q2 = %d, Q3 = %d\n", queue_size(&(env_ready_queues[0])), queue_size(&(env_ready_queues[1])), queue_size(&(env_ready_queues[2])), queue_size(&(env_ready_queues[3])));
	    //cprintf("nice_value %d \n",next_env->nice_value);
		env_run(next_env);
	}
	else
	{
		/*2015*///No more envs... curenv doesn't exist any more! return back to command prompt
		curenv = NULL;
		//lcr3(K_PHYSICAL_ADDRESS(ptr_page_directory));
		lcr3(phys_page_directory);

		//cprintf("SP = %x\n", read_esp());

		scheduler_status = SCH_STOPPED;
		//cprintf("[sched] no envs - nothing more to do!\n");
		while (1)
			run_command_prompt(NULL);

	}
}

//=============================
// [3] Initialize RR Scheduler:
//=============================
void sched_init_RR(uint8 quantum)
{

	// Create 1 ready queue for the RR
	num_of_ready_queues = 1;
#if USE_KHEAP
	sched_delete_ready_queues();
	env_ready_queues = kmalloc(sizeof(struct Env_Queue));
	quantums = kmalloc(num_of_ready_queues * sizeof(uint8)) ;
#endif
	quantums[0] = quantum;
	kclock_set_quantum(quantums[0]);
	init_queue(&(env_ready_queues[0]));

	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_RR;
	//=========================================
	//=========================================
}

//===============================
// [4] Initialize MLFQ Scheduler:
//===============================
void sched_init_MLFQ(uint8 numOfLevels, uint8 *quantumOfEachLevel)
{
#if USE_KHEAP
	//=========================================
	//DON'T CHANGE THESE LINES=================
	sched_delete_ready_queues();
	//=========================================
	//=========================================

	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_MLFQ;
	//=========================================
	//=========================================
#endif
}

int quant;
//===============================
// [5] Initialize BSD Scheduler:
//===============================
int q ;
void sched_init_BSD(uint8 numOfLevels, uint8 quantum)
{
#if USE_KHEAP
    //TODO: [PROJECT'23.MS3 - #4] [2] BSD SCHEDULER - sched_init_BSD


	 quantums = kmalloc(1 * sizeof(uint8)) ;
	 env_ready_queues = kmalloc(64*sizeof(struct Env_Queue));
	 quantums[0]=quantum;

	 num_of_ready_queues=numOfLevels;

	   for(int i=0;i<numOfLevels;i++){

	        init_queue(&env_ready_queues[i]);
	    }


    //=========================================
    //DON'T CHANGE THESE LINES=================
    scheduler_status = SCH_STOPPED;
    scheduler_method = SCH_BSD;
    //=========================================
    //=========================================
#endif

}


//=========================
// [6] MLFQ Scheduler:
//=========================
struct Env* fos_scheduler_MLFQ()
{
	panic("not implemented");
	return NULL;
}


void print_envs(){

	struct Env* current_env = NULL;
	cprintf("load_avg: %d \n",fix_round(load_avg));
	for(int i=0;i<num_of_ready_queues;i++)
		    {
				cprintf("in queue %d : \n",i);
				LIST_FOREACH(current_env,&env_ready_queues[i])
		        {

					cprintf("\t enviromnt : %d prog_name: %s recentcpu : %d prority: %d\n",current_env->env_id,current_env->prog_name,fix_round(current_env->recent_cpu100),current_env->priority);

		        }

				cprintf("\n");
		    }

}

//=========================
// [7] BSD Scheduler:
//=========================
bool new_priority_clac(struct Env* e){

	fixed_point_t recent_cpu_scaled = fix_unscale(e->recent_cpu100, 4);
	int nice_scaled = e->nice_value * 2;
	fixed_point_t priority_fixed = fix_sub(fix_sub(fix_int(PRI_MAX), recent_cpu_scaled), fix_int(nice_scaled));

	// Convert the fixed-point result to an integer for the final priority value.
	int old_priority = e->priority;
	e->priority = fix_trunc(priority_fixed);

	if(e->priority >PRI_MAX)
			e->priority = PRI_MAX;
	else if(e->priority < 0 )
		e->priority = 0;

	if(old_priority == e->priority && e->in_q)
		return 0;
	else
		return 1 ;
}

void recalc_priority(){

	struct Env* loop_env = NULL;
	struct Env* current_env = NULL;
	//print_envs();

	for(int i=num_of_ready_queues-1;i>=0;i--)
	    {


			LIST_FOREACH(loop_env,&env_ready_queues[i]){

				// calculate priority
				bool new_p = new_priority_clac(loop_env);

				if(new_p == 1 ){
					current_env = loop_env;
					LIST_REMOVE(&env_ready_queues[i],loop_env);
					enqueue(&(env_ready_queues[current_env->priority]), current_env);
					current_env->in_q =1;
				}
				else{
					// for debugging
				}
			}
	    }/*
				if(LIST_SIZE(&env_ready_queues[i])>0){

				    tail_env = LIST_LAST(&env_ready_queues[i]);
					while(1){

						if(LIST_SIZE(&env_ready_queues[i]) == 0 || tail_env == NULL){

							break;
						}
						current_env = tail_env;
						tail_env = LIST_PREV(tail_env) ;

						bool new_p = new_priority_clac(current_env);
						if(new_p == 1 ){
							LIST_REMOVE(&env_ready_queues[i],current_env);
							enqueue(&(env_ready_queues[current_env->priority]), current_env);
						}
						else{
							// for debugging
						}

					}




			}*/


		}

bool initial = 0;


struct Env* fos_scheduler_BSD()
{

	//TODO: [PROJECT'23.MS3 - #5] [2] BSD SCHEDULER - fos_scheduler_BSD

	// may be we need  to check if the enviroment is still running so we push again in the queue

	struct Env* next_env = NULL;
	struct Env* current_env = NULL;

	if (curenv != NULL)
	{

		enqueue(&(env_ready_queues[curenv->priority]), curenv);
	}

	for(int i=num_of_ready_queues-1;i>=0;i--)
	    {
			 next_env=dequeue(&env_ready_queues[i]);

			 if(next_env != NULL)
				 break;


	    }

	if(next_env != NULL){

		kclock_set_quantum(quantums[0]);
	}


	if(next_env == NULL){

		load_avg.f =0;
	}

	return next_env;
}

int get_num_process(){

	int cnt =0 ;

	if(curenv !=NULL)
		cnt++;

	for(int i=0;i<num_of_ready_queues;i++)
	    {

			cnt+=LIST_SIZE(&env_ready_queues[i]);

	    }
	return cnt;
}





void recent_cpu_clac(struct Env* e ){


	fixed_point_t loadm2 = fix_scale(load_avg, 2);

	fixed_point_t loadm2p2 = fix_add(loadm2, fix_int(1));

	fixed_point_t priorityp1 = fix_mul(fix_div(loadm2, loadm2p2), e->recent_cpu100);

	e->recent_cpu100 = fix_add(priorityp1,fix_int(e->nice_value));


}


void recalc_recent_cpu(){

	struct Env* current_env = NULL;
	for(int i=0;i<num_of_ready_queues;i++)
	    {

	        LIST_FOREACH(current_env,&env_ready_queues[i])
	        {
	        	recent_cpu_clac(current_env);
	        }

	    }
}


void print_quantums(){
	//cprintf("quantums[0] %d in ticks %d  load_avg %d",quantums[0],ticks,load_avg);
}


int ticks_every_second=0;

bool in_second(){

	print_quantums();
	if((ticks_every_second*quantums[0]) >=1000 && ticks_every_second!=0){
		ticks_every_second=0;
		return 1;
	}
	else
		return 0 ;

}


void update_load(){

	load_avg = fix_add(fix_mul(fix_frac(59, 60), load_avg),fix_mul(fix_frac(1, 60), fix_int(get_num_process())));

}


void add_1(){
	curenv->recent_cpu100 = fix_add(curenv->recent_cpu100 , fix_int(1));
}

void add_1_tick(){
	ticks_every_second++;
}

//========================================
// [8] Clock Interrupt Handler
//	  (Automatically Called Every Quantum)
//========================================
void clock_interrupt_handler()
{

	if(isSchedMethodRR())
		return;
	//TODO: [PROJECT'23.MS3 - #5] [2] BSD SCHEDULER - Your code is here
	{

		// update recent every tick

		add_1_tick();
		add_1();
		// each second update recent for all_proccess && update Load_avg
		print_quantums();
		if(in_second() == 1 /*timer_ticks () % TIMER_FREQ == 0*/){

						//load_avg
			update_load();
			//recent_cpu for running proceess
			recent_cpu_clac(curenv);
			// recent_cpu for all process except running
			recalc_recent_cpu();

		}
		// for every 4 ticks calculate the priority
		if(ticks%4 == 0 && ticks!=0){

			// calculate priority for the running && the curnenv position will be updated in calling fos_schedular
			bool new = new_priority_clac(curenv);
			// recalculate for all process except the running
			recalc_priority();
			//print_envs();

		}


	}


	/********DON'T CHANGE THIS LINE***********/
	ticks++ ;
	if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_TIME_APPROX))
	{
		update_WS_time_stamps();
	}
	fos_scheduler();
	/*****************************************/
}

//===================================================================
// [9] Update LRU Timestamp of WS Elements
//	  (Automatically Called Every Quantum in case of LRU Time Approx)
//===================================================================
void update_WS_time_stamps()
{
	struct Env *curr_env_ptr = curenv;

	if(curr_env_ptr != NULL)
	{
		struct WorkingSetElement* wse ;
		{
			int i ;
#if USE_KHEAP
			LIST_FOREACH(wse, &(curr_env_ptr->page_WS_list))
			{
#else
			for (i = 0 ; i < (curr_env_ptr->page_WS_max_size); i++)
			{
				wse = &(curr_env_ptr->ptr_pageWorkingSet[i]);
				if( wse->empty == 1)
					continue;
#endif
				//update the time if the page was referenced
				uint32 page_va = wse->virtual_address ;
				uint32 perm = pt_get_page_permissions(curr_env_ptr->env_page_directory, page_va) ;
				uint32 oldTimeStamp = wse->time_stamp;

				if (perm & PERM_USED)
				{
					wse->time_stamp = (oldTimeStamp>>2) | 0x80000000;
					pt_set_page_permissions(curr_env_ptr->env_page_directory, page_va, 0 , PERM_USED) ;
				}
				else
				{
					wse->time_stamp = (oldTimeStamp>>2);
				}
			}
		}

		{
			int t ;
			for (t = 0 ; t < __TWS_MAX_SIZE; t++)
			{
				if( curr_env_ptr->__ptr_tws[t].empty != 1)
				{
					//update the time if the page was referenced
					uint32 table_va = curr_env_ptr->__ptr_tws[t].virtual_address;
					uint32 oldTimeStamp = curr_env_ptr->__ptr_tws[t].time_stamp;

					if (pd_is_table_used(curr_env_ptr->env_page_directory, table_va))
					{
						curr_env_ptr->__ptr_tws[t].time_stamp = (oldTimeStamp>>2) | 0x80000000;
						pd_set_table_unused(curr_env_ptr->env_page_directory, table_va);
					}
					else
					{
						curr_env_ptr->__ptr_tws[t].time_stamp = (oldTimeStamp>>2);
					}
				}
			}
		}
	}
}
