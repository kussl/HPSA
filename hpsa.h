#define BARON_PATH std::string(std::getenv("BARON_PATH")) 
#define HPSA_PATH std::string(std::getenv("HPSA_PATH")) 
#define SOL_PATH (HPSA_PATH + std::string("/solution/"))
#define P_SOL_PATH (HPSA_PATH + std::string("/p_solution/"))
#define BARON_RES_PATH (HPSA_PATH + std::string("/baron"))

#define AG_HPSA_HEADER 1 