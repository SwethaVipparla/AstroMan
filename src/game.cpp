#include <algorithm>
#include <sstream>
#include <iostream>

#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(100.0f, 10.0f);
// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);
const float BALL_VELOCITY(500.0f);
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const float BALL_RADIUS = 30.0f;

SpriteRenderer  *Renderer;
GameObject        *Player;
BallObject        *Ball;

Game::Game(unsigned int width, unsigned int height)
        : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{   

}

Game::~Game()
{
    delete Renderer;
    delete Player;
}

void Game::Init()
{
    // load shaders
    ResourceManager::LoadShader("../src/shaders/sprite.vs", "../src/shaders/sprite.frag", nullptr, "sprite");
    
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").SetMatrix4("projection", projection);

    // load textures
    ResourceManager::LoadTexture("../src/textures/background.jpg", false, "face");
    ResourceManager::LoadTexture("../src/textures/obstacle.png", true, "block");
    ResourceManager::LoadTexture("../src/textures/block_solid.png", true, "block_solid");
    ResourceManager::LoadTexture("../src/textures/awesomeface.png", false, "backgound");
    ResourceManager::LoadTexture("../src/textures/player.png", true, "player");

    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    // Text = new TextRenderer(this->Width, this->Height);
    // Text->Load("fonts/OCRAEXT.TTF", 24);

    GameLevel one; 
    one.Load(this->Width, this->Height);
    this->Levels.push_back(one);
    this->Level = 0;

    glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    // Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("player"));
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, 
                                              -BALL_RADIUS * 2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
        ResourceManager::GetTexture("player"));
}

void Game::Update(float dt)
{
    Ball->Move(dt, this->Width);
    this->DoCollisions();
}

void Game::ProcessInput(float dt)
{
    if (this->State == GAME_ACTIVE)
    {
        float velocity = BALL_VELOCITY * dt;
        // move playerboard
        if (this->Keys[GLFW_KEY_A])
        {
            if (Ball->Position.x >= 0.0f)
                Ball->Position.x -= velocity;
        }
        if (this->Keys[GLFW_KEY_D])
        {
            if (Ball->Position.x <= this->Width - Ball->Size.x)
                Ball->Position.x += velocity;
        }
        if (this->Keys[GLFW_KEY_W])
        {
            if (Ball->Position.y >= 0.0f)
                Ball->Position.y -= velocity;
        }
        if (this->Keys[GLFW_KEY_S])
        {
            if (Ball->Position.y <= this->Width - Ball->Size.y)
                Ball->Position.y += velocity;
        }
    }
}

void Game::Render()
{
    
    if(this->State == GAME_ACTIVE)
    {
        // draw background
        Renderer->DrawSprite(ResourceManager::GetTexture("face"), 
            glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f
        );
        // draw level
        this->Levels[this->Level].Draw(*Renderer);

        // draw player
        Ball->Draw(*Renderer); 

    }
}

bool CheckCollision(BallObject &one, GameObject &two);
// Collision CheckCollision(BallObject &one, GameObject &two);


bool CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
    // get center point circle first 
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(
        two.Position.x + aabb_half_extents.x, 
        two.Position.y + aabb_half_extents.y
    );
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // add clamped value to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // retrieve vector between center circle and closest point AABB and check if length <= radius
    difference = closest - center;
    return glm::length(difference) < one.Radius;
}    

void Game::DoCollisions()
{
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        if (!box.Destroyed)
        {
            if (CheckCollision(*Ball, box))
            {
                if (!box.IsSolid)
                    box.Destroyed = true;
            }
        }
    }
}  
