#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}



//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{

	return (void*) sys_sbrk(increment);
}
void print_pagesH(struct UHeap_list list)
{
	cprintf("HHHHHHHHHHHHHHHHHHHHH\n");
	struct U_heap* blk ;

	cprintf("PAGE_ALLOC List:\n");
	LIST_FOREACH(blk, &list)
	{

		cprintf("(pages: %d, isFree: %d ,VA: %x)\n", blk->pages, blk->is_free,blk->vir_addf) ;
	}
	cprintf("HHHHHHHHHHHHHHHHHHHHH\n");


}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
bool init =0;
void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0)
		return NULL ;


	//==============================================================
	//TODO: [PROJECT'23.MS2 - #09] [2] USER HEAP - malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");

	if(size<= DYN_ALLOC_MAX_BLOCK_SIZE){
	        if(sys_isUHeapPlacementStrategyFIRSTFIT()){
	            void* ret = alloc_block_FF(size);
	            return ret;
	        }
	        else if(sys_isUHeapPlacementStrategyBESTFIT()){
	            void* ret = alloc_block_BF(size);
	            return ret;
	        }
	    }


	 else{

			struct U_heap *first_block;

			if(init == 0 ){
			first_block = (struct U_heap*) alloc_block_FF(sizeof(struct U_heap));
			first_block->vir_addf = (int)sys_get_hard_limit() + PAGE_SIZE;
			first_block->pages = ((USER_HEAP_MAX - first_block->vir_addf)>>12);
			first_block->is_free=1;
			LIST_INIT(&UHlist);
			LIST_INSERT_HEAD(&UHlist, first_block);

			init = 1 ;
			}


	        struct U_heap *page;
	        uint32 rounded_size = ROUNDUP(size, PAGE_SIZE);
	        int num_of_req_pages = rounded_size/PAGE_SIZE;


	        LIST_FOREACH(page, &UHlist)if(page->is_free == 1 && page->pages >= num_of_req_pages){
	            int i = page->vir_addf;
	            i+=PAGE_SIZE*num_of_req_pages;


	        page->is_free = 0;
	        sys_allocate_user_mem(page->vir_addf,size);
	        if(page-> pages == num_of_req_pages)
	            return (void*)page->vir_addf;


	        struct U_heap *new_block;
	        new_block = (struct U_heap*) alloc_block_FF(sizeof(struct U_heap));
	        new_block->pages = page->pages - num_of_req_pages;
	        new_block->is_free = 1;
	        new_block->vir_addf = i;
	        page->pages = num_of_req_pages;

	 		LIST_INSERT_AFTER(&UHlist,page, new_block);

	        return (void*)page->vir_addf;

	            }
	        }

	return NULL;
}


//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #11] [2] USER HEAP - free() [User Side]
	// Write your code here, remove the panic and write your code


		if(virtual_address>=(void*)USER_HEAP_START && sys_get_hard_limit()>virtual_address){
			free_block(virtual_address);
			return ;
		}


		if(virtual_address>= sys_get_hard_limit()&& virtual_address<=(void*)USER_HEAP_MAX){

				 struct U_heap *cur_free;
				LIST_FOREACH(cur_free, &UHlist){

					//serach for exact virtual address
					if((void*)cur_free->vir_addf==virtual_address)
						break;
				}
				if(cur_free==NULL)
					return;
				cprintf("\n %x \n",cur_free);




				struct U_heap *cur_free_prev=NULL;
				struct U_heap *cur_free_next=NULL;


				if(cur_free->prev_next_info.le_next!=NULL){

					cur_free_next=LIST_NEXT(cur_free);
				}


				if(cur_free->prev_next_info.le_prev != NULL){

					cur_free_prev=LIST_PREV(cur_free);
				}

				//next & prev has free pages
				if(cur_free_prev !=NULL && cur_free_next!=NULL && cur_free_next->is_free && cur_free_prev->is_free) {


						//cprintf("next&prev\n");

						//case 1 next&prev  is_free
						cur_free_prev->pages=cur_free_prev->pages + cur_free->pages +cur_free_next->pages;
						cur_free_prev->is_free=1;
						cur_free_next->is_free=0;
						cur_free->is_free=0;


						//umap all unused frames
						sys_free_user_mem((int) virtual_address,cur_free->pages*PAGE_SIZE);


						cur_free_next->pages=0;
						cur_free->pages=0;

						void*ptr = cur_free_next;
						void*ptr2 = cur_free;
						free_block(ptr);
						free_block(ptr2);

						LIST_REMOVE(&UHlist,cur_free_next);
						LIST_REMOVE(&UHlist,cur_free);

				}
				  //case 2 next  is_free
				else if(cur_free_next!=NULL && cur_free_next->is_free){


					//cprintf("next\n");
					cur_free->pages=cur_free->pages+cur_free_next->pages;
					cur_free->is_free=1;
					cur_free_next->is_free=0;


					sys_free_user_mem((int) virtual_address,cur_free->pages*4096);


					cur_free_next->pages=0;
					void*ptr = cur_free_next;
					free_block(ptr);
					LIST_REMOVE(&UHlist,cur_free_next);
					//print_blocks_list(heap);

					//panic("stop");

				}

				else if(cur_free_prev!=NULL && cur_free_prev->is_free){


					//cprintf("prev\n");
					//cprintf("pre \n");

					//case 3 prev  is_free
					cur_free_prev->pages=cur_free_prev->pages+cur_free->pages;
					cur_free_prev->is_free=1;
					cur_free->is_free=0;
					//cur_free->size=0;


					sys_free_user_mem((int) virtual_address,cur_free->pages*4096);


					cur_free->pages=0;
					void*ptr = cur_free;
					//cprintf("%x <------ptr \n",ptr);
					//cprintf("%x <------ptr-16 \n",ptr-16);
					//print_heap();

					free_block(ptr);

					//print_heap();
					LIST_REMOVE(&UHlist,cur_free);


				}
				else{


					//cprintf("%d free now ",sys_calculate_free_frames());
					//cprintf("%d",cur_free->pages);
					sys_free_user_mem((int) virtual_address,cur_free->pages*4096);
					//cprintf("%d free now ",sys_calculate_free_frames());
					cur_free->is_free=1;
				}



				return;
			}


		panic("invalid address");
}



//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
