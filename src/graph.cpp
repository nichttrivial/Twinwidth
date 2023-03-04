#include "graph.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <bitset>
#include <cstdlib>

Graph::Graph(vx n){
  contractionSeq = {};
  numVerts = n;

  vx neededWords = 1 + n / (8*sizeof(vx));
  blackAdjMat = std::vector<std::vector<vx>>(n, std::vector<vx>(neededWords, 0));
  redAdjMat = std::vector<std::vector<vx>>(n, std::vector<vx>(neededWords, 0));
}

void Graph::addBlackEdge(vx v, vx w){
  vx word = w / (sizeof(vx) * 8);
  vx offset = w % (sizeof(vx) * 8);
  blackAdjMat[v][word] |= (1ULL << offset);

  word = v / (sizeof(vx) * 8);
  offset = v % (sizeof(vx) * 8);
  blackAdjMat[w][word] |= (1ULL << offset);
}

void Graph::addRedEdge(vx v, vx w){
  vx word = w / (sizeof(vx) * 8);
  vx offset = w % (sizeof(vx) * 8);
  redAdjMat[v][word] |= 1ULL << offset;

  word = v / (sizeof(vx) * 8);
  offset = v % (sizeof(vx) * 8);
  redAdjMat[w][word] |= 1ULL << offset;
}

void Graph::removeBlackEdge(vx v, vx w){
  vx word = w / (sizeof(vx) * 8);
  vx offset = w % (sizeof(vx) * 8);
  blackAdjMat[v][word] &= ~(1ULL << offset);

  word = v / (sizeof(vx) * 8);
  offset = v % (sizeof(vx) * 8);
  blackAdjMat[w][word] &= ~(1ULL << offset);
}

void Graph::removeRedEdge(vx v, vx w){
  vx word = w / (sizeof(vx) * 8);
  vx offset = w % (sizeof(vx) * 8);
  redAdjMat[v][word] &= ~(1ULL << offset);

  word = v / (sizeof(vx) * 8);
  offset = v % (sizeof(vx) * 8);
  redAdjMat[w][word] &= ~(1ULL << offset);
}

vx Graph::redDegree(vx v) const{
  int count = 0;
  for(auto& x : redAdjMat[v]) count += std::popcount(x);
  return count;
}

vx Graph::blackDegree(vx v) const{
  int count = 0;
  for(auto& x : blackAdjMat[v]) count += std::popcount(x);
  return count;
}

bool Graph::isRedEdge(vx v, vx w) const {
  vx word = w / (sizeof(vx) * 8);
  vx offset = w % (sizeof(vx) * 8);
  return (redAdjMat[v][word] & (1ULL << offset));
}

bool Graph::isBlackEdge(vx v, vx w) const {

  vx word = w / (sizeof(vx) * 8);
  vx offset = w % (sizeof(vx) * 8);
  auto rval = (blackAdjMat[v][word] & (1ULL << offset));


  return rval;
}

void Graph::inspect() const {

  for(auto& v : redAdjMat){
    for(auto& w : v){
      std::cout << std::bitset<8*sizeof(vx)>(w);
    }
    std::cout << "\n";
  }
  std::cout << "\n\n";
  for(auto& v : blackAdjMat){
    for(auto& w : v){
      std::cout << std::bitset<8*sizeof(vx)>(w);
    }
    std::cout << "\n";
  }
  std::cout << "Width " << currentWidth() << "\n\n\n";

}

Graph initGraphFromFile(const std::string& fname){
  bool problemLine = false;
  int n,m;
  std::ifstream f(fname, std::ios::in);
  Graph G;
  if (f.is_open()) {

    std::string line;
    while (std::getline(f, line)) {
      if(line[0] == 'c') continue;

      if(line[0] == 'p'){
        std::stringstream ss(line.substr(6));
        ss >> n >> m;
        G = Graph(n);

        problemLine = true;
        continue;
      }
      if(!problemLine) continue;
      std::stringstream ss(line);
      vx v,w;
      ss >> v >> w;
      v--;
      w--;
      G.addBlackEdge(v,w);
      m--;
    }
    f.close();
  }
  else {
    std::cerr << "Unable to open file\n";
  }
  if(!problemLine) LOG( "Input file did not coontain a problem line. No Graph has been generated.\n");
  if(m != 0)  LOG("File Parsing did not produce the number of edges indicated in the problem line");
  return G;
}

Graph Graph::contract(vx v, vx w){
  if(v > w) std::swap(v,w);
  if(v == w) return *this;
  Graph G(numVerts-1);
  for (vx i = 0; i < numVerts; i++) {
    //copy edges outside of v,w
    if(i == w || i == v) continue;
    for (vx j = i+1; j < numVerts; j++) {
      if (j == w || j == v) continue;
      if(isBlackEdge(i,j)) G.addBlackEdge(i - (i>w),j - (j>w));
      if(isRedEdge(i,j)) G.addRedEdge(i - (i>w),j - (j>w));
    }
  }
  for (vx i = 0; i < numVerts; i++) {
    if(i == v || i == w) continue;
    // process edges (i, v)
    if( isRedEdge(i,v) || isRedEdge(i,w) || (isBlackEdge(i,v) != isBlackEdge(i,w)) ){
      G.addRedEdge(v, i - (i>w));
    }else{
      if(isBlackEdge(i,v))  G.addBlackEdge(v, i - (i>w));
    }
  }
  G.contractionSeq = contractionSeq;
  G.contractionSeq.push_back(v);
  G.contractionSeq.push_back(w);
  return G;
}

vx Graph::currentWidth() const {
  vx rval = 0;
  for (vx i = 0; i < numVerts; i++) {
    rval = std::max(rval, redDegree(i));
  }
  return rval;
}

std::pair<vx,vx> Graph::resultingWidth(vx v, vx w) const {
  vx max = 0;
  vx atV = 0;
  vx redChange = 0;

  for (vx i = 0; i < numVerts; i++) {
    if(i == v || i == w) continue;
    vx curRedDegree = redDegree(i);
    if(isBlackEdge(i,v) != isBlackEdge(i,w)){curRedDegree++;}
    if(isRedEdge(i,v) && isRedEdge(i,w)){curRedDegree--; redChange--;}
    if(isBlackEdge(i,v) && !isBlackEdge(i,w) && !isRedEdge(i,w) || isBlackEdge(i,w) && !isBlackEdge(i,v) && !isRedEdge(i,v)) redChange++;
    max = std::max(max, curRedDegree);
  }
  for (vx i = 0; i < numVerts; i++) {
    if(i == v || i == w) continue;
    if(isRedEdge(i,v) || isRedEdge(i,w) || (isBlackEdge(i,v) != isBlackEdge(i,w) ) ) atV++;
  }
  if(isRedEdge(v,w)) redChange--;
  return {std::max(atV, max), redChange};
}

Graph Graph::doBestContraction(){
  vx incumbent = numVerts;
  vx redChange = numVerts;
  vx bestV, bestW;
  for (vx i = 0; i < numVerts; i++) {
    for (vx j = i+1; j < numVerts; j++) {
      auto [inc, rc] = resultingWidth(i,j);
      if(inc < incumbent || inc == incumbent && rc  < redChange){
        incumbent = inc;
        redChange = rc;
        bestV = i;
        bestW = j;
        if (incumbent == 0) goto Breakout;
      }
    }
  }
  Breakout:
  return contract(bestV, bestW);
}

void Graph::logToFile(std::string fname) const {
  std::ofstream f(fname, std::ios::out);
  if (f.is_open()) {
    f << "p tww " << numVerts << " " << 1 << "\n";
    for (vx v = 0; v < numVerts; v++) {
      for (vx w = v+1; w < numVerts; w++) {
        if(isBlackEdge(v,w)) f << v+1 << " " << w+1 << "\n";
      }
    }
    f.close();
  }
}

std::vector<size_t> Graph::getNeighbours(size_t v) const{
  std::vector<size_t> rval = {};
  for (size_t i = 0; i < numVerts; i++) {
    if(i == v) continue;
    if(isBlackEdge(i,v)) rval.push_back(i);
  }
  std::sort(rval.begin(), rval.end());
  return rval;
}


void unpackContractionSequence(std::vector<vx>& cs){
  auto n = cs.size();
  for(int i = n-1; i > 0; i-=2){
    for(int j = i+1; j < n; j++){
      if(cs[j] >= cs[i]) cs[j]++;
    }
  }
}

Graph randomGraph(vx n, vx m){
  Graph G(n);
  for (vx i = 0; i < m; i++) {
    vx v = std::rand() % n;
    vx w = std::rand() % n;
    G.addBlackEdge(v,w);
  }
  return G;
}
