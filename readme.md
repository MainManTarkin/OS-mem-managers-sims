# Simulated Memory Managers

## Project Description 


This program was created for Cathage's operating systems class. The guidelines for the project were to create three
different types of memory managers. A __slab__ allocater that allocates a fixed size block of bytes, a __best fit__ allocator that looks for a free block that is closest to the requested size, and a next fit allocator that returns the next most free block.

----
## Build

Build for a posix compliant system.
all three files are built separately.

> g++ best.cpp -o best.a 

> g++ next.cpp -o next.a

> g++ slab.cpp -o slab.a

----
## Best Fit Program

The best fit program as stated in the description above looks for a free block that closest to the request size. 
Since this a simulated memory manager, the program takes commands from STDIN to allocate and free blocks of memory.

----
## Next Fit Program

The next fit program as stated in the project description looks for the nearest free memory block that meets the requested size.
Just like the __Best Fit Program__, this is simulated and is controlled using commands from STDIN.

----

## Running The Best Fit And Next Fit Program

the progam is ran with the following arguments:

* -h : print all of the program arguments
* -k : the amount of free memory are simulated memory manger has to work with

## Best Fit And Next Fit Program Commands

the following commands to control the simulation:

* -a : used to allocate a block of memory and should be followed by the number of bytes the allocation should be
* -f : used to free a block of memory should be followed by the _address_ of the block that you want to free
* -p : prints all the blocks of memory that the virtual memory manger has
* -q : end the simulation and thus the program

----

## Slab Program

The slab as stated in the aboce project description allocates fixed byte size blocks of 512 bytes. This program does not feature any
arguments or commands. It is simulated only withen the __main()__ function block that print all results of the test to the standard output.