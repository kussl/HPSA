#include <iostream>
#include <iostream>
#include <fstream>
#include <unistd.h>

#ifndef AG_GRAPH_HEADER
#define AG_GRAPH_HEADER 1 
#include "graph.h"
#endif 

#ifndef AG_HPSA_HEADER
#define AG_HPSA_HEADER 1 
#include "hpsa.h"
#endif 

using namespace std;


extern void baron_interface(std::vector<std::string> &names); 
extern void baron_interface(Graph G, std::string name); 
extern void baron_interface(Graph G, int i, bool bg); 
extern std::string create_baron_file(Graph &G, int i); 
extern std::string create_baron_file(Graph &G, int index, std::vector<int> candidate_nodes, double improvement); 
extern void baron_files(std::vector<Graph> X, std::vector<std::string> &names); 
void collectres(size_t size); 
void test_gen_BARON_imp_serial(Graph G); 

/*
Run improvement options in parallel using a combination of OpenMP and BARON threads. 
*/

void test_gen_BARON_imp(Graph G, int P){
	cout<<"Number of nodes: "<<G.size()<<endl; 
	double improvement = 0.8; 
	size_t size = 0; 

	/*
	Loop through all rule nodes. Each rule node (or a subset of rule nodes) is a candidate placement. 
	Make a separate BARON problem for each rule node and compute the improvement in parallel. 
	*/
	std::vector<int> candidate_nodes = G.imp_candidate_nodes();
	size = candidate_nodes.size(); 

	cout<<"Number of candidate placements: "<<candidate_nodes.size()<<endl; 
	std::vector<std::string> names;

	/*
	This for-loop takes subsets of rule nodes in each iteration (more than one). 
	*/
	int i,j=0,k; 
	int subset_size = candidate_nodes.size() / P; 

	if(size <= P){
		test_gen_BARON_imp_serial(G);
		return; 
	}

	cout<<"Placements in each subset: "<<subset_size<<endl;


	for (i=0; i<P; ++i){
		std::vector<int> v;
		for(k=0; j<size && k < subset_size; ++j, ++k)
			v.push_back(candidate_nodes[j]); 

		std::string spec = create_baron_file(G, i, v, improvement);
		std::string name = BARON_RES_PATH + "/program_"+std::to_string(i)+".bar";
		ofstream programfile; 
		programfile.open(name);
		programfile<<spec; 
		programfile.close();
		names.push_back(name); 
	}

	size = P; 

	baron_interface(names); 
	collectres(size); 
}

/* 
Run improvement options serially.  
*/
void test_gen_BARON_imp_serial(Graph G){
	cout<<"Number of nodes: "<<G.size()<<endl; 
	
	double improvement = 0.8; 
	size_t size = 0; 

	/*
	Loop through all rule nodes. Each rule node is a candidate placement. 
	Make a separate BARON problem for each rule node and compute the improvement in parallel. 
	*/
	std::vector<int> candidate_nodes = G.imp_candidate_nodes();
	size = candidate_nodes.size(); 

	cout<<"Number of candidate placements: "<<candidate_nodes.size()<<endl; 
	std::vector<std::string> names;
	std::vector<int> v;
	for(int i =0; i<size; ++i){
		v.push_back(candidate_nodes[i]); 
	}

	std::string spec = create_baron_file(G, 0, v, improvement);
	std::string path = BARON_RES_PATH; 
	std::string name = path+"/program_"+std::to_string(0)+".bar";
	ofstream programfile; 
	programfile.open(name);
	programfile<<spec; 
	programfile.close();

	baron_interface(G, name); 
	collectres(1); 
}

void test_gen_BARON(Graph G){
	cout<<"Number of nodes: "<<G.size()<<endl; 

	std::string spec = create_baron_file(G, 0);
	std::string path = BARON_RES_PATH; 
	std::string name = path+"/program_"+std::to_string(0)+".bar";
	ofstream programfile; 
	programfile.open(name);
	programfile<<spec; 
	programfile.close();
	std::vector<Graph> X; 
	std::vector<std::string> names;
	X.push_back(G); 

	baron_files(X, names); 
	baron_interface(names);
	collectres(1); 	
}

/*
	Partition the graph for parallelized probability propagation. 
*/

void test_gen_BARON_omp(Graph G){
	size_t size = 0; 
	std::vector<Graph> X = G.partitiongraph(); 

	cout<<"Number of nodes: "<<G.size()<<endl; 
	size = X.size(); 

	cout<<"Number of subgraphs: "<<size<<endl; 

	std::vector<std::string> names;
	baron_files(X, names); 
	baron_interface(names);
	collectres(size); 
}

/*
Multiple improvements test:
This function receives a graph G and a vector of improvement instruments.
Each improvement instrument is represented by a vector of nodes and 
a probability value. The vector of nodes are the improvement target
rule nodes of the given improvement instrument. 
*/

void test_BARON_multiple_improvements(Graph G) {

}


void generate_graph(Graph &G, int goallayers, int subgoals, int rules, int facts){
	GraphGenerator GG; 
	cout<<"Generating a synthetic AG: layers: "<<goallayers<<", subgoals: "<<subgoals<<", rules: "<<rules<<", facts: "<<facts<<endl; 
	//int goallayers, int subgoals, int rules, int facts
	G = GG.treetopology(goallayers,subgoals,rules,facts);
	G.print(SOL_PATH+"graph.dot");
}


void collectres(size_t size){
	std::string sizestr = std::to_string(size); 
	pid_t pid,wpid;
	int status; 
	std::string path = HPSA_PATH+"/readres.py"; 

	if ((pid = fork()) == 0){
		int ret = execl(path.c_str(), "readres.py", 
			sizestr.c_str(), NULL);
		cout<<"Error in executing Python script: "<<ret<<endl;
		exit(0);  
	}
	while ((wpid = wait(&status)) > 0)
		;

    cout<<"Done collecting results.\n";
}