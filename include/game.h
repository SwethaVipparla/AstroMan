#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <tuple>

#include "game_object.h"
#include "game_level.h"

enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

// Represents the four possible (collision) directions
enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

typedef std::tuple<bool, Direction, glm::vec2> Collision; 

class Game
{
public:
    // game state
    GameState               State;	
    bool                    Keys[1024];
    bool                    KeysProcessed[1024];
    unsigned int            Width, Height;
    std::vector<GameLevel>  Levels;
    // std::vector<PowerUp>    PowerUps;
    unsigned int            Level;
    unsigned int            Lives;
    // constructor/destructor
    Game(unsigned int width, unsigned int height);
    ~Game();
    // initialize game state (load all shaders/textures/levels)
    void Init();
    // game loop
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();
    void DoCollisions();
    // reset
    void ResetLevel();
    void ResetPlayer();
    // powerups
    void UpdatePowerUps(float dt);
};

#endif