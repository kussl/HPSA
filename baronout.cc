//This is a BARON interface 
#include <iomanip>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "graph.h"

using namespace std;

std::string rule_constraints(Graph &G, Node n, int e){
	std::string constraints; 

	constraints="e"+std::to_string(e)+": -x"+std::to_string(n.nodeid())+"+";

	std::vector<int> P = G.preds(n.nodeid());
	double weight = 1.0/P.size();
	for(int j = 0; j < P.size(); ++j){
		constraints+="x"+std::to_string(P[j]);
		if (j < (P.size()-1)) {
			constraints+= "*";
		} else { 
			constraints += "== 0;";
		}
	}
	constraints+="\n";
	return constraints; 
}

/*
Adds rule node constraints with the addition of an improvement node. 
For now, last argument plays no role. 
*/

std::string rule_constraints(Graph &G, Node n, int e, std::vector<Instrument> v, std::vector<std::string> &tnodes, bool &found){
	std::string constraints, binary_constraints; 

	constraints="e"+std::to_string(e)+": -x"+std::to_string(n.nodeid())+"+";

	std::vector<int> P = G.preds(n.nodeid());

	for(int j = 0; j < P.size(); ++j){
		constraints+="x"+std::to_string(P[j]);
		constraints+= "*";
	}

	constraints.replace(constraints.length()-1,1,"*");
	int inst_size = v.size();

	for(int k = 0; k < inst_size; ++k){
		std::string tid = std::to_string(n.nodeid())+"_"+std::to_string(k); 
		if(std::find(v[k].targets.begin(), v[k].targets.end(), n.nodeid()) != v[k].targets.end()){
			constraints+= "((1-t"+tid+")+t"+tid+"*(1-"+std::to_string(v[k].nodeP())+"))*"; 
			binary_constraints+= "e_t"+tid+": t"+tid+" - t"+tid+"^2 == 0;\n";
			found = true; 
			tnodes.push_back(tid); 
		}
	}
	constraints.replace(constraints.length()-1,1," == 0;\n");
	constraints+=binary_constraints;

	return constraints; 
}

//Goal constraints do not use the selector variable here.
//The selection is averaged. 
std::string goal_constraints_avg(Graph &G, Node n, int e){
	std::string constraints; 
	constraints="e"+std::to_string(e)+": -x"+std::to_string(n.nodeid())+"+";
	int nid = n.nodeid(); 
	std::string nids = std::to_string(nid); 
	std::vector<int> P = G.preds(nid);
	double weight = 1.0/P.size();

	for(int j = 0; j < P.size(); ++j){
		constraints+=std::to_string(weight)+"*x"+std::to_string(P[j])+"+";
	}
	constraints.replace(constraints.length()-1,1,"== 0;\n");

	constraints+="\n";
	return constraints; 
}

std::string goal_constraints(Graph &G, Node n, int e){
	std::string constraints; 
	constraints="e"+std::to_string(e)+": -x"+std::to_string(n.nodeid())+"+";
	int nid = n.nodeid(); 
	std::string nids = std::to_string(nid); 
	std::vector<int> P = G.preds(nid);
	double weight = 1.0/P.size();

	for(int j = 0; j < P.size(); ++j){
		//constraints+=std::to_string(weight)+"*x"+std::to_string(P[j])+"+";
		/*add aux variables for the given goal variable.*/ 
		constraints+="y"+std::to_string(j)+"x"+nids+"*x"+std::to_string(P[j])+"+";
		
	}
	constraints.replace(constraints.length()-1,1,"= 0;\n");

	/*add constraint on aux variables. */
	constraints+="e"+std::to_string(e)+ "g: "; 

	for(int j = 0; j < P.size(); j++) {
		constraints+= "y"+std::to_string(j)+"x"+nids; 
		if (j < (P.size()-1)) {
			constraints+= "+";
		} else { 
			constraints += "== 1;";
		}
	}

	constraints+="\n";
	return constraints; 
}

std::string add_improvement_constraints(std::vector<std::string> rulenodes, int e, int m){
	/*Add rule node constraint*/
	std::string constraints="e"+std::to_string(e)+": ";
	int no_rule_nodes = rulenodes.size(); 
	//std::cout<<"No rule nodes: "<<no_rule_nodes<<std::endl; 
	for (int i = 0; i <no_rule_nodes; ++i)
		constraints+= "t"+rulenodes[i]+"+";
	
	std::string total; 
	if(m <= 0)
		total = std::to_string(rulenodes.size()-1); 
	else 
		total = std::to_string(m); 


	constraints.replace(constraints.length()-1,1," == "+total+";\n");
	return constraints; 
}

std::string add_baron_constraints(Graph &G){
	std::string constraints = "EQUATIONS "; 
	int n = G.size(); 
	std::vector<Node>::iterator nit = G.graph_nodes(); 
	std::vector<Node>::iterator onit = nit; 
	std::vector<int> rulenodes;
	int e =0; 
	
	/*
	Declare the constraints. 
	*/
	for(int i =0; i<n; ++i){
		constraints+= "e"+std::to_string(i);
		if (onit->nodetype()==Goal){
			//constraints+= ",e"+std::to_string(i)+"g";
		}
		constraints+= ",";
		++onit; 
	}
	constraints.replace(constraints.length()-1,1,";\n\n");
	e = 0; 

	/*
	For each node (depending on type), add the corresponding constraint. 
	*/
	for (int i=0; i<n; ++i){
		if (nit->nodetype()==Rule) {
			constraints += rule_constraints(G, *nit, e); 
			rulenodes.push_back(nit->nodeid()); 
		}
		else if (nit->nodetype()==Goal) {
			constraints += goal_constraints_avg(G, *nit, e); 
		}
		else if (nit->nodetype()==Fact){
			constraints+="e"+std::to_string(e)+": x"+std::to_string(nit->nodeid())+"==";
			constraints+=std::to_string(nit->nodeP())+";\n";
		}
		++nit;
		++e; 
	}

	
	return constraints; 
}


std::string add_baron_constraints(Graph &G, std::vector<Instrument> v, int m){
	std::string eq_declarations = "EQUATIONS "; 
	std::string constraints = " "; 
	int n = G.size(); 
	std::vector<Node>::iterator nit = G.graph_nodes(); 
	std::vector<Node>::iterator onit = nit; 
	std::vector<int> rulenodes;
	std::vector<std::string> tnodes;
	int e =0; 
	
	/*
	Declare the constraints. 
	Add an extra constraint to limit the number of selections.
	*/
	for(int i =0; i<n+1; ++i){
		eq_declarations+= "e"+std::to_string(i);
		if (onit->nodetype()==Goal){
			//constraints+= ",e"+std::to_string(i)+"g";
		}
		eq_declarations+= ",";
		++onit; 
	}

	
	e = 0; 

	/*
	For each node (depending on type), add the corresponding constraint. 
	*/

	for (int i=0; i<n; ++i){
		int nodeid = nit->nodeid(); 
		if (nit->nodetype()==Rule) {
			//Only add an improvement constraint if the node is in the candidates vector. 
			int inst_size = v.size();
			bool found = false; 
			
			std::string c = rule_constraints(G, *nit, e, v, tnodes, found);  
			if(found){
				constraints += c; 
			}
			else  { 
				constraints += rule_constraints(G, *nit, e); 
			}
			rulenodes.push_back(nit->nodeid()); 
		}
		else if (nit->nodetype()==Goal) {
			constraints += goal_constraints_avg(G, *nit, e); 
		}
		else if (nit->nodetype()==Fact){
			constraints+="e"+std::to_string(e)+": x"+std::to_string(nit->nodeid())+"==";
			constraints+=std::to_string(nit->nodeP())+";\n";
		}
		++nit;
		++e; 
	}

	/*
	Add the extra constraints for t variables
	*/
	for(int i = 0; i < tnodes.size(); ++i)
		eq_declarations+="e_t"+tnodes[i]+",";

	eq_declarations.replace(eq_declarations.length()-1,1,";\n\n");

	constraints+= add_improvement_constraints(tnodes, e, m); 
	
	return eq_declarations+ constraints; 
}

std::string add_aux_vars(Graph &G, std::vector<std::string> &vars, int nodeid){
	std::string aux_declarations;
	std::vector<int> P = G.preds(nodeid);
	for(int j = 0; j < P.size(); ++j){
		aux_declarations+= "y"+std::to_string(j)+"x"+std::to_string(nodeid)+",";
		vars.push_back( "y"+std::to_string(j)+"x"+std::to_string(nodeid)); 

	}
	return aux_declarations;
}

std::string declare_baron_vars(int n, std::vector<Node>::iterator nit, Graph &G){
	std::string declaration = "VARIABLES "; 
	std::vector<Node>::iterator onit = nit; 
	std::vector<std::string> vars;
	std::string aux_declarations = "VARIABLES "; 

	for (int i=0; i<n; ++i){
		declaration+= "x"+std::to_string(onit->nodeid());
		vars.push_back("x"+std::to_string(onit->nodeid())); 

		if(onit->nodetype()==Goal){
			//add aux variables  
			//aux_declarations+= add_aux_vars(G, vars, onit->nodeid()); 
		}

		if (i < (n-1)) {
			declaration+= ",";
		} else { 
			declaration += ";";
		}
		++onit;
	}

	onit = nit; 

	//Set LB and UP: 
	declaration += "\n";
	aux_declarations.erase(aux_declarations.size() - 1);
	//declaration += aux_declarations+";\n"; 

	declaration += "LOWER_BOUNDS{\n"; 

	for (int i=0; i<n; ++i){
		declaration+= vars[i]+": 0; \n";
	}

	declaration += "}";

	onit = nit; 

	declaration += "\n";
	declaration += "UPPER_BOUNDS{\n"; 

	for (int i=0; i<n; ++i){
		declaration+= vars[i]+": 1; \n";
	}

	declaration += "}";
	return declaration; 
}

/*
Add binary variables for rule nodes as candidate 
placements of security improvement nodes. 
For each rule node, add a new improvement node.
All added nodes represent a SINGLE improvement option.

*/

/*
Declare variables with single/multiple improvement instruments. 
*/
std::string declare_baron_vars(int n, std::vector<Node>::iterator nit, Graph &G, 
	std::vector<Instrument> v){
	std::string declaration = "VARIABLES "; 
	std::vector<Node>::iterator onit = nit; 
	std::vector<std::string> vars,tvars;
	std::string aux_declarations = "VARIABLES "; 
	std::string imp_declarations = "VARIABLES ";
	int imp_i = 0;  

	for (int i=0; i<n; ++i){
		declaration+= "x"+std::to_string(onit->nodeid());
		vars.push_back("x"+std::to_string(onit->nodeid())); 


		if(onit->nodetype()==Goal){
			//add aux variables  
			//aux_declarations+= add_aux_vars(G, vars, onit->nodeid()); 
		}

		else if(onit->nodetype()==Rule){
			//add an improvement node, if candidate node is in candidate_nodes. 
			//Search all instruments for candidates
			int inst_size = v.size(); 

			for(int k = 0; k < inst_size; ++k){
				if(std::find(v[k].targets.begin(), v[k].targets.end(), onit->nodeid()) != v[k].targets.end()){
					std::string tnodeid = "t"+std::to_string(onit->nodeid())+"_"+std::to_string(k);  
					imp_declarations+= tnodeid+",";
					imp_i++;
					tvars.push_back(tnodeid); 
				}
			}
		}

		if (i < (n-1)) {
			declaration+= ",";
		} else { 
			declaration += ";";
		}
		++onit;
	}

	//Remove the last colon. 
	imp_declarations.replace(imp_declarations.length()-1,1,"");

	onit = nit; 

	//Set LB and UP: 
	declaration += "\n";
	aux_declarations.erase(aux_declarations.size() - 1);

	//declaration += aux_declarations+";\n"; 
	declaration += imp_declarations+";\n";

	declaration += "LOWER_BOUNDS{\n"; 

	for (int i=0; i<n; ++i){
		declaration+= vars[i]+": 0; \n";
	}

	for(int i=0; i<tvars.size(); ++i)
		declaration+= tvars[i]+": 0; \n";

	declaration += "}";

	onit = nit; 

	declaration += "\n";
	declaration += "UPPER_BOUNDS{\n"; 

	for (int i=0; i<n; ++i){
		declaration+= vars[i]+": 1; \n";
	}

	for(int i=0; i<tvars.size(); ++i)
		declaration+= tvars[i]+": 1;\n";
	
	declaration += "}";
	return declaration; 
}

std::string options_macos(int index){
	std::string spec; 
	spec = "OPTIONS {\n"; 
	spec += "results: 1;\n";
	spec += "resname: \"baron/"+std::to_string(index)+".res\";\n";
	spec += "TimName: \"baron/"+std::to_string(index)+".tim\";\n";
	spec += "OptName: \"baron/"+std::to_string(index)+".opt\";\n";
	spec += "times: 1;\n";
	spec += "PrLevel: 0; \n";
	spec += "LocRes: 0; \n"; 
	spec += "DoLocal: 1;\n";
	//spec += "FirstLoc: 1;\n";
	spec += "LPSol: 8;\n";
	//spec += "MaxTime: 4;\n";
	//spec += "MaxIter: 50;\n"; 
	spec += "CplexLibName: \"/Applications/CPLEX_Studio1210/cplex/bin/x86-64_osx/libcplex12100.dylib\";";
	spec += "}\n";
	return spec; 
}

std::string create_baron_file(Graph &G, int index){
	int n = G.size(); 
	std::string leafnode = std::to_string(G.graph_nodes()[0].nodeid()); 
	std::vector<Node>::iterator nit = G.graph_nodes(); 
	std::string spec = options_macos(index); 

	spec += declare_baron_vars(n, nit, G);
	spec+="\n";
	spec+= add_baron_constraints(G); 
	spec+="\n";
	spec+="OBJ: minimize x"+leafnode+";";

	return spec; 
}

/*
This function is different from above in receiving a set of candidate nodes to place
the improvement on (candidate_nodes) and the improvement probability value. 
The caller should have a list of possible placements and send this one a subset of that list. 
This will create a BARON file with improvement nodes attached to the candidate_nodes. 
*/

std::string create_baron_file(Graph &G, int index, std::vector<int> candidate_nodes, double improvement, int m){
	int n = G.size();
	std::string leafnode = std::to_string(G.graph_nodes()[0].nodeid());  
	std::vector<Node>::iterator nit = G.graph_nodes(); 
	std::string spec = options_macos(index); 
	std::vector<Instrument> v; 
	Instrument t = Instrument(improvement);
	t.set_targets(candidate_nodes);
	v.push_back(t); 

	//Pass the candidate nodes to create corresponding improvement nodes. 
	spec += declare_baron_vars(n, nit, G, v);
	spec+="\n";
	spec+= add_baron_constraints(G, v, m); 
	spec+="\n";
	spec+="OBJ: minimize x"+leafnode+";";

	return spec; 
}

/*
This function is used to supply multiple improvement instruments to BARON.
*/

std::string create_baron_file(Graph &G, int index, std::vector<Instrument> v, int m){
	int n = G.size(); 
	std::string leafnode = std::to_string(G.graph_nodes()[0].nodeid()); 
	std::vector<Node>::iterator nit = G.graph_nodes(); 
	std::string spec = options_macos(index); 


	//Pass the candidate nodes to create corresponding improvement nodes. 
	spec += declare_baron_vars(n, nit, G, v);
	spec+="\n";
	spec+= add_baron_constraints(G, v, m); 
	spec+="\n";
	spec+="OBJ: minimize x"+leafnode+";";

	return spec; 
}



