#include "modDecomp.hpp"
#include <limits>
#include <cassert>

enum type {NODATA = -1, OPENPAREN = -2, CLOSEPAREN = -3 };

struct List{
  List* nxt = nullptr;
  List* prv = nullptr;
  int data = NODATA;

  void insertLeft(List* i);
  void insertRight(List* i);

  ~List();
};

void List::insertLeft(List* i){
  i->nxt = this;
  i->prv = prv;
  if(prv) prv->nxt = i;
  prv = i;
}
void List::insertRight(List* i){
  i->nxt = nxt;
  i->prv = this;
  if(nxt) nxt->prv = i;
  nxt = i;
}
List::~List(){
  while (prv) {
    auto tmp = prv;
    prv = prv->prv;
    tmp->prv = nullptr;
    tmp->nxt = nullptr;
    delete tmp;
  }
  while(nxt){
    auto tmp = nxt;
    nxt = nxt->nxt;
    tmp->prv = nullptr;
    tmp->nxt = nullptr;
    delete tmp;
  }
}

void display(List* l){
  std::cout << "c ";
  while(l){
    switch (l->data) {
      case OPENPAREN:
        std::cout << "(";
        break;
      case CLOSEPAREN:
        std::cout << ")";
        break;
      default:
        std::cout << l->data;
    }
    if(l->nxt){
      l = l->nxt;
    }else{
      std::cout << std::endl;
      break;
    }
  }
}

void vPrint(std::vector<size_t>& v){
  for(auto x : v) std::cout << x << " ";
  std::cout << "\n";
}

std::vector<size_t> listDiff(const std::vector<size_t>& v1, const std::vector<size_t>& v2){
  //Assumes sorted vectors as input!
  std::vector<size_t> rval;
  auto i1 = v1.begin();
  auto i2 = v2.begin();
  while(i1 != v1.end() && i2 != v2.end()){
    if(*i1 == *i2){
      i1++;
      i2++;
      continue;
    }
    if(*i1 < *i2) {
      rval.push_back(*i1);
      i1++;
    }else{
      rval.push_back(*i2);
      i2++;
    }
  }
  rval.insert(rval.end(), i1, v1.end());
  rval.insert(rval.end(), i2, v2.end());
  return rval;
}

constexpr size_t NOCUT = -1;

std::vector<size_t> getLastCut(const std::vector<size_t>& fp, const std::vector<size_t>& fpInv, const Graph& G){
  std::vector<size_t> rval = {};
  auto n = fp.size();
  for (size_t i = 0; i < n-1; i++) {
      auto v = fp[i];
      auto w = fp[i+1];
      auto nv = G.getNeighbours(v);
      auto nw = G.getNeighbours(w);
      auto diff = listDiff(nv,nw);
      if(diff.size() == 0){
        rval.push_back(NOCUT);
        continue;
      }
      auto maxIndex = i + 1;
      for(auto x : diff){
        maxIndex = std::max(maxIndex, fpInv[x]);
      }
      if(maxIndex <= i+1) maxIndex = NOCUT;
      if(maxIndex != NOCUT){
        rval.push_back(fp[maxIndex]);
      }else{
        rval.push_back(NOCUT);
      }
  }
  return rval;
}

std::vector<size_t> getFirstCut(const std::vector<size_t>& fp, const std::vector<size_t>& fpInv, const Graph& G){
  std::vector<size_t> rval = {};
  auto n = fp.size();
  for (size_t i = 0; i < n-1; i++) {
      auto v = fp[i];
      auto w = fp[i+1];
      auto nv = G.getNeighbours(v);
      auto nw = G.getNeighbours(w);
      auto diff = listDiff(nv,nw);
      if(diff.size() == 0){
        rval.push_back(NOCUT);
        continue;
      }
      auto minIndex = i;
      for(auto x : diff){
        minIndex = std::min(minIndex, fpInv[x]);
      }
      if(minIndex >= i) minIndex = NOCUT;
      if(minIndex != NOCUT){
        rval.push_back(fp[minIndex]);
      }else{
        rval.push_back(NOCUT);
      }
  }
  return rval;
}

List* Parenthesize(const std::vector<size_t>& fp, const std::vector<size_t>& fpInv, const std::vector<size_t>& firstCut, const std::vector<size_t>& lastCut ){
  auto n = fp.size();
  std::vector<List*> dataList;
  for (size_t i = 0; i < n; i++) {
    auto tmp = new List;
    tmp->data = fp[i];
    if(dataList.size() > 0) {tmp->prv = dataList.back(); (dataList.back())->nxt = tmp;}
    dataList.push_back(tmp);
  }

  for (size_t i = 0; i < n-1; i++) {
    if(firstCut[i] != NOCUT){
      List* tmp = new List;
      tmp->data = OPENPAREN;
      dataList[fpInv[firstCut[i]]]->insertLeft(tmp);
      tmp = new List;
      tmp->data = CLOSEPAREN;
      dataList[i]->insertRight(tmp);
    }
    if(lastCut[i] != NOCUT){
      List* tmp = new List;
      tmp->data = CLOSEPAREN;
      dataList[fpInv[lastCut[i]]]->insertRight(tmp);
      tmp = new List;
      tmp->data = OPENPAREN;
      dataList[i+1]->insertLeft(tmp);
    }
  }
  List* tmp = new List;
  tmp->data = OPENPAREN;
  dataList[0]->insertLeft(tmp);

  tmp = new List;
  tmp->data = CLOSEPAREN;
  dataList.back()->insertRight(tmp);

  tmp = dataList[0];
  while(tmp->prv) tmp = tmp->prv;
  return tmp;
}

std::vector<size_t> invert(const std::vector<size_t>& permutation){
  auto n = permutation.size();
  std::vector<size_t> rval(n);
  for (size_t i = 0; i < n; i++) {
    rval[permutation[i]] = i;
  }
  return rval;
}

MDT* parensToTree(List* l){
  MDT* root = new MDT;
  auto cur = root;

  while(l){
    switch (l->data) {
      case OPENPAREN:
      {
        auto tmp = new MDT;
        cur->children.push_back(tmp);
        tmp->parent = cur;
        cur = tmp;
        break;
      }
      case CLOSEPAREN:
      {
        cur = cur->parent;
        break;
      }
      case NODATA:
      {
        LOG("Invalid parenthesized FP List Node encountered")
        break;
      }
      default:
      {
        auto tmp = new MDT;
        cur->children.push_back(tmp);
        tmp->parent = cur;
        tmp->storedVx = l->data;
        tmp->first = l->data;
        tmp->last = l->data;
        tmp->lastCut = l->data;
        tmp->firstCut = l->data;

      }
    }
    l = l->nxt;
  }
  //Root is known dummy node, so remove it
  cur = root->children.front();
  root->children = {};
  delete root;
  cur->parent = nullptr;
  return cur;
}

void computeModuleBoundaries(MDT* root){
  if(root->first != UNSET && root->last != UNSET) return;


  for(auto r : root->children){
    computeModuleBoundaries(r);
  }

  int maxV = -1;
  int minV = std::numeric_limits<int>::infinity();

  root->first = root->children.front()->first;
  root->last = root->children.back()->last;
}

void dbg(MDT* root){
  LOG("Node from " << root->first << " to " << root->last)
  LOG("Cutters: " << root->firstCut << "  " << root->lastCut)
  for(auto x : root->children) dbg(x);
}

void computeModuleCuts(MDT* root,  const std::vector<size_t>& fpInv, const std::vector<size_t>& firstCut, const std::vector<size_t>& lastCut){

  if(root->firstCut != UNSET && root->lastCut != UNSET) return;
  for(auto r : root->children){
    computeModuleCuts(r, fpInv, firstCut, lastCut);
  }

  int maxCut = root->children.front()->lastCut;
  int minCut = root->children.front()->firstCut;

  for(auto r : root->children){
    if(fpInv[r->lastCut] > fpInv[maxCut]) maxCut = r->lastCut;
    if(fpInv[r->firstCut] < fpInv[minCut]) minCut = r->firstCut;
  }


  auto it = root->children.begin();
  auto lastItem = root->children.end();
  std::advance(lastItem, -1);

  while (it != lastItem) {
    int finalV = (*it)->last;
    auto indexOfFinalV = fpInv[finalV];
    if(indexOfFinalV < lastCut.size()){
      //INVALID CUTS EXIST, DO CHECK
      if(lastCut[indexOfFinalV] != NOCUT && fpInv[lastCut[indexOfFinalV]] > fpInv[maxCut]) maxCut = lastCut[indexOfFinalV];
      if(firstCut[indexOfFinalV] != NOCUT && fpInv[firstCut[indexOfFinalV]] < fpInv[minCut]) minCut = firstCut[indexOfFinalV];
    }else{
    }
    it++;

  }

  root->firstCut = minCut;
  root->lastCut = maxCut;
}

void removeNode(MDT* node){
  auto pt = node->parent;
  for(auto x : node->children){
    x->parent = pt;
  }
  auto it = pt->children.begin();
  while(it != pt->children.end()){
    if(*it == node){
      //splice

      pt->children.splice(it, node->children);
      pt->children.erase(it);
      assert(node->children.size() == 0);
      delete node;
      return;
    }
    it++;
  }
}

void findDummyNodes(MDT* root, const std::vector<size_t>& fpInv, std::vector<MDT*>& toRemove){
  if(root->children.size() == 0) return;
  for(auto r : root->children){
    findDummyNodes(r, fpInv, toRemove);
  }
  if(root->parent == nullptr) return;
  if(root->children.size() == 1){toRemove.push_back(root); return;}
  if(fpInv[root->lastCut] <= fpInv[root->last] && fpInv[root->firstCut] >= fpInv[root->first]) return; //Actual Module, shouldn't be supressed.
  toRemove.push_back(root);
}

void suppressDummyNodes(MDT* root, const std::vector<size_t>& fpInv){
  std::vector<MDT*> dummyNodes;
  findDummyNodes(root, fpInv, dummyNodes);
  for(auto x : dummyNodes) removeNode(x);
}

void reportModules(MDT* root, const std::vector<size_t>& fp, const std::vector<size_t>& fpInv){

  if(root->children.size() == 0) return;
  for(auto r : root->children){
    reportModules(r, fp, fpInv);
  }


  auto startIndex = fpInv[root->first];
  auto endIndex = fpInv[root->last];

  if((startIndex != 0 || endIndex != fpInv.size()-1) && startIndex != endIndex){ //Supress the trivial modules
    std::cout << "c Identified Module: ";
    for (size_t i = startIndex; i <= endIndex; i++) {
      std::cout << fp[i] << " ";
    }
    std::cout << "\n";
  }
}

void cleanupTree(MDT* root, const std::vector<size_t>& fpInv, const std::vector<size_t>& firstCut, const std::vector<size_t>& lastCut){
  auto cur = root;
  computeModuleBoundaries(root);
  computeModuleCuts(root, fpInv, firstCut, lastCut);
  suppressDummyNodes(root, fpInv);
}

MDT* getModularDecomposition(const Graph& G){
    auto fp = factorizingPermutation(G);
    auto fpInv = invert(fp);
    auto fc = getFirstCut(fp, fpInv, G);
    auto lc = getLastCut(fp, fpInv, G);
    auto parens = Parenthesize(fp, fpInv, fc, lc);
    auto tree = parensToTree(parens);
    delete parens;
    cleanupTree(tree, fpInv, fc, lc);
    reportModules(tree, fp, fpInv);
    return tree;
}

size_t decompositionWidth(MDT* tree){
  size_t maxWidth = tree->children.size();
  for(auto x : tree->children) maxWidth = std::max(maxWidth, decompositionWidth(x));
  return maxWidth;
}
