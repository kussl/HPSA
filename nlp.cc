#include <iomanip>
#include <iostream>
#include <vector>
#include <nlopt.hpp>
#include "graph.h"

using namespace std; 

typedef struct constraint_data { 
	Graph *G; 
	int node_id; 
	double P; 
}constraint_data; 


double ultimate_goal(const std::vector<double> &x, std::vector<double> &grad, void *my_func_data){
	if (!grad.empty()) {
		for(int i = 0; i<x.size(); ++i){
			grad[i] = 0; 
		}
		grad[0] = 1; 
	}
	return x[0]; 
}

double rule_constraint(const std::vector<double> &x, std::vector<double> &grad, void *data){
	constraint_data *d = reinterpret_cast<constraint_data*>(data);
	std::vector<int> P = d->G->preds(d->node_id);
	double constraint = 1.0; 
	
	if (!grad.empty()) {
		// for(int i = 0; i<x.size(); ++i){
		// 	grad[i] = 0; 
		// }
		
		// for(int j = 0; j<P.size(); ++j){
		// 	grad[j] = 1; 
		// 	for(int i = 0; i < P.size(); ++i){
		// 		if(i==j)
		// 			continue; 
		// 		grad[i] *= x[P[i]];
		// 	}
		// }
	}

	for(int i = 0; i < P.size(); ++i){
		constraint *= x[P[i]];
	}
	return constraint - x[d->node_id]; 
}

double goal_constraint(const std::vector<double> &x, std::vector<double> &grad, void *data){
	constraint_data *d = reinterpret_cast<constraint_data*>(data);
	std::vector<int> P = d->G->preds(d->node_id);
	double constraint = 0.0; 
	double weight = 1.0/P.size();

	if (!grad.empty()) {
		// for(int i = 0; i<x.size(); ++i){
		// 	grad[i] = 0; 
		// }
		
		// for(int j = 0; j<P.size(); ++j){
		// 	grad[j] = 0; 
		// 	for(int i = 0; i < P.size(); ++i){
		// 		if(i==j)
		// 			continue; 
		// 		grad[i] += (weight*x[P[i]]);
		// 	}
		// }
	}

	for(int i = 0; i < P.size(); ++i){
		constraint += (weight*x[P[i]]);
	}

	return constraint - x[d->node_id]; 
}

double fact_constraint(const std::vector<double> &x, std::vector<double> &grad, void *data){
	constraint_data *d = reinterpret_cast<constraint_data*>(data);
	return d->P - x[d->node_id]; 
}

std::vector<double> run_nlp(Graph &G){
	int n = G.size(); 

	std::list<Node>::iterator nit = G.graph_nodes();

	//LN_COBYLA, LD_SLSQP, LD_MMA
	nlopt::opt opt(nlopt::LN_COBYLA, n);
	std::vector<double> lb(n),ub(n);

	opt.set_max_objective(ultimate_goal, NULL);

	constraint_data data[n]; 

	opt.set_xtol_rel(1e-4);
	//opt.set_ftol_rel(0.5);
	//opt.set_maxtime(5*60);

	std::vector<double> x(n);

	//Supply the constraints 
	//Iterate through the nodes of the graph and add rule constraints for rule nodes. 

	for(int i=0; i< n; ++i){
		lb[i] = 0; 
		ub[i] = 1; 
		data[i].G = &G;
		data[i].node_id = nit->nodeid();
		//cout<<nit->nodeid()<<":"<<nit->nodeP()<<":"<<nit->nodetype()<<" "; 
		if (nit->nodetype()==Rule) {
			opt.add_equality_constraint(rule_constraint, &data[i], 1e-8); 
		} else if (nit->nodetype()==Goal) {	 
			opt.add_equality_constraint(goal_constraint, &data[i], 1e-8); 
		} else if(nit->nodetype()==Fact){
			data[i].P = nit->nodeP(); 
			opt.add_equality_constraint(fact_constraint, &data[i], 1e-8); 
		}
		++nit; 
	} 
	//cout<<endl; 

	opt.set_lower_bounds(lb);
	opt.set_upper_bounds(ub);

	double f;

	try{
	    nlopt::result result = opt.optimize(x, f);
	    cout<<"Solution found: "; 
	    cout<< std::setprecision(4) << f << std::endl;
	    /*for(int i =0; i<x.size(); ++i)
	    	cout<<x[i]<<" "; 
	   	cout<<endl;*/ 

	}
	catch(std::exception &e) {
	    std::cout << "nlopt failed: " << e.what() << std::endl;
	}

	return x; 
}
