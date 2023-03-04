#include "partitionRefinement.hpp"
#include "graph.hpp"

void FactorizingPermutationEnv::represent() const {
  auto tmp = listStart;
  while(tmp){
    std::cout << tmp->item << " ->";
    tmp = tmp->nxt;
  }
  std::cout << "\n";
  int classCounter = 0;
  tmp = listStart;
  PartitionClass* cur = nullptr;
  while(tmp){
    if(cur != tmp->parentPartition) {cur = tmp->parentPartition; classCounter++;}
    std::cout << classCounter << " ->";
    tmp = tmp->nxt;
  }
  std::cout << "\n";
  classCounter = 0;
  tmp = listStart;
  cur = nullptr;
  while(tmp){
    if(cur != tmp->parentPartition) {
      cur = tmp->parentPartition;
      classCounter++;
      std::cout << "Class " << classCounter << " : from " << cur->minChild->item << " to " << cur->maxChild->item << " size " << cur->size << "\n";
    }
    tmp = tmp->nxt;
  }

  std::cout << "\n -------------------------------------------- \n\n";

}

void ItemList::moveBefore(ItemList* pos){
  if(pos->parentPartition != parentPartition) std::cerr << "Item move breaks partition" << std::endl;
  if(pos == this) return;
  if(prv) prv->nxt = nxt;
  if(nxt) nxt->prv = prv;
  if(parentPartition->minChild == this) parentPartition->minChild = nxt;
  if(parentPartition->maxChild == this) parentPartition->maxChild = prv;

  nxt = pos;
  prv = pos->prv;
  if(prv) prv->nxt = this;
  pos->prv = this;
  if(pos == parentPartition->minChild) parentPartition->minChild = this;
}

void printPartitionSystem(ItemList* items){
  ItemList* tmp = items;
  PartitionClass* last = nullptr;
  while(tmp != nullptr){
    if(tmp->parentPartition != last) {
      std::cout << "\n// ";
      last = tmp->parentPartition;
    }
    std::cout << tmp->item << " ";
    tmp = tmp->nxt;
  }
  std::cout << "\n";
}

std::vector<ItemList*> getPivot(FactorizingPermutationEnv& env, const Graph& G, const PartitionClass* pivotSet, ItemList* item){
  if(item == nullptr) LOG("WARNING, nullptr item found for pivot");
  std::vector<ItemList*> rval;
  for (auto i : G.getNeighbours(item->item)) {
    if(env.listEntries[i]->parentPartition == nullptr) LOG("WARNING, orphaned item found")
    if(env.listEntries[i]->parentPartition != pivotSet) rval.push_back(env.listEntries[i]);
  }
  return rval;
}

void addPivot(FactorizingPermutationEnv& env, PartitionClass* oldPart, PartitionClass* newPart){
  for(const auto& x : env.pivotSets){
    if(x == oldPart){
       env.pivotSets.push_back(newPart);
      return;
    }
  }
  PartitionClass* bigger;
  if(oldPart->size > newPart->size){
     env.pivotSets.push_back(newPart);
     bigger = oldPart;
  }else{
     env.pivotSets.push_back(oldPart);
     bigger = newPart;
  }
  auto ModCopy = env.Modules;
  while(ModCopy.size()){
    auto x = ModCopy.front();
    ModCopy.pop();
    if(x == oldPart){
      x = bigger;
      return;
    }
  }
  env.Modules.push(bigger);
}

bool insertRight(FactorizingPermutationEnv& env, PartitionClass* oldPart, PartitionClass* newPart){
  auto iter = env.center;
  int i = 1;
  int indexCent = 0;
  int indexPiv = 0;
  int indexOldP = 0;

  while(iter){
    i++;
    if(iter == env.center) {indexCent = i;}
    if(iter->parentPartition == env.activePivotClass) {indexPiv = i; break;}
    if(iter->parentPartition == oldPart) {indexOldP = i;}
    iter = iter->nxt;
  }
  if(indexCent < indexOldP && indexOldP < indexPiv) return false;
  return true;
}

void refine(FactorizingPermutationEnv& env, std::vector<ItemList*>& pivotSet){
  for(auto& i : pivotSet){
    //Move the elements from the pivot sets to the start of their respective partition classes
    if(i->parentPartition->minChild == i->parentPartition->maxChild) continue; // Single Member classes are ignored, because they can't be split

    i->parentPartition->moveCount++; // Count the number of moves in the partition

    if(i == i->parentPartition->minChild) continue; //Item is already at the right position, don't do any updates
    if(i == i->parentPartition->maxChild) i->parentPartition->maxChild = i->prv; // Update the max child if necessary

    i->moveBefore(i->parentPartition->minChild);
    i->parentPartition->minChild = i; //update the partitions start point
  }

  /* All partitions have been split according to the pivot set */
  env.resetListStart();

  for(auto i : pivotSet){
    if(i->parentPartition->moveCount == 0) continue; //already processed
    if(i->parentPartition->moveCount == i->parentPartition->size){ // no true split
      i->parentPartition->moveCount = 0;
      continue;
    }
    i = i->parentPartition->minChild; //Set to start of the correct partition
    auto p = new PartitionClass;
    PartitionClass* oldParent = i->parentPartition;

    p->minChild = i;
    p->size = oldParent->moveCount;
    p->moveCount = 0;
    oldParent->moveCount = 0;
    oldParent->size -= p->size;


    for(int j = 0; j < p->size; j++){
      i->parentPartition = p;
      i = i->nxt;
    }
    oldParent->minChild = i;
    p->maxChild = i->prv;

    if(insertRight(env, p, oldParent)){
      if(p->minChild->prv != nullptr) p->minChild->prv->nxt = oldParent->minChild;
      if(oldParent->maxChild->nxt != nullptr) oldParent->maxChild->nxt->prv = p->maxChild;

      p->maxChild->nxt = oldParent->maxChild->nxt;
      oldParent->minChild->prv = p->minChild->prv;

      oldParent->maxChild->nxt = p->minChild;
      p->minChild->prv = oldParent->maxChild;
    }
    addPivot(env, oldParent, p);
  }
}

void splitAt(FactorizingPermutationEnv& env, const Graph& G, ItemList* x){

  auto parent = x->parentPartition;
  auto tmp = parent->minChild;

  x->moveBefore(tmp);


  bool done = false;
  int nonNeighbours = 0;

  while (!done) {
    if(tmp == tmp->parentPartition->maxChild) done = true;
    auto next = tmp->nxt;
    if(tmp != x && !G.isBlackEdge(x->item, tmp->item)){ tmp->moveBefore(x); nonNeighbours++;}
    tmp = next;
  }
  env.resetListStart();

  PartitionClass* nbs = new PartitionClass;
  PartitionClass* nonnbs = new PartitionClass;

  nonnbs->minChild = parent->minChild;
  nonnbs->maxChild = x->prv;
  nonnbs->size = nonNeighbours;
  tmp = nonnbs->minChild;
  for (size_t i = 0; i < nonNeighbours; i++) {
    tmp->parentPartition = nonnbs;
    tmp = tmp->nxt;
  }

  nbs->minChild = x->nxt;
  nbs->maxChild = parent->maxChild;
  nbs->size = parent->size - nonNeighbours - 1;
  tmp = nbs->minChild;
  for (size_t i = 0; i < nbs->size; i++) {
    tmp->parentPartition = nbs;
    tmp = tmp->nxt;
  }

  parent->minChild = x;
  parent->maxChild = x;
  parent->size = 1;
  parent->firstPivot = nullptr;

  env.center = x;

  if(nonnbs->size < nbs->size){
    env.Modules.push(nbs);
    if(nonnbs->size > 0){
      env.pivotSets.push_back(nonnbs);
    }else{
      delete nonnbs;
    }
  }else{
    env.Modules.push(nonnbs);
    if(nbs->size > 0){
      env.pivotSets.push_back(nbs);
    }else{
      delete nbs;
    }
  }

}

bool initPartition(FactorizingPermutationEnv& env, const Graph& G){
  auto tmp = env.listStart;
  while(tmp){
    if(tmp->parentPartition->size != 1) goto FOUND_NONSINGLETON;
    tmp = tmp->nxt;
  }
  return false; //All partitions are singletons, break out of loop to return factorizing permutation
  FOUND_NONSINGLETON:
  if(env.Modules.size() == 0){
    auto cls = tmp->parentPartition;
    if(cls->firstPivot) tmp = cls->firstPivot; //if a first pivot is set, we use it; otherwise we use tmp as an arbitrary member of the class
    splitAt(env, G, tmp);
  }else{
    auto mod = env.Modules.front();
    env.Modules.pop();
    tmp = mod->minChild;
    tmp->parentPartition->firstPivot = tmp;
  }
  return true;
}

std::vector< size_t > factorizingPermutation(const Graph& G){
  ItemList* front = nullptr;
  PartitionClass* p = new PartitionClass;
  FactorizingPermutationEnv env;
  size_t n = G.getNumVertices();

  for(int i = 0; i < n; i++){
    auto tmp = new ItemList;
    env.listEntries.push_back(tmp);
    tmp->prv = front;
    if(front != nullptr){
      front->nxt = tmp;
    }else{
      p->minChild = tmp;
    }
    front = tmp;
    front->item = i;
    front->parentPartition = p;
    p->size++;
  }
  p->maxChild = front;
  /*init done*/

  env.listStart = p->minChild;
  //env.represent();

  while(initPartition(env,G)){
    while(env.pivotSets.size()){
      //env.represent();
      auto pivotClass = env.pivotSets.back();
      env.pivotSets.pop_back();
      auto pivotElement = pivotClass->minChild;
      if (pivotElement == nullptr) LOG("ERROR")

      env.activePivotClass = pivotClass;

      while(true){
        env.activePivotElement = pivotElement;
        if (pivotElement == nullptr) LOG("ERROR")

        auto currentPivotSet = getPivot(env, G, pivotClass, pivotElement);
        refine(env, currentPivotSet);
        while(env.listStart->prv != nullptr) env.listStart = env.listStart->prv;
        if(pivotElement == pivotClass->maxChild) break;
        pivotElement = pivotElement->nxt;
      }
    }
  }

  std::vector<size_t> rval = {};
  auto initial = env.listStart;
  while(initial != nullptr){
    rval.push_back(initial->item);
    initial = initial->nxt;
  }
  for(auto& i : env.listEntries){
    if(--i->parentPartition->size == 0) delete i->parentPartition;
    delete i;
  }
  return rval;
}
