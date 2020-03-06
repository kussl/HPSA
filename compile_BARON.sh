clang++ -Xpreprocessor -fopenmp  -lm \
 graph.cc baronout.cc baroninterface.cc barontests.cc main.cc -lomp -std=c++11
