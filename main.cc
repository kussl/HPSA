#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <ctime>
#include <chrono>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include "omp.h"
#include "graph.h"



#define BARON_PATH std::string(std::getenv("BARON_PATH")) 
#define HPSA_PATH std::string(std::getenv("HPSA_PATH")) 
#define SOL_PATH (HPSA_PATH + std::string("/solution/"))
#define P_SOL_PATH (HPSA_PATH + std::string("/p_solution/"))
#define BARON_RES_PATH (HPSA_PATH + std::string("/baron/"))

//#define EACH_RULE_CANDIDATE 1

using namespace std;
extern std::string create_baron_file(Graph &G, int i); 
extern std::string create_baron_file(Graph &G, int index, std::vector<int> candidate_nodes, double improvement); 


void test_gen_BARON_imp_serial(int goallayers, int subgoals, int rules, int facts); 
void test_gen_BARON_imp(int goallayers, int subgoals, int rules, int facts);

void timestamp(clock_t &begin_time){
	std::cout <<"Click ticks: "<<float(clock()-begin_time)/CLOCKS_PER_SEC<<endl;
	begin_time = clock();
}

void graph_generator(Graph &G, int goallayers, int subgoals, int rules, int facts){
	GraphGenerator GG; 
	cout<<"Generating a synthetic AG.\n"; 
	//int goallayers, int subgoals, int rules, int facts
	G = GG.treetopology(goallayers,subgoals,rules,facts);
	cout<<"Number of nodes: "<<G.size()<<endl; 
}




void baron_interface(Graph G, int i, bool bg){
	std::string spec = create_baron_file(G, i); 
	std::string name = BARON_RES_PATH+"/program_"+std::to_string(i)+".bar";
	ofstream programfile; 
	programfile.open(name);
	programfile<<spec; 
	programfile.close();

	if (!bg)
		system(("baron "+name + " &").c_str());
	else 
		system(("baron "+name).c_str());
}

void baron_interface(Graph G, std::string name){
	pid_t pid; 
	int status;

	if ((pid = fork()) == 0){
		cout<<"Child executing.\n"; 
		cout<<"Child done.\n";	
		int ret = execl(BARON_PATH.c_str(), "baron", 
			name.c_str());
		cout<<"Child returned: "<<ret<<endl;
		exit(0);  
	}
	else { 
		cout<<"Parent waiting.\n"; 
		waitpid(pid, &status, 0);
		
	    cout<<"Exit successful.\n";
	}
}

void baron_files(std::vector<Graph> X, std::vector<std::string> &names){
	std::string path = BARON_RES_PATH; 
	for(int i=0; i<X.size()-1;++i){
		std::string spec = create_baron_file(X[i], i);
		std::string name = path+"/program_"+std::to_string(i)+".bar";

		ofstream programfile; 
		programfile.open(name);
		programfile<<spec; 
		programfile.close();
		names.push_back(name); 
	}
}

void baron_interface(std::vector<std::string> &names){

	pid_t wpid,pid,pids[names.size()]; 
	int status,i;

	#pragma omp parallel private(i) //num_threads(3)  
	{
		#pragma omp for nowait schedule(guided) 
		for(i=0; i<names.size()-1;++i){
			if ((pid = fork()) == 0){
				int ret = execl(BARON_PATH.c_str(), "baron", 
					names[i].c_str());
				cout<<"Error in executing BARON: "<<ret<<endl;
				exit(0);  
			}
		}
	}
 
	cout<<"Parent waiting.\n"; 
	// for(int i =0; i < X.size(); ++i)
	//  	waitpid(pids[i], &status, 0);	
	while ((wpid = wait(&status)) > 0)
		;

    cout<<"Computation done in parallel.\n";
}

void check_sol_dir(){
	//Check if sol path exists
	struct stat sb; 
	char * cstr = new char [SOL_PATH.length()+1];
  	std::strcpy (cstr, SOL_PATH.c_str());
	
	stat(cstr, &sb);
	cout<<"Checking directories: "<<endl<<cstr<<endl; 

	if(!S_ISDIR(sb.st_mode)){
		cout<<"Creating solution directory (Linux/UNIX/Mac)."<<endl; 
		mkdir(cstr, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	cstr = new char [P_SOL_PATH.length()+1];
  	std::strcpy (cstr, P_SOL_PATH.c_str());

  	cout<<cstr<<endl; 

  	memset(&sb, 0, sizeof(sb)); 
	
	stat(cstr, &sb);
	if(!S_ISDIR(sb.st_mode)){
		cout<<"Creating parallel solution directory (Linux/UNIX/Mac)."<<endl; 
		mkdir(cstr, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	cstr = new char [BARON_RES_PATH.length()+1];
  	std::strcpy (cstr, BARON_RES_PATH.c_str());

  	cout<<cstr<<endl; 

  	memset(&sb, 0, sizeof(sb)); 
	
	stat(cstr, &sb);
	if(!S_ISDIR(sb.st_mode)){
		cout<<"Creating baron solution directory (Linux/UNIX/Mac)."<<endl; 
		mkdir(cstr, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
}


void test_gen_BARON(int goallayers, int subgoals, int rules, int facts){
	GraphGenerator GG; 
	cout<<"Generating a synthetic AG: layers: "<<goallayers<<", subgoals: "<<subgoals<<", rules: "<<rules<<", facts: "<<facts<<endl; 
	//int goallayers, int subgoals, int rules, int facts
	Graph G = GG.treetopology(goallayers,subgoals,rules,facts);



	G.print(SOL_PATH+"graph.dot");

	cout<<"Number of nodes: "<<G.size()<<endl; 

	std::string spec = create_baron_file(G, 0);
	std::string path = BARON_RES_PATH; 
	std::string name = path+"/program_"+std::to_string(0)+".bar";
	ofstream programfile; 
	programfile.open(name);
	programfile<<spec; 
	programfile.close();
	baron_interface(G, name); 	
}


void test_gen_BARON_omp(int goallayers, int subgoals, int rules, int facts){
	GraphGenerator GG; 
	cout<<"Generating a synthetic AG: layers: "<<goallayers<<", subgoals: "<<subgoals<<", rules: "<<rules<<", facts: "<<facts<<endl; 
	//int goallayers, int subgoals, int rules, int facts
	Graph G = GG.treetopology(goallayers,subgoals,rules,facts);
	G.print(SOL_PATH+"graph.dot");

	std::vector<Graph> X = G.partitiongraph(); 

	cout<<"Number of nodes: "<<G.size()<<endl; 
	cout<<"Number of subgraphs: "<<X.size()<<endl; 


	std::vector<std::string> names;
	baron_files(X, names); 
	baron_interface(names);
}

void collectres(size_t size){
	std::string sizestr = std::to_string(size); 
	pid_t pid,wpid;
	int status; 
	std::string path = HPSA_PATH+"/readres.py"; 

	if ((pid = fork()) == 0){
		int ret = execl(path.c_str(), "readres.py", 
			sizestr.c_str());
		cout<<"Error in executing Python script: "<<ret<<endl;
		exit(0);  
	}
	while ((wpid = wait(&status)) > 0)
		;

    cout<<"Done collecting results.\n";
}

/*
Run improvement options in parallel using a combination of openmp and BARON threads. 
*/

void test_gen_BARON_imp(int goallayers, int subgoals, int rules, int facts, int P){
	GraphGenerator GG; 
	cout<<"Generating a synthetic AG: layers: "<<goallayers<<", subgoals: "<<subgoals<<", rules: "<<rules<<", facts: "<<facts<<endl; 
	//int goallayers, int subgoals, int rules, int facts
	Graph G = GG.treetopology(goallayers,subgoals,rules,facts);
	G.print(SOL_PATH+"graph.dot");

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
	This for-loop goes for each rule node as a a candidate
	*/

	#ifdef EACH_RULE_CANDIDATE

	for(int i =0; i<size; ++i){
		std::vector<int> v;
		v.push_back(candidate_nodes[i]); 

		std::string spec = create_baron_file(G, i, v, improvement);
		std::string name = BARON_RES_PATH + "/program_"+std::to_string(i)+".bar";
		ofstream programfile; 
		programfile.open(name);
		programfile<<spec; 
		programfile.close();
		names.push_back(name); 
	}
	#else 

	/*
	This for-loop takes subsets of rule nodes in each iteration (more than one). 
	*/
	int i,j=0,k; 
	int subset_size = candidate_nodes.size() / P; 

	if(size <= P){
		test_gen_BARON_imp_serial(goallayers,subgoals,rules,facts);
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

	#endif 

	baron_interface(names); 
	collectres(size); 
}

/* 
Run improvement options serially.  
*/
void test_gen_BARON_imp_serial(int goallayers, int subgoals, int rules, int facts){
	GraphGenerator GG; 
	cout<<"Generating a synthetic AG: layers: "<<goallayers<<", subgoals: "<<subgoals<<", rules: "<<rules<<", facts: "<<facts<<endl; 
	//int goallayers, int subgoals, int rules, int facts
	Graph G = GG.treetopology(goallayers,subgoals,rules,facts);
	G.print(SOL_PATH+"graph.dot");

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
	collectres(2); 
}




int main(int argc, char**argv){
	int goallayers, subgoals, rules, facts, choice; 

	if(argc < 6){
		cout<<"Give me arguments."<<endl;
		return -1; 
	}

	Graph G; 

	goallayers = atoi(argv[1]);
	subgoals = atoi(argv[2]);
	rules = atoi(argv[3]);
	facts = atoi(argv[4]);
	choice = atoi(argv[5]); 

	check_sol_dir(); 

	//test_gen(goallayers,subgoals,rules,facts,choice);
	if (choice == 0){
		int P = atoi(argv[6]); 
		test_gen_BARON_imp(goallayers,subgoals,rules,facts,P);
	}
	else if (choice == 1)
		test_gen_BARON_imp_serial(goallayers,subgoals,rules,facts);
	/* else if (choice == 2)
		test_nlopt(goallayers,subgoals,rules,facts,0);
	else if (choice == 3){
		graph_generator(G, goallayers, subgoals, rules, facts); 
		test_gen_KNITRO(G); 
	}*/
	return 0;
}
