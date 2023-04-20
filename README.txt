This are 3 seperate programs done to learn different operations that are constantly done by our operating systems. Every program is supposed to simulate what an operating system does when running throughout a period of time. It helped me understand the logic behind them and how different methods and data structures are efficiently used for the optimization of the processes.

DeadLock Detector:

This program takes in a series of requests and assignment edges. It processes them one by one, assigning hypothetical resources to hypothetical processes. An algorithm within my code was developed to determine if any of these requests causes a deadlock. At the end of program execution it will return the index of the request that caused the deadlock and which processes are involved in such deadlock.

	How to run:
	- call the makefile: 'make'
	- run with an input test file (a txt file with a series of requests, in this case test1.txt): './deadlock < test1.txt'



Task Scheduler:

This program implements a round-robin CPU scheduling simulator. The input to the simulator will be a set of processes and a time slice for the round robin scheduling. Each process is described by an id, arrival time and CPU burst. The simulator code will simulate RR scheduling on these processes and for each process it will calculate its start time and finish time. At the end of execution the program will print out a table with the history of each process (when it arrived, its total burst, when it started and finished), as well as the sequence order of processes running.

	How to run:
	- call the makefile: 'make'
	- run with an input test file (a txt file with a series of processes, in this case test1.txt): './scheduler #1 #2 < test1.txt'
		- where the input numbers are: 
			- #1 the quantum of the CPU scheduler (the time slice each process can run for until CPU switches to working on another process 
			- #4 the maximum number of processes that you want to be reported in the output sequence



Memory Management System:

This program inplements a worst-fit dynamic partition memory allocation simulator that approximates some of the functionality of malloc() and free() in the standard C library. The input to this simulator is be a page size (a positive integer) and list of allocation and deallocation requests. The simulator processes all requests and computes some statistics. At the end of program execution it will print out the total number of pages requested, the address of the maximum free partition and its size after processing every allocation and deallocation request.

	How to run:
	- call the makefile: 'make'
	- run with an input test file (a txt file with a series of allocation and deallocation requests, in this case test1.txt): './memsim # < test1.txt'
		- where input # is the page size to be used