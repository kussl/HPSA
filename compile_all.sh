#g++ -I /usr/local/lib/ nlp.cc graph.cc main.cc -lnlopt -lm
#g++ nlp.cc graph.cc baronout.cc main.cc -fopenmp -lnlopt -lm
clang++ -L $HOME/knitro-12.1.1-MacOS-64/lib -Xpreprocessor -fopenmp -lnlopt \
-lknitro -lm -I $HOME/knitro-12.1.1-MacOS-64/examples/C++/include -I \
$HOME/knitro-12.1.1-MacOS-64/include AGknitro.cc \
nlp.cc graph.cc baronout.cc baroninterface.cc barontests.cc main.cc -lomp
