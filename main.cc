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

extern void test_BARON_multiple_improvements(Graph G, short parallel,  int P); 
extern void test_BARON_single_placement_serial(Graph G); 
extern void test_BARON_single_placement_parallel(Graph G, int P); 
extern void generate_graph(Graph &G, int goallayers, int subgoals, int rules, int facts); 


extern void propagate_probabilities(Graph G, int P); 
extern void improve_security(Graph G, int P); 

void check_sol_dir(){
	//Check if sol path exists
	struct stat sb; 
	char * cstr = new char [SOL_PATH.length()+1];
  	std::strcpy (cstr, SOL_PATH.c_str());
	
	stat(cstr, &sb);
	//cout<<"Checking directories: "<<endl<<cstr<<endl; 

	if(!S_ISDIR(sb.st_mode)){
		cout<<"Creating solution directory (Linux/UNIX/Mac)."<<endl; 
		mkdir(cstr, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	cstr = new char [P_SOL_PATH.length()+1];
  	std::strcpy (cstr, P_SOL_PATH.c_str());

  	//cout<<cstr<<endl; 

  	memset(&sb, 0, sizeof(sb)); 
	
	stat(cstr, &sb);
	if(!S_ISDIR(sb.st_mode)){
		cout<<"Creating parallel solution directory (Linux/UNIX/Mac)."<<endl; 
		mkdir(cstr, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	cstr = new char [BARON_RES_PATH.length()+1];
  	std::strcpy (cstr, BARON_RES_PATH.c_str());

  	//cout<<cstr<<endl; 

  	memset(&sb, 0, sizeof(sb)); 
	
	stat(cstr, &sb);
	if(!S_ISDIR(sb.st_mode)){
		cout<<"Creating baron solution directory (Linux/UNIX/Mac)."<<endl; 
		mkdir(cstr, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
}


/*
Some frequently used graph configurations.
*/
void graph_options(Graph &G, int option){
	switch (option){
		case 1: 
			generate_graph(G, 4,6,8,15);
			break; 
		case 2: 
			generate_graph(G,5,6,8,15);
			break;
		case 3:
			generate_graph(G,6,6,8,15);
			break; 
		case 4:
			generate_graph(G,5,8,8,15);
			break; 
		case 5:
			generate_graph(G,6,8,8,15);
			break;
		case 6:
			generate_graph(G,7,8,8,15);
			break; 
		case 7:
			generate_graph(G,6,10,8,15);
			break; 
		case 8:
			generate_graph(G,8,8,10,12);
			break;
		case 9:
			generate_graph(G,7,8,10,15);
			break; 
		case 10:
			generate_graph(G,8,8,10,15);
			break;
	}	

}


int main(int argc, char**argv){
	char * choice; 
	int P = 2; 

	if(argc < 3){
		cout<<"Give me arguments."<<endl;
		return -1; 
	}

	check_sol_dir(); 

	Graph G; 

	graph_options(G, atoi(argv[1])); 

	choice = argv[2]; 

	if(!strcmp(choice,"--propagate")){
		propagate_probabilities(G, P); 
	}
	else if(!strcmp(choice,"--improve")){
		cout<<"Improvement.\n";
		improve_security(G, P); 
	}

	/*goallayers = atoi(argv[1]);
	subgoals = atoi(argv[2]);
	rules = atoi(argv[3]);
	facts = atoi(argv[4]);
	choice = atoi(argv[5]); 

	check_sol_dir(); 

	generate_graph(G, goallayers,subgoals,rules,facts); 


	
	//k=1 m>1 parallel improvement
	if (choice == 0){
		int P = atoi(argv[6]); 
		test_gen_BARON_imp(G,P);
	}
	//k=1 m>1 serial improvement
	else if (choice == 1){
		test_gen_BARON_imp_serial(G);
	}
	//Parallel propagation 
	else if (choice == 2){
		int P = atoi(argv[6]); 
		test_gen_BARON_omp(G, P); 
	}
	//Serial Propagation
	else if (choice == 3){
		test_gen_BARON(G); 
	}
	//k>1, m>1 serial improvement
	else if (choice == 4){
		test_BARON_multiple_improvements(G, 0, 0); 
	}
	//k>1, m>1 multiple improvement (improvement partitioning)
	else if (choice == 5){ 
		int P = atoi(argv[6]); 
		test_BARON_multiple_improvements(G, 1, P); 
	}
	//k>1, m>1 multiple improvement (tree partitioning)
	else if (choice == 6){
		int P = atoi(argv[6]); 
		test_BARON_multiple_improvements(G, 2, P); 
	}
	//k>1, m=1 single placement improvement (serial)
	else if (choice == 7){
		test_BARON_single_placement_serial(G); 
	}
	//k>1, m=1 single placement improvement (serial)
	else if (choice == 8){
		int P = atoi(argv[6]); 
		test_BARON_single_placement_parallel(G,P); 
	}*/



	return 0;
}
