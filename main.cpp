#include "GLAD/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <array>
#include <ctime>
#include <cstdlib>

// Window size
const int WINDOW_WIDTH = 300;
const int WINDOW_HEIGHT = 600;

// Board dimensions (10 columns x 20 rows)
const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;
const int BLOCK_SIZE = 30; // pixel size of one block

// Tetromino shapes (4x4 grid)
const std::array<std::array<std::array<int, 4>, 4>, 7> tetrominoes = { {// I
                                                                       {{{0, 0, 0, 0},
                                                                         {1, 1, 1, 1},
                                                                         {0, 0, 0, 0},
                                                                         {0, 0, 0, 0}}},
                                                                         // J
                                                                         {{{1, 0, 0, 0},
                                                                           {1, 1, 1, 0},
                                                                           {0, 0, 0, 0},
                                                                           {0, 0, 0, 0}}},
                                                                           // L
                                                                           {{{0, 0, 1, 0},
                                                                             {1, 1, 1, 0},
                                                                             {0, 0, 0, 0},
                                                                             {0, 0, 0, 0}}},
                                                                             // O
                                                                             {{{1, 1, 0, 0},
                                                                               {1, 1, 0, 0},
                                                                               {0, 0, 0, 0},
                                                                               {0, 0, 0, 0}}},
                                                                               // S
                                                                               {{{0, 1, 1, 0},
                                                                                 {1, 1, 0, 0},
                                                                                 {0, 0, 0, 0},
                                                                                 {0, 0, 0, 0}}},
                                                                                 // T
                                                                                 {{{0, 1, 0, 0},
                                                                                   {1, 1, 1, 0},
                                                                                   {0, 0, 0, 0},
                                                                                   {0, 0, 0, 0}}},
                                                                                   // Z
                                                                                   {{{1, 1, 0, 0},
                                                                                     {0, 1, 1, 0},
                                                                                     {0, 0, 0, 0},
                                                                                     {0, 0, 0, 0}}}} };

// Board array: 0 for empty, others for color index
int board[BOARD_HEIGHT][BOARD_WIDTH] = { 0 };

// Current piece data
struct Piece
{
    int x, y;     // position on the board
    int type;     // 0-6
    int rotation; // 0-3
};

Piece currentPiece;

// Time control
double lastFallTime = 0;
double fallDelay = 0.5; // seconds per automatic fall

GLFWwindow* window;

// Rotate piece 90 degrees clockwise
std::array<std::array<int, 4>, 4> rotatePiece(const std::array<std::array<int, 4>, 4>& shape, int rotation)
{
    std::array<std::array<int, 4>, 4> result = {};
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            switch (rotation % 4)
            {
            case 0:
                result[y][x] = shape[y][x];
                break;
            case 1:
                result[y][x] = shape[3 - x][y];
                break;
            case 2:
                result[y][x] = shape[3 - y][3 - x];
                break;
            case 3:
                result[y][x] = shape[x][3 - y];
                break;
            }
        }
    }
    return result;
}

// Check collision of current piece with board or boundaries
bool checkCollision(const Piece& piece)
{
    auto shape = rotatePiece(tetrominoes[piece.type], piece.rotation);
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (shape[y][x] == 0)
                continue;
            int boardX = piece.x + x;
            int boardY = piece.y + y;
            if (boardX < 0 || boardX >= BOARD_WIDTH || boardY < 0 || boardY >= BOARD_HEIGHT)
                return true;
            if (board[boardY][boardX] != 0)
                return true;
        }
    }
    return false;
}

// Lock piece into the board
void lockPiece(const Piece& piece)
{
    auto shape = rotatePiece(tetrominoes[piece.type], piece.rotation);
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (shape[y][x] != 0)
            {
                int boardX = piece.x + x;
                int boardY = piece.y + y;
                if (boardY >= 0 && boardY < BOARD_HEIGHT && boardX >= 0 && boardX < BOARD_WIDTH)
                    board[boardY][boardX] = piece.type + 1;
            }
        }
    }
}

// Clear full lines and move above lines down
int clearLines()
{
    int linesCleared = 0;
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        bool lineFull = true;
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            if (board[y][x] == 0)
            {
                lineFull = false;
                break;
            }
        }
        if (lineFull)
        {
            linesCleared++;
            for (int ty = y; ty < BOARD_HEIGHT - 1; ty++)
            {
                for (int x = 0; x < BOARD_WIDTH; x++)
                {
                    board[ty][x] = board[ty + 1][x];
                }
            }
            // Clear top row
            for (int x = 0; x < BOARD_WIDTH; x++)
            {
                board[BOARD_HEIGHT - 1][x] = 0;
            }
            y--; // recheck same row because rows moved down
        }
    }
    return linesCleared;
}

// Create new piece at top center
void newPiece()
{
    currentPiece.type = rand() % 7;
    currentPiece.rotation = 0;
    currentPiece.x = BOARD_WIDTH / 2 - 2;
    currentPiece.y = BOARD_HEIGHT - 4;
    if (checkCollision(currentPiece))
    {
        std::cout << "Game Over\n";
        glfwSetWindowShouldClose(window, true);
    }
}

// Move piece left or right
void movePiece(int dx)
{
    Piece moved = currentPiece;
    moved.x += dx;
    if (!checkCollision(moved))
        currentPiece = moved;
}

// Rotate piece
void rotatePieceCW()
{
    Piece rotated = currentPiece;
    rotated.rotation = (rotated.rotation + 1) % 4;
    if (!checkCollision(rotated))
        currentPiece = rotated;
}

// Soft drop piece (move down)
bool softDrop()
{
    Piece dropped = currentPiece;
    dropped.y -= 1;
    if (!checkCollision(dropped))
    {
        currentPiece = dropped;
        return true;
    }
    return false;
}

// Draw a single block at board coordinate with color
void drawBlock(int x, int y, int color)
{
    float fx = x * BLOCK_SIZE;
    float fy = y * BLOCK_SIZE;

    float r = 1, g = 1, b = 1;
    switch (color)
    {
    case 1:
        r = 0.0f;
        g = 1.0f;
        b = 1.0f;
        break; // Cyan (I)
    case 2:
        r = 0.0f;
        g = 0.0f;
        b = 1.0f;
        break; // Blue (J)
    case 3:
        r = 1.0f;
        g = 0.65f;
        b = 0.0f;
        break; // Orange (L)
    case 4:
        r = 1.0f;
        g = 1.0f;
        b = 0.0f;
        break; // Yellow (O)
    case 5:
        r = 0.0f;
        g = 1.0f;
        b = 0.0f;
        break; // Green (S)
    case 6:
        r = 0.5f;
        g = 0.0f;
        b = 0.5f;
        break; // Purple (T)
    case 7:
        r = 1.0f;
        g = 0.0f;
        b = 0.0f;
        break; // Red (Z)
    }

    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(fx, fy);
    glVertex2f(fx + BLOCK_SIZE, fy);
    glVertex2f(fx + BLOCK_SIZE, fy + BLOCK_SIZE);
    glVertex2f(fx, fy + BLOCK_SIZE);
    glEnd();

    // Optional: Draw a darker border
    glColor3f(r * 0.5f, g * 0.5f, b * 0.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(fx, fy);
    glVertex2f(fx + BLOCK_SIZE, fy);
    glVertex2f(fx + BLOCK_SIZE, fy + BLOCK_SIZE);
    glVertex2f(fx, fy + BLOCK_SIZE);
    glEnd();
}

// Render board and current piece
void renderGame()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw board blocks
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            if (board[y][x] != 0)
            {
                drawBlock(x, y, board[y][x]);
            }
        }
    }

    // Draw current piece
    auto shape = rotatePiece(tetrominoes[currentPiece.type], currentPiece.rotation);
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (shape[y][x])
            {
                drawBlock(currentPiece.x + x, currentPiece.y + y, currentPiece.type + 1);
            }
        }
    }
}

// Keyboard input callback
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    switch (key)
    {
    case GLFW_KEY_LEFT:
        movePiece(-1);
        break;
    case GLFW_KEY_RIGHT:
        movePiece(1);
        break;
    case GLFW_KEY_DOWN:
        softDrop();
        break;
    case GLFW_KEY_UP:
        rotatePieceCW();
        break;
    case GLFW_KEY_SPACE:
        // Hard drop
        while (softDrop())
            ;
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, true);
        break;
    }
}

int main()
{
    srand((unsigned int)time(nullptr));

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tetris", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glfwSetKeyCallback(window, keyCallback);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    newPiece();

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();

        if (currentTime - lastFallTime > fallDelay)
        {
            if (!softDrop())
            {
                lockPiece(currentPiece);
                clearLines();
                newPiece();
            }
            lastFallTime = currentTime;
        }

        renderGame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}