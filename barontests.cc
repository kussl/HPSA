#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <cmath>
#include <ctime>
#include <thread>

#ifndef AG_GRAPH_HEADER
#define AG_GRAPH_HEADER 1 
#include "graph.h"
#endif 

#ifndef AG_HPSA_HEADER
#define AG_HPSA_HEADER 1 
#include "hpsa.h"
#endif 

using namespace std;


extern void baron_interface(std::vector<std::string> &names, int num_threads=0); 
extern void baron_interface(std::string name); 
extern void baron_interface(Graph G, int i, bool bg); 
extern std::string create_baron_file(Graph &G, int i); 
extern std::string create_baron_file(Graph &G, int index, std::vector<int> candidate_nodes, double improvement, int m); 
extern std::string create_baron_file(Graph &G, int index, std::vector<Instrument> v, int m); 
extern void baron_files(std::vector<Graph> X, std::vector<std::string> &names); 
void collectres(size_t size, double extra_time, bool silent);
void collectres(size_t size);
void readsolution(std::vector<Graph> &X); 



void propagate_probabilities_serial(Graph G){
	cout<<"Propagating probabilities (serial), number of nodes: "<<G.size()<<endl; 

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
	baron_interface(names, -1); //Set num_threads to -1 to run one thread only.
	collectres(1); 	
}

/*
	1. Partition the graph. 
	2. Compute expected success for each subtree in parallel. 
	3. Form a final graph F using the subtree goal nodes. 
	4. Compute the final expected success on F. 
*/

void propagate_probabilities_parallel(Graph G, int P){
	size_t size = 0; 
	double preparation_time = 0.0; 

	std::vector<Graph> X = G.partitiongraph(); 

	cout<<"Propagating probabilities (parallel), number of nodes: "<<G.size()<<endl; 
	size = X.size(); 

	cout<<"Number of subgraphs: "<<size<<endl; 

	std::vector<std::string> names;
	baron_files(X, names); 
	baron_interface(names, P);
	//Record the time for the extra effort. 
	const clock_t begin_time = clock();

	readsolution(X);
	Graph AG = G.combine_subtrees(X); 
	cout<<"Final solution: "<<endl; 
	collectres(size); 
	preparation_time = double( clock () - begin_time ) /  CLOCKS_PER_SEC; 

	std::cout << "Time (s) spent for preparation: "<<preparation_time<<endl;

	std::cout<<"Combining the parallel results and recomputing the final value: \n"; 
	
	propagate_probabilities_serial(AG); 
}




/*
For each of the k instruments, create a target set
by going through the rule nodes sequentially, dividing 
them into k subsets. 
*/
void create_targets(Graph G, std::vector<Instrument> &v, int k){
	std::vector<int> rule_nodes = G.imp_candidate_nodes();
	int no_rule_nodes = rule_nodes.size();
	int nodes_per_inst = no_rule_nodes / k;  
	double P = 0.9; 
	int i = 0; 
	for(int j = 0; j < k; ++j){
		std::vector<int> targets;
		Instrument t = Instrument(P*.9); 
		
		for(int s = 0; i < no_rule_nodes && s < nodes_per_inst; ++i, ++s){
			targets.push_back(rule_nodes[i]); 
		}
		t.set_targets(targets); 
		v.push_back(t); 
	}
}

/*
For each of the k instruments, add ALL the rule 
nodes as their target sets. 
*/
int all_rule_nodes_targets(Graph G, std::vector<Instrument> &v, int k){
	std::vector<int> rule_nodes = G.imp_candidate_nodes();
	int no_rule_nodes = rule_nodes.size();
	double P = 1.0; 
	int tau = no_rule_nodes; //number of targets for each instrument. 
	for(int i = 0; i < k; ++i){
		std::vector<int> targets;
		Instrument t = Instrument(P*=.9); 
		for(int j=0; j< no_rule_nodes; ++j){
			targets.push_back(rule_nodes[j]); 
		}
		t.set_targets(targets); 
		v.push_back(t); 
	}
	return tau; 
}


std::string produce_single_file(Graph G, int index, std::vector<Instrument> v, int m){
	std::string spec = create_baron_file(G, index, v, m);
	std::string path = BARON_RES_PATH; 
	std::string name = path+"/program_"+std::to_string(index)+".bar";
	ofstream programfile; 
	programfile.open(name);
	programfile<<spec; 
	programfile.close();
	return name; 
}


void improve_BARON_serial(Graph G, int m, int k){
	int no_rule_nodes = G.count_type(Rule); 
	std::vector<Instrument> v; 
	int tau = all_rule_nodes_targets(G, v, k); 
	
	cout<<"Number of nodes: "<<G.size()<<endl; 
	cout<<"Number of rule nodes: "<<no_rule_nodes<<endl; 
	cout<<"Number of instrument types: k="<<k<<endl; 
	cout<<"Acceptable placements: m="<<m<<endl; 
	cout<<"Number of improvement targets: tau="<<tau<<endl; 

	std::string name = produce_single_file(G, 0, v, m);

	baron_interface(name); 
	collectres(1, 0.0, false); 
}

void improve_BARON_tree_partition(Graph G, std::vector<Graph> X, int m, int k, int P){
	size_t size = 0; 
	//Number of subproblems to solve. 
	size = X.size(); 
	int no_rule_nodes = G.count_type(Rule); 
	int tau = no_rule_nodes; 
	
	//To hold the names of BARON files for each subproblem.
	std::vector<std::string> names;


	//For each subproblem, create the improvement targets, and produce a BARON file.
	int sub_m = floor(m/size);
	if(sub_m < 1)
		sub_m = 1;  

	for (int i = 0; i < size; ++i){
		std::vector<Instrument> v; 
		tau = all_rule_nodes_targets(X[i], v, k); 

		std::string name = produce_single_file(X[i], i, v, sub_m);
		names.push_back(name); 
	}

	cout<<"Number of nodes: "<<G.size()<<endl; 
	cout<<"Number of rule nodes: "<<no_rule_nodes<<endl; 
	cout<<"Number of instrument types: k="<<k<<endl; 
	cout<<"Total acceptable placements: m="<<m<<endl; 
	cout<<"Number of improvement targets: tau="<<tau<<endl; 
	cout<<"Number of subtrees, L="<<size<<endl; 
	cout<<"Acceptable placements for each subproblem, m="<<sub_m<<endl; 

	/*
	Solve the improvement in each subproblem. 
	*/
	baron_interface(names, P);
	//Record the time for the extra effort. 
	const clock_t begin_time = clock();
	cout<<"Reading solutions."<<endl; 

	/*
	Read the solution and combine the subtrees back into a full attack tree.
	*/
	readsolution(X);
	Graph AG = G.combine_subtrees(X); 
	double preparation_time = float( clock () - begin_time ) /  CLOCKS_PER_SEC; 
	cout<<"S1: \n";

	collectres(size, preparation_time, false); 

	//Determine m for the final tree: 
	std::vector<int> pred = AG.preds(0);
	int final_m = pred.size(); 
	if(m==1){
		final_m = 1; 
	}

	if( m < size){
		/*
		Now consider the placement only for the rule nodes that were not included 
		in the subtrees. 
		*/
		names.clear(); 
		for (int i = 0; i < size; ++i){
			std::vector<Instrument> v; 
			std::string name = produce_single_file(X[i], i, v, sub_m);
			names.push_back(name); 
		}
		/*
		Solve the subtrees without any improvement. 
		*/
		baron_interface(names, P);
		readsolution(X);
		AG = G.combine_subtrees(X); 

		preparation_time = float( clock () - begin_time ) /  CLOCKS_PER_SEC; 

		cout<<"S2: \n";
		collectres(size, preparation_time, false); 


		/*
		Now solve the improvement only for those rule nodes that were not
		included in the subtrees with solutions from subtrees that were not improved.
		*/
		cout<<"S3: \n";
		improve_BARON_serial(AG,final_m,k);
	}
	else { 
		improve_BARON_serial(AG,final_m,k);
	}
}

void improve_BARON_target_partition(Graph G, int m, int k, int P){
	int graph_size = G.size(); 
	int no_rule_nodes = G.count_type(Rule); 
	int tau = no_rule_nodes; 
	std::vector<Instrument> v; 
	
	//To hold the names of BARON files for each subproblem.
	std::vector<std::string> names;

	/*
	Partition the targets into multiple sets and 
	apply them to each subtree. Assume k=1 for now. 
	Each subproblem has the entire attack tree. However, 
	each one tests the application of the instrument on a
	subset of applicable targets. This is good when the 
	number of improvement targets is large. 
	Here, for experiments, we assume the entire rule node set
	is the target. 
	We'll split them into log2 chunks. 
	*/
	
	int splits = ceil(log2(tau));
	int placements_per_split = ceil(tau/splits); 
	

	/*
	We'll artificially set k to the number of splits.
	The function below will assign all instruments the same
	improvement value, making them equivalent. 
	We'll produce k target sets and then produce their
	subproblems accordingly. 
	*/
	create_targets(G,v,k);

	cout<<"Number of nodes: "<<graph_size<<endl; 
	cout<<"Number of rule nodes: "<<no_rule_nodes<<endl; 
	cout<<"Number of instrument types: k="<<k<<endl; 
	cout<<"Total acceptable placements: m="<<m<<endl; 
	cout<<"Number of improvement targets: tau="<<tau<<endl; 
	cout<<"Number of target splits="<<splits<<endl; 
	cout<<"Targets per split="<<placements_per_split<<endl; 


	for(int i = 0; i < k; ++i){
		/*
		Here, produce a single BARON file for each target set.
		Thus, we create v_single add one target set to it. 
		*/
		std::vector<Instrument> v_single;
		v_single.push_back(v[i]); 
		
		std::string name  = produce_single_file(G, i, v_single, m);
		names.push_back(name); 
		
	}
	
	baron_interface(names, P); 
	collectres(splits); 
}



void propagate_probabilities(Graph G, int P){
	propagate_probabilities_serial(G); 
	propagate_probabilities_parallel(G, P); 
}


void improve_security(Graph G, int P){
	int m = 1;
	int k = 1; 
	//Partition the tree to multiple subtrees
	std::vector<Graph> X = G.partitiongraph(); 
	//m = X.size(); 
	// cout<<"Parallel (tree partition): \n"; 
	// improve_BARON_tree_partition(G,X,m,k,P);  
	int no_rule_nodes = G.count_type(Rule); 
	k = ceil(log2(no_rule_nodes));
	cout<<"Serial: \n";
	improve_BARON_serial(G, m, k);
	cout<<"Parallel (target partition): \n"; 
	improve_BARON_target_partition(G,m,k,P);
}


void generate_graph(Graph &G, int goallayers, int subgoals, int rules, int facts){
	GraphGenerator GG; 
	//cout<<"Generating a synthetic AG: layers: "<<goallayers<<", subgoals: "<<subgoals<<", rules: "<<rules<<", facts: "<<facts<<endl; 
	//int goallayers, int subgoals, int rules, int facts
	//G = GG.hierarchical_topology(goallayers,subgoals,rules,facts);
	G = GG.treetopology(goallayers,subgoals,rules,facts);
	G.print(SOL_PATH+"graph.dot");
}


void collectres(size_t size, double extra_time, bool silent){
	std::string sizestr = std::to_string(size); 
	std::string extra_time_str = std::to_string(extra_time); 
	std::string silent_str = std::to_string(silent); 
	pid_t pid,wpid;
	int status; 
	std::string path = HPSA_PATH+"/readres.py"; 

	if ((pid = fork()) == 0){
		int ret = execl(path.c_str(), "readres.py", 
			sizestr.c_str(), extra_time_str.c_str(), silent_str.c_str(), NULL);
		cout<<"Error in executing Python script: "<<ret<<endl;
		exit(0);  
	}
	while ((wpid = wait(&status)) > 0)
		;

    cout<<"==============================\n";
}
void collectres(size_t size){
	std::string sizestr = std::to_string(size); 
	std::string extra_time_str = std::to_string(0); 
	std::string silent_str = std::to_string(0); 

	pid_t pid,wpid;
	int status; 
	std::string path = HPSA_PATH+"/readres.py"; 

	if ((pid = fork()) == 0){
		int ret = execl(path.c_str(), "readres.py", 
			sizestr.c_str(), extra_time_str.c_str(), silent_str.c_str(), NULL);
		cout<<"Error in executing Python script: "<<ret<<endl;
		exit(0);  
	}
	while ((wpid = wait(&status)) > 0)
		;

    cout<<"==============================\n";
}
