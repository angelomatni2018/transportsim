#include "steiner_tree_solver/spanning_tree_solver.h"

using namespace world;

void SpanningTreeSolver::addRoadsTo(const Network& inputNetwork, Network& outputNetwork) {
  RoadCandidateNetworkGraph candidateGraph(inputNetwork);
  MinimumSpanningTree mst(candidateGraph);

  // Adjust the connections made by edges in MST to require ONLY ONE roadway into each building node
  auto mstEdges = mst.MstEdges();
  for (auto node : candidateGraph.Nodes()) {
    // spdlog::trace("Node {}", to_string(node->PrimaryLocation()));
    if (!node->IsType(Building::Type))
      continue;

    // Locate each MST edge's roadway into node ; get mean of locations of such edge's roadways
    Point mean{0, 0};
    auto edges = candidateGraph.Edges(node);
    auto adjEdgesInMst = 0;
    for (auto edge : edges) {
      // Ignore edges not in the MST
      if (mstEdges.find(edge) == mstEdges.end())
        continue;
      ++adjEdgesInMst;

      auto candidateEdge = static_cast<const CandidateNetworkEdge*>(edge);
      auto roadIdx = candidateEdge->GetIdxOfRoadwayInto(node);
      // For now we do not support edges between adjacent buildings; I can't imagine in what valid topology such an edge could occur
      if (roadIdx == -1) {
        spdlog::trace("RoadCandidateNetworkGraph corrupt edge: {} -> {}", to_string(candidateEdge->Nodes().first->PrimaryLocation()),
                      to_string(candidateEdge->Nodes().second->PrimaryLocation()));
        throw "RoadCandidateNetworkGraph corrupt edge";
      }

      // spdlog::trace("\tEdge {}", to_string(candidateEdge->Roads));
      mean = mean + Point{candidateEdge->Roads[roadIdx]};
    }

    mean = mean / adjEdgesInMst;
    Location meanOnGrid = {round(mean.first), round(mean.second)};
    // Align location to be closest to the node (i.e prefer orthogonal adjacency to diagonal adjacency)
    auto alignAndPreferNodeAdjacency = [](int v, int pref) {
      int m = STRUCTURE_BASE_SIZE_UNIT;
      int rem = v % m;
      int vLessRem = v - rem;
      if (rem == (m / 2)) {
        return v > pref ? vLessRem : (vLessRem + m);
      }
      return (rem < (m / 2)) ? vLessRem : (vLessRem + m);
    };

    auto [nodeX, nodeY] = node->PrimaryLocation();
    Location alignedMean = {alignAndPreferNodeAdjacency(meanOnGrid.first, nodeX), alignAndPreferNodeAdjacency(meanOnGrid.second, nodeY)};
    Location singleRoadwayIntoNode = alignedMean;
    const std::vector<Location> offsets = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    for (auto offset : offsets) {
      if (!inputNetwork.HasStructureAt(singleRoadwayIntoNode))
        break;
      singleRoadwayIntoNode = Location{nodeX, nodeY} + STRUCTURE_BASE_SIZE_UNIT * offset;
    }
    if (inputNetwork.HasStructureAt(singleRoadwayIntoNode)) {
      spdlog::trace("Network is too dense; need a more robust solver for condition that only one roadway can connect into any one building");
      throw "SpanningTreeSolver: network is too dense";
    }
    // if (singleRoadwayIntoNode == node->PrimaryLocation()) {
    //   // Choose any edge's roadway as the only entry into the node
    //   auto candidateEdge = static_cast<const CandidateNetworkEdge*>(edges.front());
    //   singleRoadwayIntoNode = candidateEdge->Roads[candidateEdge->GetIdxOfRoadwayInto(node)];
    // }
    // spdlog::trace("\tMean {}", to_string(singleRoadwayIntoNode));

    // Connect all other MST edges to the chosen singleRoadwayIntoNode instead of each edge going directly into the node as is now
    for (auto edge : edges) {
      // Ignore edges not in the MST
      if (mstEdges.find(edge) == mstEdges.end())
        continue;

      auto candidateEdge = static_cast<CandidateNetworkEdge*>(const_cast<NetworkEdge*>(edge));
      auto& roads = candidateEdge->Roads;
      auto roadwayIdx = candidateEdge->GetIdxOfRoadwayInto(node);
      auto atBeginning = candidateEdge->Nodes().first == node;
      auto roadwayLoc = roads[roadwayIdx];
      // if (singleRoadwayIntoNode == roadwayLoc)
      //   continue;

      candidateEdge->Roads.erase(roads.begin() + roadwayIdx);
      // NOTE: indexing explanation: for "road" list [ThisNode, Road1, Road2, OtherNode] we removed Road1; Road2 is still at roadwayIdx
      // However, if [OtherNode, Road1, Road2, ThisNode] then we removed Road2 and Road1 is at (roadwayIdx - 1)
      roadwayIdx = roadwayIdx + (atBeginning ? 0 : -1);
      auto secondToLastRoadwayLoc = roads[roadwayIdx];

      ShareRoadEmptyLocationPathfinder pathfinder(candidateGraph);
      auto path = pathfinder.Solve(inputNetwork, secondToLastRoadwayLoc, singleRoadwayIntoNode);
      // spdlog::trace("\tPath {} -> {} length {}", to_string(secondToLastRoadwayLoc), to_string(singleRoadwayIntoNode), path.size());
      // We skip path[0] because pathfinder's solution INCLUDES endpoints (path[0] == secondToLastRoadwayLoc)
      for (auto i = 1; i < path.size(); ++i) {
        // NOTE: indexing explanation: for "road" list [ThisNode, Road2, OtherNode] we keep inserting at Road2's index (roadwayIdx)
        // since the path is from Road2 -> singleRoadwayIntoNode
        // However, if [OtherNode, Road1, ThisNode] then we insert in order after Road1: (roadwayIdx + 1), (roadwayIdx + 2) and so on
        roads.insert(roads.begin() + roadwayIdx + (atBeginning ? 0 : i), path[i]);
      }

      // spdlog::trace("\tEnd result of roads");
      // for (auto road : roads) {
      //   spdlog::trace("\t\t{}", to_string(road));
      // }
    }
  }

  for (auto edge : mstEdges) {
    auto candidateEdge = static_cast<const CandidateNetworkEdge*>(edge);
    for (int i = 1; i < candidateEdge->Roads.size() - 1; ++i) {
      auto& loc = candidateEdge->Roads[i];
      RoadSegment* road = nullptr;
      if (!outputNetwork.HasStructureAt(loc)) {
        road = static_cast<RoadSegment*>(outputNetwork.Add(RoadSegment(loc)));
        road->SetDirections(0);
      } else {
        auto el = outputNetwork.StructureAt(loc);
        if (el->IsType(RoadSegment::Type)) {
          road = static_cast<RoadSegment*>(const_cast<WorldElement*>(el));
        } else {
          // TODO: Should we throw on not passing through a road segment?
          spdlog::debug("Passing through a structure other than a road segment at location {}", to_string(loc));
          continue;
        }
      }

      auto headingToBefore = candidateEdge->Roads[i - 1] - loc;
      auto headingToAfter = candidateEdge->Roads[i + 1] - loc;
      road->SetDirections(road->DirectionsMask() | Roadway::DirectionWhenHeading(headingToBefore) | Roadway::DirectionWhenHeading(headingToAfter));
    }
  }
};

std::unique_ptr<Network> SpanningTreeSolver::ConnectStructures(const Network& network) {
  auto connectedNetwork = std::make_unique<Network>();
  for (auto el : network.Elements()) {
    connectedNetwork->AddCopyOf(el);
  }
  addRoadsTo(network, *connectedNetwork);
  return connectedNetwork;
}