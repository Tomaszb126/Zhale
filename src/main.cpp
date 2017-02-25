#include <SFML/Graphics.hpp>
#include <stdint.h>
#include <iostream>
#include <vector>

typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int32_t  int32;
typedef float    real32;
typedef double   real64;

class Input {
public:
  static const uint16 keyCount = 256;

  bool keysDown[keyCount] = {};
  bool keysPressed[keyCount] = {};
  bool keysReleased[keyCount] = {};

  void clear() {
    memset(keysPressed , 0, sizeof(bool) * keyCount);
    memset(keysReleased, 0, sizeof(bool) * keyCount);
  }
};

enum TILE_TYPE {
  TT_VOID,
  TT_WALL,
  TT_FLOOR
};

typedef std::vector<std::vector<TILE_TYPE>> TileMap2D;
typedef std::vector<TileMap2D> TileMap3D;

class TileMap {
private:
  // 3 Dimensional TileMap
  TileMap3D tileMap;

public:
  void render(sf::RenderWindow& renderWindow, sf::Vector3f cameraPosition) {
    const real32 tileSize = 32.0f;
    uint16 levelZ = (uint16)cameraPosition.z;
    TileMap2D tileMap2D = tileMap[levelZ];

    uint32 mapHeight = (uint32)tileMap2D.size();
    if(mapHeight > 0)
    {
      uint32 mapWidth = (uint32)tileMap2D[0].size();
      for(uint32 y = 0; y < mapHeight; y++)
	for(uint32 x = 0; x < mapWidth; x++)
	{

	}
    } else {std::cout << "Map is not properly loaded height is equal 0\n"; }

  }
};

class Level {
private:
  TileMap tileMap;
public:
  bool loadFromFile(std::string filename)
  {

  }
};

int main()
{
  sf::RenderWindow window(sf::VideoMode(1280, 720), "Zhale");
  window.setVerticalSyncEnabled(true);
  window.setPosition(sf::Vector2i(0,0));
  sf::CircleShape shape(100.f);
  shape.setFillColor(sf::Color::Green);

  Input input;

  Level level;
  level.loadFromFile("maps/test.png");

  while (window.isOpen())
  {
    input.clear();
    sf::Event event;
    while (window.pollEvent(event))
    {
      switch (event.type){
      case sf::Event::Closed :
	window.close();
	break;
      case sf::Event::KeyPressed :
	input.keysPressed[event.key.code] = true;
	break;
      }
    }

    if(input.keysPressed[sf::Keyboard::Q]) window.close();

    window.clear();
    window.draw(shape);
    window.display();
  }

  return 0;
}
