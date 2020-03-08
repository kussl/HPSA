#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <thread>
#include <sstream>

#include "omp.h"

#ifndef AG_GRAPH_HEADER
#define AG_GRAPH_HEADER 1 
#include "graph.h"
#endif 

#ifndef AG_HPSA_HEADER
#define AG_HPSA_HEADER 1 
#include "hpsa.h"
#endif 

using namespace std; 

extern std::string create_baron_file(Graph &G, int i); 
extern std::string create_baron_file(Graph &G, int index, std::vector<int> candidate_nodes, double improvement); 


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

void baron_interface(std::string name){
	pid_t pid; 
	int status;

	if ((pid = fork()) == 0){
		int ret = execl(BARON_PATH.c_str(), "baron", 
			name.c_str(), NULL);
		exit(0);  
	}
	else { 
		waitpid(pid, &status, 0);
	}
}

void baron_files(std::vector<Graph> X, std::vector<std::string> &names){
	std::string path = BARON_RES_PATH; 
	for(int i=0; i<X.size();++i){
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
	int status,i, size = names.size(); 

	unsigned no_threads = std::thread::hardware_concurrency();
	cout<<"No threads available: "<<no_threads<<endl; 

	#pragma omp parallel private(i) num_threads(no_threads/4)  
	{
		#pragma omp for nowait schedule(guided) 
		for(i=0; i<size;++i){
			if ((pid = fork()) == 0){
				int ret = execl(BARON_PATH.c_str(), "baron", 
					names[i].c_str(), NULL);
				cout<<"Error in executing BARON: "<<ret<<endl;
				exit(0);  
			}
		}
	}
 
	while ((wpid = wait(&status)) > 0)
		;

    cout<<"Computation done in parallel.\n";
}

void readsolution(std::vector<Graph> &X){
	std::string line; 
	ifstream file; 
	int no_files = X.size(); 
	for(int i = 0; i < no_files; ++i){
		std::string name = BARON_RES_PATH+"/"+std::to_string(i)+".res";
		file.open(name);
		while ( getline(file, line) ){
			if(line.find("The best solution found is")!= string::npos){
				getline(file, line); 
				getline(file, line); 
				break; 
			}
		} 
		//Now collect the results and record them in the graph.
		int j = 0; 
		int g_size = X[i].size(); 
		while( getline(file, line, ' ')){
			//std::istringstream iss (line);

			if(line.find("x")!=string::npos){
				char *dup = strdup(line.c_str());
				char *token = strtok(dup, "			");
				token++; 
				int index = atoi(token); 
				
				//cout<<index<<" "; 
				token = strtok(NULL, "			");
				token = strtok(NULL, "			");
				double P = atof(token); 
				//cout<<P<<endl; 
				free(dup);
				X[i].update_P(j,P);
				//cout<<X[i].graph_nodes()[j].nodeid()<<" set to "<<P<<endl; 
				++j;  
			}
			if(j == (g_size-1))
				break; 
		}
		file.close(); 
	}
}
