#include "steiner_tree_solver/steiner_point_search_by_clustering.h"

using namespace world;

void SteinerPointSearchByClustering::addRoadsFromMst(MinimumSpanningTree& mst, Network& network) {
  for (auto edge : mst.MstEdges()) {
    auto candidateEdge = static_cast<const CandidateNetworkEdge*>(edge);
    for (int i = 1; i < candidateEdge->Roads.size() - 1; ++i) {
      auto& loc = candidateEdge->Roads[i];
      RoadSegment* road = nullptr;
      if (!network.HasStructureAt(loc)) {
        road = static_cast<RoadSegment*>(network.Add(RoadSegment(loc)));
      } else {
        auto el = network.StructureAt(loc);
        if (el->IsType(RoadSegment::Type)) {
          road = static_cast<RoadSegment*>(const_cast<WorldElement*>(el));
        }
      }

      auto headingToBefore = candidateEdge->Roads[i - 1] - loc;
      auto headingToAfter = candidateEdge->Roads[i + 1] - loc;
      road->SetDirections(Roadway::DirectionWhenHeading(headingToBefore) | Roadway::DirectionWhenHeading(headingToAfter));
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

  RoadCandidateNetworkGraph candidateGraph(network);
  MinimumSpanningTree mst(candidateGraph);
  addRoadsFromMst(mst, *connectedNetwork);

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
      auto loc = Location{*point->pointToPositionAtMedian};
      loc = {(loc.first / STRUCTURE_BASE_SIZE_UNIT) * STRUCTURE_BASE_SIZE_UNIT, (loc.second / STRUCTURE_BASE_SIZE_UNIT) * STRUCTURE_BASE_SIZE_UNIT};
      spdlog::trace("{} {} {}", i, to_string(*point->pointToPositionAtMedian), to_string(loc));
      if (steinerNetwork.HasStructureAt(loc))
        continue;
      auto intermediate = steinerNetwork.Add(Building({STRUCTURE_BASE_SIZE_UNIT, STRUCTURE_BASE_SIZE_UNIT}, loc));
      intermediates.emplace(intermediate);
    }

    spdlog::trace("{} intermediates", intermediates.size());
    spdlog::trace("pre {} versus {}", steinerNetwork.Size(), connectedNetwork->Size());
    RoadCandidateNetworkGraph candidateGraph(steinerNetwork);
    MinimumSpanningTree mst(candidateGraph);
    addRoadsFromMst(mst, steinerNetwork);
    spdlog::trace("post {} versus {}", steinerNetwork.Size(), connectedNetwork->Size());
    if (steinerNetwork.Size() < connectedNetwork->Size()) {
      for (auto intermediate : intermediates) {
        connectedNetwork->Clear();
        for (auto& [loc, el] : steinerNetwork.SpatialMap()) {
          if (intermediates.find(el) != intermediates.end()) {
            auto road = static_cast<Roadway*>(connectedNetwork->Add(RoadSegment(loc)));
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