#include "steiner_tree_solver/steiner_point_search_by_clustering.h"

using namespace world;

void SteinerPointSearchByClustering::addRoadsTo(const Network& inputNetwork, Network& outputNetwork) {
  RoadCandidateNetworkGraph candidateGraph(inputNetwork);
  MinimumSpanningTree mst(candidateGraph);

  // Adjust the connections made by edges in MST to require ONLY ONE roadway into each building node
  auto mstEdges = mst.MstEdges();
  for (auto node : candidateGraph.Nodes()) {
    spdlog::error("Node {}", to_string(node->PrimaryLocation()));
    if (!node->IsType(Building::Type))
      continue;

    // Locate each MST edge's roadway into node ; get mean of locations of such edge's roadways
    Point mean{0, 0};
    auto edges = candidateGraph.Edges(node);
    int edgesInMst = 0;
    for (auto edge : edges) {
      // Ignore edges not in the MST
      if (mstEdges.find(edge) == mstEdges.end())
        continue;
      ++edgesInMst;

      auto candidateEdge = static_cast<const CandidateNetworkEdge*>(edge);
      auto roadIdx = candidateEdge->GetIdxOfRoadwayInto(node);
      // For now we do not support edges between adjacent buildings; I can't imagine in what valid topology such an edge could occur
      if (roadIdx == -1) {
        spdlog::error("RoadCandidateNetworkGraph corrupt edge");
        abort();
      }

      spdlog::error("\tEdge {}", to_string(candidateEdge->Roads));
      mean = mean + Point{candidateEdge->Roads[roadIdx]};
    }

    mean = mean / edgesInMst;
    Location meanOnGrid = {round(mean.first), round(mean.second)};
    // Align location to be closest to the node (i.e prefer orthogonal adjacency to diagonal adjacency)
    auto alignToStructureWhilePreferringProximityToNode = [](int v, int pref) {
      auto modRemove = (v - (v % STRUCTURE_BASE_SIZE_UNIT));
      if (v % STRUCTURE_BASE_SIZE_UNIT == (STRUCTURE_BASE_SIZE_UNIT / 2)) {
        return v > pref ? modRemove : (modRemove + STRUCTURE_BASE_SIZE_UNIT);
      }
      return (v % STRUCTURE_BASE_SIZE_UNIT < (STRUCTURE_BASE_SIZE_UNIT / 2)) ? modRemove : (modRemove + STRUCTURE_BASE_SIZE_UNIT);
    };

    Location singleRoadwayIntoNode = {alignToStructureWhilePreferringProximityToNode(meanOnGrid.first, node->PrimaryLocation().first),
                                      alignToStructureWhilePreferringProximityToNode(meanOnGrid.second, node->PrimaryLocation().second)};
    if (singleRoadwayIntoNode == node->PrimaryLocation()) {
      // Choose any edge's roadway as the only entry into the node
      auto candidateEdge = static_cast<const CandidateNetworkEdge*>(edges.front());
      singleRoadwayIntoNode = candidateEdge->Roads[candidateEdge->GetIdxOfRoadwayInto(node)];
    }
    spdlog::error("\tMean {}", to_string(singleRoadwayIntoNode));

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
      if (singleRoadwayIntoNode == roadwayLoc)
        continue;

      candidateEdge->Roads.erase(roads.begin() + roadwayIdx);
      // NOTE: indexing explanation: for "road" list [ThisNode, Road1, Road2, OtherNode] we removed Road1; Road2 is still at roadwayIdx
      // However, if [OtherNode, Road1, Road2, ThisNode] then we removed Road2 and Road1 is at (roadwayIdx - 1)
      roadwayIdx = roadwayIdx + (atBeginning ? 0 : -1);
      auto secondToLastRoadwayLoc = roads[roadwayIdx];

      EmptyLocationPathfinder pathfinder;
      auto path = pathfinder.Solve(inputNetwork, secondToLastRoadwayLoc, singleRoadwayIntoNode);
      spdlog::error("\tPath {} -> {} length {}", to_string(secondToLastRoadwayLoc), to_string(singleRoadwayIntoNode), path.size());
      // We skip path[0] because pathfinder's solution INCLUDES endpoints (path[0] == secondToLastRoadwayLoc)
      for (auto i = 1; i < path.size(); ++i) {
        // NOTE: indexing explanation: for "road" list [ThisNode, Road2, OtherNode] we keep inserting at Road2's index (roadwayIdx)
        // since the path is from Road2 -> singleRoadwayIntoNode
        // However, if [OtherNode, Road1, ThisNode] then we insert in order after Road1: (roadwayIdx + 1), (roadwayIdx + 2) and so on
        roads.insert(roads.begin() + roadwayIdx + (atBeginning ? 0 : i), path[i]);
      }

      spdlog::error("\tEnd result of roads");
      for (auto road : roads) {
        spdlog::error("\t\t{}", to_string(road));
      }
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
          // TODO: Should we abort on not passing through a road segment?
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

std::unique_ptr<Network> SteinerPointSearchByClustering::ConnectStructures(const Network& network) {
  auto connectedNetwork = std::make_unique<Network>();
  PtrVec<Point> pp;
  PtrVec<PointToPosition> ptp;

  int numBuildings = 0;
  std::unordered_set<Location, pair_hash> buildingLocs;
  for (auto building : network.Buildings()) {
    connectedNetwork->AddCopyOf(building);
    ++numBuildings;
    buildingLocs.emplace(building->PrimaryLocation());
  }

  addRoadsTo(network, *connectedNetwork);

  for (int i = 0; i < numBuildings - 2; ++i) {
    KMeans km(buildingLocs, i + 1);
    std::unordered_set<PointToPosition*> points;
    std::unordered_map<const Point*, PointToPosition*> pointMap;
    for (auto& cluster : km.Get()) {
      PointToPosition* pos = ptp.Add(PointToPosition(pp.Add(Point(cluster->centroid))));
      for (auto loc : cluster->members) {
        // TODO: Solve mst once to determine adjacent cluster members to add
        pos->clusterPoints.emplace(pp.Add(Point(loc)));
      }
      points.emplace(pos);
      pointMap[pos->pointToPositionAtMedian] = pos;
    }
    VirtualGraph clusterCentroidGraph;
    for (auto& [one, _] : pointMap) {
      for (auto& [other, _] : pointMap) {
        if (one == other)
          continue;
        clusterCentroidGraph.Add(one, other);
      }
    }
    MinimumSpanningTree clusterCentroidMst(clusterCentroidGraph);
    for (auto edge : clusterCentroidMst.MstEdges()) {
      auto [one, other] = edge->Nodes();
      auto onePos = static_cast<VirtualWorldElement*>(one)->ExactPosition();
      auto otherPos = static_cast<VirtualWorldElement*>(other)->ExactPosition();
      pointMap[onePos]->clusterPoints.emplace(const_cast<Point*>(otherPos));
      pointMap[otherPos]->clusterPoints.emplace(const_cast<Point*>(onePos));
    }
    GeometricMedian::PositionAtMediansToMinimizeSumOfDistances(points);

    std::unordered_set<WorldElement*> intermediates;
    Network steinerNetwork;
    for (auto building : network.Buildings()) {
      steinerNetwork.AddCopyOf(building);
    }
    for (auto& point : points) {
      auto loc = Location{round(point->pointToPositionAtMedian->first), round(point->pointToPositionAtMedian->second)};
      loc = Network::AlignLocation(loc);
      spdlog::trace("{} {} {}", i, to_string(*point->pointToPositionAtMedian), to_string(loc));
      if (steinerNetwork.HasStructureAt(loc))
        continue;
      auto intermediate = steinerNetwork.Add(Building({STRUCTURE_BASE_SIZE_UNIT, STRUCTURE_BASE_SIZE_UNIT}, loc));
      intermediates.emplace(intermediate);
    }

    spdlog::trace("{} intermediates", intermediates.size());
    spdlog::trace("pre {} versus {}", steinerNetwork.Size(), connectedNetwork->Size());
    addRoadsTo(steinerNetwork, steinerNetwork);
    spdlog::trace("post {} versus {}", steinerNetwork.Size(), connectedNetwork->Size());

    if (steinerNetwork.Size() < connectedNetwork->Size()) {
      for (auto intermediate : intermediates) {
        connectedNetwork->Clear();
        for (auto& [loc, el] : steinerNetwork.SpatialMap()) {
          if (intermediates.find(el) != intermediates.end()) {
            auto road = static_cast<Roadway*>(connectedNetwork->Add(RoadSegment(loc)));
            // TODO: Only apply directions based on adjacent nodes
            road->SetDirections(255);
          } else {
            connectedNetwork->AddCopyOf(el);
          }
        }
      }
    }
  }

  return connectedNetwork;
}