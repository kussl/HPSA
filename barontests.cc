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
void collectres(size_t size); 
void improve_BARON_serial(Graph G); 
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

Graph combine_subtrees(Graph G, std::vector<Graph> X){
	Graph AG;
	//iterator for X:
	int k = 0; 

	//Add the leaf node 
	int leaf_id = AG.addnode(AUTO_ID, Goal, 0); 
	//Its predecessors come from G 
	std::vector<int> pred = G.preds(0);
	int no_preds = pred.size(); 
	for(int i = 0; i < no_preds; ++i){
		AG.addnode(pred[i], Rule, 0); 
		AG.addedge(pred[i], leaf_id);	
		/*
		Add the leaf nodes in the subtrees of X as
		predecessors of the rule nodes in AG. 
		*/ 
		std::vector<int> rule_pred = G.preds(pred[i]); 
		int no_rule_preds = rule_pred.size(); 
		for(int j = 0; j<no_rule_preds; ++j){
			Node n = X[k].graph_nodes()[0];
			++k; 
			AG.addnode(n.nodeid(), Fact, n.nodeP()); 
			AG.addedge(rule_pred[j], pred[i]);  
		}
	}
	return AG; 
}

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
	Graph AG = combine_subtrees(G,X); 
	cout<<"Final solution: "<<endl; 
	collectres(size); 
	preparation_time = double( clock () - begin_time ) /  CLOCKS_PER_SEC; 

	std::cout << "Time (s) spent for preparation: "<<preparation_time<<endl;

	std::cout<<"Combining the parallel results and recomputing the final value: \n"; 
	
	propagate_probabilities_serial(AG); 
}

void propagate_probabilities(Graph G, int P){
	propagate_probabilities_serial(G); 
	propagate_probabilities_parallel(G, P); 
}



/*
Run improvement options in parallel using a combination of OpenMP and BARON threads. 
*/

void improve_BARON_parallel(Graph G, int P){
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
		improve_BARON_serial(G);
		return; 
	}

	cout<<"Placements in each subset: "<<subset_size<<endl;


	for (i=0; i<P; ++i){
		std::vector<int> v;
		for(k=0; j<size && k < subset_size; ++j, ++k)
			v.push_back(candidate_nodes[j]); 

		std::string spec = create_baron_file(G, i, v, improvement, 1);
		std::string name = BARON_RES_PATH + "/program_"+std::to_string(i)+".bar";
		ofstream programfile; 
		programfile.open(name);
		programfile<<spec; 
		programfile.close();
		names.push_back(name); 
	}

	size = P; 

	baron_interface(names, P); 
	collectres(size); 
}

/* 
Run improvement options serially.  
*/
void improve_BARON_serial(Graph G){
	cout<<"Number of nodes: "<<G.size()<<endl; 
	
	double improvement = 0.8; 
	size_t size = 0; 
	int m = 1; 
	int index = 0; 

	std::vector<int> candidate_nodes = G.imp_candidate_nodes();
	size = candidate_nodes.size(); 

	cout<<"Candidate placements (tau): "<<candidate_nodes.size()<<", rule nodes: "<<G.count_type(Rule)<<", m="<<m<<endl; 
	std::vector<std::string> names;
	std::vector<int> v;

	for(int i =0; i<size; ++i){
		v.push_back(candidate_nodes[i]); 
	}

	std::string spec = create_baron_file(G, index, v, improvement, m);
	std::string path = BARON_RES_PATH; 
	std::string name = path+"/program_"+std::to_string(0)+".bar";
	ofstream programfile; 
	programfile.open(name);
	programfile<<spec; 
	programfile.close();

	baron_interface(name); 
	collectres(1); 
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
	double P = 1.0; 
	int i = 0; 
	for(int j = 0; j < k; ++j){
		std::vector<int> targets;
		Instrument t = Instrument(P*=.9); 
		
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
void all_rule_nodes_targets(Graph G, std::vector<Instrument> &v, int k){
	std::vector<int> rule_nodes = G.imp_candidate_nodes();

	int no_rule_nodes = rule_nodes.size();
	double P = 1.0; 
	for(int i = 0; i < k; ++i){
		std::vector<int> targets;
		Instrument t = Instrument(P*=.9); 
		//cout<<"Instrument "<<i<<": ";
		for(int j=0; j< no_rule_nodes; ++j){
			targets.push_back(rule_nodes[j]); 
		//	cout<<rule_nodes[j]<<" ";
		}
		//cout<<endl; 
		t.set_targets(targets); 
		v.push_back(t); 
	}
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


void multiple_improvements_case2_sequential(Graph G, int m, int k ){
	//To sync the value of m with the parallel version.
	std::vector<Graph> X = G.partitiongraph(); 
	int size = X.size();
	

	int no_rule_nodes = G.count_type(Rule); 
	cout<<"Number of instruments: k="<<k<<endl; 
	cout<<"Number of rule nodes: "<<no_rule_nodes<<endl; 
	cout<<"Acceptable placements: m="<<m<<endl; 


	std::vector<Instrument> v; 
	all_rule_nodes_targets(G, v, k); 

	std::string name = produce_single_file(G, 0, v, m);

	baron_interface(name); 
	collectres(1); 
}

/*
Multiple improvements test:
This function receives a graph G and a vector of improvement instruments.
Each improvement instrument is represented by a vector of nodes and 
a probability value. The vector of nodes are the improvement target
rule nodes of the given improvement instrument. 
m: number of acceptable placements 
k: number of available instruments (v.size())
Two cases are considered: 
case 1: 1 < m < k 
case 2: m >= k 
*/



void multiple_improvements_case2_parallel(Graph G, int m, int k, int P){
	std::vector<Instrument> v; 
	std::vector<std::string> names;
	create_targets(G, v, k); 

	/* 
	For each instrument, create a separate optimization problem
	to be solved in parallel. 
	*/
	//m /= k; 
	cout<<"Sub-m: "<<m<<endl; 

	/*
	Create a program for every log_2(k) improvements.
	*/
	int splits = floor(log2(k)); 
	


	cout<<"Splits: "<<splits<<endl;

	for(int i = 0; i < k; i+=splits){
		std::vector<Instrument> v_single;
		for(int j = 0; j < splits && (i+j) < k; ++j){
			v_single.push_back(v[i+j]); 
		}
		std::string name  = produce_single_file(G, i, v_single, m);
		names.push_back(name); 
	}
	
	baron_interface(names, P); 
	collectres(splits); 
}

void multiple_improvements_case2_parallel2(Graph G, int m, int k,int P){
	size_t size = 0; 
	std::vector<Graph> X = G.partitiongraph(); 

	cout<<"Number of nodes: "<<G.size()<<endl; 
	size = X.size(); 

	cout<<"Number of subgraphs, L="<<size<<endl; 

	/*
		m: number of acceptable placements (relative to the number of rule nodes).
		k: number of instruments. 
	*/
	

	int no_rule_nodes = G.count_type(Rule); 

	cout<<"Number of instruments: k="<<k<<endl; 
	cout<<"Number of rule nodes: "<<no_rule_nodes<<endl; 
	cout<<"Acceptable placements: m="<<m<<endl; 

	/*
		Each subproblem receives a portion of the m acceptable placements. 
	*/
	m = ceil(m/(double)size);

	cout<<"Acceptable placements for each subproblem, m="<<m<<endl; 

	std::vector<std::string> names;
	for (int i = 0; i < size; ++i){
		std::vector<Instrument> v; 
		all_rule_nodes_targets(G, v, k); 

		std::string name = produce_single_file(G, i, v, m);
		names.push_back(name); 
	}

	baron_files(X, names); 
	baron_interface(names, P);
	//Record the time for the extra effort. 
	const clock_t begin_time = clock();
	cout<<"Reading solutions."<<endl; 

	readsolution(X);
	Graph AG = combine_subtrees(G,X); 
	cout<<"Final solution: "<<endl; 
	collectres(size); 

	std::cout << "Time (s) spent for preparation: "<<float( clock () - begin_time ) /  CLOCKS_PER_SEC<<endl;

	improve_BARON_serial(AG); 
}


void test_BARON_multiple_improvements(Graph G, short parallel, int P) {
	int m = G.count_type(Rule) / 3; 
	int k;
	k = floor(log2(m)); 
	if (k < 1)
		k = 1; 
	int size = G.size();

	cout<<"Graph nodes: "<<G.size()<<", k: "<<k<<", m: "<<m<<endl; 

	if( m >= k ){
		if(parallel==0){
			m = size * 2; 
			k = floor(log2(m)); 
			if (k < 1){
				k = 2; 
			}
			multiple_improvements_case2_sequential(G, m, k); 	
		}
		else if(parallel==1){ 
			multiple_improvements_case2_parallel(G, m, k, P); 
		}
		else if(parallel==2){
			m = size * 2; 
			k = floor(log2(m)); 
			if (k < 1)
				k = 1; 		
			m /= k; 	
			multiple_improvements_case2_parallel2(G, m, k, P); 
		}
	}
}

void test_BARON_single_placement_serial(Graph G){
	int no_rule_nodes = G.count_type(Rule); 
	int k = 4; 
	int m = G.count_type(Rule) / 3; 
	k = floor(log2(m)); 
	if (k < 1)
		k = 1; 
	m = 1; 
	int n = G.size(); 

	cout<<"Graph nodes: "<<n<<" Rule nodes: "<<no_rule_nodes<<", k: "<<k<<", m: "<<m<<endl; 

	multiple_improvements_case2_sequential(G, m, k); 
}

void test_BARON_single_placement_parallel(Graph G, int P){
	int no_rule_nodes = G.count_type(Rule); 
	int k = 4; 
	int m = G.count_type(Rule) / 3; 
	k = floor(log2(m))*6; 
	if (k < 1)
		k = 1; 
	m = 1; 
	int n = G.size(); 

	cout<<"Graph nodes: "<<n<<" Rule nodes: "<<no_rule_nodes<<", k: "<<k<<", m: "<<m<<endl; 

	multiple_improvements_case2_parallel(G, m, k, P); 
}


void improve_security(Graph G, int P){
	improve_BARON_serial(G); 
}


void generate_graph(Graph &G, int goallayers, int subgoals, int rules, int facts){
	GraphGenerator GG; 
	//cout<<"Generating a synthetic AG: layers: "<<goallayers<<", subgoals: "<<subgoals<<", rules: "<<rules<<", facts: "<<facts<<endl; 
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

    cout<<"==============================\n";
}
