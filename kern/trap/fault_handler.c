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


void fetch_frame_from_mem(struct Env *e, uint32 fault_va){
    struct FrameInfo *pll = NULL;
    allocate_frame(&pll);
    map_frame(e->env_page_directory, pll, fault_va, PERM_USER | PERM_WRITEABLE|PERM_MARK|PERM_PRESENT);
    pll->va = fault_va;

    int ret = pf_read_env_page(e, (void *) fault_va);
    if (ret == E_PAGE_NOT_EXIST_IN_PF) {
        if ((fault_va >= USTACKBOTTOM && fault_va < USTACKTOP) ||
            (fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX)) {

            // just nothing

        } else {
			cprintf("\n in fetch_frame_from_mem.c %xr7\n",fault_va);

            sched_kill_env(e->env_id);
        }
    }
}

int rm_ram_secList_add_disk(struct Env *e){

    struct WorkingSetElement *temp = LIST_LAST(&e->SecondList);;

    // there is no any frames in sec list
    if(temp==NULL)
    	return 0;

    remove_to_disk(e,temp);
    return 1;
}

int rm_ram_wsList_add_disk(struct Env *e){
	//TODO: remove frame from ws list
	// return 1 if you do it else return 0

	if(LIST_SIZE(&e->page_WS_list) != 0){

	      if(e->page_last_WS_element !=NULL){

		uint32 page_permissions = pt_get_page_permissions(e->env_page_directory,e->page_last_WS_element->virtual_address);
		if (page_permissions & PERM_MODIFIED)
		  {
		      uint32 *ptr_to_remove;
		      struct FrameInfo * frame_to_remove = get_frame_info(e->env_page_directory, e->page_last_WS_element->virtual_address,&ptr_to_remove);
		     int ret = pf_update_env_page(e,e->page_last_WS_element->virtual_address, frame_to_remove);
	          }
		struct WorkingSetElement* same_ptr = e->page_last_WS_element ;
		e->page_last_WS_element = LIST_NEXT(same_ptr);
		env_page_ws_invalidate(e,same_ptr->virtual_address);
		unmap_frame(e->env_page_directory,same_ptr->virtual_address);
		pt_set_page_permissions(e->env_page_directory, (int) same_ptr->virtual_address, PERM_MARK,PERM_PRESENT );



		   return 1 ;
		}
		else{
			struct WorkingSetElement* lastws = LIST_LAST(&e->page_WS_list);
		        uint32 page_permissions = pt_get_page_permissions(e->env_page_directory,lastws->virtual_address);
		        if (page_permissions & PERM_MODIFIED)
		         {
			        uint32 *ptr_to_remove;
	        	        struct FrameInfo * frame_to_remove = get_frame_info(e->env_page_directory,lastws->virtual_address, &ptr_to_remove);
		                int ret = pf_update_env_page(e,lastws->virtual_address,frame_to_remove);
		        }
						env_page_ws_invalidate(e,lastws->virtual_address);
						unmap_frame(e->env_page_directory,lastws->virtual_address);
						  pt_set_page_permissions(e->env_page_directory, (int) lastws->virtual_address, PERM_MARK,PERM_PRESENT );

				return 1 ;
		    }

		}
		else{
			return 0 ;
		  }

}

int from_sec_to_act(struct Env *e, uint32 fault_va){

    struct WorkingSetElement *temp = NULL;
    uint32 * ptr_t;
    struct FrameInfo *fr = get_frame_info(e->env_page_directory, fault_va, &ptr_t);

    if (fr != 0) {

        // set present pet
        pt_set_page_permissions(e->env_page_directory, fault_va, PERM_PRESENT, 0);
        LIST_REMOVE(&e->SecondList, fr->element);
        temp = LIST_LAST(&e->ActiveList);
        LIST_REMOVE(&e->ActiveList, temp);
        pt_set_page_permissions(e->env_page_directory, temp->virtual_address, 0, PERM_PRESENT);
        LIST_INSERT_HEAD(&e->SecondList, temp);
        LIST_INSERT_HEAD(&e->ActiveList, fr->element);

        return 1;
    }
    return 0;

}

void remove_to_disk(struct Env *e, struct WorkingSetElement *temp){

	    // there is no any frames in sec list


	    uint32 * ptr_t;
	    struct FrameInfo *fr = NULL;

	    // check if modified write it on disk
	    if ((temp->virtual_address & PERM_MODIFIED) ||
	        (temp->virtual_address >= USTACKBOTTOM && temp->virtual_address < USTACKTOP) ||
	        (temp->virtual_address >= USER_HEAP_START && temp->virtual_address < USER_HEAP_MAX)) {

	        pt_set_page_permissions(e->env_page_directory, temp->virtual_address, 0, PERM_MODIFIED);

	        fr = get_frame_info(e->env_page_directory, temp->virtual_address, &ptr_t);
	        pf_update_env_page(e, temp->virtual_address, fr);

	    }

	    // erase ws, unmap frame, set mark 0
	    env_page_ws_invalidate(e, (int) temp->virtual_address);
//	    pt_set_page_permissions(e->env_page_directory, (int) temp->virtual_address, 0, PERM_MARK);
	    unmap_frame(e->env_page_directory, temp->virtual_address);
	    pt_set_page_permissions(e->env_page_directory, temp->virtual_address, 0,PERM_PRESENT );

}


void page_fault_handler(struct Env *curenv, uint32 fault_va) {


#if USE_KHEAP
    struct WorkingSetElement *victimWSElement = NULL;
        uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));


#else
    int iWS = curenv->page_last_WS_index;
    uint32 wsSize = env_page_ws_get_size(curenv);
#endif

    // fifo placement
    if (wsSize < (curenv->page_WS_max_size) && !(isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))) {
        //TODO: [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement

        // alloc frame to this va and copy data from disk to it i its exist
        fetch_frame_from_mem(curenv, fault_va);

        struct WorkingSetElement *work = env_page_ws_list_create_element(curenv, fault_va);
        LIST_INSERT_TAIL(&curenv->page_WS_list, work);

        // if this last ws set last last_ws from null to first_ws
        if (LIST_SIZE(&curenv->page_WS_list) == (curenv->page_WS_max_size)) {
            if (curenv->page_last_WS_element == NULL ||
                curenv->page_last_WS_element == LIST_LAST(&curenv->page_WS_list)) {

                curenv->page_last_WS_element = LIST_FIRST(&curenv->page_WS_list);
            }
        }


    } else {

        // fifo replacement
        if (isPageReplacmentAlgorithmFIFO()) {

            //TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement

            // alloc frame to this va and copy data from disk to it i its exist
            fetch_frame_from_mem(curenv, fault_va);

            struct WorkingSetElement *work = env_page_ws_list_create_element(
                    curenv, fault_va);


            uint32 page_permissions = pt_get_page_permissions(curenv->env_page_directory,
                                                              curenv->page_last_WS_element->virtual_address);
            if (page_permissions & PERM_MODIFIED) {
                uint32 * ptr_to_remove;
                struct FrameInfo *frame_to_remove = get_frame_info(
                        curenv->env_page_directory,
                        curenv->page_last_WS_element->virtual_address,
                        &ptr_to_remove);
                int ret = pf_update_env_page(curenv,
                                             curenv->page_last_WS_element->virtual_address,
                                             frame_to_remove);
            }


            if (curenv->page_last_WS_element
                == LIST_FIRST(&curenv->page_WS_list)) {


                env_page_ws_invalidate(curenv, curenv->page_last_WS_element->virtual_address);

                unmap_frame(curenv->env_page_directory, curenv->page_last_WS_element->virtual_address);
         	    pt_set_page_permissions(curenv->env_page_directory, (int) curenv->page_last_WS_element->virtual_address, PERM_MARK,PERM_PRESENT );

                LIST_INSERT_HEAD(&curenv->page_WS_list, work);
                curenv->page_last_WS_element = LIST_NEXT(LIST_FIRST(&curenv->page_WS_list));

            } else if (curenv->page_last_WS_element
                       == LIST_LAST(&curenv->page_WS_list)) {

                env_page_ws_invalidate(curenv, curenv->page_last_WS_element->virtual_address);

                unmap_frame(curenv->env_page_directory, curenv->page_last_WS_element->virtual_address);
          	    pt_set_page_permissions(curenv->env_page_directory, (int) curenv->page_last_WS_element->virtual_address, PERM_MARK,PERM_PRESENT );

                LIST_INSERT_TAIL(&curenv->page_WS_list, work);
                curenv->page_last_WS_element = LIST_FIRST(
                        &curenv->page_WS_list);
            } else {

                struct WorkingSetElement *same_ptr;
                same_ptr = curenv->page_last_WS_element;
                LIST_INSERT_AFTER(&curenv->page_WS_list,
                                  curenv->page_last_WS_element, work);
                curenv->page_last_WS_element = LIST_NEXT(LIST_NEXT(
                        curenv->page_last_WS_element));

                env_page_ws_invalidate(curenv, same_ptr->virtual_address);


                unmap_frame(curenv->env_page_directory, same_ptr->virtual_address);
          	    pt_set_page_permissions(curenv->env_page_directory, (int) same_ptr->virtual_address, PERM_MARK,PERM_PRESENT );

            }


        }


        if (isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX)) {


            //TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement

            // replacement
            if (curenv->page_WS_max_size == LIST_SIZE(&curenv->ActiveList) + LIST_SIZE(&curenv->SecondList)) {

                struct WorkingSetElement *temp = NULL;

                // if not exist in sec list
                if (!from_sec_to_act(curenv,  fault_va) ){

                	 // remove frame from 2nd list ram and write it in disk qs 7 0
                	if(!rm_ram_secList_add_disk(curenv)){
                		temp = LIST_LAST(&curenv->ActiveList);
                		 LIST_REMOVE(&curenv->ActiveList, temp);
                		 remove_to_disk(curenv,temp);
                	}
                	else{

                	 // add last ws in active to head of sec
                    temp = LIST_LAST(&curenv->ActiveList);
                    LIST_REMOVE(&curenv->ActiveList, temp);
                    pt_set_page_permissions(curenv->env_page_directory, temp->virtual_address, 0, PERM_PRESENT);
                    LIST_INSERT_HEAD(&curenv->SecondList, temp);
                	}

                    // read from disk and push front in Active
                    fetch_frame_from_mem(curenv, fault_va);

                    struct WorkingSetElement *work = env_page_ws_list_create_element(curenv, fault_va);
                    LIST_INSERT_HEAD(&curenv->ActiveList, work);

                }

            } else {
                //placement
                struct WorkingSetElement *temp = NULL;
                if (!from_sec_to_act(curenv,  fault_va) ){

                    if (LIST_SIZE(&curenv->ActiveList) == curenv->ActiveListSize) {
                        struct WorkingSetElement *temp = NULL;
                        temp = LIST_LAST(&curenv->ActiveList);
                        LIST_REMOVE(&curenv->ActiveList, temp);
                        pt_set_page_permissions(curenv->env_page_directory, temp->virtual_address, 0, PERM_PRESENT);
                        LIST_INSERT_HEAD(&curenv->SecondList, temp);

                        // read from disk and push front in Active
                        fetch_frame_from_mem(curenv, fault_va);

                        struct WorkingSetElement *work = env_page_ws_list_create_element(curenv, fault_va);
                        LIST_INSERT_HEAD(&curenv->ActiveList, work);


                    } else {
                        // read from disk and push front in Active
                        fetch_frame_from_mem(curenv, fault_va);

                        struct WorkingSetElement *work = env_page_ws_list_create_element(curenv, fault_va);
                        LIST_INSERT_HEAD(&curenv->ActiveList, work);
                    }

                }
            }

        }
    }
}

void __page_fault_handler_with_buffering(struct Env *curenv, uint32 fault_va) {
    panic("this function is not required...!!");
}


