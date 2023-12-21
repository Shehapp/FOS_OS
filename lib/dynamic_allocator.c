/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->size ;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->is_free ;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockMetaData* blk ;

	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{

		cprintf("(size: %d, isFree: %d ,points_to: %x)\n", blk->size, blk->is_free,blk) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================

//declare the list
struct MemBlock_LIST heap;

void declare_heap(struct BlockMetaData* first_block){

	LIST_INIT(&heap);
	LIST_INSERT_HEAD(&heap, first_block);
}



bool is_initialized=0;

void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{

	//cprintf("%d,,,.,.,,.,\n",((KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE));

	//brk = (void*)(daStart+initSizeOfAllocatedSpace);
	if (initSizeOfAllocatedSpace == 0)
			return ;

	is_initialized=1;
	//declare the block
	struct BlockMetaData *first_block ;
	//LIST_INIT(&first_block);
	//cprintf("dastart -> %x \n",daStart);
	first_block=(struct BlockMetaData *)daStart;


	first_block->is_free=1;
	first_block->size=initSizeOfAllocatedSpace;
	//cprintf("Done\n");
	declare_heap(first_block);


//	struct BlockMetaData *blk;



//
//	LIST_FOREACH(blk, &heap)
//	{
//		// 1 0
//		// 2 4
//		// 3 4
//		void * cur =(void*)blk;
//
//		cprintf("%x \n",blk+36+sizeOfMetaData()/16);
//		cprintf("%x  blk \n",blk);
//		cprintf("%x  blk+1\n",blk+1);
//
//		cprintf("%x cur + meta \n",cur+sizeOfMetaData());
//		cprintf("%x  blk+sizeOfMetaData()\n",blk+sizeOfMetaData());
//		cprintf("%x  blk+sizeOfMetaData()/16\n",blk+(sizeOfMetaData()/16));
//		cprintf("%x  blk+sizeOfMetaData()+16\n",blk+sizeOfMetaData()+16);
//		cprintf("%x \n",daStart);
//
//	}
	//print_blocks_list(heap);
	//
	// 1  2  3  4  5  6
	//=========================================
	//DON'T CHANGE THESE LINES=================

	//=========================================
	//=========================================


	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
	//panic("initialize_dynamic_allocator is not implemented yet");
}


void print_heap(){

	print_blocks_list(heap);
}



//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:


/*
 *
 * */
//=========================================
void *alloc_block_FF(uint32 size)
{




		//cprintf("%d,,,.,.,,.,\n",((KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE));
		//cprintf("%d,,,.,.,,.,\n",((dlimit-KERNEL_HEAP_START)/PAGE_SIZE));



	int tot_size=size+sizeOfMetaData();
	//cprintf("ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss \n");


	int tot_size=size+sizeOfMetaData();

	/*int negative = -1;
	int fonund = 0;*/
	/*cprintf("+++++++++");
	print_blocks_list(heap);
	cprintf("%d needed size \n",size);*/
	if(size==0){
		return NULL;
	}

		if(!is_initialized){


		//cprintf("@YWA\n");
		uint32 required_size = size + sizeOfMetaData();

		//cprintf("%x  %d<----- our brk",sbrk(0),sbrk(0));
		uint32 da_start = (uint32)sbrk(required_size);

		//cprintf("1\n");
		uint32 da_break = (uint32)sbrk(0);

		//cprintf("2\n");
		//cprintf("%x---- %x---- %d \n",da_start,da_break,da_break-da_start);
		initialize_dynamic_allocator(da_start,da_break-da_start);

		//cprintf("done\n");

	}


	    struct BlockMetaData* blk ;
		LIST_FOREACH(blk, &heap)
		{
			if(blk->is_free && size==(blk->size-sizeOfMetaData())){


							blk->is_free=0;
							return (blk+(sizeOfMetaData()/16));

			}


			if(blk->is_free&& (blk->size-sizeOfMetaData()-size)<=16 && (blk->size-sizeOfMetaData()-size)>=0){

				blk->is_free=0;
				return (blk+(sizeOfMetaData()/16));

			}


			else if(blk->is_free && tot_size<(blk->size-sizeOfMetaData())){

				struct BlockMetaData *new_blk;

				void * cur =(void*)blk;
				new_blk=cur+(size+sizeOfMetaData());
				new_blk->is_free=1;
				new_blk->size=blk->size-(size+sizeOfMetaData());
				// filled
				blk->size=tot_size;//+sizeOfMetaData();
				blk->is_free=0;
				LIST_INSERT_AFTER(&heap,blk,new_blk);
				//print_blocks_list(heap);
				//print_blocks_list(heap);
				return (blk+(sizeOfMetaData()/16));

			}


		}

		void* brk = sbrk(tot_size);

		if(brk==(void *)-1){

			return NULL;
		}
		else{

			//return brk;
		struct BlockMetaData *need_b;
		struct BlockMetaData *new_b;
						// address
		need_b=brk;
						// empty
		need_b->is_free=0;
		need_b->size=(tot_size);

		new_b=brk+(tot_size);
		new_b->is_free=1;
		new_b->size=4096-(tot_size);

		LIST_INSERT_AFTER(&heap,  LIST_LAST(&heap), need_b);
		LIST_INSERT_AFTER(&heap,  LIST_LAST(&heap), new_b);

		//print_blocks_list(heap);
		//print_blocks_list(heap);
		//return alloc_block_FF(size) ;
		//cprintf("size - > time %d",LIST_SIZE(&heap));

		return (need_b+(sizeOfMetaData()/16));

		}




	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
//	panic("alloc_block_FF is not implemented yet");

		return NULL;
}


//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
//@author: shahppppp
void *alloc_block_BF(uint32 size) {
    //TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()

    uint64 tot_size = size + sizeOfMetaData();
    int meta_size = sizeOfMetaData() / 16;


    if (size == 0) {
        return NULL;
    }

    struct BlockMetaData *blk;
    uint64 mn = 1000000000;


    uint8 io=-1;
    int which=-1;
    int i=0;
    LIST_FOREACH(blk, &heap)
    {


//    	cprintf("size => %d \n",blk->size);
        if (blk->is_free && size == (blk->size - sizeOfMetaData())) {

            blk->is_free = 0;
            return (blk + meta_size);

        }

        else if(blk->is_free&& (blk->size-sizeOfMetaData()-size)<=16 && (blk->size-sizeOfMetaData()-size)>=0 && blk->size <= mn){
        	mn = blk->size;
            which=i;
            io=0;



		}
        else if (blk->is_free && tot_size <= (blk->size - sizeOfMetaData()) && blk->size <= mn) {
            mn = blk->size;
            which=i;
            io=1;
        }
        i++;
    }

    if (which >-1) {
        struct BlockMetaData *new_blk;
        i=0;
        LIST_FOREACH(blk, &heap)
        {
            if(which==i){
                struct BlockMetaData *new_blk;

                if(io==0){
                	blk->is_free=0;
                	return (blk + meta_size);

                }

                void *cur = (void *) blk;
                new_blk = cur + tot_size;
                new_blk->is_free = 1;
                new_blk->size = blk->size - tot_size;
                blk->size = tot_size;
                blk->is_free = 0;
                LIST_INSERT_AFTER(&heap, blk, new_blk);
                return (blk + meta_size);
            }
            i++;
        }

    }


//	panic("alloc_block_BF is not implemented yet");
    return NULL;
}

//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()
	//panic("free_block is not implemented yet");


	//print_blocks_list(heap);



/*	const char *src = "SDHGHHK_23waljhKGKsadh,ODa";
	char* dst=(char*)0xf0010000;
	str2lower(dst,src);
	cprintf("%s \n",dst);
	panic("shehapooooo");*/


/*	const char *src = "SDHGHHK_23waljhKGKsadh,ODa";
	char* dst=(char*)0xf0010000;
	str2lower(dst,src);
	cprintf("%s \n",dst);
	panic("shehapooooo");*/





	//cprintf("Before \n");
	//print_blocks_list(heap);

	//cprintf(" our blk in va %x \n",va);



	//cprintf("Before \n");
	//print_blocks_list(heap);

	//cprintf(" our blk in va %x \n",va);


	struct BlockMetaData *cur_free = ((struct BlockMetaData *)va - 1) ;

	//cprintf(" our blk in %d",get_block_size(va));
	//cprintf(" our blk in %x",cur_free);


	struct BlockMetaData *cur_free_prev=NULL;
	struct BlockMetaData *cur_free_next=NULL;


	if(cur_free->prev_next_info.le_next!=NULL){

		cur_free_next=LIST_NEXT(cur_free);
	}


	if(cur_free->prev_next_info.le_prev != NULL){

		cur_free_prev=LIST_PREV(cur_free);
	}



	//cprintf("p %x  fs %x nx %x ",cur_free_prev,cur_free,cur_free_next);

	//panic("stop");

	if(cur_free_prev !=NULL && cur_free_next!=NULL && cur_free_next->is_free && cur_free_prev->is_free) {



		//print_blocks_list(heap);
		//cprintf("both \n");



			//case 1 next&prev  is_free
			cur_free_prev->size=cur_free_prev->size + cur_free->size +cur_free_next->size;
			cur_free_prev->is_free=1;
			cur_free_next->is_free=0;
			cur_free_next->size=0;
			cur_free->is_free=0;
			cur_free->size=0;



			LIST_REMOVE(&heap,cur_free_next);
			LIST_REMOVE(&heap,cur_free);




	}
	else if(cur_free_next!=NULL && cur_free_next->is_free){



		//cprintf("next \n");


		//print_blocks_list(heap);
	    //case 2 next  is_free
		cur_free->size=cur_free->size+cur_free_next->size;
		cur_free->is_free=1;
		cur_free_next->is_free=0;
		cur_free_next->size=0;


		LIST_REMOVE(&heap,cur_free_next);
		//print_blocks_list(heap);

		//panic("stop");

	}
	else if(cur_free_prev!=NULL && cur_free_prev->is_free){


		//cprintf("pre \n");

		//case 3 prev  is_free
		cur_free_prev->size=cur_free_prev->size+cur_free->size;
		cur_free_prev->is_free=1;
		cur_free->is_free=0;
		cur_free->size=0;
		LIST_REMOVE(&heap,cur_free);



	}
	else{

		//cprintf("concl \n");
		cur_free->is_free=1;
	}


	//cprintf("after \n");
	//print_blocks_list(heap);



}

//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:

/*
 * *** if new_size > cur_size
			if(free momken 2a5od mnoh)
				if(lw 2l 2l ba2y <=meta_data_size)
					//5od 2l size bta3 next koloo

			 	else
			 		//merge w  and create new blk



		else

 *
 * */
//=========================================
void *realloc_block_FF(void* va, uint32 new_size) {
    //TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()


    if (new_size == 0) {
        free_block(va);
        return NULL;
    }
    if (va == NULL) {
        return alloc_block_FF(new_size);
    }


    struct BlockMetaData *blk = ((struct BlockMetaData *) va - 1);
    uint32 free_size = blk->size - sizeOfMetaData();
    uint32 meta_size = sizeOfMetaData() / 16;

    // case:1#  if new_size is equal cur_size
    if (new_size == free_size){
    	//cprintf("MOMKEN1A7a\n");
        return blk + meta_size;}

    // case:2#  if new_size is less than cur_size
    if (free_size > new_size) {
        // case:2.1#  if i there is no free space to store meta_data or store meta_date with size 0

        if (free_size - new_size <= sizeOfMetaData() && (blk->prev_next_info.le_next==NULL ||blk->prev_next_info.le_next->is_free==0) ) {
            //make it free and send to ff

        	//cprintf("MOMKEN1\n");
        	return blk+meta_size;

            /*free_block(va);
           return alloc_block_FF(new_size);*/
        }


        // f
        // 30


        // case:2.2#  if i there are spaces

        // update cur size
        blk->size = new_size+sizeOfMetaData();

        // add new free block
        struct BlockMetaData *new_blk;
        void *cur = (void *) blk;
        new_blk = cur + blk->size;
        new_blk->is_free = 1;
        new_blk->size = free_size - (new_size);
        LIST_INSERT_AFTER(&heap, blk, new_blk);
        //blk->is_free=1;


    	//cprintf("MOMKEN2\n");
        free_block(new_blk+1);

       // free_block(new_blk);

        return (blk + meta_size);

    }

    // case:3#  if new_size is greater than cur_size
    if (free_size < new_size) {


        // case:3.1#  if i can't use next block
        if (blk->prev_next_info.le_next == NULL ||
            blk->prev_next_info.le_next->is_free == 0 ||
            blk->prev_next_info.le_next->size + free_size < new_size
                ) {
        	//cprintf("momkensh 3.1\n");
        	//notlb size 3ndo w ba2y meta ;
            //make it free and send to ff
            uint32* destination= alloc_block_FF(new_size);
            if(destination==NULL)
            	return NULL;
            memcpy(destination, va, blk->size);

            free_block(va);
            return destination;
        }

        if((blk->prev_next_info.le_next->size + free_size >= new_size &&
             blk->prev_next_info.le_next->size + free_size - new_size <= sizeOfMetaData())){

        	//notlb size 3ndo w ba2y meta ;
        	//cprintf("momkensh 3.1.1\n");
        	blk->size+=blk->prev_next_info.le_next->size;
        	blk->prev_next_info.le_next->size=0;
            blk->prev_next_info.le_next->is_free=0;
  	        LIST_REMOVE(&heap,blk->prev_next_info.le_next);
  	        return (blk + meta_size);

        }


        // case:3.2#  if there are spaces

        // update cur size
        blk->size = new_size+sizeOfMetaData();

        // add new free block
        struct BlockMetaData *new_blk;
        void *cur = (void *) blk;
        new_blk = cur + blk->size;
        new_blk->is_free = 1;
        new_blk->size = free_size+blk->prev_next_info.le_next->size -new_size;
        blk->prev_next_info.le_next->size=0;
        blk->prev_next_info.le_next->is_free=0;
        LIST_REMOVE(&heap,blk->prev_next_info.le_next);
        LIST_INSERT_AFTER(&heap, blk, new_blk);
        //cprintf("momkensh 3.2\n");
        return (blk + meta_size);

    }

//	panic("realloc_block_FF is not implemented yet");
    return NULL;
}
