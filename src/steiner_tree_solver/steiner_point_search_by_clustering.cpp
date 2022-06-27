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
    AgglomerativeClustering clustering(buildingLocs, i + 1);
    // std::unique_ptr<KMeans> km;
    // try {
    //   km = std::make_unique<KMeans>(buildingLocs, i + 1);
    //   // spdlog::trace("KM: {} clusters succeeded", i + 1);
    //   // for (auto& cluster : km->Get()) {
    //   //   spdlog::trace("Centroid {}", to_string(cluster->centroid));
    //   // }
    // } catch (const char* msg) {
    //   // spdlog::trace("KM: {} clusters FAILED", i + 1);
    //   continue;
    // }

    std::unordered_set<PointToPosition*> points;
    std::unordered_map<const Point*, PointToPosition*> pointMap;
    for (auto& cluster : clustering.Get()) {
      PointToPosition* pos = ptp.Add(PointToPosition(pp.Add(Point(cluster->centroid))));
      for (auto loc : cluster->members) {
        // TODO: Solve mst once to determine adjacent cluster members to add
        pos->clusterPoints.emplace(pp.Add(Point(loc)));
        // spdlog::trace("Initial cluster {} has member {}", to_string(*pos->pointToPositionAtMedian), to_string(loc));
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
      // spdlog::trace("Connecting cluster centroids {} and {}", to_string(*onePos), to_string(*otherPos));
      pointMap[onePos]->clusterPoints.emplace(const_cast<Point*>(otherPos));
      pointMap[otherPos]->clusterPoints.emplace(const_cast<Point*>(onePos));
    }
    GeometricMedian::PositionAtMediansToMinimizeSumOfDistances(points, 0.1);

    std::unordered_set<WorldElement*> intermediates;
    Network steinerNetwork;
    for (auto building : network.Buildings()) {
      steinerNetwork.AddCopyOf(building);
    }
    for (auto& point : points) {
      auto loc = Location{round(point->pointToPositionAtMedian->first), round(point->pointToPositionAtMedian->second)};
      loc = Network::AlignLocation(loc);
      // spdlog::trace("Aligned median: {}", to_string(loc));
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
          auto loc = el->PrimaryLocation();
          auto road = static_cast<Roadway*>(connectedNetwork->Add(RoadSegment(loc)));
          road->SetDirections(0);

          for (auto offset : std::vector<Location>{{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}}) {
            Location other = loc + STRUCTURE_BASE_SIZE_UNIT * offset;
            if (steinerNetwork.HasStructureAt(other)) {
              auto otherEl = steinerNetwork.StructureAt(other);
              auto connects = otherEl->IsType(Building::Type);
              if (otherEl->IsType(Roadway::Type)) {
                auto otherRoad = static_cast<const Roadway*>(otherEl);
                connects = otherRoad->DirectionsMask() & otherRoad->DirectionTowardsLocation(loc);
              }
              if (connects) {
                road->SetDirections(road->DirectionsMask() | road->DirectionTowardsLocation(other));
              }
            }
          }
        } else {
          connectedNetwork->AddCopyOf(el);
        }
      }
      // } else {
      //   break;
    }
  }

  return connectedNetwork;
}