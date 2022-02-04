#include "game_level.h"

#include <fstream>
#include <sstream>
#include <iostream>


void GameLevel::Load(unsigned int levelWidth, unsigned int levelHeight)
{
    // clear old data
    this->Bricks.clear();
    // load from file
    unsigned int tileCode;
    GameLevel level;
    std::string line;
    std::vector<std::vector<unsigned int>> tileData(18, std::vector<unsigned int> (18));

    for(int i = 0; i < 18; i++)
    {
        for(int j = 0; j < 18; j++)
        {
            if(i == 0 || i == 17 || j == 0 || j == 17)
                tileData[i][j] = 1;
            else 
            {
                if ((0 + rand() % 5) == 1)
                    tileData[i][j] = 1 + rand() % 5;
                else
                    tileData[i][j] = 0;
            }
        }
    }

    for(int i = 0; i < 18; i++)
    {
        for(int j = 0; j < 18; j++)
        {
            std::cout << tileData[i][j] << "  ";
        }
        std::cout << std::endl;
    }

    if (tileData.size() > 0)
        this->init(tileData, levelWidth, levelHeight);
}

void GameLevel::Draw(SpriteRenderer &renderer)
{
    for (GameObject &tile : this->Bricks)
        if (!tile.Destroyed)
            tile.Draw(renderer);
}

bool GameLevel::IsCompleted()
{
    for (GameObject &tile : this->Bricks)
        if (!tile.IsSolid && !tile.Destroyed)
            return false;
    return true;
}

void GameLevel::init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight)
{
    // calculate dimensions
    unsigned int height = tileData.size();
    unsigned int width = tileData[0].size(); // note we can index vector at [0] since this function is only called if height > 0
    float unit_width = levelWidth / static_cast<float>(width), unit_height = levelHeight / height; 

    // initialize level tiles based on tileData		
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            // check block type from level data (2D level array)
            if (tileData[y][x] == 1) // solid
            {
                glm::vec2 pos(unit_width * x, unit_height * y);
                glm::vec2 size(unit_width*2, unit_height*1.5);
                GameObject obj(pos, size, ResourceManager::GetTexture("block_solid"), glm::vec3(0.8f, 0.8f, 0.7f));
                obj.IsSolid = true;
                this->Bricks.push_back(obj);
            }
            else if (tileData[y][x] > 1)	// non-solid; now determine its color based on level data
            {
                glm::vec3 color = glm::vec3(1.0f); // original: white
                if (tileData[y][x] == 2)
                    color = glm::vec3(0.2f, 0.6f, 1.0f);
                else if (tileData[y][x] == 3)
                    color = glm::vec3(0.0f, 0.7f, 0.0f);
                else if (tileData[y][x] == 4)
                    color = glm::vec3(0.8f, 0.8f, 0.4f);
                else if (tileData[y][x] == 5)
                    color = glm::vec3(1.0f, 0.5f, 0.0f);

                glm::vec2 pos(unit_width * x, unit_height * y);
                glm::vec2 size(unit_width / 3, unit_height / 2);
                this->Bricks.push_back(GameObject(pos, size, ResourceManager::GetTexture("block"), color));
            }
        }
    }
}