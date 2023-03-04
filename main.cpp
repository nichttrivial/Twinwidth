#include <iostream>
#include <chrono>
#include <string>
#include "src/log.hpp"
#include "src/graph.hpp"
#include "src/partitionRefinement.hpp"
#include "src/modDecomp.hpp"


class Timer {
private:
  std::chrono::time_point<std::chrono::system_clock> startTime;
  std::string name;
public:
  Timer (std::string name) : name{name} {startTime = std::chrono::system_clock::now();}
  virtual ~Timer (){
    auto endTime = std::chrono::system_clock::now();
    LOG( name << " took " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << "ms.");
  };
};

int main(int argc, char const *argv[]) {
  std::string infile;

  srand(time(nullptr));
  Graph G;
  if(argc == 2){
    G = initGraphFromFile(argv[1]);
  }else{
    LOG("Please provide a file name.")
    G = randomGraph(500, 25000);
  }

  auto T = new Timer("Compute Decomposition tree");
  auto r = getModularDecomposition(G);
  delete T;
  LOG("Size of largest submodule: " << decompositionWidth(r))
  delete r;
  return 0;
}
