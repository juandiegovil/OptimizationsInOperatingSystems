
#include "scheduler.h"
#include "common.h"
#include <iostream>
#include <cmath>

// this is the function you should implement
//
// runs Round-Robin scheduling simulator
// input:
//   quantum = time slice
//   max_seq_len = maximum length of the reported executing sequence
//   processes[] = list of process with populated IDs, arrivals, and bursts
// output:
//   seq[] - will contain the execution sequence but trimmed to max_seq_len size
//         - idle CPU will be denoted by -1
//         - other entries will be from processes[].id
//         - sequence will be compressed, i.e. no repeated consecutive numbers
//   processes[]
//         - adjust finish_time and start_time for each process
//         - do not adjust other fields
//
void simulate_rr(
    int64_t quantum, 
    int64_t max_seq_len,
    std::vector<Process> & processes,
    std::vector<int> & seq
) {
    //initializing remaining_bursts, sequence, ready queue, job queue
    std::vector<int64_t> remaining_bursts;
    std::vector<int> sequence;
    std::vector<int> rq, jq;
    bool waiting_for_finish = false;
    bool waiting_for_arrival = false;
    //indexes will keep track of what processes have been done, rather than needing to remove a process from the queue
    long unsigned int rqIndex = 0, jqIndex = 0;
    int64_t curr_time = 0;

    //adding all process indexes to job queue, remaining_bursts
    for (long unsigned int i = 0; i < processes.size(); i++) {
        jq.push_back(i);
        remaining_bursts.push_back(processes[i].burst);
        //adding processes with arrival time 0 immediately to ready queue
        if (processes[jq[jqIndex]].arrival == curr_time) {
            rq.push_back(jq[jqIndex]);
            //adding to sequence
            if ((long int)sequence.size() != max_seq_len){
                sequence.push_back(processes[jq[jqIndex]].id);
            }
            jqIndex++; //one job "removed" from job queue
        }
    }

    //main while loop for going through queue
    while(1){
        //both jq and rq are empty
        if (rqIndex == rq.size() && jqIndex == jq.size()){
            break; //we're done
        }

        //rq is empty
        else if (rqIndex == rq.size()){
            //adding idle value to sequence
            if ((long int)sequence.size() != max_seq_len){
                sequence.push_back(-1);
            }
            //skipping time to next arrival
            curr_time = processes[jq[jqIndex]].arrival;
            //adding jobs with arrival at current time to ready queue
            while(1){
                if (processes[jq[jqIndex]].arrival == curr_time){
                    rq.push_back(jq[jqIndex]);
                    waiting_for_arrival = false;
                    if ((long int)sequence.size() != max_seq_len){
                        sequence.push_back(processes[jq[jqIndex]].id);
                    }
                    jqIndex++;
                } 
                else break;
            }
        }

        //jq is empty
        else if (jqIndex == jq.size()){
            //if not waiting for a process to finish
            if (!waiting_for_finish) {
                //look for the process that will be next to finish
                int64_t smallest_burst = remaining_bursts[rq[rqIndex]];
                for (long unsigned int i = rqIndex + 1; i < rq.size(); i++){
                    if (remaining_bursts[rq[i]] < smallest_burst) smallest_burst = remaining_bursts[rq[i]];
                }
                //getting the N for how many times we can run the quantum throught the queue before a process finishes (-1 to play it safe)
                int64_t N = std::floor(smallest_burst/quantum) - 1;
                
                //if N >= 1 then we make appropriate changes to curr_time and remaining bursts, otherwise run as normal
                if (N >= 1) {
                    //iterating through each process on ready queue
                    for (long unsigned int i = rqIndex; i < rq.size(); i++){
                        //subtract values of all remaining_bursts
                        remaining_bursts[rq[i]] -= N * quantum;

                        //changing start_time values
                        if (processes[rq[i]].start_time == -1) {
                            processes[rq[i]].start_time = curr_time + (i - rqIndex) * quantum;
                        }
                    }

                    //adding big step to current time
                    curr_time += N * quantum * (rq.size() - rqIndex);

                    //add to sequence as neccessary
                    for (int j = 0; j < N; j++) {
                        for (long unsigned int i = rqIndex; i < rq.size(); i++) {
                            if ((long int)sequence.size() == max_seq_len){ //sequence full check
                                break;
                            }
                            int next = processes[rq[i]].id;
                            if (next != sequence[sequence.size() - 1]) { //removing repetition for condensed sequence
                                sequence.push_back(processes[rq[i]].id);
                            }
                        }
                        if ((long int)sequence.size() == max_seq_len){ //sequence full check
                            break;
                        }
                        if ((rq.size() - rqIndex) == 1){ //for repeated attempt to insert the one process into sequence, therefore not filling up the sequence
                            break;
                        }
                    }
                }
                waiting_for_finish = true;
            }
            
            //we're waiting for a process that is close to finish to do so, so we run as we used to before final optimization
            else {
                //changing start_time value
                if (processes[rq[rqIndex]].start_time == -1) {
                    processes[rq[rqIndex]].start_time = curr_time;
                }

                //if last time on ready queue (process will finish)
                if (remaining_bursts[rq[rqIndex]] <= quantum){
                    curr_time += remaining_bursts[rq[rqIndex]];
                    remaining_bursts[rq[rqIndex]] = 0;
                    processes[rq[rqIndex]].finish_time = curr_time;
                    //we got a finish 
                    waiting_for_finish = false;
                } 

                //only removing quantum from remaining bursts, process will need to be added to ready queue again
                else {
                    curr_time += quantum;
                    remaining_bursts[rq[rqIndex]] -= quantum;
                    rq.push_back(rq[rqIndex]);
                    if ((long int)sequence.size() != max_seq_len){ //sequence full check
                        int next = processes[rq[rqIndex]].id;
                        if (next != sequence[sequence.size() - 1]) { //removing repetition for condensed sequence
                            sequence.push_back(processes[rq[rqIndex]].id);
                        }
                    }
                }
                rqIndex++;
            }
        }

        //neither rq or jq are empty
        else{
            //if not waiting for a proccess to finish or a process to arrive
            if (!waiting_for_finish && !waiting_for_arrival){
                //look for the process that will be next to finish
                int64_t smallest_burst = remaining_bursts[rq[rqIndex]];
                for (long unsigned int i = rqIndex + 1; i < rq.size(); i++){
                    if (remaining_bursts[rq[i]] < smallest_burst) smallest_burst = remaining_bursts[rq[i]];
                }
                //getting the N for how many times we can run the quantum throught the queue before a process finishes (-1 to play it safe)
                int64_t N1 = std::floor(smallest_burst/quantum) - 1;                
                //getting the N for how many times we can run the quantum throught the queue before a process arrives (-1 to play it safe)
                int64_t N2 = std::floor((processes[jq[jqIndex]].arrival - curr_time)/(quantum * (rq.size() - rqIndex))) - 1;  
                int64_t N;              
                //if N1 < N2 take N1, proccess to finish is closer than arrival
                if (N1 < N2) {
                    N = N1;
                    waiting_for_finish = true;
                }
                //if N2 < N1 take N2, proccess to arrive is closer than finish
                else if (N2 < N1){
                    N = N2;
                    waiting_for_arrival = true;
                }
                //if equal set waiting for finish and arrival to true, we'll wait for both to happen
                else {
                    N = N1;
                    waiting_for_arrival = true;
                    waiting_for_finish = true;
                }
                
                //if N >= 1 then we make appropriate changes to curr_time and remaining bursts, otherwise run as normal
                if (N >= 1) {

                    for (long unsigned int i = rqIndex; i < rq.size(); i++){
                        //subtract values of all remaining_bursts
                        remaining_bursts[rq[i]] -= N * quantum;

                        //changing start_time value
                        if (processes[rq[i]].start_time == -1) {
                            processes[rq[i]].start_time = curr_time + (i - rqIndex) * quantum;
                        }
                    }

                    //adding big step to current time
                    curr_time += N * quantum * (rq.size() - rqIndex);

                    //add to sequence as neccessary
                    for (int j = 0; j < N; j++) {
                        for (long unsigned int i = rqIndex; i < rq.size(); i++) {
                            if ((long int)sequence.size() == max_seq_len){ //sequence full check
                                break;
                            }
                            int next = processes[rq[i]].id;
                            if (next != sequence[sequence.size() - 1]) { //removing repetition for condensed sequence
                                sequence.push_back(processes[rq[i]].id);
                            }

                        }
                        if ((long int)sequence.size() == max_seq_len){ //sequence full check
                            break;
                        }
                        if ((rq.size() - rqIndex) == 1){ //for repeated attempt to insert the one process into sequence, therefore not filling up the sequence
                            break;
                        }
                    }
                }
            }

            //we're waiting for a process that is close to finish or arriving to do so, so we run as we used to before final optimization
            else {

                //changing start_time value
                if (processes[rq[rqIndex]].start_time == -1) {
                    processes[rq[rqIndex]].start_time = curr_time;
                }

                //if last time on ready queue (process will finish)
                if (remaining_bursts[rq[rqIndex]] <= quantum){
                    curr_time += remaining_bursts[rq[rqIndex]];
                    remaining_bursts[rq[rqIndex]] = 0;
                    processes[rq[rqIndex]].finish_time = curr_time;
                    //we got a finish 
                    waiting_for_finish = false;
                    //adding to the ready queue if arrival time already happened or is at current time
                    while(1){
                        if (jqIndex != jq.size() && processes[jq[jqIndex]].arrival <= curr_time){
                            rq.push_back(jq[jqIndex]);
                            //we got an arrival
                            waiting_for_arrival = false;
                            if ((long int)sequence.size() != max_seq_len){ //sequence full check
                                sequence.push_back(processes[jq[jqIndex]].id);
                            }
                            jqIndex++;
                        } 
                        else break;
                    }
                } 

                //only removing quantum, process will need to be added to ready queue again
                else {
                    curr_time += quantum;
                    remaining_bursts[rq[rqIndex]] -= quantum;
                   
                    //adding to the ready queue if arrival time already happened or is at current time
                    while(1){
                        if (jqIndex != jq.size() && processes[jq[jqIndex]].arrival < curr_time){
                            rq.push_back(jq[jqIndex]);
                            //we got an arrival
                            waiting_for_arrival = false;

                            if ((long int)sequence.size() != max_seq_len){ //sequence full check
                                sequence.push_back(processes[jq[jqIndex]].id);
                            }
                            jqIndex++;
                        } 
                        else break;
                    }
                    //adding process again to ready queue
                    rq.push_back(rq[rqIndex]);
                    if ((long int)sequence.size() != max_seq_len){ //sequence full check
                        int next = processes[rq[rqIndex]].id;
                        if (next != sequence[sequence.size() - 1]) { //removing repetition for condensed sequence
                            sequence.push_back(processes[rq[rqIndex]].id);
                        }
                    }
                    //adding new process after to ready queue if enters at same time
                    while(1){
                        if (jqIndex != jq.size() && processes[jq[jqIndex]].arrival == curr_time){
                            rq.push_back(jq[jqIndex]);
                            //we got an arrival
                            waiting_for_arrival = false;
                            if ((long int)sequence.size() != max_seq_len){ //sequence full check
                                sequence.push_back(processes[jq[jqIndex]].id);
                            }
                            jqIndex++;
                        } 
                        else break;
                    }
                }
                rqIndex++;
            }
        }

    }

    seq.clear();
    seq = sequence; //copying final sequence
}
