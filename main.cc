#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <ctime>
#include <chrono>
#include <sys/stat.h>
#include <math.h>
#include <cstring>


#include "graph.h"
#include "hpsa.h"


using namespace std;
extern void test_gen_BARON_imp_serial(Graph G); 
extern void test_gen_BARON_imp(Graph G, int P);
extern void test_gen_BARON_omp(Graph G); 
extern void test_gen_BARON(Graph G); 
extern void test_BARON_multiple_improvements(Graph G, int m, int k, bool parallel); 
extern void test_BARON_multiple_improvements(Graph G, bool parallel, int P); 
extern void generate_graph(Graph &G, int goallayers, int subgoals, int rules, int facts); 

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

	generate_graph(G, goallayers,subgoals,rules,facts); 

	if (choice == 0){
		int P = atoi(argv[6]); 
		test_gen_BARON_imp(G,P);
	}
	else if (choice == 1){
		test_gen_BARON_imp_serial(G);
	}
	else if (choice == 2){
		test_gen_BARON_omp(G); 
	}
	else if (choice == 3){
		test_gen_BARON(G); 
	}
	else if (choice == 4){
		int m = G.count_type(Rule) / 3; 
		int k;
		if(m > 2)
			k = m / 2; 
		else
			k = 1;  
		test_BARON_multiple_improvements(G, m, k, false); 
	}
	else if (choice == 5){
		int m = G.count_type(Rule) / 3; 
		int k;
		if(m > 2)
			k = m / 2; 
		else
			k = 1;  
		test_BARON_multiple_improvements(G, m, k, true); 
	}
	else if (choice == 6){
		int P = atoi(argv[6]); 
		test_BARON_multiple_improvements(G, true, P); 
	}
	return 0;
}
