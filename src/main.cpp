#include <SFML/Graphics.hpp>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <algorithm>

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
  TT_FLOOR,
  TT_STAIRCASE_UP,
  TT_STAIRCASE_DOWN
};

const sf::Color staircaseDownColor = sf::Color(231,20,129);
const sf::Color staircaseUpColor   = sf::Color(19,144,146);

typedef std::vector<std::vector<TILE_TYPE>> TileMap2D;
typedef std::vector<TileMap2D> TileMap3D;

class Level {
private:
  TileMap3D tileMap3D;
public:
  bool loadFromFile(const std::string& baseFilename, uint32 levelCount)
  {
    tileMap3D.resize(levelCount);

    TileMap2D& tileMap2D = tileMap3D[0];

    for(uint32 i = 0; i < levelCount; i++)
    {
      std::string filename = baseFilename + std::to_string(i+1) + ".png";
      tileMap3D[i] = loadFromFile2D(filename);
      if(tileMap3D[i].size() == 0) {
	std::cout << "Level: " << filename << " couldn't be loaded !\n";
	return false;
      }
    }

    if(tileMap2D.size() > 0) return true;
    else return false;
  }

  TileMap2D loadFromFile2D(const std::string& filename)
  {
    TileMap2D tileMap2D;
    sf::Image image;
    if(image.loadFromFile(filename))
    {
      sf::Vector2u size = image.getSize();
      tileMap2D.resize(size.y);
      for(uint32 y = 0; y < size.y; y++)
      {
	tileMap2D[y].resize(size.x);
	for(uint32 x = 0; x < size.x; x++)
	{
	  TILE_TYPE tt = TT_VOID;
	  sf::Color pixelColor = image.getPixel(x, y);
	  if      (pixelColor == sf::Color::White)   tt = TT_FLOOR;
	  else if (pixelColor == sf::Color::Black)   tt = TT_WALL;
	  else if (pixelColor == staircaseDownColor) tt = TT_STAIRCASE_DOWN;
	  else if (pixelColor == staircaseUpColor)   tt = TT_STAIRCASE_UP;

	  tileMap2D[y][x] = tt;
	}
      }
    }
    return tileMap2D;
  }

  void render(sf::RenderWindow& renderWindow, sf::Vector3f cameraPosition) {
    const real32 tileSize = 32.0f;
    // uint16 cameraZ = (uint16)cameraPosition.z;
    // uint32 startZ = std::min((uint32)tileMap3D.size()-1, (uint32)cameraZ);

    int32 startZ = std::max((int32)tileMap3D.size()-1, (int32)0);

    for(int32 z = startZ; z >= 0; --z)
    {
      const TileMap2D& tileMap2D = tileMap3D[z];
      uint32 mapHeight = (uint32)tileMap2D.size();
      if(mapHeight > 0)
      {
	uint32 mapWidth = (uint32)tileMap2D[0].size();
	for(uint32 y = 0; y < mapHeight; y++)
	  for(uint32 x = 0; x < mapWidth; x++)
	  {
	    TILE_TYPE tileType = tileMap2D[y][x];
	    if(tileType == TT_VOID) continue;

	    sf::Vector2f position((x - cameraPosition.x) * tileSize, (y - cameraPosition.y) * tileSize);
	    sf::RectangleShape rs(sf::Vector2f(tileSize, tileSize));
	    rs.setPosition(position);
	    switch(tileType)
	    {
	    case TT_WALL :
	      rs.setFillColor(sf::Color::Black);
	      break;
	    case TT_FLOOR :
	      rs.setFillColor(sf::Color::White);
	      break;
	    case TT_STAIRCASE_UP :
	      rs.setFillColor(staircaseUpColor);
	      break;
	    case TT_STAIRCASE_DOWN :
	      rs.setFillColor(staircaseDownColor);
	      break;
	    }
	    renderWindow.draw(rs);

	  }
      } else {std::cout << "Map is not properly loaded height is equal to 0\n"; }
    }
  }
};

int main()
{
  sf::RenderWindow window(sf::VideoMode(1280, 720), "Zhale");
  window.setVerticalSyncEnabled(true);
  window.setPosition(sf::Vector2i(0,0));

  Input input;
  Level level;
  if(!level.loadFromFile("../maps/test", 3))
  {
    std::cout << "Level couldn't be loaded \n";
  };

  sf::Vector3f cameraPosition;
  real32 movementSpeed = 1.0f;

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
	input.keysDown   [event.key.code] = true;
	break;
      case sf::Event::KeyReleased :
	input.keysReleased[event.key.code] = true;
	input.keysDown    [event.key.code] = false;
	break;
      }
    }

    if(input.keysPressed[sf::Keyboard::Q]) window.close();

    if(input.keysDown[sf::Keyboard::W]) cameraPosition.y -= movementSpeed;
    if(input.keysDown[sf::Keyboard::S]) cameraPosition.y += movementSpeed;

    if(input.keysDown[sf::Keyboard::A]) cameraPosition.x -= movementSpeed;
    if(input.keysDown[sf::Keyboard::D]) cameraPosition.x += movementSpeed;

    window.clear(sf::Color::Yellow);
    level.render(window, cameraPosition);
    window.display();
  }

  return 0;
}
