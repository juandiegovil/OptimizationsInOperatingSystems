
#include "memsim.h"
#include <cassert>
#include <iostream>
#include <list>
#include <unordered_map>
#include <set>
#include <cmath>


struct Partition {
  bool free;
  int tag;
  int64_t size, addr;

  //declaring tag, size and address
  Partition (int64_t s, int64_t a){
    free = true;
    size = s;
    addr = a;
  }
};

typedef std::list<Partition>::iterator PartitionRef;

//comparison structure for set
struct scmp {
  bool operator()(const PartitionRef & c1, const PartitionRef & c2) const {
    if (c1->size == c2->size) 
      return c1->addr < c2->addr;
    else
      return c1->size > c2->size;
  }
};

// I suggest you implement the simulator as a class, like the one below.
// If you decide not to use this class, feel free to remove it.
struct Simulator {
  // all partitions, in a linked list
  std::list<Partition> all_blocks;
  // quick access to all tagged partitions
  std::unordered_map<long, std::vector<PartitionRef>> tagged_blocks;
  // sorted partitions by size/address
  std::set<PartitionRef, scmp> free_blocks;

  // initializing a pageSize variable that is remembered
  int64_t pageSize;

  // declaring the number of pages added (for final results)
  int64_t n_pages = 0;

  Simulator(int64_t page_size)
  {
    //declaring page size
    pageSize = page_size;
  }
  void allocate(int tag, int size)
  {
    //adding an initial empty block equal to pageSize
    if (all_blocks.empty()) {
      all_blocks.push_back(Partition(pageSize, 0));
      //first empty block
      free_blocks.insert(all_blocks.begin());
      n_pages++;
    }
    
    //initializing largest free block size and the partition black that we'll be working with
    int largest_size = -1;
    auto the_block = all_blocks.begin();

    if (free_blocks.empty()){
      largest_size = -1;
    }
    //using the first element of our free_blocks set if it is not empty
    else {
      the_block = *free_blocks.begin();
      largest_size = the_block->size;
    }

    //no suitable partition is found
    if (largest_size < size){

      //go to the end to add pages
      auto it = all_blocks.end();
      it--;

      //using the space available from the last block if it is free
      int64_t size_needed = size;
      if (it->free) size_needed = size - it->size;

      //number of pages needed to be added
      int64_t number_pages = std::ceil((float)size_needed/pageSize);
      n_pages += number_pages;

      //if free block at the end simply make it larger
      if (it->free){
        //erasing and adding free_block with new size
        free_blocks.erase(it);
        it->size += number_pages * pageSize;
        free_blocks.insert(it);
      }
      //else add new free block with number of pages required
      else {
        //adding new free block at the end and adding it free_blocks
        all_blocks.insert(all_blocks.end(), Partition(number_pages * pageSize, it->addr + it->size));
        it++;
        it->free = true;
        free_blocks.insert(it);
      }
      
      //changing the block that is going to be used to the new added free block
      the_block = *free_blocks.begin();
      largest_size = the_block->size;
    }

    //changing tag
    the_block->tag = tag;
    the_block->free = false;
    int64_t previous_size = the_block->size;

    //if nothing in tagged blocks add new element
    if (tagged_blocks.empty()) {
      tagged_blocks.insert({tag, {the_block}});
    }
    //else find the tag key and add another iterator to it
    else {
      auto tag_it = tagged_blocks.find(tag);

      //didn't find key so iterator will be end
      if (tag_it == tagged_blocks.end()) {
        //add tag to the the end
        tagged_blocks.insert({tag, {the_block}});
      }
      //key found, push_back an iterator
      else {
        tag_it->second.push_back(the_block);
      }
    }

    //erasing the new occupied block
    free_blocks.erase(the_block);

    //splitting the partition if there is extra space
    if (largest_size != size) {
      //new size and add empty partition at the end, has to be added to free_blocks too
      the_block->size = size;
      all_blocks.insert(std::next(the_block), Partition(previous_size - size, the_block->addr + size));
      the_block++;
      the_block->free = true;
      free_blocks.insert(the_block);
    }

  }
  void deallocate(int tag)
  {
    // Pseudocode for deallocation request:
    // - for every partition
    //     - if partition is occupied and has a matching tag:
    //         - mark the partition free
    //         - merge any adjacent free partitions

    //finding tag in tagged_blocks
    auto tag_it = tagged_blocks.find(tag);

    //found the key in tagged_blocks
    if (tag_it != tagged_blocks.end()){
      //deleting each block that is occupied by the tag we looked for
      while (!tag_it->second.empty()) {
        auto it = tag_it->second[0];
        //erasing iterator in tagged_blocks
        tag_it->second.erase(tag_it->second.begin());
        //freeing partition in all_blocks
        it->free = true;
        free_blocks.insert(it);

        //merging partition below if possible
        if (it != all_blocks.begin()){
          it--;
          auto new_it = it;
          //if free block, delete the second block after merging it with the first block
          if (it->free) {
            free_blocks.erase(it);
            it->size = it->size + std::next(it)->size;
            free_blocks.insert(it);
           
            it++;
            free_blocks.erase(it);
            all_blocks.erase(it);
            it = new_it;
          }
          else {
            it++;
          }
        }
        
        //merging partition above if possible
        if (it != --all_blocks.end()){
          //if second block free block, delete it after merging it with the first block
          if (std::next(it)->free){

            free_blocks.erase(it);
            it->size = it->size + std::next(it)->size;
            free_blocks.insert(it);
            
            it++;
            free_blocks.erase(it);
            all_blocks.erase(it);

          }
        }
      }
      //erasing tag key in tagged_blocks
      tagged_blocks.erase(tag_it);
    }

  }
  MemSimResult getStats()
  {
    MemSimResult result;
    //if no free partitions then just report 0s
    if (free_blocks.empty()) {
      result.max_free_partition_size = 0;
      result.max_free_partition_address = 0;
    }
    else {
      result.max_free_partition_size = (*free_blocks.begin())->size;
      result.max_free_partition_address = (*free_blocks.begin())->addr;
    }
    result.n_pages_requested = n_pages;
    return result;
  }
  
};

// re-implement the following function
// ===================================
// parameters:
//    page_size: integer in range [1..1,000,000]
//    requests: array of requests
// return:
//    some statistics at the end of simulation
MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests)
{
  // if you decide to use the simulator class above, you likely do not need
  // to modify the code below at all
  Simulator sim(page_size);
  for (const auto & req : requests) {
    if (req.tag < 0) {
      sim.deallocate(-req.tag);
    } else {
      sim.allocate(req.tag, req.size);
    }
    sim.check_consistency();
  }

  return sim.getStats();
}
