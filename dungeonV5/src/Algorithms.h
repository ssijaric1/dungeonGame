// Algorithms.h - IMPROVED VERSION
#pragma once
#include <vector>
#include <utility>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <climits>
#include "MDPSolver.h"

namespace DungeonAlgorithms {

    // ==================== CONSTANTS ====================
    const int GRID_SIZE = 10;
    const int DIRECTIONS[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

    // ==================== STRUCTS ====================
    struct SearchResult {
        std::vector<std::pair<int, int>> path;
        std::vector<std::pair<int, int>> exploredNodes;
    };

    // ==================== UTILITY FUNCTIONS ====================
    inline bool isValid(int x, int y) {
        return x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE;
    }

    inline std::vector<std::pair<int, int>> reconstructPath(
        const std::map<std::pair<int, int>, std::pair<int, int>>& parent,
        const std::pair<int, int>& start,
        const std::pair<int, int>& goal) {

        std::vector<std::pair<int, int>> path;
        std::pair<int, int> current = goal;

        while (current != start) {
            path.push_back(current);
            auto it = parent.find(current);
            if (it == parent.end()) {
                return std::vector<std::pair<int, int>>();
            }
            current = it->second;
        }

        path.push_back(start);
        std::reverse(path.begin(), path.end());
        return path;
    }

    // ==================== COST FUNCTION ====================
    // IMPROVED: Better reflects actual game penalties
    inline int getMoveCost(int cellType) {
        switch (cellType) {
        case 2: return 0;    // Reward - FREE! (you gain 10 gold)
        case 3: return 15;   // Bandit - EXPENSIVE! (lose half your gold)
        case 4: return 8;    // Mine - COSTLY (lose 5 gold + need to answer question)
        default: return 1;   // Empty, Player, Exit - normal movement
        }
    }

    // ==================== ALGORITHM IMPLEMENTATIONS ====================

    // BFS - Unweighted, finds shortest path by number of steps
    inline SearchResult bfsSearch(
        const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start,
        std::pair<int, int> goal) {

        SearchResult result;
        std::queue<std::pair<int, int>> q;
        std::set<std::pair<int, int>> visited;
        std::map<std::pair<int, int>, std::pair<int, int>> parent;

        q.push(start);
        visited.insert(start);
        result.exploredNodes.push_back(start);
        parent[start] = { -1, -1 };

        while (!q.empty()) {
            auto current = q.front();
            q.pop();

            if (current == goal) {
                result.path = reconstructPath(parent, start, goal);
                return result;
            }

            for (int i = 0; i < 4; i++) {
                int newX = current.first + DIRECTIONS[i][0];
                int newY = current.second + DIRECTIONS[i][1];

                if (isValid(newX, newY)) {
                    std::pair<int, int> neighbor = { newX, newY };

                    if (visited.find(neighbor) == visited.end()) {
                        visited.insert(neighbor);
                        result.exploredNodes.push_back(neighbor);
                        parent[neighbor] = current;
                        q.push(neighbor);
                    }
                }
            }
        }

        return result;
    }

    // DFS - Unweighted, explores depth-first
    inline SearchResult dfsSearch(
        const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start,
        std::pair<int, int> goal) {

        SearchResult result;
        std::stack<std::pair<int, int>> s;
        std::set<std::pair<int, int>> visited;
        std::map<std::pair<int, int>, std::pair<int, int>> parent;

        s.push(start);
        visited.insert(start);
        result.exploredNodes.push_back(start);
        parent[start] = { -1, -1 };

        while (!s.empty()) {
            auto current = s.top();
            s.pop();

            if (current == goal) {
                result.path = reconstructPath(parent, start, goal);
                return result;
            }

            for (int i = 3; i >= 0; i--) {
                int newX = current.first + DIRECTIONS[i][0];
                int newY = current.second + DIRECTIONS[i][1];

                if (isValid(newX, newY)) {
                    std::pair<int, int> neighbor = { newX, newY };

                    if (visited.find(neighbor) == visited.end()) {
                        visited.insert(neighbor);
                        result.exploredNodes.push_back(neighbor);
                        parent[neighbor] = current;
                        s.push(neighbor);
                    }
                }
            }
        }

        return result;
    }

    // DIJKSTRA / UCS - Weighted, finds minimum cost path
    // Considers: Rewards (cost=0), Bandits (cost=15), Mines (cost=8)
    inline SearchResult dijkstraSearch(
        const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start,
        std::pair<int, int> goal) {

        SearchResult result;

        struct Node {
            std::pair<int, int> position;
            int cost;

            bool operator>(const Node& other) const {
                return cost > other.cost;
            }
        };

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        std::map<std::pair<int, int>, int> distance;
        std::map<std::pair<int, int>, std::pair<int, int>> parent;

        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                distance[{i, j}] = INT_MAX;
            }
        }

        distance[start] = 0;
        parent[start] = { -1, -1 };
        pq.push({ start, 0 });
        result.exploredNodes.push_back(start);

        while (!pq.empty()) {
            Node current = pq.top();
            pq.pop();

            if (current.position == goal) {
                result.path = reconstructPath(parent, start, goal);
                return result;
            }

            if (current.cost > distance[current.position]) {
                continue;
            }

            for (int i = 0; i < 4; i++) {
                int newX = current.position.first + DIRECTIONS[i][0];
                int newY = current.position.second + DIRECTIONS[i][1];

                if (isValid(newX, newY)) {
                    std::pair<int, int> neighbor = { newX, newY };
                    int moveCost = getMoveCost(grid[newX][newY]);
                    int newCost = current.cost + moveCost;

                    if (newCost < distance[neighbor]) {
                        distance[neighbor] = newCost;
                        parent[neighbor] = current.position;
                        pq.push({ neighbor, newCost });

                        if (std::find(result.exploredNodes.begin(), result.exploredNodes.end(), neighbor)
                            == result.exploredNodes.end()) {
                            result.exploredNodes.push_back(neighbor);
                        }
                    }
                }
            }
        }

        return result;
    }

    // A* SEARCH - Weighted with heuristic, finds optimal path faster than Dijkstra
    // Uses Manhattan distance heuristic + considers tile costs
    inline SearchResult aStarSearch(
        const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start,
        std::pair<int, int> goal) {

        SearchResult result;

        // Heuristic: Manhattan distance (admissible and consistent)
        auto heuristic = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return abs(a.first - b.first) + abs(a.second - b.second);
            };

        struct Node {
            std::pair<int, int> position;
            int f, g, h;  // f = g + h

            bool operator>(const Node& other) const {
                return f > other.f;
            }
        };

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        std::map<std::pair<int, int>, int> gScore;
        std::map<std::pair<int, int>, std::pair<int, int>> parent;
        std::set<std::pair<int, int>> closedSet;

        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                gScore[{i, j}] = INT_MAX;
            }
        }

        gScore[start] = 0;
        int hStart = heuristic(start, goal);
        pq.push({ start, hStart, 0, hStart });
        parent[start] = { -1, -1 };
        result.exploredNodes.push_back(start);

        while (!pq.empty()) {
            Node current = pq.top();
            pq.pop();

            if (current.position == goal) {
                result.path = reconstructPath(parent, start, goal);
                return result;
            }

            if (closedSet.find(current.position) != closedSet.end()) {
                continue;
            }

            closedSet.insert(current.position);

            for (int i = 0; i < 4; i++) {
                int newX = current.position.first + DIRECTIONS[i][0];
                int newY = current.position.second + DIRECTIONS[i][1];

                if (isValid(newX, newY)) {
                    std::pair<int, int> neighbor = { newX, newY };

                    if (closedSet.find(neighbor) != closedSet.end()) {
                        continue;
                    }

                    int moveCost = getMoveCost(grid[newX][newY]);
                    int tentativeGScore = current.g + moveCost;

                    if (tentativeGScore < gScore[neighbor]) {
                        parent[neighbor] = current.position;
                        gScore[neighbor] = tentativeGScore;
                        int h = heuristic(neighbor, goal);
                        int f = tentativeGScore + h;
                        pq.push({ neighbor, f, tentativeGScore, h });

                        if (std::find(result.exploredNodes.begin(), result.exploredNodes.end(), neighbor)
                            == result.exploredNodes.end()) {
                            result.exploredNodes.push_back(neighbor);
                        }
                    }
                }
            }
        }

        return result;
    }

    // GREEDY BEST-FIRST SEARCH - Only uses heuristic (ignores actual path cost)
    // NOT optimal, but shows interesting behavior
inline SearchResult greedySearch(
    const int grid[GRID_SIZE][GRID_SIZE],
    std::pair<int, int> start,
    std::pair<int, int> goal) {

    SearchResult result;
    
    // Heuristic function
    auto heuristic = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        return abs(a.first - b.first) + abs(a.second - b.second);
    };

    struct Node {
        std::pair<int, int> position;
        int h;  // Only heuristic, NO g cost

        bool operator>(const Node& other) const {
            return h > other.h;
        }
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    std::set<std::pair<int, int>> visited;
    std::map<std::pair<int, int>, std::pair<int, int>> parent;

    pq.push({ start, heuristic(start, goal) });
    parent[start] = { -1, -1 };

    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();

        if (visited.find(current.position) != visited.end()) {
            continue;
        }

        visited.insert(current.position);
        result.exploredNodes.push_back(current.position);

        if (current.position == goal) {
            result.path = reconstructPath(parent, start, goal);
            return result;
        }

        for (int i = 0; i < 4; i++) {
            int newX = current.position.first + DIRECTIONS[i][0];
            int newY = current.position.second + DIRECTIONS[i][1];

            if (isValid(newX, newY)) {
                std::pair<int, int> neighbor = { newX, newY };

                if (visited.find(neighbor) == visited.end()) {
                    parent[neighbor] = current.position;
                    pq.push({ neighbor, heuristic(neighbor, goal) });
                }
            }
        }
    }

    return result;  // No path found
}

    inline SearchResult mdpSearch(
        const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start,
        std::pair<int, int> goal,
        int currentGold = 0) {

        SearchResult result;

       
        auto mdpResult = DungeonMDP::solveMDP(grid, start, goal, currentGold);
        result.path = mdpResult.path;
        result.exploredNodes = mdpResult.exploredNodes;

        std::cout << "MDP Expected Value: " << mdpResult.expectedValue << std::endl;
        std::cout << "Solution Found: " << (mdpResult.solutionFound ? "Yes" : "No") << std::endl;
       

        return result;
    }

    inline void printPath(const std::vector<std::pair<int, int>>& path) {
        std::cout << "Path (" << path.size() << " steps): ";
        for (const auto& p : path) {
            std::cout << "(" << p.first << "," << p.second << ") ";
        }
        std::cout << std::endl;
    }
}