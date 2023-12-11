/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE) {
    assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
    _PageRepAlgoType = LRU_TYPE;
}

void setPageReplacmentAlgorithmCLOCK() { _PageRepAlgoType = PG_REP_CLOCK; }

void setPageReplacmentAlgorithmFIFO() { _PageRepAlgoType = PG_REP_FIFO; }

void setPageReplacmentAlgorithmModifiedCLOCK() { _PageRepAlgoType = PG_REP_MODIFIEDCLOCK; }

/*2018*/ void setPageReplacmentAlgorithmDynamicLocal() { _PageRepAlgoType = PG_REP_DYNAMIC_LOCAL; }

/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps) {
    _PageRepAlgoType = PG_REP_NchanceCLOCK;
    page_WS_max_sweeps = PageWSMaxSweeps;
}


//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE) { return _PageRepAlgoType == LRU_TYPE ? 1 : 0; }

uint32 isPageReplacmentAlgorithmCLOCK() {
    if (_PageRepAlgoType == PG_REP_CLOCK) return 1;
    return 0;
}

uint32 isPageReplacmentAlgorithmFIFO() {
    if (_PageRepAlgoType == PG_REP_FIFO) return 1;
    return 0;
}

uint32 isPageReplacmentAlgorithmModifiedCLOCK() {
    if (_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1;
    return 0;
}

/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal() {
    if (_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1;
    return 0;
}

/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK() {
    if (_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1;
    return 0;
}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt) { _EnableModifiedBuffer = enableIt; }

uint8 isModifiedBufferEnabled() { return _EnableModifiedBuffer; }

void enableBuffering(uint32 enableIt) { _EnableBuffering = enableIt; }

uint8 isBufferingEnabled() { return _EnableBuffering; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length; }

uint32 getModifiedBufferLength() { return _ModifiedBufferLength; }

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env *curenv, uint32 fault_va) {
    //panic("table_fault_handler() is not implemented yet...!!");
    //Check if it's a stack page
    uint32 *ptr_table;
#if USE_KHEAP
    {
        ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
    }
#else
    {
        __static_cpt(curenv->env_page_directory, (uint32) fault_va, &ptr_table);
    }
#endif
}

//Handle the page fault

void page_fault_handler(struct Env *curenv, uint32 fault_va) {


#if USE_KHEAP
    struct WorkingSetElement *victimWSElement = NULL;
        uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));


#else
    int iWS = curenv->page_last_WS_index;
    uint32 wsSize = env_page_ws_get_size(curenv);
#endif

    if (wsSize < (curenv->page_WS_max_size) && !(isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))) {
        //cprintf("PLACEMENT=========================WS Size = %d\n", wsSize );
        //TODO: [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement
        // Write your code here, remove the panic and write your code
    	//cprintf("\n ws list not full \n");



        struct FrameInfo *pll = NULL;
        allocate_frame(&pll);
        map_frame(curenv->env_page_directory, pll, fault_va, PERM_USER | PERM_WRITEABLE);
        int ret = pf_read_env_page(curenv, (void *) fault_va);
        if (ret == E_PAGE_NOT_EXIST_IN_PF) {
            if ((fault_va >= USTACKBOTTOM && fault_va < USTACKTOP) ||
                (fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX)) {

                map_frame(curenv->env_page_directory, pll, fault_va, PERM_USER | PERM_WRITEABLE);

               pll->va = fault_va;

            } else {
                sched_kill_env(curenv->env_id);
            }
          } else {


            map_frame(curenv->env_page_directory, pll, fault_va, PERM_USER | PERM_WRITEABLE);
            pll->va = fault_va;
        }
        struct WorkingSetElement *work = env_page_ws_list_create_element(curenv, fault_va);
//      if(curenv->page_last_WS_element == NULL){
    	  LIST_INSERT_TAIL(&curenv->page_WS_list, work);
//      }
//      else{
//
//  		cprintf("________________m4 NULL____________\n");
//    	  LIST_INSERT_BEFORE(&curenv->page_WS_list,curenv->page_last_WS_element,work);
//
//      }
        uint32 *ptr_t;
        struct FrameInfo *fr = get_frame_info(curenv->env_page_directory, fault_va, &ptr_t);
        fr->element = work;

        // env_page_ws_print(curenv);
        if (LIST_SIZE(&curenv->page_WS_list) == (curenv->page_WS_max_size) ) {
        	if( curenv->page_last_WS_element == NULL || curenv->page_last_WS_element == LIST_LAST(&curenv->page_WS_list) )
        		curenv->page_last_WS_element = LIST_FIRST(&curenv->page_WS_list);

        	else{
        		//cprintf("________________m4 NULL____________\n");
        		//curenv->page_last_WS_element = LIST_NEXT(curenv->page_last_WS_element);
        	}


        } else {

          if(curenv->page_last_WS_element == NULL)
            curenv->page_last_WS_element = NULL;
        }

        //refer to the project presentation and documentation for details
    } else {
//        cprintf("REPLACEMENT=========================WS Size = %d\n", wsSize );
        //refer to the project presentation and documentation for details
    	//cprintf("\n tarrek#1 \n");

        if (isPageReplacmentAlgorithmFIFO()) {
        	//sys_check_WS_list()
        	//cprintf(" _________IN_FIFO__________ \n");
        	//sys_check_WS_list()
        	//fault_va=ROUNDDOWN(fault_va,PAGE_SIZE);
        	//env_page_ws_print(curenv);
            //TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement
            // Write your code here, remove the panic and write your code
//            panic("page_fault_handler() FIFO Replacement is not implemented yet...!!");
			//TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement
			// Write your code here, remove the panic and write your code
			//panic("page_fault_handler() FIFO Replacement is not implemented yet...!!");

        	//cprintf(" _________y1 __________ \n");
        	struct FrameInfo *plf = NULL;
        	allocate_frame(&plf);

        	//cprintf(" _________y2 __________ \n");
        	map_frame(curenv->env_page_directory, plf, fault_va, PERM_USER | PERM_WRITEABLE);
        	int ref = pf_read_env_page(curenv, (void *) fault_va);

        	//cprintf(" _________y3 __________ \n");
        	if (ref == E_PAGE_NOT_EXIST_IN_PF)
        	{

            	//cprintf(" _________Not in disk __________ \n");
        	    if ((fault_va >= USTACKBOTTOM && fault_va < USTACKTOP)
        	            || (fault_va >= USER_HEAP_START
        	                && fault_va < USER_HEAP_MAX))
        	    {

                	//cprintf(" _________HNA1__________ \n");
        	        map_frame(curenv->env_page_directory, plf, fault_va,
        	                  PERM_USER | PERM_WRITEABLE);
        	        plf->va = fault_va;

        	    }
        	    else
        	    {
        	        sched_kill_env(curenv->env_id);
        	    }
        	}
        	else
        	{
            	//cprintf(" _________HNA2__________ \n");
        	    map_frame(curenv->env_page_directory, plf, fault_va,
        	              PERM_USER | PERM_WRITEABLE);
        	    plf->va = fault_va;
        	}
        	struct WorkingSetElement* work = env_page_ws_list_create_element(
        	                                     curenv, fault_va);

        	if (wsSize < (curenv->page_WS_max_size))
        	{


        	    LIST_INSERT_TAIL(&curenv->page_WS_list, work);

        	    if (LIST_SIZE(&curenv->page_WS_list)
        	            == (curenv->page_WS_max_size))
        	    {
        	        curenv->page_last_WS_element = LIST_FIRST(
        	                                           &curenv->page_WS_list);

        	    }
        	    else
        	    {
        	        curenv->page_last_WS_element = NULL;
        	    }

        	}
        	else
        	{

            	//cprintf(" __________ws array full _________ \n");

        	    uint32 page_permissions = pt_get_page_permissions(curenv->env_page_directory,curenv->page_last_WS_element->virtual_address);
        	    if (page_permissions & PERM_MODIFIED)
        	    {
        	        uint32 *ptr_to_remove;
        	        struct FrameInfo * frame_to_remove = get_frame_info(
        	                curenv->env_page_directory,
        	                curenv->page_last_WS_element->virtual_address,
        	                &ptr_to_remove);
        	        int ret = pf_update_env_page(curenv,
        	                                     curenv->page_last_WS_element->virtual_address,
        	                                     frame_to_remove);
        	    }

        	    if (curenv->page_last_WS_element
        	            == LIST_FIRST(&curenv->page_WS_list))
        	    {

                	//cprintf(" _________HNA5__________ \n");

                	env_page_ws_invalidate(curenv,curenv->page_last_WS_element->virtual_address);


                	unmap_frame(curenv->env_page_directory,curenv->page_last_WS_element->virtual_address);

        	       // LIST_REMOVE(&curenv->page_WS_list,
        	       //             curenv->page_last_WS_element);
        	        LIST_INSERT_HEAD(&curenv->page_WS_list, work);
        	        curenv->page_last_WS_element = LIST_NEXT(LIST_FIRST( &curenv->page_WS_list));

        	    }
        	    else if (curenv->page_last_WS_element
        	             == LIST_LAST(&curenv->page_WS_list))
        	    {
        	        //					env_page_ws_invalidate(curenv,
        	        //							curenv->page_last_WS_element->virtual_address);

                	env_page_ws_invalidate(curenv,curenv->page_last_WS_element->virtual_address);

                	unmap_frame(curenv->env_page_directory,curenv->page_last_WS_element->virtual_address);


                	//cprintf(" _________HNA6__________ \n");

        	       // LIST_REMOVE(&curenv->page_WS_list,
        	       //             curenv->page_last_WS_element);
        	        LIST_INSERT_TAIL(&curenv->page_WS_list, work);
        	        curenv->page_last_WS_element = LIST_FIRST(
        	                                           &curenv->page_WS_list);
        	    }
        	    else
        	    {

        	    	// 1 5 3 4
                	//cprintf(" _________HNA8__________ \n");

        	        struct WorkingSetElement* same_ptr;
        	        same_ptr = curenv->page_last_WS_element;
        	        LIST_INSERT_AFTER(&curenv->page_WS_list,
        	                          curenv->page_last_WS_element, work);
        	        curenv->page_last_WS_element = LIST_NEXT(LIST_NEXT(
        	                                           curenv->page_last_WS_element));
        	        //env_page_ws_invalidate(curenv,same_ptr->virtual_address);


        	        env_page_ws_invalidate(curenv,same_ptr->virtual_address);


                	unmap_frame(curenv->env_page_directory,same_ptr->virtual_address);
        	        //LIST_REMOVE(&curenv->page_WS_list, same_ptr);
        	    }
        	}

        	uint32 *ptr_t;
        	struct FrameInfo * fr = get_frame_info(curenv->env_page_directory,
        	                                       fault_va, &ptr_t);
        	fr->element = work;

        	//cprintf(" _________END_FIFO__________ \n");
        	// 1 3 4 5
        }


        if (isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX)) {

        	//fault_va=ROUNDDOWN(fault_va,PAGE_SIZE);
        	uint32 w=0;
        	struct freeFramesCounters counters = calculate_available_frames();
        		//	cprintf("Free Frames = %d : Buffered = %d, Not Buffered = %d\n", counters.freeBuffered + counters.freeNotBuffered, counters.freeBuffered ,counters.freeNotBuffered);
        		w+= counters.freeBuffered + counters.freeNotBuffered;
        		 counters = calculate_available_frames();
        			//	cprintf("================ Modified Frames = %d\n", counters.modified) ;
        			w+= counters.modified;
        	//cprintf("\n size= %d\n",w);
        	//env_page_ws_print(curenv);
        	//cprintf("\n tarrek#3 \n");
            //TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement
            // Write your code here, remove the panic and write your code
        	//cprintf("\n %d %d %d\n",curenv->page_WS_max_size,LIST_SIZE(&curenv->ActiveList),LIST_SIZE(&curenv->SecondList));
            if (curenv->page_WS_max_size == LIST_SIZE(&curenv->ActiveList) + LIST_SIZE(&curenv->SecondList)) {
                // replacement
            	//cprintf("\n tarrek#4 \n");

                // case#1: search in second list
                struct WorkingSetElement *temp = NULL;
                uint32 *ptr_t;
                // issue must set presnt
                struct FrameInfo *fr = get_frame_info(curenv->env_page_directory, fault_va, &ptr_t);
                if (fr != 0) {
                	//cprintf("\n tarrek#5 \n");
                    // set present pet
                    pt_set_page_permissions(curenv->env_page_directory, fault_va, PERM_PRESENT, 0);
                    LIST_REMOVE(&curenv->SecondList, fr->element);
                    temp = LIST_LAST(&curenv->ActiveList);
                    LIST_REMOVE(&curenv->ActiveList, temp);
                    pt_set_page_permissions(curenv->env_page_directory, temp->virtual_address, 0, PERM_PRESENT);
                    LIST_INSERT_HEAD(&curenv->SecondList, temp);
                    LIST_INSERT_HEAD(&curenv->ActiveList, fr->element);
                } else {
                    temp = LIST_LAST(&curenv->SecondList);
                    // check if modified
                    if ((temp->virtual_address & PERM_MODIFIED) ||
                    		(temp->virtual_address  >= USTACKBOTTOM && temp->virtual_address  < USTACKTOP) ||
                    		                            (temp->virtual_address  >= USER_HEAP_START && temp->virtual_address  < USER_HEAP_MAX)) {
                        pt_set_page_permissions(curenv->env_page_directory, temp->virtual_address, 0, PERM_MODIFIED);
                        fr = get_frame_info(curenv->env_page_directory, temp->virtual_address, &ptr_t);
                        pf_update_env_page(curenv, temp->virtual_address, fr);
                    }

                    // erase ws, unmap frame, set mark 0
                    LIST_REMOVE(&curenv->SecondList, temp);
                    env_page_ws_invalidate(curenv, (int) temp->virtual_address);
                    pt_set_page_permissions(curenv->env_page_directory, (int) temp->virtual_address, 0, PERM_MARK);
    				unmap_frame(curenv->env_page_directory, temp->virtual_address);

                    temp = LIST_LAST(&curenv->ActiveList);
                    LIST_REMOVE(&curenv->ActiveList, temp);
                    pt_set_page_permissions(curenv->env_page_directory, temp->virtual_address, 0, PERM_PRESENT);
                    LIST_INSERT_HEAD(&curenv->SecondList, temp);



                    // read from disk and push front in Active
                    struct FrameInfo *pll = NULL;
                    allocate_frame(&pll);
                    map_frame(curenv->env_page_directory, pll, fault_va, PERM_USER | PERM_WRITEABLE);
                    pll->va = fault_va;

                    int ret = pf_read_env_page(curenv, (void *) fault_va);
                    if (ret == E_PAGE_NOT_EXIST_IN_PF) {
                        if ((fault_va >= USTACKBOTTOM && fault_va < USTACKTOP) ||
                            (fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX)) {

                           // nothing
                        } else {
                            sched_kill_env(curenv->env_id);
                        }
                    }
                    struct WorkingSetElement *work = env_page_ws_list_create_element(curenv, fault_va);
                    LIST_INSERT_HEAD(&curenv->ActiveList, work);

                    uint32 *ptr_t;
                    struct FrameInfo *fr = get_frame_info(curenv->env_page_directory, fault_va, &ptr_t);
                    fr->element = work;

                }

            } else {
                //placement
            	//cprintf("\n tarrek#6 \n");
//            	//CASE#1
                struct WorkingSetElement *temp = NULL;
                uint32 *ptr_t;
                // issue must set presnt
                struct FrameInfo *fr = get_frame_info(curenv->env_page_directory, fault_va, &ptr_t);
                if (fr != 0) {
                	//cprintf("\n tarrek#5 \n");
                    // set present pet
                    pt_set_page_permissions(curenv->env_page_directory, fault_va, PERM_PRESENT, 0);
                    LIST_REMOVE(&curenv->SecondList, fr->element);
                    temp = LIST_LAST(&curenv->ActiveList);
                    LIST_REMOVE(&curenv->ActiveList, temp);
                    pt_set_page_permissions(curenv->env_page_directory, temp->virtual_address, 0, PERM_PRESENT);
                    LIST_INSERT_HEAD(&curenv->SecondList, temp);
                    LIST_INSERT_HEAD(&curenv->ActiveList, fr->element);
                }else{



            	if(LIST_SIZE(&curenv->ActiveList) == curenv->ActiveListSize){
                struct WorkingSetElement *temp = NULL;
            	temp = LIST_LAST(&curenv->ActiveList);
            	LIST_REMOVE(&curenv->ActiveList, temp);
            	pt_set_page_permissions(curenv->env_page_directory, temp->virtual_address, 0, PERM_PRESENT);
            	LIST_INSERT_HEAD(&curenv->SecondList, temp);

                // read from disk and push front in Active
                struct FrameInfo *pll = NULL;
                allocate_frame(&pll);
                map_frame(curenv->env_page_directory, pll, fault_va, PERM_USER | PERM_WRITEABLE);
                pll->va = fault_va;

                int ret = pf_read_env_page(curenv, (void *) fault_va);
                if (ret == E_PAGE_NOT_EXIST_IN_PF) {
                    if ((fault_va >= USTACKBOTTOM && fault_va < USTACKTOP) ||
                        (fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX)) {

                       // nothing
                    } else {
                        sched_kill_env(curenv->env_id);
                    }
                }
                struct WorkingSetElement *work = env_page_ws_list_create_element(curenv, fault_va);
                LIST_INSERT_HEAD(&curenv->ActiveList, work);
                uint32 *ptr_t;
                struct FrameInfo *fr = get_frame_info(curenv->env_page_directory, fault_va, &ptr_t);
                fr->element = work;

            	}else{
                    // read from disk and push front in Active
                    struct FrameInfo *pll = NULL;
                    allocate_frame(&pll);
                    map_frame(curenv->env_page_directory, pll, fault_va, PERM_USER | PERM_WRITEABLE);
                    pll->va = fault_va;

                    int ret = pf_read_env_page(curenv, (void *) fault_va);
                    if (ret == E_PAGE_NOT_EXIST_IN_PF) {
                        if ((fault_va >= USTACKBOTTOM && fault_va < USTACKTOP) ||
                            (fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX)) {

                           // nothing
                        } else {
                            sched_kill_env(curenv->env_id);
                        }
                    }
                    struct WorkingSetElement *work = env_page_ws_list_create_element(curenv, fault_va);
                    LIST_INSERT_HEAD(&curenv->ActiveList, work);
                    uint32 *ptr_t;
                    struct FrameInfo *fr = get_frame_info(curenv->env_page_directory, fault_va, &ptr_t);
                    fr->element = work;

            	}


            	/*
            	 *
            	 * full->f   not_full->n
            	 * a s
            	 *
            	 * #1 a_f and s_n
            	 * push new in head of a and tail of a in head of s
            	 *
            	 * #2 a_n and s_n   *****
            	 * push new in head of a
            	 *
            	 * #3 a_n and s_f
            	 * push new in head of a
            	 *
            	 */
            }
            }

            //cprintf("____DONE___ replacement \n");
            //TODO: [PROJECT'23.MS3 - BONUS] [1] PAGE FAULT HANDLER - O(1) implementation of LRU replacement
        }
    }
}

void __page_fault_handler_with_buffering(struct Env *curenv, uint32 fault_va) {
    panic("this function is not required...!!");
}
