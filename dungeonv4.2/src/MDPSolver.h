// MDPSolver.h - OPTIMIZED GREEDY EDITION (Array-based)
#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>
#include <iostream>
#include <set>

namespace DungeonMDP {

    const int GRID_SIZE = 10;
    const int DIRECTIONS[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

    // ==========================================
    // TUNED PARAMETERS
    // ==========================================
    const double MINE_SUCCESS_PROBABILITY = 0.7;
    const int MIN_GOLD_FOR_WIN = 20;

    // Capped at 50 to prevent state explosion, but enough to be greedy.
    // State space = 10 * 10 * 51 = 5100 states (Tiny for array access)
    const int MAX_GOLD_TRACKED = 50;

    const double GAMMA = 0.99;    // High discount for long-term greed
    const double THETA = 0.0001;  // Precise convergence
    const int MAX_ITERATIONS = 5000; // Fast enough with arrays

    enum Action {
        RIGHT = 0,
        LEFT = 1,
        DOWN = 2,
        UP = 3,
        NUM_ACTIONS = 4
    };

    struct MDPResult {
        std::vector<std::pair<int, int>> path;
        std::vector<std::pair<int, int>> exploredNodes;
        double expectedValue;
        bool solutionFound;
    };

    class MDPSolver {
    private:
        const int (*grid)[GRID_SIZE];
        int startX, startY, startGold;
        std::pair<int, int> exitPos;

        // OPTIMIZATION: Use raw arrays instead of std::map for O(1) access.
        // V[x][y][gold]
        double V[GRID_SIZE][GRID_SIZE][MAX_GOLD_TRACKED + 1];
        // Policy[x][y][gold]
        Action policy[GRID_SIZE][GRID_SIZE][MAX_GOLD_TRACKED + 1];

        bool isValid(int x, int y) const {
            return x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE;
        }

        int getCellType(int x, int y) const {
            return grid[x][y];
        }

        // ==========================================================
        // REWARD LOGIC
        // ==========================================================
        double getImmediateReward(int cellType, int currentGold, int newGold) const {
            switch (cellType) {
            case 0: // EMPTY
                return -0.05; // Low cost to encourage exploration

            case 1: // PLAYER
                return 0.0;

            case 2: // REWARD (+10)
                // If we are already maxed out, this tile is just an empty tile.
                // This prevents the agent from dancing on the tile forever.
                if (currentGold >= MAX_GOLD_TRACKED) return -0.05;
                return 150.0; // Huge incentive

            case 3: // BANDIT
                // Punishment scales with wealth
                return -50.0 - (currentGold - newGold) * 5.0;

            case 4: // MINE
                return -10.0;

            case 5: // EXIT
                if (newGold < MIN_GOLD_FOR_WIN) {
                    return -10000.0; // Death
                }
                else {
                    // Base Win + GREED BONUS
                    // 100 points per extra gold piece.
                    double extraGold = (double)(newGold - MIN_GOLD_FOR_WIN);
                    return 2000.0 + (extraGold * 100.0);
                }

            default:
                return -0.1;
            }
        }

        // Helper to clamp gold state
        int clampGold(int g) const {
            if (g < 0) return 0;
            if (g > MAX_GOLD_TRACKED) return MAX_GOLD_TRACKED;
            return g;
        }

        void initializeValueFunction() {
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int y = 0; y < GRID_SIZE; y++) {
                    for (int g = 0; g <= MAX_GOLD_TRACKED; g++) {
                        V[x][y][g] = 0.0;
                        policy[x][y][g] = RIGHT; // Default
                    }
                }
            }
        }

        void valueIteration() {
            // No BFS needed. We iterate strictly over the known bounds.
            // This avoids queue overhead.

            for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
                double maxDelta = 0.0;

                // Iterate over every possible state
                for (int x = 0; x < GRID_SIZE; x++) {
                    for (int y = 0; y < GRID_SIZE; y++) {
                        for (int g = 0; g <= MAX_GOLD_TRACKED; g++) {

                            // Check Terminal State (Exit)
                            if (x == exitPos.first && y == exitPos.second) {
                                // Calculate terminal reward exactly once based on gold held
                                double terminalReward;
                                if (g < MIN_GOLD_FOR_WIN) terminalReward = -10000.0;
                                else terminalReward = 2000.0 + (g - MIN_GOLD_FOR_WIN) * 100.0;

                                V[x][y][g] = terminalReward;
                                continue;
                            }

                            double currentVal = V[x][y][g];
                            double bestValue = -std::numeric_limits<double>::infinity();
                            Action bestAction = RIGHT;

                            // Try all 4 actions
                            for (int a = 0; a < NUM_ACTIONS; a++) {
                                int dx = DIRECTIONS[a][0];
                                int dy = DIRECTIONS[a][1];
                                int nx = x + dx;
                                int ny = y + dy;

                                double actionValue = 0.0;

                                if (!isValid(nx, ny)) {
                                    // Hit wall: stay in same state, small penalty implied by rewards? 
                                    // Usually walls just return to state s.
                                    // Reward for hitting wall? Let's say -1.0
                                    actionValue = -1.0 + GAMMA * V[x][y][g];
                                }
                                else {
                                    int cell = getCellType(nx, ny);

                                    if (cell != 4) { // Deterministic Transition
                                        int nextGold = g;
                                        if (cell == 2) nextGold = g + 10;
                                        else if (cell == 3) nextGold = g / 2;

                                        nextGold = clampGold(nextGold);

                                        double r = getImmediateReward(cell, g, nextGold);
                                        actionValue = r + GAMMA * V[nx][ny][nextGold];
                                    }
                                    else { // Mine (Stochastic)
                                        // Success
                                        int gSuccess = g;
                                        double rSuccess = getImmediateReward(cell, g, gSuccess);
                                        double valSuccess = rSuccess + GAMMA * V[nx][ny][clampGold(gSuccess)];

                                        // Fail
                                        int gFail = g - 5;
                                        double rFail = getImmediateReward(cell, g, clampGold(gFail));
                                        double valFail = rFail + GAMMA * V[nx][ny][clampGold(gFail)];

                                        actionValue = (MINE_SUCCESS_PROBABILITY * valSuccess) +
                                            ((1.0 - MINE_SUCCESS_PROBABILITY) * valFail);
                                    }
                                }

                                if (actionValue > bestValue) {
                                    bestValue = actionValue;
                                    bestAction = static_cast<Action>(a);
                                }
                            }

                            V[x][y][g] = bestValue;
                            policy[x][y][g] = bestAction;

                            double diff = std::abs(currentVal - V[x][y][g]);
                            if (diff > maxDelta) maxDelta = diff;
                        }
                    }
                }

                if (maxDelta < THETA) break;
            }
        }

        std::vector<std::pair<int, int>> extractPath() const {
            std::vector<std::pair<int, int>> path;
            int cx = startX;
            int cy = startY;
            int cg = clampGold(startGold);

            path.push_back({ cx, cy });

            // Limit steps to prevent infinite loops if policy cycles
            for (int step = 0; step < 200; step++) {
                if (cx == exitPos.first && cy == exitPos.second) break;

                Action a = policy[cx][cy][cg];
                int nx = cx + DIRECTIONS[a][0];
                int ny = cy + DIRECTIONS[a][1];

                if (!isValid(nx, ny)) break;

                // Update Gold State for the simulation
                int cell = getCellType(nx, ny);
                if (cell == 2) cg += 10;
                else if (cell == 3) cg /= 2;
                else if (cell == 4) {
                    // For path visualization, assume we succeed at mines
                    // or assume no gold change? Let's assume neutral to keep path stable.
                }

                cg = clampGold(cg);
                cx = nx;
                cy = ny;

                path.push_back({ cx, cy });
            }
            return path;
        }

    public:
        MDPSolver(const int gridIn[GRID_SIZE][GRID_SIZE],
            std::pair<int, int> start,
            std::pair<int, int> exit,
            int initialGold = 0)
            : grid(gridIn), exitPos(exit) {

            startX = start.first;
            startY = start.second;
            startGold = clampGold(initialGold);
            initializeValueFunction();
        }

        MDPResult solve() {
            MDPResult result;

            // 1. Run optimization
            valueIteration();

            // 2. Extract best path
            result.path = extractPath();

            // 3. Collect explored nodes (just for visualization)
            // We consider a node "explored" if it has a non-zero value or was visited.
            // Since we iterated everything, technically everything is explored.
            // But we can just return the path nodes + neighbors for visual clarity.
            std::set<std::pair<int, int>> distinct;
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int y = 0; y < GRID_SIZE; y++) {
                    // If value is significantly different from 0, we "thought" about it.
                    if (std::abs(V[x][y][0]) > 0.1 || std::abs(V[x][y][startGold]) > 0.1) {
                        distinct.insert({ x,y });
                    }
                }
            }
            result.exploredNodes.assign(distinct.begin(), distinct.end());

            result.expectedValue = V[startX][startY][startGold];
            result.solutionFound = !result.path.empty() &&
                result.path.back() == exitPos;

            std::cout << "MDP (Fast Array): Value=" << result.expectedValue
                << " Steps=" << result.path.size() << std::endl;

            return result;
        }
    };

    inline MDPResult solveMDP(
        const int grid[GRID_SIZE][GRID_SIZE],
        std::pair<int, int> start,
        std::pair<int, int> goal,
        int initialGold = 0) {

        MDPSolver solver(grid, start, goal, initialGold);
        return solver.solve();
    }
}