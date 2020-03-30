#include <iostream>
#include <fstream>
#include <random>

#include "graph.h"

using namespace std;

int Graph::addnode(int node_id, NType type, double P){
	if(node_id == AUTO_ID){
		node_id = this->nodes.size();
	}
	Node N(node_id,type,P); 
	nodes.push_back(N); 
	gnodes.push_back(N); 
	return node_id;
}

// bool Graph::nodeexists(int node_id, std::list<Node>::iterator &it){
// 	int size = this->node.size(); 
// 	for(int i = 0; i < size; ++i)
// 		if(this->nodes[i].isnode(node_id))
// 			return true; 
// 	return false;  
// }

bool Graph::nodeexists(int node_id){
	int size = this->nodes.size(); 
	for(int i = 0; i < size; ++i)
		if(this->nodes[i].isnode(node_id))
			return true; 
	return false;  
}

bool Graph::nodeexists(int node_id, Node &N){
	int size = this->nodes.size(); 
	for(int i = 0; i < size; ++i)
		if(this->nodes[i].isnode(node_id)){
			N = this->nodes[i]; 
			return true; 
		}
	return false;  
}

bool Graph::removenode(int node_id) {
	std::vector<Node>::iterator it;
	for(it = this->nodes.begin(); it!=this->nodes.end();++it)
		if(it->isnode(node_id)){
			this->nodes.erase(it); 
			return true; 
		}
	return false; 
}

bool Graph::addedge(int from, int to){ 
	if(!nodeexists(from) || !nodeexists(to)){
		return false; 
	}
	Edge E(from, to); 
	this->edges.push_back(E);
	return true; 
}

bool Graph::edgeexists(int from,int to){
	int size = this->edges.size(); 
	for(int i = 0; i < size; ++i)
		if(this->edges[i].isedge(from, to)){
			return true; 
		}
	return false;  
}

bool Graph::removeedge(int from, int to){
	std::vector<Edge>::iterator it;
	for(it = this->edges.begin(); it!=this->edges.end();++it)
		if(it->isedge(from, to)){
			this->edges.erase(it); 
			return true; 
		}
	return false; 
}

std::list<int> Graph::predecessors(int node_id){
	std::list<int> P; 
	std::vector<Edge>::iterator it;
	for (it = this->edges.begin(); it != this->edges.end(); ++it){
			if(it->inedge(node_id)){
				P.push_back(it->from_node());
			}
	}
	return P;
}

std::vector<int> Graph::preds(int node_id){
	std::vector<int> P; 
	std::vector<Edge>::iterator it;
	for (it = this->edges.begin(); it != this->edges.end(); ++it){
			if(it->inedge(node_id)){
				P.push_back(it->from_node());
			}
	}
	return P;
}

std::vector<int> Graph::gnodepreds(int node_id){
	return this->gnodes[node_id].preds; 
}


std::list<int> Graph::successor(int node_id){
	std::list<int> P; 
	std::vector<Edge>::iterator it;
	for (it = this->edges.begin(); it != this->edges.end(); ++it){
			if(it->outedge(node_id)){
				P.push_back(it->to_node());
			}
	}
	return P;
}

void Graph::update_P(std::vector<double> x) {
	int size = x.size(); 
	for(int i = 0; i <  size; ++i){
		this->nodes[i].updateP(x[i]); 
	}
}

void Graph::update_P(int index, double P) {
	this->nodes[index].updateP(P); 
}

void Graph::print(std::string filename){
	std::vector<Edge>::iterator it;
	std::vector<Node>::iterator nit;
	ofstream dotfile; 
	dotfile.open(filename);
	dotfile<< "digraph G {\n";
	for (nit = this->nodes.begin(); nit!=this->nodes.end(); ++nit){
		if(nit->nodetype() == Fact){
			dotfile << nit->nodeid() << " [shape=box,label=\""<< nit->nodeid()<<" ("<< nit->nodeP() <<")"<< "\"] \n";
		} else if(nit->nodetype() == Rule){
			dotfile << nit->nodeid() << " [shape=ellipse,label=\""<< nit->nodeid()<<" ("<< nit->nodeP() <<")"<< "\"] \n";
		}
		else { 
			dotfile << nit->nodeid() << " [shape=circle,label=\""<< nit->nodeid()<<" ("<< nit->nodeP() <<")"<< "\"] \n";
		}
	}
	for (it = this->edges.begin(); it != this->edges.end(); ++it){
		Node from, to; 
		this->nodeexists(it->from_node(), from); 
		this->nodeexists(it->to_node(), to); 
		dotfile << from.nodeid() <<  "->" << to.nodeid()<<endl; 
	}
	dotfile <<"}";
	dotfile.close();
}

std::vector<Node>::iterator Graph::graph_nodes(){
	std::vector<Node>::iterator nit;
	nit = this->nodes.begin(); 
	return nit; 
}


std::vector<Edge>::iterator Graph::graph_edges(){
	std::vector<Edge>::iterator eit;
	eit = this->edges.begin(); 
	return eit; 
}


void Graph::fillprobabilities(){
	int n = this->size(); 
	std::vector<Node>::iterator nit = this->graph_nodes(); 
	for (int i =0; i<n; i++,++nit){
		prob.push_back(nit->nodeP()); 
	}
}

void Graph::fillpreds(){
	int n = this->size(); 
	std::vector<Node>::iterator nit = this->graph_nodes(); 
	for (int i =0; i<n; i++,++nit){
		std::vector<int> v = preds(nit->nodeid());
		nit->preds = v; 
		this->gnodes[i].preds = v; 
	}
}

double Graph::getnodeprob(int node_id){
	return this->prob[node_id]; 
}

void Graph::construct_node_indexes(){
	int n = this->size(); 
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < n; ++j)
			if(i == this->gnodes[j].nodeid()){
				this->nodeindexes.push_back(&this->gnodes[j]);
				break;  
			}
	}
}


/*
Receive an attack graph and separate it as multiple 
independent subgraphs for which the probabilities can be 
computed independently. 
1. For each goal node u other than the leaf node, create a subgraph G 
of the original attack graph AG
with all the predecessors of u. Add G to a set of subgraphs X.
2. For each subgraph G in X, compute ECSA. 
3. Create a graph H with a fact node for the leaf node of each subgraph
G in X. 
4. Set the ECSA for the fact nodes in H as the computed ECSA of the
leaf nodes in X. 
5. Complete the graph H by adding all the children of each leaf node
in each G in X. 
6. Compute the ECSA for H. 
The conjecture is that the ECSA for the leaf node in H equals the
ECSA for the leaf node in AG. 
*/


void Graph::addpredecessors(int node_id, int g_node_id, Graph &G){
	std::list<int> pred = predecessors(node_id); 
	if(pred.size()==0)
		return; 
	std::list<int>::iterator pit;
	for(pit = pred.begin(); pit != pred.end(); ++pit){
		Node N; 
		nodeexists(*pit, N); 
		int new_node_id = G.addnode(N.nodeid(), N.nodetype(), N.nodeP()); 
		G.addedge(new_node_id,g_node_id);
		addpredecessors(N.nodeid(), new_node_id, G); 
	}
}


std::vector<Graph> Graph::partitiongraph(){
	std::vector<Node>::iterator it; 
	std::vector<Graph> X; 

	for (it = this->nodes.begin(); it != this->nodes.end(); ++it){
		if(it->nodetype()==Goal) {
			//is this the leaf? 
			if(it->nodeid() == 0){
				continue;
			}
			std::list<int> pred = predecessors(it->nodeid());  
			//Otherwise, make a graph for it. 
			Graph G; 
			int node_id = G.addnode(it->nodeid(), Goal, 0); 
			//Add all predecessor nodes and edges. 
			addpredecessors(it->nodeid(), node_id, G); 
			X.push_back(G); 
		}
	}
	return X; 
}

Graph Graph::combine_subtrees(std::vector<Graph> X){
	Graph AG;
	//iterator for X:
	int k = 0; 

	//Add the leaf node 
	int leaf_id = AG.addnode(AUTO_ID, Goal, 0); 
	//Its predecessors come from G 
	std::vector<int> pred = this->preds(0);
	int no_preds = pred.size(); 
	for(int i = 0; i < no_preds; ++i){
		AG.addnode(pred[i], Rule, 0); 
		AG.addedge(pred[i], leaf_id);	
		
		// Add the leaf nodes in the subtrees of X as
		// predecessors of the rule nodes in AG. 
		 
		std::vector<int> rule_pred = this->preds(pred[i]); 
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



std::vector<int> Graph::imp_candidate_nodes(){
	std::vector<int> candidates;
	std::vector<Node>::iterator it; 
	for (it = this->nodes.begin(); it != this->nodes.end(); ++it){
		if(it->nodetype()==Rule) {
			candidates.push_back(it->nodeid()); 
		}
	}
	return candidates; 
}

int Graph::count_type(NType type){
	int size = this->nodes.size(), counter = 0 ; 
	for(int i = 0; i < size; ++i)
		if(this->nodes[i].nodetype() == type)
			++counter; 
	return counter; 
}

Graph GraphGenerator::treetopology(int goallayers, int subgoals, int rules, int facts){
	Graph G; 
	//Create graph goal 
	int ultimate_goal = G.addnode(AUTO_ID,Goal,0); 
	double P = .8; 
	std::random_device rd;
	std::mt19937 e2(rd());
	std::uniform_real_distribution<> dist(0.5, .9);

	//Layers of goal to fact relationships
	for (int l=0; l<goallayers; ++l){
		int ultimate_rule = G.addnode(AUTO_ID,Rule,0); 
		G.addedge(ultimate_rule,ultimate_goal);
		
		//Create subgoals	
		for(int i=0; i<subgoals; ++i){
			int sub_goal_id = G.addnode(AUTO_ID,Goal,0);
			G.addedge(sub_goal_id,ultimate_rule);

			//For each goal, create rules
			for (int j=0; j<rules; ++j){
				int rule_id = G.addnode(AUTO_ID,Rule,0); 
				G.addedge(rule_id, sub_goal_id);
				
				//For each rule, create facts
				for (int k=0; k<facts; ++k){
					//P = dist(e2); 
					if(k%2 == 0)
						P = 0.99;
					else if(k%3 == 0)
						P = 0.98; 
					else if(k%4 == 0)
						P = 1;
					else 
						P = 1;
					int fact_id = G.addnode(AUTO_ID,Fact,P); 
					G.addedge(fact_id, rule_id);
				}
			}
		}
	}
	G.construct_node_indexes(); 
	G.fillpreds(); 

	return G; 
}

/*
In this topology, a total of L layers are created. 
The first layer contains only the ultimate goal node and the last layer contains only the fact nodes. 
In the L-2 intermediate layers, we alternate between generating rule or goal nodes based on the type of
the predecessor. 
For each layer, we generate the appropriate type of nodes and attach them to the nodes created in the last layer. 
*/

Graph GraphGenerator::hierarchical_topology(int L, int goal_nodes, int rule_nodes, int fact_nodes){
	Graph G; 
	
	//Add the ultimate goal node. 
	int ultimate_goal = G.addnode(AUTO_ID,Goal,0); 
	NType last_layer_type = Goal; 
	std::vector<int> last_layer_nodes;
	last_layer_nodes.push_back(ultimate_goal);
	NType this_layer_type = Goal;

	for(int l = 1; l < (L-1); ++l){
		std::vector<int> this_layer_nodes;
		int no_nodes_to_add = rule_nodes; 
		
		if(last_layer_type == Rule){
			this_layer_type = Goal;
			no_nodes_to_add = goal_nodes; 
		}else if(last_layer_type == Goal){
			this_layer_type = Rule;
			no_nodes_to_add = rule_nodes; 
		}

		//cout<<"Type: "<<this_layer_type<<", last: "<<last_layer_type<<", nodes to add: "<<no_nodes_to_add<<", layer: "<<l<<endl;  

		int no_nodes_last = last_layer_nodes.size(); 

		for(int i = 0; i < no_nodes_last ;++i){
			for (int j = 0; j < no_nodes_to_add; ++j){
				int node_id = G.addnode(AUTO_ID,this_layer_type,0);
				G.addedge(node_id,last_layer_nodes[i]);
				this_layer_nodes.push_back(node_id); 
			}
		}

		last_layer_nodes.clear();
		int added_nodes = this_layer_nodes.size();  
		for(int i = 0; i < added_nodes; ++i)
			last_layer_nodes.push_back(this_layer_nodes[i]); 
		last_layer_type = this_layer_type; 

	}

	//Now add the fact nodes.
	int no_nodes_last = last_layer_nodes.size(); 

	//Initial fact value
	double P = 1; 

	for(int i = 0; i < no_nodes_last ;++i){
		for (int j = 0; j < fact_nodes; ++j){
			if(j%2 == 0)
				P = 0.99;
			else if(j%3 == 0)
				P = 0.98;
			else if(j%4 == 0)
				P = 0.97; 

			int node_id = G.addnode(AUTO_ID,Fact,P);
			G.addedge(node_id,last_layer_nodes[i]);
		}
	}
	return G; 
}

