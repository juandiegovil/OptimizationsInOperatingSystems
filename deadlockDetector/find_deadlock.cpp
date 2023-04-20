// this is the ONLY file you should edit and submit to D2L
// JUAN VILLARREAL____UCID: 30072174____CPSC 457: ASSIGNMENT 4

#include "find_deadlock.h"
#include "common.h"
#include <iostream>
//#include <string>

/// this is the function you need to (re)implement
///
/// parameter edges[] contains a list of request- and assignment- edges
///   example of a request edge, process "p1" resource "r1"
///     "p1 -> r1"
///   example of an assignment edge, process "XYz" resource "XYz"
///     "XYz <- XYz"
///
/// You need to process edges[] one edge at a time, and run a deadlock
/// detection after each edge. As soon as you detect a deadlock, your function
/// needs to stop processing edges and return an instance of Result structure
/// with 'index' set to the index that caused the deadlock, and 'procs' set
/// to contain names of processes that are in the deadlock.
///
/// To indicate no deadlock was detected after processing all edges, you must
/// return Result with index=-1 and empty procs.
///
class FastGraph {

public:
    std::vector<std::vector<int>> adj_list;
    std::vector<int> out_counts;
};

Result find_deadlock(const std::vector<std::string> & edges)
{
    //initializing empty result, graph, conversions
    Result result;
    FastGraph graph;
    Word2Int conversions;
    int resultIndex = 0; //resulting deadlock index
    std::vector<int> unchanged_out_counts; //to come back to out counts before changes on next iteration of loop
    std::vector<std::string> names; //to convert the process/resources int back to their names

    //going through each edge
    for (auto edge : edges){

        graph.out_counts = unchanged_out_counts; //back to original out_count values
        bool deadlock = false;

        //aplitting the edge into process, arrow, resource and converting their names to ints
        auto splitEdge = split(edge);
        splitEdge[0].append(".p");
        splitEdge[2].append(".r");
        long unsigned int index = conversions.get(splitEdge[0]);
        long unsigned int index2 = conversions.get(splitEdge[2]);

        //adding the resource/process if it hasn't been added
        if (graph.adj_list.size() == index) {
            graph.adj_list.push_back({});
            graph.out_counts.push_back(0);
            unchanged_out_counts.push_back(0);
            names.push_back(splitEdge[0]);
        }
        if (graph.adj_list.size() == index2) {
            graph.adj_list.push_back({});
            graph.out_counts.push_back(0);
            unchanged_out_counts.push_back(0);
            names.push_back(splitEdge[2]);
        }

        //adding to the adjacent list and the out count based on the arrow direction
        if (splitEdge[1] == "<-") {
            graph.adj_list[index].push_back(index2);
            graph.out_counts[index2]++;
            unchanged_out_counts[index2]++;
        }
        else {
            graph.adj_list[index2].push_back(index);
            graph.out_counts[index]++;
            unchanged_out_counts[index]++;
        }

        //nodes with zero out counts
        std::vector<int> zeros;
        long unsigned int zeroIndex = 0; //index where we last handled a zero node

        //getting all the zero nodes
        for (long unsigned int i = 0; i < graph.out_counts.size(); i++) {
            if (graph.out_counts[i] == 0) zeros.push_back(i);
        }
        
        //toposort
        while(1){
            
            //if there's still zero nodes to handle
            if (zeroIndex < zeros.size()) {

                //changing out counts with zero to -1
                graph.out_counts[zeros[zeroIndex]] = -1;

                //changing out counts for nodes on adj_list
                for (auto node : graph.adj_list[zeros[zeroIndex]]) {
                    graph.out_counts[node]--;

                    //adding new zero nodes
                    if (graph.out_counts[node] == 0){
                        zeros.push_back(node);
                    }
                }
                zeroIndex++;
            }

            //if every node is in the zeros vector (toposort finished)
            else if (zeros.size() == graph.out_counts.size()) {
                result.index = -1;
                break;
            }

            //otherwise we're in deadlock
            else {
                result.index = resultIndex;
                for (long unsigned int i = 0; i < graph.out_counts.size(); i++){
                    //adding every node in the deadlock to the procs[] list, excluding those that are resources
                    if (graph.out_counts[i] > 0){ 
                        if (names[i].substr(names[i].size() - 2, 2) == ".p") {
                            result.procs.push_back(names[i].substr(0, names[i].size() - 2));
                        }
                    }
                }
                deadlock = true;
                break;
            }

        }
        
        if (deadlock) break;

        resultIndex++;
    }

    return result;
}
