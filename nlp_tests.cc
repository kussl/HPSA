//Testing various interfaces
extern std::vector<double> run_nlp(Graph &G); 

void test_nlopt(int goallayers, int subgoals, int rules, int facts, bool parallel){
	std::time_t result = std::time(nullptr);
	clock_t begin_time; 

	GraphGenerator GG; 
	cout<<"Generating a synthetic AG.\n"; 
	//int goallayers, int subgoals, int rules, int facts
	Graph G = GG.treetopology(goallayers,subgoals,rules,facts);
	//Separate the graph into independent subgraphs.
	std::vector<Graph> X = G.partitiongraph(); 

	cout<<"Number of nodes: "<<G.size()<<endl; 
	cout<<"Number of subgraphs: "<<X.size()<<endl; 
	 
	if (parallel){
		begin_time = clock();

		#pragma omp parallel num_threads(X.size())
		{
			#pragma omp for
			for(int i = 0; i<X.size(); ++i){
				std::vector<double> x = run_nlp(X[i]);
				X[i].update_P(x); 
				X[i].print(P_MAIN_PATH+to_string(i)+".dot");
			}
		}
		//timestamp(begin_time); 

		//Combine graphs
		Graph AG;
		//Add the leaf node 
		int leaf_id = AG.addnode(AUTO_ID, Goal, 0); 
		//Its predecessors come from G 
		std::list<int> pred = G.predecessors(0);
		std::list<int>::iterator it;
		std::vector<int> rules, goals; 
		
		for (it = pred.begin(); it != pred.end(); ++it){
			int node_id = AG.addnode(AUTO_ID, Rule, 0);  
			rules.push_back(node_id); 
			AG.addedge(node_id, leaf_id); 
		}
		//divide the subgraphs in X among the predecessors 
		int k = 0; 

		//Now link them.
		//For each subgraph, add its leaf to AG.
		for (int i = 0; i<X.size(); ++i){
			std::list<Node>::iterator nodes = X[i].graph_nodes(); 
			//add the leaf as a fact
			int node_id = AG.addnode(AUTO_ID, Fact, nodes->nodeP()); 
			//cout<<rules[k]<<" "; 
			AG.addedge(node_id, rules[k]); 
			k = (k + 1)% goallayers; 
		}
		
		std::vector<double> x = run_nlp(AG);

		timestamp(begin_time); 

		AG.update_P(x);  
		AG.print(P_MAIN_PATH+"x.dot"); 

	}
	else { 
		begin_time = clock();
		std::vector<double> x = run_nlp(G);
		timestamp(begin_time); 

		G.update_P(x); 
		G.print(MAIN_PATH+"graph.dot");
		//Print solution to file. 
		ofstream sol; 
		sol.open(MAIN_PATH+"sol.csv");
		for (int i=0; i<x.size(); ++i){
    		sol<<i<<","<<x[i]<<endl; 
    	}
	}		
}