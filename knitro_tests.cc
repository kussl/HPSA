#include "AGknitro.h"

extern inline void printSolutionResults(knitro::KTRISolver & solver, int solveStatus); 
/*
Serial interface for KNITRO
*/
void test_gen_KNITRO(Graph G){
	size_t no_vars, no_const; 
	no_vars = no_const = G.size(); 
	G.fillprobabilities(); 

	AGProblem* problem = new AGProblem(no_vars, no_const);
	problem->initialize(G); 


	knitro::KTRSolver solver(problem, KTR_GRADOPT_FORWARD, KTR_HESSOPT_BFGS);

	solver.setParam(KN_PARAM_DEBUG, 1);
	//solver.setParam(KN_PARAM_NEWPOINT, 0);
	solver.setParam(KN_PARAM_OUTLEV, 6);
	//solver.setParam(KN_PARAM_OUTMODE, 1);
	solver.setParam(KN_PARAM_MA_TERMINATE, 1); //local optimum 
	solver.setParam(KN_PARAM_FINDIFF_TERMINATE, 1); 
	solver.setParam(KN_PARAM_OPTTOL, 0.5);
	// solver.setParam(KN_PARAM_MIP_STRONG_MAXIT, 4); 
	// solver.setParam(KN_PARAM_CG_MAXIT, 4); 
	//solver.setParam(KN_PARAM_MAXIT, 4); 
	//solver.setParam(KN_PARAM_MIP_HEURISTIC_MAXIT, 2); 

	int solveStatus = solver.solve();
	printSolutionResults(solver, solveStatus);
}