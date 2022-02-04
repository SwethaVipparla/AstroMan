#include <algorithm>
#include <sstream>
#include <iostream>
#include <ft2build.h>
#include <freetype/freetype.h>

#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include "text_renderer.h"

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
BallObject *Enemy;
TextRenderer *Text;
TextRenderer *gameEnded;
TextRenderer *newLevel;

time_t start = time(0);

Game::Game(unsigned int width, unsigned int height)
        : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{   

}

Game::~Game()
{
    delete Renderer;
    delete Ball;
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
    ResourceManager::LoadTexture("../src/textures/obstacle.png", true, "obstacle");
    ResourceManager::LoadTexture("../src/textures/block_solid.jpg", false, "block_solid");
    ResourceManager::LoadTexture("../src/textures/awesomeface.png", false, "backgound");
    ResourceManager::LoadTexture("../src/textures/player.png", true, "player");
    ResourceManager::LoadTexture("../src/textures/door.jpg", true, "door");
    ResourceManager::LoadTexture("../src/textures/enemy.png", true, "enemy");

    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    // Text = new TextRenderer(this->Width, this->Height);
    // Text->Load("fonts/OCRAEXT.TTF", 24);
    int level;
    GameLevel one;
    one.Load(this->Width, this->Height, 1);
    GameLevel two; 
    two.Load(this->Width, this->Height, 2);
    GameLevel three; 
    three.Load(this->Width, this->Height, 3);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Level = 0;
    this->Score = 0;

    glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, (this->Height - PLAYER_SIZE.y)/2);
    // Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("player"));
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, 
                                              -BALL_RADIUS * 2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
        ResourceManager::GetTexture("player"));
    glm::vec2 e1Pos = glm::vec2(this->Height / 2, this->Width / 2);
    Enemy = new BallObject(e1Pos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
                            ResourceManager::GetTexture("enemy"));
    
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("../src/ocraext.TTF", 100);
}

void Game::Update(float dt)
{
    Ball->Move(dt, this->Width);
    Enemy->Move(dt, this->Width);
    this->DoCollisions();

    if(Ball->Position.y <= 0.01f)
    {
        this->Level = (this->Level + 1) % 3;
        Ball->Position.y = (this->Height - PLAYER_SIZE.y)/2;
    }
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
        std::stringstream scr, lev, tim;
        scr << this->Score;
        lev << this->Level + 1;

        Enemy->Draw(*Renderer);

        double seconds = difftime(time(0), start);
        char times[100];
        sprintf(times, "%02d:%02d:%02d", (int)(seconds / 3600), ((int)(seconds / 60)) % 60, (int)(int(seconds) % 60));
        tim << times;
        Text->RenderText("Score: " + scr.str() + "   Level: " + lev.str() + "   Elapsed Time: " + tim.str(), 1.0f, 1.0f, 0.25f);
    }
}

bool CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);


Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}    

bool CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
    // get center point circle first 
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // now retrieve vector between center circle and closest point AABB and check if length < radius
    difference = closest - center;
    
    if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0, 0));
}

void Game::DoCollisions()
{
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        if (!box.Destroyed)
        {
            if(box.IsDoor)
                continue;
            Collision collision = CheckCollision(*Ball, box);
            Collision enemy1 = CheckCollision(*Ball, *Enemy);
    
        

            if (std::get<0>(collision)) // if collision is true
            {
                 if (!box.IsSolid)
                {
                    box.Destroyed = true;
                    this->Score += 10;
                }
                else
                {   
                }
                // collision resolution
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                    if (!(!box.IsSolid)) // don't do collision resolution on non-solid bricks if pass-through activated
                    {
                        if (dir == LEFT || dir == RIGHT) // horizontal collision
                        {
                            Ball->Velocity.x = -Ball->Velocity.x; // reverse horizontal velocity
                            // relocate
                            float penetration = Ball->Radius - std::abs(diff_vector.x);
                            if (dir == LEFT)
                                Ball->Position.x += penetration; // move ball to right
                            else
                                Ball->Position.x -= penetration; // move ball to left;
                        }
                        else // vertical collision
                        {
                            Ball->Velocity.y = -Ball->Velocity.y; // reverse vertical velocity
                            // relocate
                            float penetration = Ball->Radius - std::abs(diff_vector.y);
                            if (dir == UP)
                                Ball->Position.y -= penetration; // move ball bback up
                            else
                                Ball->Position.y += penetration; // move ball back down
                        }
                    }
            }
        }    
    }
}  


