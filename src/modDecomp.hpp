#ifndef MODULARDECOMP_HPP
#define MODULARDECOMP_HPP

#include "partitionRefinement.hpp"
#include "graph.hpp"
#include <list>

constexpr int UNSET = -1;

struct MDT{ //ModularDecompositionTree
  MDT* parent = nullptr;
  std::list<MDT*> children = {};
  int storedVx = -1;

  int first = UNSET; //First vertex in the factorizing permutation in the module
  int last = UNSET; //Last vertex in the factorizing permutation in the module
  int firstCut = UNSET; //Last vertex in the factorizing permutation in the module
  int lastCut = UNSET; //Last vertex in the factorizing permutation in the module


  ~MDT(){
    for(auto x : children) {delete x;}
  }
};


MDT* getModularDecomposition(const Graph& G);

size_t decompositionWidth(MDT* tree); //Returns the size of the largest submodule
#endif
