/*
This is the knitro interface for solving the attack graph probability propagation problem. 
*/
#include "KTRSolver.h"
#include "KTRProblem.h"
#ifndef AG_GRAPH_HEADER
#define AG_GRAPH_HEADER 1 
#include "graph.h"
#endif 
#include <iomanip>


class AGProblem : public knitro::KTRProblem {
public:
	AGProblem(int x, int y): KTRProblem(x,y) {
		setObjectiveProperties();
		setVariableProperties();
		setConstraintProperties();
		setDerivativeProperties();
	}

	void initialize(Graph AG) {
		this->G = AG; 
		setConstraintProperties_actual(); 
	}


	double evaluateFC(const double* const x, double* const c, 
					  double* const objGrad, double* const jac) {
		
		int n = G.size(); 
		std::vector<Node>::iterator nit = G.graph_nodes(); 
		std::vector<Node>::iterator onit = nit; 
		for (int i=0; i<n; ++i){
			if (nit->nodetype()==Fact){
				c[i] = x[i]; 
			}
			else if (nit->nodetype()==Rule){ 
				c[i] = rule_constraints(i, x); 
			}
			else if (nit->nodetype()==Goal){
				c[i] = goal_constraints(i, x); 
			}
			++nit;
		}

		return x[0];  
	}

	int evaluateGA(const double* const x, double* const objGrad, double* const jac) {
		int i,k; 
		int n = G.size();
		std::vector<Node>::iterator nit = G.graph_nodes(); 
		std::vector<Node>::iterator onit = nit; 

		objGrad[0] = 1;  
		for(i=1; i<n; ++i){
			objGrad[i] = 0; 
		}

		for (i=0,k=0; i<n; ++i){
			if (nit->nodetype()==Fact){
				jac[k] = 1; 
			}
			//Must add Jacobian entries for each predecessor 
			else if (nit->nodetype()==Rule){ 
				 jac[k] = x[k]; 
			}
			else if (nit->nodetype()==Goal){
				 jac[k] = x[k]; 
			}
			++nit;
			++k; 
		}
		return 0;
	}

	int evaluateHess(const double* const x, double objScaler, const double* const lambda,
					double* const hess) {
		return 0;
	}


private:
	Graph G; 

	void setObjectiveProperties() {
		setObjType(knitro::KTREnums::ObjectiveType::ObjGeneral);
		setObjGoal(knitro::KTREnums::ObjectiveGoal::Minimize);
	}

	void setVariableProperties() {
		setVarLoBnds(0.0);
		setVarUpBnds(1.0); 
	}

	double rule_constraints(int node_id, const double* const x){
		std::vector<int> P = G.gnodepreds(node_id);
		double constraint = 1.0; 
		
		for(int i = 0; i < P.size(); ++i){
			constraint *= x[P[i]];
		}
		return constraint - x[node_id]; 
	}

	double goal_constraints(int node_id, const double* const x){
		std::vector<int> P = G.gnodepreds(node_id);
		double constraint = 0.0; 
		double weight = 1.0/P.size();

		for(int i = 0; i < P.size(); ++i){
			constraint += (weight*x[P[i]]);
		}
		return constraint - x[node_id]; 
	}

	void rule_deriv_1(int node_id, const double* const x, double* const jac, int &k){
		std::vector<int> P = G.gnodepreds(node_id);
		int n = P.size(); 
		double constraint = 1.0; 
		
		for(int i = 0; i < n; ++i){
			for(int j = 0; j < n; ++j){
				if(j== i){
					constraint *= 1;
				}
				else {
					constraint *= x[P[j]];
				}
			}
			jac[k]= constraint; 
			++k; 
			setJacIndexCons(k, i); 
			setJacIndexVars(k, i); 
		}

		//Now set for the variable itself. 
		for(int j = 0; j < n; ++j){
			constraint *= x[P[j]];
		}
		jac[k] = constraint - 1; 
		++k; 
		setJacIndexCons(k, node_id); 
		setJacIndexVars(k, node_id); 
	}

	void goal_deriv_1(int node_id, const double* const x, double* const jac, int &k){
		std::vector<int> P = G.gnodepreds(node_id);
		double constraint = 0.0; 
		int n = P.size(); 
		double weight = 1.0/P.size();
		
		for(int i = 0; i < n; ++i){
			for(int j = 0; j < n; ++j){
				if(j== i){
					constraint += 1;
				}
				else {
					constraint += (weight*x[P[j]]);
				}
			}
			jac[k]= constraint; 
			++k; 
			setJacIndexCons(k, i); 
			setJacIndexVars(k, i); 
		}

		//Now set for the variable itself. 
		for(int j = 0; j < n; ++j){
			constraint += (weight*x[P[j]]);
		}
		jac[k] = constraint - 1; 
		++k; 
		setJacIndexCons(k, node_id); 
		setJacIndexVars(k, node_id); 
	}

	double fact_constraints(int node_id, const double* const x){
		return x[node_id]; 
	}
	

	void setConstraintProperties() {
		int no_vars = getVarUpBnds().size(); 
		for(int i = 0; i <no_vars; ++i){
			setConTypes(i, knitro::KTREnums::ConstraintType::ConLinear); 
			setConLoBnds(i, 0.8);
			setConUpBnds(i, 0.8);
		}
	}

	void setConstraintProperties_actual(){
		double default_init = 0.8;   
		int n = G.size(); 
		std::vector<Node>::iterator nit = G.graph_nodes(); 
		std::vector<Node>::iterator onit = nit; 
		for (int i=0; i<n; ++i){
			if (nit->nodetype()==Fact){
				double P = this->G.getnodeprob(i); 
				setConTypes(i, knitro::KTREnums::ConstraintType::ConLinear); 
				setConLoBnds(i, P);
				setConUpBnds(i, P);
			}
			else if (nit->nodetype()==Rule){ 
				setConTypes(i, knitro::KTREnums::ConstraintType::ConGeneral); 
				setConLoBnds(i, 0.0);
				setConUpBnds(i, 0.0);
			}
			else { 
				setConTypes(i, knitro::KTREnums::ConstraintType::ConLinear); 
				setConLoBnds(i, 0.0);
				setConUpBnds(i, 0.0);
			}
			++nit;
		}
	}


	void setDerivativeProperties() {
		int n = G.size(); 
		for(int i = 0; i < n; ++i){
			setJacIndexCons(i, i); 
			setJacIndexVars(i, i); 
		}
	}
};

inline void printSolutionResults(knitro::KTRISolver & solver, int solveStatus) {
	if (solveStatus != 0) {
		std::cout << "Failed to solve problem, final status = " << solveStatus << std::endl;
		return;
	}
	std::cout << "---------- Solution found ----------" << std::endl << std::endl;

	std::cout.precision(2);
	std::cout << std::scientific;

	std::cout<<std::fixed; 
	// Objective value
	std::cout << std::right << std::setw(28) << "Objective value = " << solver.getObjValue() << std::endl;

	/* Solution point 
	std::cout << std::right << std::setw(29) << "Solution point = (";
	const std::vector<double>& point = solver.getXValues();
	std::vector<double>::const_iterator it = point.begin();
	while ( it != point.end()) {
		std::cout << *it;
		if (++it != point.end())
		std::cout << ", ";
	}
	std::cout << ")" << std::endl;

	if (!((solver.getProblem())->isMipProblem()))
	{
		std::cout << std::right << std::setw(28) << "Feasibility violation = " << solver.getAbsFeasError() << std::endl;
		std::cout << std::right << std::setw(28) << "KKT optimality violation = " << solver.getAbsOptError() << std::endl;
	}
	else {
		std::cout << std::right << std::setw(28) << "Absolute integrality gap = " << solver.getMipAbsGap() << std::endl;
	}
	std::cout << std::endl;*/
}