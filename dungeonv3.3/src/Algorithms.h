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

// Grid dimensions
const int GRID_SIZE = 10;

// Directions: right, left, down, up
const int DIRECTIONS[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

// Helper function to check if position is valid and not an obstacle
static bool isValid(int x, int y, const int grid[GRID_SIZE][GRID_SIZE]) {
    return x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE;
}

// Helper function to reconstruct path from parent map
static std::vector<std::pair<int, int>> reconstructPath(
    const std::map<std::pair<int, int>, std::pair<int, int>>& parent,
    const std::pair<int, int>& start,
    const std::pair<int, int>& goal) {
    
    std::vector<std::pair<int, int>> path;
    std::pair<int, int> current = goal;
    
    // Trace back from goal to start
    while (current != start) {
        path.push_back(current);
        auto it = parent.find(current);
        if (it == parent.end()) {
            // No path found
            return std::vector<std::pair<int, int>>();
        }
        current = it->second;
    }
    
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

// Breadth-First Search (BFS)
inline std::vector<std::pair<int, int>> bfsSearch(
    const int grid[GRID_SIZE][GRID_SIZE],
    std::pair<int, int> start,
    std::pair<int, int> goal) {
    
    std::queue<std::pair<int, int>> q;
    std::set<std::pair<int, int>> visited;
    std::map<std::pair<int, int>, std::pair<int, int>> parent;
    
    q.push(start);
    visited.insert(start);
    parent[start] = {-1, -1};  // Sentinel for start
    
    while (!q.empty()) {
        auto current = q.front();
        q.pop();
        
        // If we reached the goal
        if (current == goal) {
            return reconstructPath(parent, start, goal);
        }
        
        // Explore neighbors (4 directions)
        for (int i = 0; i < 4; i++) {
            int newX = current.first + DIRECTIONS[i][0];
            int newY = current.second + DIRECTIONS[i][1];
            
            if (isValid(newX, newY, grid)) {
                // In dungeon game: 0=empty, 1=player, 2=reward, 3=bandit, 4=mine, 5=exit
                // All cells except walls (if you had them) are passable
                // For now, consider all cells as passable
                std::pair<int, int> neighbor = {newX, newY};
                
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    parent[neighbor] = current;
                    q.push(neighbor);
                }
            }
        }
    }
    
    // No path found
    return std::vector<std::pair<int, int>>();
}

// Depth-First Search (DFS)
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
        
        // If we reached the goal
        if (current == goal) {
            return reconstructPath(parent, start, goal);
        }
        
        // Explore neighbors (reverse order for more natural DFS)
        for (int i = 3; i >= 0; i--) {
            int newX = current.first + DIRECTIONS[i][0];
            int newY = current.second + DIRECTIONS[i][1];
            
            if (isValid(newX, newY, grid)) {
                std::pair<int, int> neighbor = {newX, newY};
                
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    parent[neighbor] = current;
                    s.push(neighbor);
                }
            }
        }
    }
    
    // No path found
    return std::vector<std::pair<int, int>>();
}

// Dijkstra's Algorithm (with different costs for different cell types)
inline std::vector<std::pair<int, int>> dijkstraSearch(
    const int grid[GRID_SIZE][GRID_SIZE],
    std::pair<int, int> start,
    std::pair<int, int> goal) {
    
    // Custom comparator for priority queue
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
    
    // Initialize distances to infinity
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
        
        // Early exit if we reached the goal
        if (current.position == goal) {
            return reconstructPath(parent, start, goal);
        }
        
        // If we found a better path already, skip
        if (current.cost > distance[current.position]) {
            continue;
        }
        
        // Explore neighbors
        for (int i = 0; i < 4; i++) {
            int newX = current.position.first + DIRECTIONS[i][0];
            int newY = current.position.second + DIRECTIONS[i][1];
            
            if (isValid(newX, newY, grid)) {
                std::pair<int, int> neighbor = {newX, newY};
                
                // Get cost to move to this cell
                int moveCost = 1;  // Base cost for empty cells
                
                // Different costs for different cell types
                int cellType = grid[newX][newY];
                if (cellType == 2) {       // Reward - low cost (we want to pick them up)
                    moveCost = 1;
                } else if (cellType == 3) { // Bandit - high cost (avoid if possible)
                    moveCost = 5;
                } else if (cellType == 4) { // Mine - very high cost (avoid)
                    moveCost = 10;
                } else if (cellType == 5) { // Exit - goal
                    moveCost = 1;
                } else {                    // Empty or player
                    moveCost = 1;
                }
                
                int newCost = current.cost + moveCost;
                
                if (newCost < distance[neighbor]) {
                    distance[neighbor] = newCost;
                    parent[neighbor] = current.position;
                    pq.push({neighbor, newCost});
                }
            }
        }
    }
    
    // No path found
    return std::vector<std::pair<int, int>>();
}

// A* Search Algorithm
inline std::vector<std::pair<int, int>> aStarSearch(
    const int grid[GRID_SIZE][GRID_SIZE],
    std::pair<int, int> start,
    std::pair<int, int> goal) {
    
    // Heuristic function (Manhattan distance)
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
    
    // Initialize gScore
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
        
        // If we reached the goal
        if (current.position == goal) {
            return reconstructPath(parent, start, goal);
        }
        
        // Skip if we already found a better path to this node
        if (closedSet.find(current.position) != closedSet.end()) {
            continue;
        }
        
        closedSet.insert(current.position);
        
        // Explore neighbors
        for (int i = 0; i < 4; i++) {
            int newX = current.position.first + DIRECTIONS[i][0];
            int newY = current.position.second + DIRECTIONS[i][1];
            
            if (isValid(newX, newY, grid)) {
                std::pair<int, int> neighbor = {newX, newY};
                
                // Skip if already processed
                if (closedSet.find(neighbor) != closedSet.end()) {
                    continue;
                }
                
                // Calculate tentative gScore
                int cellType = grid[newX][newY];
                int moveCost = 1;
                
                // Different costs for different cell types (similar to Dijkstra)
                if (cellType == 2) {       // Reward
                    moveCost = 1;
                } else if (cellType == 3) { // Bandit
                    moveCost = 5;
                } else if (cellType == 4) { // Mine
                    moveCost = 10;
                } else if (cellType == 5) { // Exit
                    moveCost = 1;
                } else {
                    moveCost = 1;
                }
                
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
    
    // No path found
    return std::vector<std::pair<int, int>>();
}

// Simple greedy search (always moves toward goal)
inline std::vector<std::pair<int, int>> greedySearch(
    const int grid[GRID_SIZE][GRID_SIZE],
    std::pair<int, int> start,
    std::pair<int, int> goal) {
    
    std::vector<std::pair<int, int>> path;
    std::pair<int, int> current = start;
    std::set<std::pair<int, int>> visited;
    
    path.push_back(start);
    visited.insert(start);
    
    // Try to move toward goal greedily
    while (current != goal) {
        // Check if we're stuck
        if (visited.size() > GRID_SIZE * GRID_SIZE) {
            break;
        }
        
        // Calculate which direction gets us closer to goal
        int bestX = current.first;
        int bestY = current.second;
        int bestDist = abs(current.first - goal.first) + abs(current.second - goal.second);
        
        // Try all 4 directions
        for (int i = 0; i < 4; i++) {
            int newX = current.first + DIRECTIONS[i][0];
            int newY = current.second + DIRECTIONS[i][1];
            
            if (isValid(newX, newY, grid)) {
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
        
        // If we didn't find a better move, break
        if (bestX == current.first && bestY == current.second) {
            break;
        }
        
        current = {bestX, bestY};
        path.push_back(current);
        visited.insert(current);
    }
    
    return path;
}

// Test function to print a path
inline void printPath(const std::vector<std::pair<int, int>>& path) {
    std::cout << "Path (" << path.size() << " steps): ";
    for (const auto& p : path) {
        std::cout << "(" << p.first << "," << p.second << ") ";
    }
    std::cout << std::endl;
}