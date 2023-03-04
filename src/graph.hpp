#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <bit>
#include <string>
#include <vector>
#include "log.hpp"


typedef size_t vx;

class Graph{
private:
  vx numVerts;
  std::vector<vx> contractionSeq;
  std::vector<std::vector<vx>> blackAdjMat;
  std::vector<std::vector<vx>> redAdjMat;
public:
  Graph() = default;
  ~Graph() = default;
  Graph(vx n);
  vx vCount() const {return numVerts;}
  std::vector<vx> contractionSequence() const {return contractionSeq;}

  void addBlackEdge(vx v, vx w);
  void removeBlackEdge(vx v, vx w);
  bool isBlackEdge(vx v, vx w) const;
  void addRedEdge(vx v, vx w);
  void removeRedEdge(vx v, vx w);
  bool isRedEdge(vx v, vx w) const;
  void inspect() const;
  Graph contract(vx v, vx w);
  vx redDegree(vx v) const;
  vx blackDegree(vx v) const;
  vx getNumVertices() const {return numVerts;}
  vx currentWidth() const;
  std::pair<vx,vx>  resultingWidth(vx v, vx w) const;
  Graph doBestContraction();
  void logToFile(std::string fname) const;
  std::vector<size_t> getNeighbours(size_t v) const;
};

Graph initGraphFromFile(const std::string& fname);
void unpackContractionSequence(std::vector<vx>& cs);
Graph randomGraph(size_t n, size_t m);


#endif /* end of include guard: GRAPH_HPP */
