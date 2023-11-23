#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"




struct K_heap_sh
{
	bool is_free;//4byte
	uint32 vir_addf;//4byte
	uint32 pages; //4byte
	LIST_ENTRY(K_heap_sh) prev_next_info;
};



LIST_HEAD(Heap_list, K_heap_sh);

struct Heap_list hlist;

void print_pages(struct Heap_list list)
{
	cprintf("pppppppppppppppppppppppp\n");
	struct K_heap_sh* blk ;

	cprintf("PAGE_ALLOC List:\n");
	LIST_FOREACH(blk, &list)
	{

		cprintf("(pages: %d, isFree: %d ,VA: %x)\n", blk->pages, blk->is_free,blk->vir_addf) ;
	}
	cprintf("pppppppppppppppppppppppp\n");

}


int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit) {
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	//All pages in the given range should be allocated
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	//Return:
	//	On success: 0
	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM


	// intialize
	start=(void*)daStart;
	kernel_limit=(void*)daLimit;
	brk = (void*)(daStart+initSizeToAllocate);

	//cprintf("%x <----- breakk /n",brk);



	// store blks
	void* Hlimit =ROUNDUP(kernel_limit,PAGE_SIZE);
	int k  =ROUNDUP(initSizeToAllocate,PAGE_SIZE);
	//void* l7d =ROUNDUP(initSizeToAllocate,PAGE_SIZE);
	int num_pages = k /PAGE_SIZE;

	void* i=start;
	for(int j =0 ;j<num_pages;j=j+1)
	{

		struct FrameInfo *pll=NULL;

		allocate_frame(&pll);
		pll->va = (int)i;
		map_frame(ptr_page_directory,pll,(int)i,PERM_WRITEABLE | PERM_PRESENT);
		i+=PAGE_SIZE;
		//				cprintf("%x<-- \n , ",i);
	}
//
//	for(void* i=start;i<;i=i+PAGE_SIZE)
//	{
//
//		struct FrameInfo *pll=NULL;
//
//		allocate_frame(&pll);
//		pll->va = (int)i;
//		map_frame(ptr_page_directory,pll,(int)i,PERM_WRITEABLE | PERM_PRESENT);
//
//		//				cprintf("%x<-- \n , ",i);
//	}
	initialize_dynamic_allocator(daStart,initSizeToAllocate);


//	// add page
	/*struct FrameInfo *pll=NULL;
	allocate_frame(&pll);
	pll->va = daLimit;
	map_frame(ptr_page_directory,pll,daLimit,PERM_WRITEABLE | PERM_PRESENT);*/

	print_heap();


	//cprintf("%d1212 @@@@@",sizeof(struct K_heap_sh));
	struct K_heap_sh *first_block;
	first_block = (struct K_heap_sh*) alloc_block_FF(sizeof(struct K_heap_sh));
	first_block->vir_addf = daLimit + PAGE_SIZE;
	first_block->pages = ((KERNEL_HEAP_MAX - first_block->vir_addf)>>12);
	first_block->is_free=1;

	LIST_INIT(&hlist);
	LIST_INSERT_HEAD(&hlist, first_block);


	print_heap();

	cprintf("\n all pages =>  %d \n",first_block->pages );


	//panic("not implemented yet");
	return 0;
}



//

bool sbr_in =0;
void* sbrk(int increment)
{
    //TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
    /* increment > 0: move the segment break of the kernel to increase the size of its heap,
     * 				you should allocate pages and map them into the kernel virtual address space as necessary,
     * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
     * increment = 0: just return the current position of the segment break
     * increment < 0: move the segment break of the kernel to decrease the size of its heap,
     * 				you should deallocate pages that no longer contain part of the heap as necessary.
     * 				and returns the address of the new break (i.e. the end of the current heap space).
     *
     * NOTES:
     * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
     * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
     * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
     * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
     */
/*	cprintf("%x <---- obba brk \n\n", brk);
	if(brk == (void*)-1){
		cprintf("????? \n");
		brk =(void*)0xF6000000;

	}*/

	/*if(sbr_in){
		brk;
		sbr_in=0;
		return brk;
	}*/

	sbr_in=1;
	if(increment==0){
		//cprintf("????? \n");
		sbr_in=0;
				return brk;
			}

	//cprintf("_____________sbrk in kernel-______________\n");
	int move_size=ROUNDUP(increment,PAGE_SIZE);
	//print_pages(hlist);
	//	void* new_brk=brk+move_size;

		if(brk + move_size>=kernel_limit)
		{panic("------------------------>size too large");}

		if(increment>0)
		{


			//cprintf("_____________sbrk in kernel-______________\n");
			for(void*i=brk;i<brk + move_size;i=i+PAGE_SIZE){
				struct FrameInfo *pll=NULL;
					allocate_frame(&pll);
					pll->va = (int)i;
					map_frame(ptr_page_directory,pll,(int)i,PERM_WRITEABLE | PERM_PRESENT);

					//print_heap();

				}
			//cprintf("brk -->%x    old brk----> \n",brk,brk-move_size);
			//cprintf("brk -->%x    old brk----> \n",brk,brk-move_size);
			sbr_in=0;
			brk = brk+move_size;
			return brk-move_size;

		}
		if(increment<0){


			increment=increment*-1;
//
//			if(increment<=PAGE_SIZE){
//
//				brk = new_brk-increment;
//				return new_brk-increment; 
//
//			}


			//int move_size=ROUNDUP(increment,PAGE_SIZE);
			//void* new_brk=brk-increment;
			for(void*i=brk;i>=brk-increment;i=i-PAGE_SIZE)
			{

				unmap_frame(ptr_page_directory,(int)i);

		    }
			brk  = brk-increment;
			sbr_in=0;
			return brk-increment;

		}
		return 0;

    //MS2: COMMENT THIS LINE BEFORE START CODING====
 //   panic("not implemented yet");
}
















//=================================================================================//
//=================================================================================//
//=================================================================================//
//=================================================================================//
//=================================================================================//
//=================================================================================//
//============================== @Author: shehapooo ===============================//
//=================================================================================//
//=================================================================================//
//=================================================================================//
//=================================================================================//
//=================================================================================//
//=================================================================================//
void* kmalloc(unsigned int size) {
    //TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
    //refer to the project presentation and documentation for details
    // use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

    //             2kb
	// cprintf("brk ---> %x\n",brk);

    if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
       // cprintf("1st\n");
       // print_heap();
        //cprintf("%d <--- needed size",size);
       if( isKHeapPlacementStrategyFIRSTFIT()){
       void* ptra = alloc_block_FF(size);
       //cprintf("%x <--- alloc ", ptra);
       //cprintf("%x <--- brk ", brk);
      // print_heap();
       return ptra;
       }


    }
    struct K_heap_sh *page;
//    cprintf("\n fffffffffffffffffffffffffffffffffff \n");
//
//        LIST_FOREACH(page, &hlist){
//        	cprintf("free %d \n",page->is_free);
//        	cprintf("size %d \n",page->pages);
//        	cprintf("vi %x \n",page->vir_addf);
//
//        }
//        cprintf("\n fffffffffffffffffffffffffffffffffff \n");

    //print_pages(hlist);

    //cprintf("2nd \n");
    size = ROUNDUP(size, PAGE_SIZE);

    int needed_pages = size / PAGE_SIZE;

	//cprintf("\n- pages =>  %d \n", needed_pages);

    LIST_FOREACH(page, &hlist)if (page->is_free == 1 && page->pages >= needed_pages) {
    	//cprintf("\n iam here \n");
    	uint32 i = page->vir_addf;
        for ( uint32 g=needed_pages; g > 0; i = i + PAGE_SIZE, g--) {

            struct FrameInfo *pll = NULL;

            allocate_frame(&pll);
            pll->va=i;
            map_frame(ptr_page_directory, pll, (int) i, PERM_WRITEABLE | PERM_PRESENT);

        }
        page->is_free = 0;
    	if(page->pages == needed_pages)
    		return (void *)page->vir_addf;

    	//print_pages(hlist);
    	struct K_heap_sh *new_one=NULL;
    	//cprintf("OBBBBA \n");
    	//print_heap();
   		new_one = (struct K_heap_sh*) alloc_block_FF(sizeof(struct K_heap_sh));
   		//cprintf("OBBBBA \n");
   		new_one->vir_addf = i;
   		new_one->pages=page->pages - needed_pages;
  		new_one->is_free=1;
 		LIST_INSERT_AFTER(&hlist,page, new_one);
 		page->pages = needed_pages;
    	page->is_free = 0;
 		//cprintf("after allocation",size);
 		//print_pages(hlist);
    	//page->is_free = 0;

		return  (void *)page->vir_addf;
    }
        return NULL;
}
















void kfree(void* virtual_address)
{

	//print_pages(hlist);

    //TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
    //refer to the project presentation and documentation for details
    // Write your code here, remove the panic and write your code
    //panic("kfree() is not implemented yet...!!");
	cprintf("needed_free_virtual_address->%x \n",virtual_address);
	if(virtual_address>=(void*)KERNEL_HEAP_START && virtual_address<=kernel_limit ){


		//cprintf("fffffffffffffffffffffffff\n");
		//cprintf("fffffffffffffffffffffffff\n");
		//cprintf("fffffffffffffffffffffffff\n");

		//print_heap();
		free_block(virtual_address);
		//cprintf("AFTER \n");
		//print_heap();
		return ;

	}

	if(virtual_address>= kernel_limit&& virtual_address<=(void*)KERNEL_HEAP_MAX){

		 struct K_heap_sh *cur_free;
		LIST_FOREACH(cur_free, &hlist){

			//serach for exact virtual address
			if((void*)cur_free->vir_addf==virtual_address)
				break;
		}




		struct K_heap_sh *cur_free_prev=NULL;
		struct K_heap_sh *cur_free_next=NULL;


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

				uint32 frame_mapped = cur_free->vir_addf ;
				for(int i=0;i<cur_free->pages;i++)
				{
					unmap_frame(ptr_page_directory,frame_mapped);
					frame_mapped+=PAGE_SIZE;

				}

			/*	uint32 framee_mapped = cur_free_next->vir_addf ;
				for(int i=0;i<cur_free_next->pages;i++)
				{
					unmap_frame(ptr_page_directory,framee_mapped);
					frame_mapped+=PAGE_SIZE;


				}*/
				cur_free_next->pages=0;
				cur_free->pages=0;

				void*ptr = cur_free_next;
				void*ptr2 = cur_free;
				free_block(ptr);
				free_block(ptr2);

				LIST_REMOVE(&hlist,cur_free_next);
				LIST_REMOVE(&hlist,cur_free);


		}
		  //case 2 next  is_free
		else if(cur_free_next!=NULL && cur_free_next->is_free){


			//cprintf("next\n");
			cur_free->pages=cur_free->pages+cur_free_next->pages;
			cur_free->is_free=1;
			cur_free_next->is_free=0;



			uint32 frame_mapped = cur_free->vir_addf ;
			for(int i=0;i<cur_free->pages;i++)
			{

					unmap_frame(ptr_page_directory,frame_mapped);
					frame_mapped+=PAGE_SIZE;

			}
			cur_free_next->pages=0;
			void*ptr = cur_free_next;
			free_block(ptr);
			LIST_REMOVE(&hlist,cur_free_next);
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



			//cprintf("did i 1 \n");

			uint32 frame_mapped = cur_free->vir_addf ;
			for(int i=0;i<cur_free->pages;i++)
			{
					unmap_frame(ptr_page_directory,frame_mapped);
					frame_mapped+=PAGE_SIZE;

			}

			//cprintf("did i 2 \n");
			cur_free->pages=0;
			void*ptr = cur_free;
			//cprintf("%x <------ptr \n",ptr);
			//cprintf("%x <------ptr-16 \n",ptr-16);
			print_heap();

			free_block(ptr);

			print_heap();
			LIST_REMOVE(&hlist,cur_free);


		}
		else{

			uint32 frame_mapped = cur_free->vir_addf ;
			for(int i=0;i<cur_free->pages;i++)
			{
					//uint32 ph = kheap_physical_address(frame_mapped);

					//unmap remove frame
					unmap_frame(ptr_page_directory,frame_mapped);
					frame_mapped+=PAGE_SIZE;


					//cprintf("%x <---framed to virual ",to_frame_info(ph)->va);
			}


			cur_free->is_free=1;
		}



		return;
	}




	panic("invalid address");


}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
    //TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
    //refer to the project presentation and documentation for details
    // Write your code here, remove the panic and write your code
	 // panic("kheap_virtual_address() is not implemented yet...!!");

    //EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	//cprintf("hhhhh-%x -hhhhhhhh\n",physical_address);
	//cprintf("HELOOOO \n");
	uint32 off = (physical_address & 0x00000FFF);
	uint32 frm_ph = (physical_address & 0xFFFFF000);
	struct FrameInfo *pll =to_frame_info(physical_address);
	//cprintf("%x <--- va frame \n",pll->va);
	if( pll->va ==0 )
		return 0;
	uint32 va = pll->va + off;
	/*uint32 offset = j;
	allPAs[i] = (ptr_table[j] & 0xFFFFF000) + offset;*/


    //cprintf("1 -- virtual-> %x  physical-> %x \n",va,physical_address);


    //change this "return" according to your answer
    return va;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
    //TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
    //refer to the project presentation and documentation for details
    // Write your code here, remove the panic and write your code
  //  panic("kheap_physical_address() is not implemented yet...!!");

	//(ptr_table[j] & 0xFFFFF000)+(va & 0x00000FFF)
//cprintf("hhhhh-%x -hhhhhhhh\n",virtual_address);
		uint32 *ptr_t = NULL;
	    int ret = get_page_table(ptr_page_directory,virtual_address,&ptr_t);

	    uint32 table_entry = ptr_t[PTX(virtual_address)];

	    uint32 physical_addr =  (table_entry & 0xFFFFF000) + (virtual_address & 0x00000FFF);

	    //cprintf("virtual-> %x  physical-> %x \n",virtual_address,physical_addr);
	    return physical_addr;

}


void kfreeall()
{
    panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
    panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
    panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
    //TODO: [PROJECT'23.MS2 - BONUS] [1] KERNEL HEAP - krealloc()
    // Write your code here, remove the panic and write your code
    return NULL;
    panic("krealloc() is not implemented yet...!!");
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
