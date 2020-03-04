#include <list>
#include <vector>
#define AUTO_ID -1 
#define AG_GRAPH_HEADER 1 


enum NType {Fact, Rule, Goal};

class Node{
	int node_id; //Node ID, used instead of pointers.
	double P; //Success probability for the node
	NType type; //Type of the node, fact, rule, or goal
	public:
		//Set to public for efficiency. 
		//Predecessors of this node. 
		std::vector<int> preds; 

		//Constructors
		Node (NType type=Fact){
			this->type = type; 
			this->P = 0.8; 
		}
		Node (int id,NType type, double P) { 
			this->node_id = id; 
			this->type = Fact;  
			this->type = type;
			this->P = P; 
		}
		void set(int id){ this->node_id = id;} 
		bool isnode(int id){ return this->node_id == id; }
		int nodeid() { return this->node_id; }
		NType nodetype(){ return this->type; }
		double nodeP() { return this->P; }
		void updateP(double P) { this->P = P; }
		
};

class Edge{
	int from; 
	int to; 
public:
	Edge(int from, int to){
		this->from = from; 
		this->to = to; 
	}
	bool isedge(int from, int to){ return this->from == from && this->to == to; }
	bool inedge(int to){ return this->to == to; }
	bool outedge(int from){ return this->from == from; }
	int from_node(){ return this->from; }
	int to_node()  { return this->to; }
};

class Graph{
	std::list<Node> nodes; 
	std::list<Edge> edges; 
	std::vector<Node> gnodes;
	std::vector<Node*> nodeindexes;

	std::vector<double> prob;
public:
	Graph(){

	} 
	int addnode(int node_id, NType type, double P);
	bool nodeexists(int node_id, std::list<Node>::iterator &it);
	bool nodeexists(int node_id);
	bool nodeexists(int node_id, Node &N);
	bool removenode(int node_id);
	bool addedge(int from, int to);
	bool edgeexists(int from,int to,std::list<Edge>::iterator &it);
	bool removeedge(int from, int to);
	std::list<int> predecessors(int node_id);
	std::vector<int> preds(int node_id);
	std::vector<int> gnodepreds(int node_id);
	std::list<int> successor(int node_id);
	void update_P(std::vector<double> x);
	void print(std::string filename="graph.dot");
	std::list<Node>::iterator graph_nodes(); 
	std::list<Edge>::iterator graph_edges();
	void fillprobabilities(); 
	void fillpreds(); 
	double getnodeprob(int node_id); 

	void construct_node_indexes(); 

	void addpredecessors(int node_id, int g_node_id,Graph &G); 
	std::vector<Graph> partitiongraph();

	std::vector<int> imp_candidate_nodes(); 

	int size() { return this->nodes.size(); }
	int noedges() { return this->edges.size(); }	
};

class GraphGenerator{
public: 
	GraphGenerator() {
	}
	Graph treetopology(int goallayers=1, int subgoals=2, int rules=2, int facts=2);
};

class ImpInstrument {
	Node improvement; 
	std::vector<Node> targets;
}

