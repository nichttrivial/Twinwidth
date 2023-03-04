#ifndef PARTITIONREFINEMENT_HPP
#define PARTITIONREFINEMENT_HPP
#include <vector>
#include <iostream>
#include <queue>
#include "graph.hpp"

// From "Partition Refinement Techniques: An Interesting Algorithmic Toolkit", Habib et al. '99

std::vector< size_t > factorizingPermutation(const Graph& G);

struct PartitionClass;

struct ItemList{
  ItemList* nxt = nullptr;
  ItemList* prv = nullptr;

  size_t item;
  PartitionClass* parentPartition;
  void moveBefore(ItemList* pos);
};

struct PartitionClass{
  ItemList* minChild = nullptr;
  ItemList* maxChild = nullptr;
  unsigned int size = 0;
  unsigned int moveCount = 0;

  ItemList* firstPivot = nullptr;
};


struct FactorizingPermutationEnv{
  std::vector<PartitionClass*> pivotSets = {};
  ItemList* center = nullptr;
  ItemList* listStart = nullptr;
  std::vector<ItemList*> listEntries = {};
  std::queue<PartitionClass*> Modules = {};

  PartitionClass* activePivotClass = nullptr;
  ItemList* activePivotElement = nullptr;

  void represent() const;
  inline void resetListStart(){while (listStart->prv) listStart = listStart->prv;}
};

void printPartitionSystem(ItemList* items);
std::vector<ItemList*> getPivot(FactorizingPermutationEnv& env, const Graph& G, const PartitionClass* pivotSet, ItemList* item);
void addPivot(FactorizingPermutationEnv& env, PartitionClass* oldPart, PartitionClass* newPart);
void refine(FactorizingPermutationEnv& env, std::vector<ItemList*>& pivotSet);



#endif /* end of include guard: PARTITIONREFINEMENT_HPP */
