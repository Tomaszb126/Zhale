#include <SFML/Graphics.hpp>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>

typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int32_t  int32;
typedef float    f32;
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

enum WALL_SIDE {
  WS_TOP,
  WS_RIGHT,
  WS_BOTTOM,
  WS_LEFT
};

struct CollisionResult {
  f32 timeT;
  sf::Vector2f collisionPoint;
  WALL_SIDE ws;
};

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

  void render(sf::RenderWindow& renderWindow, f32 tileSize, sf::Vector3f cameraPosition) {

    sf::Vector2u screenResolution  = renderWindow.getSize();
    sf::Vector2f resolutionInTiles ((f32)screenResolution.x / tileSize, (f32)screenResolution.y / tileSize);
    sf::Vector2f halfResInTiles    (resolutionInTiles.x / 2.0f, resolutionInTiles.y / 2.0f);

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
    if(tileMap3D.size() > position.z && tileMap3D[0].size() > position.y && tileMap3D[0].size() > position.x)
    {
      return tileMap3D[(uint32)position.z][(uint32)position.y][(uint32)position.x];
    }
    return TT_WALL;
  }

  bool isSolid(const sf::Vector2f& position, uint32 level) const
  {
    if(getTile({position.x, position.y, (f32)level}) == TT_WALL) return true;
    return false;
  }

  bool doesIntersectWithSolid(const sf::FloatRect& rect, uint32 level) const
  {
    if(isSolid({rect.left, rect.top}, level) ||
       isSolid({rect.left + rect.width, rect.top}, level) ||
       isSolid({rect.left + rect.width, rect.top + rect.height}, level) ||
       isSolid({rect.left, rect.top + rect.height}, level)
       ) return true;

    // sf::Vector2f center(rect.left + rect.width / 2.0f, rect.top + rect.height / 2.0f);
    return false;
  }

  static std::list<sf::Vector2i> getCollidingTiles(const sf::Vector2f& startPosition, const sf::Vector2f& deltaVector)
  {
    std::list<sf::Vector2i> result;
    sf::Vector2i startPositionI ((int)(deltaVector.x >= 0 ? startPosition.x : startPosition.x + deltaVector.x),
				 (int)(deltaVector.y >= 0 ? startPosition.y : startPosition.y + deltaVector.y));

    sf::Vector2i endPositionI ((int)(deltaVector.x >= 0 ? startPosition.x + deltaVector.x : startPosition.x),
			       (int)(deltaVector.y >= 0 ? startPosition.y + deltaVector.y : startPosition.y));

    sf::Vector2i currentPosition = startPositionI;

    while(currentPosition.y < endPositionI.y + 1)
    {
      while(currentPosition.x < endPositionI.x + 1)
      {
	result.push_back({currentPosition.x, currentPosition.y});
	currentPosition.x++;
      }
      currentPosition.y++;
    }

    return result;
  }
};

class Player {
public:
  sf::Vector3f position;
  sf::Vector2f dimensions;
  const float movementSpeed = 5.0f;

  void move(const Input& input, const Level& level, f32 lastDelta)
  {
    sf::Vector2f deltaVector;

    if(input.keysDown[sf::Keyboard::W]) deltaVector.y -= movementSpeed;
    if(input.keysDown[sf::Keyboard::S]) deltaVector.y += movementSpeed;

    if(input.keysDown[sf::Keyboard::A]) deltaVector.x -= movementSpeed;
    if(input.keysDown[sf::Keyboard::D]) deltaVector.x += movementSpeed;

    deltaVector *= lastDelta;

    sf::Vector3f newPosition (position.x + deltaVector.x, position.y + deltaVector.y, position.z);

    sf::FloatRect playerRect(position.x - dimensions.x / 2.0f, position.y - dimensions.y / 2.0f, dimensions.x, dimensions.y);

    // CollisionResult cr = level.checkCollisions(playerRect, deltaVector, (uint32)position.z);
    // std::cout << "timeT: " << cr.timeT << "\t ws: " << cr.ws << "\t x: " << cr.collisionPoint.x <<
    //   "\t y: " << cr.collisionPoint.y << std::endl;

    // if(cr.timeT == 1.0f) position = newPosition;
  }

  void render(sf::RenderWindow& renderWindow, f32 tileSize)
  {
    sf::RectangleShape rs({dimensions.x * tileSize, dimensions.y * tileSize});
    rs.setPosition({position.x * tileSize, position.y * tileSize});
    // Setting draw origin to the center of the shape
    rs.setOrigin(dimensions.x * tileSize / 2.0f, dimensions.y * tileSize / 2.0f);
    rs.setFillColor(sf::Color::Magenta);
    renderWindow.draw(rs);
  }
};

struct IntersectionResult {
  bool intersectionHappened;
  sf::Vector2f intersectionPoint;
};

IntersectionResult getPointOfIntersection(const sf::Vector2f& p0, const sf::Vector2f& p1,
					  const sf::Vector2f& p2, const sf::Vector2f& p3)
{
  IntersectionResult result = {};

  float s1_x, s1_y, s2_x, s2_y;
  s1_x = p1.x - p0.x;     s1_y = p1.y - p0.y;
  s2_x = p3.x - p2.x;     s2_y = p3.y - p2.y;

  float s,t;
  s = (-s1_y * (p0.x - p2.x) + s1_x * (p0.y - p2.y)) / (-s2_x * s1_y + s1_x * s2_y);
  t = ( s2_x * (p0.y - p2.y) - s2_y * (p0.x - p2.x)) / (-s2_x * s1_y + s1_x * s2_y);

  if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
  {
    // Collision detected
    result.intersectionPoint.x = p0.x + (t * s1_x);
    result.intersectionPoint.y = p0.y + (t * s1_y);

    result.intersectionHappened = true;
  }
  else {
    result.intersectionHappened = false;
  }

  return result;
}

int main()
{
  sf::Vector2u resolution(1280, 720);

  sf::RenderWindow window(sf::VideoMode(resolution.x, resolution.y), "Zhale");
  window.setVerticalSyncEnabled(true);
  window.setPosition({0,0});

  Input input;
  Level level;
  Player player;
  player.position   = sf::Vector3f(2.0f, 2.0f, 0);
  player.dimensions = sf::Vector2f(0.5f, 0.5f);
  if(!level.loadFromFile("../maps/test", 3))
  {
    std::cout << "Level couldn't be loaded \n";
  };

  // Centering the camera
  float tileSize = 64.0f;
  sf::Vector3f cameraPosition(-(f32)resolution.x / tileSize / 2.0f, - (f32)resolution.y / tileSize / 2.0f, 0);
  f32 movementSpeed = 1.0f;
  sf::Vector2i mousePosition;
  sf::Clock clock;

  // Test Stuff
  // ---------------
  sf::Vector2f points[] { sf::Vector2f(10,10), sf::Vector2f(150,150), sf::Vector2f(40,10), sf::Vector2f(100,150) };

  int currentPointIndex = 0;

  while (window.isOpen())
  {
    sf::Vector2f& currentPoint = points[currentPointIndex];

    f32 lastDelta = clock.getElapsedTime().asSeconds();
    clock.restart();

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
      case sf::Event::MouseMoved :
	mousePosition = sf::Mouse::getPosition(window);
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

    sf::Vector2f mousePositionInTiles(mousePosition.x / tileSize, mousePosition.y / tileSize);

    if(level.isSolid(mousePositionInTiles, 0)) window.clear(sf::Color::Yellow);
    else window.clear(sf::Color::Black);

    level.render(window, tileSize, cameraPosition);
    player.move(input, level, lastDelta);
    player.render(window, tileSize);

    // if(input.keysDown[sf::Keyboard::A]) currentPoint.x -= movementSpeed;
    // if(input.keysDown[sf::Keyboard::D]) currentPoint.x += movementSpeed;
    // if(input.keysDown[sf::Keyboard::S]) currentPoint.y += movementSpeed;
    // if(input.keysDown[sf::Keyboard::W]) currentPoint.y -= movementSpeed;

    // if(input.keysReleased[sf::Keyboard::C]) currentPointIndex = (currentPointIndex+1) % 4;

    // sf::Vertex line[] = {
    //   sf::Vertex(points[0], sf::Color::Black),
    //   sf::Vertex(points[1], sf::Color::Black),
    //   sf::Vertex(points[2], sf::Color::Red),
    //   sf::Vertex(points[3], sf::Color::Red)
    // };

    // sf::RectangleShape rectangle;
    // IntersectionResult ir = getPointOfIntersection(points[0], points[1], points[2], points[3]);

    // rectangle.setPosition(ir.intersectionPoint);
    // rectangle.setSize({10,10});
    // rectangle.setOrigin({5,5});
    // rectangle.setFillColor(sf::Color::Black);

    // window.clear(sf::Color::White);

    // window.draw(line, 2, sf::Lines);
    // window.draw(&line[2], 2, sf::Lines);
    // if(ir.intersectionHappened) window.draw(rectangle);

    window.display();
  }

  return 0;
}
