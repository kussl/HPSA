
#include "AGknitro.h"


double rule_constraints(int index, Node v, Graph &G, std::vector<double> &x){
	std::vector<int> P = G.preds(index);
	double constraint = 1.0; 
	
	for(int i = 0; i < P.size(); ++i){
		constraint *= x[P[i]];
	}
	return constraint - x[index]; 
}
double goal_constraints(int index, Node v, Graph &G, std::vector<double> &x){
	std::vector<int> P = G.preds(index);
	double constraint = 0.0; 
	double weight = 1.0/P.size();

	for(int i = 0; i < P.size(); ++i){
		constraint += (weight*x[P[i]]);
	}

	return constraint - x[index]; 
}
double fact_constraints(int index, Node v, std::vector<double> &x){
	return v.nodeP() - x[index]; 
}

void set_constraints(Graph G, std::vector<double> &x, std::vector<double> &c){
	int n = G.size(); 
	std::vector<Node>::iterator nit = G.graph_nodes(); 
	std::vector<Node>::iterator onit = nit; 

	for (int i=0; i<n; ++i){
		int nodetype = nit->nodetype(); 

		if (nodetype==Rule) {
			c[i] = rule_constraints(i,*nit,G,x); 
		}
		else if (nodetype==Goal) {
			c[i] = goal_constraints(i,*nit,G,x); 
		}
		else if (nodetype==Fact){
			c[i] = fact_constraints(i,*nit,x); 
		}
		++nit;
	}
}


// double AGProblem::evaluateFC(
// 	std::vector<double>& x, //variables
// 	std::vector<double>& c, //constraints
// 	std::vector<double>& objGrad,
// 	std::vector<double>& jac, 
// 	Graph G){

// 	set_constraints(G,x,c);  

// 	return x[0];
// }