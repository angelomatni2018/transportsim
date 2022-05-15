#include "pathing/pathreconciler.h"
#include "spdlog/spdlog.h"

using namespace world;

void PathEvent::Delay(double amount) {
    for (int i = index; i < path->orderedPathEvents.size(); ++i) {
        path->orderedPathEvents[i]->timeAtPoint += amount;
    }
}

bool PathReconciler::Reconcile(const std::unordered_set<Path *> paths) {
    if (paths.size() == 0) {
        spdlog::debug("PathReconciler: Passed 0 paths; returning false");
        return false;
    }

    // spdlog::trace("PathReconciler: Start");
    // for (auto path : paths) {
    //     spdlog::trace(to_string(*path));
    // }

    std::unordered_map<Location, PathEvent *, pair_hash> locToCurrentEvent;
    auto trackLocationsOfEvent = [&locToCurrentEvent](PathEvent *event) -> bool {
        for (auto loc : event->locations) {
            if (locToCurrentEvent.find(loc) != locToCurrentEvent.end()) {
                spdlog::debug("PathReconciler: location {} already taken by {}", to_string(loc), to_string(*locToCurrentEvent[loc]));
                return false;
            }
            locToCurrentEvent[loc] = event;
        }
        return true;
    };
    auto untrackLocationsOfEvent = [&locToCurrentEvent](PathEvent *event) {
        for (auto loc : event->locations) {
            locToCurrentEvent.erase(loc);
        }
    };
    auto getBlockingEvents = [&locToCurrentEvent](PathEvent *event) -> std::unordered_set<PathEvent *> {
        std::unordered_set<PathEvent *> blockers;
        for (auto loc : event->locations) {
            if (locToCurrentEvent.find(loc) != locToCurrentEvent.end() && locToCurrentEvent[loc] != event) {
                blockers.emplace(locToCurrentEvent[loc]);
            }
        }
        return blockers;
    };

    std::unordered_map<PathEvent *, int> pathEventToLastTimeBlocked;
    // NOTE: priority_queue sorts greatest to least, so comparison operators below seem flipped
    auto eventCompare = [&pathEventToLastTimeBlocked](PathEvent *left, PathEvent *right) {
        auto leftTime = left->timeAtPoint;
        auto rightTime = right->timeAtPoint;
        if (equalish(leftTime, rightTime)) {
            auto leftLastBlockedIdx = pathEventToLastTimeBlocked[left];
            auto rightLastBlockedIdx = pathEventToLastTimeBlocked[right];
            // process events blocked less recently (lower iteration index)
            return leftLastBlockedIdx > rightLastBlockedIdx;
        }
        // process events that happen earlier
        return leftTime > rightTime;
    };

    std::priority_queue<PathEvent *, std::vector<PathEvent *>, decltype(eventCompare)> eventQueue(eventCompare);
    int remainingToReconcile = paths.size();
    for (Path *path : paths) {
        if (!trackLocationsOfEvent(path->orderedPathEvents[0])) {
            return false;
        }

        for (auto pathEvent : path->orderedPathEvents) {
            pathEventToLastTimeBlocked[pathEvent] = -1;
        }

        if (path->orderedPathEvents.size() > 1) {
            eventQueue.push(path->orderedPathEvents[1]);
        } else {
            --remainingToReconcile;
        }
    }

    int eventsBlockedSinceLastProgressMade = 0;
    int index = -1;
    while (!eventQueue.empty() && eventsBlockedSinceLastProgressMade < remainingToReconcile * 2) {
        ++index;
        auto pathEvent = eventQueue.top();
        eventQueue.pop();

        // spdlog::trace("Processing {} last blocked at loop index {}", to_string(*pathEvent), pathEventToLastTimeBlocked[pathEvent]);

        // If a path event blocks this one:
        auto blockers = getBlockingEvents(pathEvent);
        if (blockers.size() > 0) {
            double delayToWaitForLastBlocker = 0;
            for (auto blockingEvent : blockers) {
                // The blocking event has nowhere next to go
                if (blockingEvent->path->orderedPathEvents.size() == blockingEvent->index + 1) {
                    spdlog::debug("PathReconciler: event {} has nowhere to go and is blocking event {}", to_string(*blockingEvent), to_string(*pathEvent));
                    return false;
                }

                // Delay the event
                auto blockerNextEvent = blockingEvent->path->orderedPathEvents[blockingEvent->index + 1];
                delayToWaitForLastBlocker = std::max(delayToWaitForLastBlocker, blockerNextEvent->timeAtPoint - pathEvent->timeAtPoint);
            }

            // spdlog::trace("Delaying {} by {}", to_string(*pathEvent), delayToWaitForLastBlocker);
            pathEvent->Delay(delayToWaitForLastBlocker);
            ++eventsBlockedSinceLastProgressMade;
            pathEventToLastTimeBlocked[pathEvent] = index;
            eventQueue.push(pathEvent);
            continue;
        }

        eventsBlockedSinceLastProgressMade = 0;
        auto prevEvent = pathEvent->path->orderedPathEvents[pathEvent->index - 1];
        untrackLocationsOfEvent(prevEvent);
        trackLocationsOfEvent(pathEvent);

        if (pathEvent->path->orderedPathEvents.size() > pathEvent->index + 1) {
            auto nextEvent = pathEvent->path->orderedPathEvents[pathEvent->index + 1];
            eventQueue.push(nextEvent);
        } else {
            --remainingToReconcile;
        }
    }

    // spdlog::trace("PathReconciler: End");
    // for (auto path : paths) {
    //     spdlog::trace(to_string(*path));
    // }

    return eventsBlockedSinceLastProgressMade == 0;
}
