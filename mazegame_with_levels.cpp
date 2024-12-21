#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <stack>
#include <algorithm>

using namespace std;

// Terminal settings for immediate input detection
void disableCanonicalMode() {
    termios settings;
    tcgetattr(STDIN_FILENO, &settings);
    settings.c_lflag &= ~ICANON; // Disable canonical mode
    settings.c_lflag &= ~ECHO;   // Disable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &settings);
}

void enableCanonicalMode() {
    termios settings;
    tcgetattr(STDIN_FILENO, &settings);
    settings.c_lflag |= ICANON; // Enable canonical mode
    settings.c_lflag |= ECHO;   // Enable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &settings);
}

// Global variables for player position, maze, and enemies
int playerX, playerY;               // Player position
vector<string> maze;                // Maze grid
vector<pair<int, int>> enemies;     // Enemy positions

// Directions for moving in the maze (up, down, left, right)
const vector<pair<int, int>> MOVES = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}}; // 1-step moves

// Function to initialise an empty maze
vector<string> initializeMaze(int width, int height) {
    vector<string> maze(height, string(width, '|')); // Walls everywhere
    for (int y = 1; y < height; y += 2) {
        for (int x = 1; x < width; x += 2) {
            maze[y][x] = ' '; // Open spaces at odd coordinates
        }
    }
    return maze;
}

// Function to generate a solvable maze using recursive backtracking
void generateMaze(vector<string>& maze, int width, int height) {
    vector<vector<bool>> visited(height, vector<bool>(width, false));
    stack<pair<int, int>> pathStack;

    // Start at (1, 1)
    pathStack.push({1, 1});
    visited[1][1] = true;

    while (!pathStack.empty()) {
        auto [x, y] = pathStack.top();
        vector<pair<int, int>> neighbors;

        // Find unvisited neighbors
        for (auto [dx, dy] : MOVES) {
            int nx = x + dx * 2;
            int ny = y + dy * 2;
            if (nx > 0 && ny > 0 && nx < width - 1 && ny < height - 1 && !visited[ny][nx]) {
                neighbors.push_back({nx, ny});
            }
        }

        if (!neighbors.empty()) {
            // Pick a random neighbor
            auto [nx, ny] = neighbors[rand() % neighbors.size()];

            // Remove the wall between the current cell and the neighbor
            maze[(y + ny) / 2][(x + nx) / 2] = ' ';

            // Mark the neighbor as visited and move to it
            visited[ny][nx] = true;
            pathStack.push({nx, ny});
        } else {
            // Backtrack
            pathStack.pop();
        }
    }

    // Set start and exit points
    maze[1][1] = ' ';                    // Start position
    maze[height - 2][width - 2] = 'X';   // Exit position
}

// Function to display the maze
void displayMaze(int level) {
    system("clear");
    cout << "Level: " << level << " - Use WASD to move. Press 'f' to attack. Avoid enemies 'E'! Reach 'X' to win. Press 'q' to quit.\n";
    for (int y = 0; y < maze.size(); y++) {
        for (int x = 0; x < maze[y].size(); x++) {
            if (x == playerX && y == playerY) {
                cout << 'P'; // Player position
            } else if (find(enemies.begin(), enemies.end(), make_pair(x, y)) != enemies.end()) {
                cout << 'E'; // Enemy position
            } else {
                cout << maze[y][x];
            }
        }
        cout << endl;
    }
}

// Function to handle player movement
bool movePlayer(char input) {
    int newX = playerX;
    int newY = playerY;

    switch (input) {
        case 'w': newY--; break; // Move up
        case 'a': newX--; break; // Move left
        case 's': newY++; break; // Move down
        case 'd': newX++; break; // Move right
        default: return false;   // Ignore other inputs
    }

    // Check for walls and boundaries
    if (maze[newY][newX] != '|') {
        playerX = newX;
        playerY = newY;
    }

    return false;
}

// Function to attack enemies
void attackEnemy() {
    for (auto [dx, dy] : MOVES) {
        int nx = playerX + dx;
        int ny = playerY + dy;
        auto it = find(enemies.begin(), enemies.end(), make_pair(nx, ny));
        if (it != enemies.end()) {
            enemies.erase(it); // Remove the enemy
            cout << "Enemy defeated!\n";
            return;
        }
    }
    cout << "No enemy in range to attack!\n";
}

// Function to move enemies randomly
bool moveEnemies() {
    for (auto& [ex, ey] : enemies) {
        vector<pair<int, int>> validMoves;

        // Check for valid moves
        for (auto [dx, dy] : MOVES) {
            int nx = ex + dx;
            int ny = ey + dy;
            if (maze[ny][nx] != '|' && !(nx == playerX && ny == playerY)) {
                validMoves.push_back({nx, ny});
            }
        }

        // Move to a random valid position
        if (!validMoves.empty()) {
            auto [newX, newY] = validMoves[rand() % validMoves.size()];
            ex = newX;
            ey = newY;
        }

        // Check if enemy collides with the player
        if (ex == playerX && ey == playerY) {
            return true; // Game over
        }
    }

    return false;
}

int main() {
    disableCanonicalMode();
    srand(time(0)); // Seed for random maze generation

    int level = 1;
    bool gameRunning = true;

    while (gameRunning) {
        // Generate a new maze
        int width = 15 + level * 2;
        int height = 7 + level;
        maze = initializeMaze(width, height);
        generateMaze(maze, width, height);

        // Initialise player and enemies
        playerX = 1;
        playerY = 1;
        enemies.clear();
        for (int i = 0; i < level; i++) {
            int ex, ey;
            do {
                ex = rand() % width;
                ey = rand() % height;
            } while (maze[ey][ex] != ' ' || (ex == playerX && ey == playerY));
            enemies.push_back({ex, ey});
        }

        while (true) {
            displayMaze(level);

            // Get player input
            char input = getchar();

            // Quit the game
            if (input == 'q') {
                gameRunning = false;
                break;
            }

            // Attack
            if (input == 'f') {
                attackEnemy();
                continue;
            }

            // Move the player and check for level completion
            movePlayer(input);
            if (maze[playerY][playerX] == 'X') {
                cout << "Level " << level << " completed! Loading next level..." << endl;
                sleep(2);
                level++;
                break; // Exit the current level loop and generate a new maze
            }

            // Move enemies and check for collision with player
            if (moveEnemies()) {
                cout << "You were caught by an enemy! Game Over." << endl;
                sleep(2);
                gameRunning = false;
                break;
            }
        }
    }

    enableCanonicalMode();
    cout << "Thanks for playing!" << endl;
    return 0;
}
