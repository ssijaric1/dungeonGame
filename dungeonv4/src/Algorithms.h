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

namespace DungeonAlgorithms {
    
    // ==================== CONSTANTS ====================
    const int GRID_SIZE = 10;
    const int DIRECTIONS[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    
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
    
    inline int getMoveCost(int cellType) {
        switch(cellType) {
            case 2: return 1;   // Reward
            case 3: return 5;   // Bandit
            case 4: return 10;  // Mine
            default: return 1;  // Empty, Player, Exit
        }
    }
    
    // ==================== ALGORITHM IMPLEMENTATIONS ====================
    inline std::vector<std::pair<int, int>> bfsSearch(
        const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start,
        std::pair<int, int> goal) {
        
        std::queue<std::pair<int, int>> q;
        std::set<std::pair<int, int>> visited;
        std::map<std::pair<int, int>, std::pair<int, int>> parent;
        
        q.push(start);
        visited.insert(start);
        parent[start] = {-1, -1};
        
        while (!q.empty()) {
            auto current = q.front();
            q.pop();
            
            if (current == goal) {
                return reconstructPath(parent, start, goal);
            }
            
            for (int i = 0; i < 4; i++) {
                int newX = current.first + DIRECTIONS[i][0];
                int newY = current.second + DIRECTIONS[i][1];
                
                if (isValid(newX, newY)) {
                    std::pair<int, int> neighbor = {newX, newY};
                    
                    if (visited.find(neighbor) == visited.end()) {
                        visited.insert(neighbor);
                        parent[neighbor] = current;
                        q.push(neighbor);
                    }
                }
            }
        }
        
        return std::vector<std::pair<int, int>>();
    }
    
    inline std::vector<std::pair<int, int>> dfsSearch(
        const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start,
        std::pair<int, int> goal) {
        
        std::stack<std::pair<int, int>> s;
        std::set<std::pair<int, int>> visited;
        std::map<std::pair<int, int>, std::pair<int, int>> parent;
        
        s.push(start);
        visited.insert(start);
        parent[start] = {-1, -1};
        
        while (!s.empty()) {
            auto current = s.top();
            s.pop();
            
            if (current == goal) {
                return reconstructPath(parent, start, goal);
            }
            
            for (int i = 3; i >= 0; i--) {
                int newX = current.first + DIRECTIONS[i][0];
                int newY = current.second + DIRECTIONS[i][1];
                
                if (isValid(newX, newY)) {
                    std::pair<int, int> neighbor = {newX, newY};
                    
                    if (visited.find(neighbor) == visited.end()) {
                        visited.insert(neighbor);
                        parent[neighbor] = current;
                        s.push(neighbor);
                    }
                }
            }
        }
        
        return std::vector<std::pair<int, int>>();
    }
    
    inline std::vector<std::pair<int, int>> dijkstraSearch(
        const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start,
        std::pair<int, int> goal) {
        
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
        parent[start] = {-1, -1};
        pq.push({start, 0});
        
        while (!pq.empty()) {
            Node current = pq.top();
            pq.pop();
            
            if (current.position == goal) {
                return reconstructPath(parent, start, goal);
            }
            
            if (current.cost > distance[current.position]) {
                continue;
            }
            
            for (int i = 0; i < 4; i++) {
                int newX = current.position.first + DIRECTIONS[i][0];
                int newY = current.position.second + DIRECTIONS[i][1];
                
                if (isValid(newX, newY)) {
                    std::pair<int, int> neighbor = {newX, newY};
                    int moveCost = getMoveCost(grid[newX][newY]);
                    int newCost = current.cost + moveCost;
                    
                    if (newCost < distance[neighbor]) {
                        distance[neighbor] = newCost;
                        parent[neighbor] = current.position;
                        pq.push({neighbor, newCost});
                    }
                }
            }
        }
        
        return std::vector<std::pair<int, int>>();
    }
    
    inline std::vector<std::pair<int, int>> aStarSearch(
        const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start,
        std::pair<int, int> goal) {
        
        auto heuristic = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return abs(a.first - b.first) + abs(a.second - b.second);
        };
        
        struct Node {
            std::pair<int, int> position;
            int f, g, h;
            
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
        pq.push({start, hStart, 0, hStart});
        parent[start] = {-1, -1};
        
        while (!pq.empty()) {
            Node current = pq.top();
            pq.pop();
            
            if (current.position == goal) {
                return reconstructPath(parent, start, goal);
            }
            
            if (closedSet.find(current.position) != closedSet.end()) {
                continue;
            }
            
            closedSet.insert(current.position);
            
            for (int i = 0; i < 4; i++) {
                int newX = current.position.first + DIRECTIONS[i][0];
                int newY = current.position.second + DIRECTIONS[i][1];
                
                if (isValid(newX, newY)) {
                    std::pair<int, int> neighbor = {newX, newY};
                    
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
                        pq.push({neighbor, f, tentativeGScore, h});
                    }
                }
            }
        }
        
        return std::vector<std::pair<int, int>>();
    }
    
    inline std::vector<std::pair<int, int>> greedySearch(
        const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start,
        std::pair<int, int> goal) {
        
        std::vector<std::pair<int, int>> path;
        std::pair<int, int> current = start;
        std::set<std::pair<int, int>> visited;
        
        path.push_back(start);
        visited.insert(start);
        
        while (current != goal) {
            if (visited.size() > GRID_SIZE * GRID_SIZE) {
                break;
            }
            
            int bestX = current.first;
            int bestY = current.second;
            int bestDist = abs(current.first - goal.first) + abs(current.second - goal.second);
            
            for (int i = 0; i < 4; i++) {
                int newX = current.first + DIRECTIONS[i][0];
                int newY = current.second + DIRECTIONS[i][1];
                
                if (isValid(newX, newY)) {
                    std::pair<int, int> neighbor = {newX, newY};
                    
                    if (visited.find(neighbor) == visited.end()) {
                        int dist = abs(newX - goal.first) + abs(newY - goal.second);
                        if (dist < bestDist) {
                            bestDist = dist;
                            bestX = newX;
                            bestY = newY;
                        }
                    }
                }
            }
            
            if (bestX == current.first && bestY == current.second) {
                break;
            }
            
            current = {bestX, bestY};
            path.push_back(current);
            visited.insert(current);
        }
        
        return path;
    }
    
    inline void printPath(const std::vector<std::pair<int, int>>& path) {
        std::cout << "Path (" << path.size() << " steps): ";
        for (const auto& p : path) {
            std::cout << "(" << p.first << "," << p.second << ") ";
        }
        std::cout << std::endl;
    }
}