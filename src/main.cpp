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

  void render(sf::RenderWindow& renderWindow, real32 tileSize, sf::Vector3f cameraPosition) {

    sf::Vector2u screenResolution  = renderWindow.getSize();
    sf::Vector2f resolutionInTiles = sf::Vector2f((real32)screenResolution.x / tileSize, (real32)screenResolution.y / tileSize);
    sf::Vector2f halfResInTiles    = sf::Vector2f(resolutionInTiles.x / 2.0f, resolutionInTiles.y / 2.0f);

    int32 startZ = std::max((int32)tileMap3D.size()-1, (int32)0);

    for(int32 z = startZ; z >= (int32)cameraPosition.z; --z)
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

	    sf::Vector2f position((x - cameraPosition.x - halfResInTiles.x) * tileSize,
				  (y - cameraPosition.y - halfResInTiles.y) * tileSize);

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

  TILE_TYPE getTile(const sf::Vector3f position) const
  {
    if(tileMap3D.size() > position.z && tileMap3D[(uint32)position.y-1].size() > position.y &&
       tileMap3D[(uint32)position.x-1].size() > position.x)
    {
      return tileMap3D[(uint32)position.z][(uint32)position.y][(uint32)position.x];
    }
    return TT_WALL;
  }

  bool doesIntersectWithSolid(const sf::FloatRect& rect, uint32 level) const
  {
    if(getTile(sf::Vector3f(rect.left, rect.top, (real32)level)) == TT_WALL ||
       getTile(sf::Vector3f(rect.left + rect.width, rect.top, (real32)level)) == TT_WALL ||
       getTile(sf::Vector3f(rect.left + rect.width, rect.top + rect.height, (real32)level)) == TT_WALL) return true;

    return false;
  }
};

class Player {
public:
  sf::Vector3f position;
  sf::Vector2f dimensions;
  const float movementSpeed = 0.5f;

  void move(const Input& input, const Level& level)
  {
    sf::Vector3f newPosition = position;;

    if(input.keysDown[sf::Keyboard::W]) newPosition.y -= movementSpeed;
    if(input.keysDown[sf::Keyboard::S]) newPosition.y += movementSpeed;

    if(input.keysDown[sf::Keyboard::A]) newPosition.x -= movementSpeed;
    if(input.keysDown[sf::Keyboard::D]) newPosition.x += movementSpeed;

    sf::FloatRect playerRect(position.x - dimensions.x/2.0f, position.x - dimensions.x/2.0f,
			     dimensions.x, dimensions.y);

    if(!level.doesIntersectWithSolid(playerRect, (uint32)position.z)) position = newPosition;
    else std::cout << "Collision \n ";
  }

  void render(sf::RenderWindow& renderWindow, real32 tileSize)
  {
    sf::RectangleShape rs(sf::Vector2f(dimensions.x * tileSize, dimensions.y * tileSize));
    rs.setPosition(sf::Vector2f(position.x * tileSize, position.y * tileSize));
    rs.setFillColor(sf::Color::Magenta);
    renderWindow.draw(rs);
  }
};

int main()
{
  sf::Vector2u resolution(1280, 720);

  sf::RenderWindow window(sf::VideoMode(resolution.x, resolution.y), "Zhale");
  window.setVerticalSyncEnabled(true);
  window.setPosition(sf::Vector2i(0,0));

  Input input;
  Level level;
  Player player;
  player.position   = sf::Vector3f(2.0f, 2.0f, 0);
  player.dimensions = sf::Vector2f(1.0f, 1.0f);
  if(!level.loadFromFile("../maps/test", 3))
  {
    std::cout << "Level couldn't be loaded \n";
  };

  // Centering the camera
  float tileSize = 32.0f;
  sf::Vector3f cameraPosition(-(real32)resolution.x / tileSize / 2.0f, - (real32)resolution.y / tileSize / 2.0f, 0);
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

    // if(input.keysDown[sf::Keyboard::W]) cameraPosition.y -= movementSpeed;
    // if(input.keysDown[sf::Keyboard::S]) cameraPosition.y += movementSpeed;

    // if(input.keysDown[sf::Keyboard::A]) cameraPosition.x -= movementSpeed;
    // if(input.keysDown[sf::Keyboard::D]) cameraPosition.x += movementSpeed;

    if(input.keysDown[sf::Keyboard::Add])      tileSize += movementSpeed / 4.0f;
    if(input.keysDown[sf::Keyboard::Subtract]) tileSize -= movementSpeed / 4.0f;

    window.clear(sf::Color::Yellow);
    level.render(window, tileSize, cameraPosition);
    player.move(input, level);
    player.render(window, tileSize);
    window.display();
  }

  return 0;
}
