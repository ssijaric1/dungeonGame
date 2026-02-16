#pragma once
#include <vector>
#include <utility>
#include <queue>
#include <stack>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <climits>
#include <limits>
#include <cstring> 


namespace DungeonMDP {

    constexpr int GRID_SIZE = 10;
    
    constexpr int DIRECTIONS[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

    constexpr double MINE_SUCCESS_PROBABILITY = 0.7;
    constexpr int MIN_GOLD_FOR_WIN = 20;
    constexpr int MAX_GOLD_TRACKED = 50; 
    constexpr double GAMMA = 0.99;
    constexpr double THETA = 0.0001;
    constexpr int MAX_ITERATIONS = 5000;

    enum Action { RIGHT = 0, LEFT = 1, DOWN = 2, UP = 3, NUM_ACTIONS = 4 };

    struct MDPResult {
        std::vector<std::pair<int, int>> path;
        std::vector<std::pair<int, int>> exploredNodes;
        double expectedValue;
        bool solutionFound;
    };

    class MDPSolver {
    private:
        const int(*grid)[GRID_SIZE];
        int startX, startY, startGold;
        std::pair<int, int> exitPos;

        
        double V[GRID_SIZE][GRID_SIZE][MAX_GOLD_TRACKED + 1];
        Action policy[GRID_SIZE][GRID_SIZE][MAX_GOLD_TRACKED + 1];

        inline bool isValid(int x, int y) const {
            return x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE;
        }

        inline int clampGold(int g) const {
            if (g < 0) return 0;
            if (g > MAX_GOLD_TRACKED) return MAX_GOLD_TRACKED;
            return g;
        }

        inline double getReward(int cellType, int currentGold, int newGold) const {
            // 0:Empty, 1:Player, 2:Reward, 3:Bandit, 4:Mine, 5:Exit
            if (cellType == 0) return -0.05;
            if (cellType == 1) return 0.0;
            if (cellType == 2) return (currentGold >= MAX_GOLD_TRACKED) ? -0.05 : 150.0;
            if (cellType == 3) return -50.0 - (currentGold - newGold) * 5.0;
            if (cellType == 4) return -10.0;
            return -0.1; 
        }

        void initialize() {
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int y = 0; y < GRID_SIZE; y++) {
                    for (int g = 0; g <= MAX_GOLD_TRACKED; g++) {
                        V[x][y][g] = 0.0;
                        policy[x][y][g] = RIGHT;
                    }
                }
            }

            int ex = exitPos.first;
            int ey = exitPos.second;
            for (int g = 0; g <= MAX_GOLD_TRACKED; g++) {
                if (g < MIN_GOLD_FOR_WIN) {
                    V[ex][ey][g] = -10000.0; 
                }
                else {
                    double extra = (double)(g - MIN_GOLD_FOR_WIN);
                    V[ex][ey][g] = 2000.0 + (extra * 100.0); 
                }
            }
        }

        void valueIteration() {
            int ex = exitPos.first;
            int ey = exitPos.second;

            for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
                double maxDelta = 0.0;

                for (int x = 0; x < GRID_SIZE; x++) {
                    for (int y = 0; y < GRID_SIZE; y++) {

                        
                        if (x == ex && y == ey) continue;

                        int cell = grid[x][y];

                        for (int g = 0; g <= MAX_GOLD_TRACKED; g++) {
                            double currentVal = V[x][y][g];
                            double bestValue = -std::numeric_limits<double>::infinity();
                            Action bestAction = RIGHT;

                            for (int a = 0; a < NUM_ACTIONS; a++) {
                                int nx = x + DIRECTIONS[a][0];
                                int ny = y + DIRECTIONS[a][1];
                                double actionValue = 0.0;

                                if (!isValid(nx, ny)) {
                                    actionValue = -1.0 + GAMMA * V[x][y][g];
                                }
                                else {
                                    if (cell == 4) {
                                        double valSuccess = getReward(4, g, g) + GAMMA * V[nx][ny][g];
                                        int gFail = clampGold(g - 5);
                                        double valFail = getReward(4, g, gFail) + GAMMA * V[nx][ny][gFail];

                                        actionValue = (MINE_SUCCESS_PROBABILITY * valSuccess) +
                                            ((1.0 - MINE_SUCCESS_PROBABILITY) * valFail);
                                    }
                                   
                                    else {
                                        int nextGold = g;
                                        if (cell == 2) nextGold = clampGold(g + 10);
                                        else if (cell == 3) nextGold = clampGold(g / 2);

                                        actionValue = getReward(cell, g, nextGold) + GAMMA * V[nx][ny][nextGold];
                                    }
                                }

                                if (actionValue > bestValue) {
                                    bestValue = actionValue;
                                    bestAction = static_cast<Action>(a);
                                }
                            }

                            V[x][y][g] = bestValue;
                            policy[x][y][g] = bestAction;

                            double diff = std::abs(currentVal - bestValue);
                            if (diff > maxDelta) maxDelta = diff;
                        }
                    }
                }
                if (maxDelta < THETA) break;
            }
        }

        std::vector<std::pair<int, int>> extractPath() const {
            std::vector<std::pair<int, int>> path;
            int cx = startX, cy = startY;
            int cg = clampGold(startGold);

            path.push_back({ cx, cy });

            
            for (int step = 0; step < 200; step++) {
                if (cx == exitPos.first && cy == exitPos.second) break;

                Action a = policy[cx][cy][cg];
                int nx = cx + DIRECTIONS[a][0];
                int ny = cy + DIRECTIONS[a][1];

                if (!isValid(nx, ny)) break;

                int cell = grid[nx][ny];
                if (cell == 2) cg += 10;
                else if (cell == 3) cg /= 2;
                cg = clampGold(cg);

                cx = nx; cy = ny;
                path.push_back({ cx, cy });
            }
            return path;
        }

    public:
        MDPSolver(const int gridIn[GRID_SIZE][GRID_SIZE],
            std::pair<int, int> start,
            std::pair<int, int> exit,
            int initialGold)
            : grid(gridIn), exitPos(exit), startX(start.first), startY(start.second), startGold(initialGold) {
            initialize();
        }

        MDPResult solve() {
            valueIteration();
            MDPResult result;
            result.path = extractPath();

            for (int x = 0; x < GRID_SIZE; x++) {
                for (int y = 0; y < GRID_SIZE; y++) {
                    if (std::abs(V[x][y][clampGold(startGold)]) > 0.1) {
                        result.exploredNodes.push_back({ x, y });
                    }
                }
            }
            result.expectedValue = V[startX][startY][clampGold(startGold)];
            result.solutionFound = !result.path.empty() && result.path.back() == exitPos;
            return result;
        }
    };
}


namespace DungeonAlgorithms {

    constexpr int GRID_SIZE = 10;

    struct SearchResult {
        std::vector<std::pair<int, int>> path;
        std::vector<std::pair<int, int>> exploredNodes;
    };

   
    inline bool isValid(int x, int y) {
        return x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE;
    }

    
    inline int getMoveCost(int cellType) {
        // 0:Empty, 1:Player, 2:Reward, 3:Bandit, 4:Mine, 5:Exit
        static const int costs[] = { 1, 1, 0, 15, 8, 1 };
        if (cellType >= 0 && cellType <= 5) return costs[cellType];
        return 1;
    }

    inline std::vector<std::pair<int, int>> reconstructPath(
        const std::pair<int, int> parent[GRID_SIZE][GRID_SIZE],
        const std::pair<int, int>& start,
        const std::pair<int, int>& goal) {

        std::vector<std::pair<int, int>> path;
        std::pair<int, int> current = goal;

        if (parent[goal.first][goal.second].first == -1 && current != start) return path;

        while (current != start) {
            path.push_back(current);
            current = parent[current.first][current.second];
            if (current.first == -1) break; 
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
        return path;
    }

    // BFS
    inline SearchResult bfsSearch(const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start, std::pair<int, int> goal) {

        SearchResult result;
        std::queue<std::pair<int, int>> q;
        bool visited[GRID_SIZE][GRID_SIZE] = { false };
        std::pair<int, int> parent[GRID_SIZE][GRID_SIZE];

        
        for (int i = 0; i < GRID_SIZE; ++i)
            for (int j = 0; j < GRID_SIZE; ++j) parent[i][j] = { -1, -1 };

        q.push(start);
        visited[start.first][start.second] = true;
        result.exploredNodes.push_back(start);

        int dirs[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

        while (!q.empty()) {
            auto current = q.front();
            q.pop();

            if (current == goal) {
                result.path = reconstructPath(parent, start, goal);
                return result;
            }

            for (auto& d : dirs) {
                int nx = current.first + d[0];
                int ny = current.second + d[1];

                if (isValid(nx, ny) && !visited[nx][ny]) {
                    visited[nx][ny] = true;
                    parent[nx][ny] = current;
                    result.exploredNodes.push_back({ nx, ny });
                    q.push({ nx, ny });
                }
            }
        }
        return result;
    }

    // DFS
    inline SearchResult dfsSearch(const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start, std::pair<int, int> goal) {

        SearchResult result;
        std::stack<std::pair<int, int>> s;
        bool visited[GRID_SIZE][GRID_SIZE] = { false };
        std::pair<int, int> parent[GRID_SIZE][GRID_SIZE];

        for (int i = 0; i < GRID_SIZE; ++i)
            for (int j = 0; j < GRID_SIZE; ++j) parent[i][j] = { -1, -1 };

        s.push(start);
        visited[start.first][start.second] = true;
        result.exploredNodes.push_back(start);

        int dirs[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

        while (!s.empty()) {
            auto current = s.top();
            s.pop();

            if (current == goal) {
                result.path = reconstructPath(parent, start, goal);
                return result;
            }

            
            for (int i = 3; i >= 0; i--) {
                int nx = current.first + dirs[i][0];
                int ny = current.second + dirs[i][1];

                if (isValid(nx, ny) && !visited[nx][ny]) {
                    visited[nx][ny] = true;
                    parent[nx][ny] = current;
                    result.exploredNodes.push_back({ nx, ny });
                    s.push({ nx, ny });
                }
            }
        }
        return result;
    }

    // A*
    inline SearchResult aStarSearch(const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start, std::pair<int, int> goal) {

        SearchResult result;
        struct Node {
            std::pair<int, int> pos;
            int f;
            bool operator>(const Node& other) const { return f > other.f; }
        };

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        int gScore[GRID_SIZE][GRID_SIZE];
        std::pair<int, int> parent[GRID_SIZE][GRID_SIZE];
        bool visitedVis[GRID_SIZE][GRID_SIZE] = { false }; 

        for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {
                gScore[i][j] = INT_MAX;
                parent[i][j] = { -1, -1 };
            }
        }

        auto heuristic = [&](int x, int y) {
            return std::abs(x - goal.first) + std::abs(y - goal.second);
            };

        gScore[start.first][start.second] = 0;
        pq.push({ start, heuristic(start.first, start.second) });
        result.exploredNodes.push_back(start);
        visitedVis[start.first][start.second] = true;

        int dirs[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

        while (!pq.empty()) {
            auto current = pq.top().pos;
            pq.pop();

            if (current == goal) {
                result.path = reconstructPath(parent, start, goal);
                return result;
            }

            for (auto& d : dirs) {
                int nx = current.first + d[0];
                int ny = current.second + d[1];

                if (isValid(nx, ny)) {
                    int newG = gScore[current.first][current.second] + getMoveCost(grid[nx][ny]);

                    if (newG < gScore[nx][ny]) {
                        gScore[nx][ny] = newG;
                        parent[nx][ny] = current;
                        int f = newG + heuristic(nx, ny);
                        pq.push({ {nx, ny}, f });

                        if (!visitedVis[nx][ny]) {
                            result.exploredNodes.push_back({ nx, ny });
                            visitedVis[nx][ny] = true;
                        }
                    }
                }
            }
        }
        return result;
    }

    // DIJKSTRA
    inline SearchResult dijkstraSearch(const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start, std::pair<int, int> goal) {

        
        SearchResult result;
        struct Node {
            std::pair<int, int> pos;
            int cost;
            bool operator>(const Node& other) const { return cost > other.cost; }
        };

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        int dist[GRID_SIZE][GRID_SIZE];
        std::pair<int, int> parent[GRID_SIZE][GRID_SIZE];
        bool visitedVis[GRID_SIZE][GRID_SIZE] = { false };

        for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {
                dist[i][j] = INT_MAX;
                parent[i][j] = { -1, -1 };
            }
        }

        dist[start.first][start.second] = 0;
        pq.push({ start, 0 });
        result.exploredNodes.push_back(start);
        visitedVis[start.first][start.second] = true;

        int dirs[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

        while (!pq.empty()) {
            auto current = pq.top().pos;
            int d = pq.top().cost;
            pq.pop();

            if (d > dist[current.first][current.second]) continue;
            if (current == goal) {
                result.path = reconstructPath(parent, start, goal);
                return result;
            }

            for (auto& dir : dirs) {
                int nx = current.first + dir[0];
                int ny = current.second + dir[1];

                if (isValid(nx, ny)) {
                    int newDist = dist[current.first][current.second] + getMoveCost(grid[nx][ny]);
                    if (newDist < dist[nx][ny]) {
                        dist[nx][ny] = newDist;
                        parent[nx][ny] = current;
                        pq.push({ {nx, ny}, newDist });

                        if (!visitedVis[nx][ny]) {
                            result.exploredNodes.push_back({ nx, ny });
                            visitedVis[nx][ny] = true;
                        }
                    }
                }
            }
        }
        return result;
    }

	// GREEDY BEST-FIRST SEARCH
    inline SearchResult greedySearch(const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start, std::pair<int, int> goal) {

        SearchResult result;
        auto heuristic = [&](int x, int y) { return std::abs(x - goal.first) + std::abs(y - goal.second); };

        struct Node {
            std::pair<int, int> pos;
            int h;
            bool operator>(const Node& other) const { return h > other.h; }
        };

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        bool visited[GRID_SIZE][GRID_SIZE] = { false };
        std::pair<int, int> parent[GRID_SIZE][GRID_SIZE];

        for (int i = 0; i < GRID_SIZE; ++i)
            for (int j = 0; j < GRID_SIZE; ++j) parent[i][j] = { -1, -1 };

        pq.push({ start, heuristic(start.first, start.second) });
        visited[start.first][start.second] = true;
        result.exploredNodes.push_back(start);

        int dirs[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

        while (!pq.empty()) {
            auto current = pq.top().pos;
            pq.pop();

            if (current == goal) {
                result.path = reconstructPath(parent, start, goal);
                return result;
            }

            for (auto& dir : dirs) {
                int nx = current.first + dir[0];
                int ny = current.second + dir[1];

                if (isValid(nx, ny) && !visited[nx][ny]) {
                    visited[nx][ny] = true;
                    parent[nx][ny] = current;
                    result.exploredNodes.push_back({ nx, ny });
                    pq.push({ {nx, ny}, heuristic(nx, ny) });
                }
            }
        }
        return result;
    }

    // MDP
    inline SearchResult mdpSearch(const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start, std::pair<int, int> goal, int currentGold = 0) {

        DungeonMDP::MDPSolver solver(grid, start, goal, currentGold);
        DungeonMDP::MDPResult mdpRes = solver.solve();

        SearchResult result;
        result.path = mdpRes.path;
        result.exploredNodes = mdpRes.exploredNodes;
        return result;
    }
}