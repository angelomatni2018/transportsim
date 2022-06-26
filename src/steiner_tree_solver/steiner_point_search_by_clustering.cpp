#include "steiner_tree_solver/steiner_point_search_by_clustering.h"

using namespace world;

std::unique_ptr<Network> SteinerPointSearchByClustering::ConnectStructures(const Network& network) {
  auto connectedNetwork = SpanningTreeSolver::ConnectStructures(network);
  std::unordered_set<Location, pair_hash> buildingLocs;
  for (auto building : network.Buildings()) {
    buildingLocs.emplace(building->PrimaryLocation());
  }

  PtrVec<Point> pp;
  PtrVec<PointToPosition> ptp;
  for (int i = 0; i < int(buildingLocs.size()) - 2; ++i) {
    std::unique_ptr<KMeans> km;
    try {
      km = std::make_unique<KMeans>(buildingLocs, i + 1);
    } catch (const char* msg) {
      continue;
    }

    std::unordered_set<PointToPosition*> points;
    std::unordered_map<const Point*, PointToPosition*> pointMap;
    for (auto& cluster : km->Get()) {
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
    GeometricMedian::PositionAtMediansToMinimizeSumOfDistances(points, 0.01);

    std::unordered_set<WorldElement*> intermediates;
    Network steinerNetwork;
    for (auto building : network.Buildings()) {
      steinerNetwork.AddCopyOf(building);
    }
    for (auto& point : points) {
      auto loc = Location{round(point->pointToPositionAtMedian->first), round(point->pointToPositionAtMedian->second)};
      loc = Network::AlignLocation(loc);
      // spdlog::trace("{} {} {}", i, to_string(*point->pointToPositionAtMedian), to_string(loc));
      if (steinerNetwork.HasStructureAt(loc))
        continue;
      auto intermediate = steinerNetwork.Add(RoadSegment(loc));
      intermediates.emplace(intermediate);
    }

    // spdlog::trace("{} intermediates", intermediates.size());
    // spdlog::trace("pre {} versus {}", steinerNetwork.Size(), connectedNetwork->Size());
    SpanningTreeSolver::addRoadsTo(steinerNetwork, steinerNetwork);
    // spdlog::trace("post {} versus {}", steinerNetwork.Size(), connectedNetwork->Size());

    if (steinerNetwork.Size() < connectedNetwork->Size()) {
      connectedNetwork->Clear();
      for (auto el : steinerNetwork.Elements()) {
        if (intermediates.find(el) != intermediates.end()) {
          auto road = static_cast<Roadway*>(connectedNetwork->Add(RoadSegment(el->PrimaryLocation())));
          // TODO: Only apply directions based on adjacent nodes
          road->SetDirections(255);
        } else {
          connectedNetwork->AddCopyOf(el);
        }
      }
    } else {
      break;
    }
  }

  return connectedNetwork;
}