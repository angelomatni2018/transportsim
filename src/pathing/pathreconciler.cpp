#include "pathing/pathreconciler.h"

using namespace world;

void PathEvent::delay(double amount) {
    for (int i = index; i < path->orderedPathEvents.size(); ++i) {
        if (path->orderedPathEvents[i] == nullptr) continue;
        path->orderedPathEvents[i]->timeAtPoint += amount;
    }
}

bool PathReconciler::reconcile(std::unordered_set<Path *> paths) {
    // if (paths.size() > 0) {
    //     std::cout << "Before\n";
    //     for (auto path : paths) {
    //         std::cout << *path << "\n";
    //     }
    // }

    std::unordered_map<LocDir, PathEvent *, pair_hash> locDirToCurrentEvent;
    std::unordered_map<PathEvent *, int> pathEventToLastTimeBlocked;
    // NOTE: priority_queue sorts greatest to least, so comparison operators below seem flipped
    auto eventCompare = [&pathEventToLastTimeBlocked](PathEvent *left, PathEvent *right) {
        auto leftTime = left->timeAtPoint;
        auto rightTime = right->timeAtPoint;
        if (equalWrtEpsilon(leftTime, rightTime)) {
            auto leftLastBlockedIdx = pathEventToLastTimeBlocked[left];
            auto rightLastBlockedIdx = pathEventToLastTimeBlocked[right];
            // std::cout << "Compare: " << leftLastBlockedIdx << " " << rightLastBlockedIdx << "\n";
            // process events blocked less recently (lower iteration index)
            return leftLastBlockedIdx > rightLastBlockedIdx;
        }
        // process events that happen earlier
        // std::cout << "Compare: " << leftTime << " " << rightTime << " " << (leftTime - rightTime) << "\n";
        return leftTime > rightTime;
    };

    std::priority_queue<PathEvent *, std::vector<PathEvent *>, decltype(eventCompare)> eventQueue(eventCompare);
    int remainingToReconcile = paths.size();
    for (Path *path : paths) {
        if (path->orderedPathEvents[0] == nullptr) continue;

        auto locDir = path->orderedPathEvents[0]->locDir();
        if (locDirToCurrentEvent.find(locDir) != locDirToCurrentEvent.end()) {
            return false;
        }

        for (auto pathEvent : path->orderedPathEvents) {
            pathEventToLastTimeBlocked[pathEvent] = -1;
        }

        locDirToCurrentEvent[locDir] = path->orderedPathEvents[0];
        if (path->orderedPathEvents.size() > 1 && path->orderedPathEvents[1] != nullptr) {
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

        // if (pathEvent == nullptr) { continue; }
        // std::cout << "Processing: " << *pathEvent << " " << pathEventToLastTimeBlocked[pathEvent] << "\n";
        // for (auto &[locDir, currentEvent] : locDirToCurrentEvent) {
        //     std::cout << locDir << " " << *currentEvent << "\t";
        // }
        // std::cout << "\n";

        // If a path event blocks this one:
        auto locDir = pathEvent->locDir();
        if (locDirToCurrentEvent.find(locDir) != locDirToCurrentEvent.end()) {
            auto blockingEvent = locDirToCurrentEvent[locDir];
            // The blocking event has nowhere next to go
            if (blockingEvent->path->orderedPathEvents.size() == blockingEvent->index + 1) return false;

            // Delay the event
            auto blockerNextEvent = blockingEvent->path->orderedPathEvents[blockingEvent->index + 1];
            if (blockerNextEvent != nullptr) {
                pathEvent->delay(blockerNextEvent->timeAtPoint - pathEvent->timeAtPoint);
            }
            ++eventsBlockedSinceLastProgressMade;
            pathEventToLastTimeBlocked[pathEvent] = index;

            eventQueue.push(pathEvent);
            continue;
        }

        eventsBlockedSinceLastProgressMade = 0;
        auto prevEvent = pathEvent->path->orderedPathEvents[pathEvent->index - 1];
        // std::cout << "Erasing: " << prevEvent->locDir() << " " << *prevEvent << " before " << *pathEvent << "\n";
        locDirToCurrentEvent.erase(prevEvent->locDir());
        locDirToCurrentEvent[locDir] = pathEvent;

        if (pathEvent->path->orderedPathEvents.size() > pathEvent->index + 1) {
            auto nextEvent = pathEvent->path->orderedPathEvents[pathEvent->index + 1];
            if (nextEvent != nullptr) {
                // std::cout << "Moving on: " << *pathEvent << "\n";
                eventQueue.push(nextEvent);
            } else {
                // std::cout << "Vanishing on: " << *pathEvent << "\n";
                locDirToCurrentEvent.erase(locDir);
                --remainingToReconcile;
            }
        } else {
            // std::cout << "Ending on: " << *pathEvent << "\n";
            --remainingToReconcile;
        }
    }

    // if (paths.size() > 0) {
    //     std::cout << "After\n";
    //     for (auto path : paths) {
    //         std::cout << *path << "\n";
    //     }
    // }

    return eventsBlockedSinceLastProgressMade == 0;
}
