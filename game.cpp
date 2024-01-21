
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

class Vec2 {
public:
  double x = 0;
  double y = 0;

  Vec2();
  Vec2(double x, double y) : x(x), y(y) {}

  bool operator==(const Vec2 &other) {
    if (x == other.x && y == other.y)
      return true;
    return false;
  }
  bool operator!=(const Vec2 &other) {
    if (x == other.x && y == other.y)
      return false;
    return true;
  }
  Vec2 operator + (const Vec2 &other) const{
    return Vec2(x + other.x, y + other.y);
  }
  Vec2 operator-(const Vec2 &other) const {
    return Vec2(x - other.x, y - other.y);
  }
  Vec2 operator*(const Vec2 &other) const{
    return Vec2(x * other.x, y * other.y); 
  } 
  Vec2 operator/(const Vec2 &other) {
    return Vec2(x / other.x, y / other.y);
  }

  void operator+=(const Vec2 &other) {
    x += other.x;
    y += other.y;
  }

  void operator-=(const Vec2 &other) {
    x -= other.x;
    y -= other.y;
  }
  void operator*=(const Vec2 &other) {
    x *= other.x;
    y *= other.y;
  }
  void operator/=(const Vec2 &other) {
    x /= other.x;
    y /= other.y;
  }

  float dist(const Vec2 &other) {
    return sqrt((other.x - x)*(other.x - x) + (other.y - y)* (other.y - y));
  }

  void normalize() {
    double l = length();
    x /= l;
    y /= l;
  }

  double length() {
    return sqrt((x * x )+ (y * y));
  }
};




class CCollision {
public:
    float radius = 0.0;
    CCollision(float r): radius(r) {
    }
};

class CTransform {
public:
    Vec2 pos = {0.0, 0.0};
    Vec2 speed = {0.0, 0.0};
    float angle = 0.0;

    CTransform(const Vec2& p, const Vec2& s, const double a): pos(p), speed(s), angle(a) {
    }
};

class CScore {
public:
    int score = 0;
    CScore(int s): score(s) {
    }
};

class CLifespan {
public:
    int remaining = 0;
    int total = 0;
    CLifespan(int total): total(total), remaining(total) {
    }
};

class CShape {
public:
    sf::CircleShape circle;

    CShape(float radius, int points, const sf::Color & fill, const sf::Color & outline, float thickness): circle(radius, points) {
        circle.setFillColor(fill);
        circle.setOutlineColor(outline);
        circle.setOutlineThickness(thickness);
        circle.setOrigin(radius, radius);
    }
};

class CInput {
public:
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool shoot = false;

    CInput() {}
};

class Entity {
    
    friend class EntityManager;

    std::string m_tag = "default";
    bool m_active = true;
    size_t m_id = 0;

    Entity(const size_t id, const std::string& tag): m_id(id), m_tag(tag) {
    }
public:
    std::shared_ptr<CTransform> cTransform;
    std::shared_ptr<CCollision> cCollision;
    std::shared_ptr<CShape> cShape;
    std::shared_ptr<CInput> cInput;
    std::shared_ptr<CScore> cScore;
    std::shared_ptr<CLifespan> cLifespan;

    void destroy() {
        m_active = false;
    }

    bool isActive() {
        return m_active;
    }

    std::string& tag() {
        return m_tag;
    }

    size_t id() {
        return m_id;
    }
};

class EntityManager {
    std::vector<std::shared_ptr<Entity>> m_entities;
    std::map<std::string, std::vector<std::shared_ptr<Entity>>> m_entityMap;
    std::vector<std::shared_ptr<Entity>> m_toAdd;
    int m_totalEntities = 0;
    void init() {
    }
public:
    void update() {

        for(auto& entity : m_toAdd) {
            m_entities.push_back(entity);
            m_entityMap[entity->tag()].push_back(entity);
        }
        m_toAdd.clear();

        for(auto& entity : m_entities) {
            if(entity->cTransform && entity->cShape) {
                entity->cShape->circle.setPosition(entity->cTransform->pos.x, entity->cTransform->pos.y);
                entity->cTransform->angle += entity->cTransform->speed.length();
                entity->cShape->circle.setRotation(entity->cTransform->angle);
            }
        }
        
        removeDeadEntities(m_entities);

        for(auto& [tag, entityVec]: m_entityMap) {

            removeDeadEntities(entityVec);
        }

    }

    std::shared_ptr<Entity> addEntity(const std::string & tag) {
       auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag)); 

       m_toAdd.push_back(entity);

       return entity;
    }
    void removeDeadEntities(std::vector<std::shared_ptr<Entity>> & vec){
            
        vec.erase(std::remove_if(vec.begin(), vec.end(), [](std::shared_ptr<Entity> entity) {
            return !entity->isActive();
        }), vec.end());

    }


    std::vector<std::shared_ptr<Entity>> getEntities(){
        return m_entities;
    }

    std::vector<std::shared_ptr<Entity>> getEntities(std::string group) {
        std::vector<std::shared_ptr<Entity>> entities;
        for(auto& entity : m_entities) {
            if(entity->tag() == group) {
                entities.push_back(entity);
            }
        }
        return entities;
    }
};

struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, v; float S; };
struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
struct BulletConfig { int SR, CR, FR, FG ,FB, OR ,OG, OB, OT, V, L; float S; };

class Game {
    sf::RenderWindow m_window;
    EntityManager m_entities;
    sf::Font m_font;
    sf::Text m_text;
    PlayerConfig m_playerConfig;
    EnemyConfig m_enemyConfig;
    BulletConfig m_bulletConfig;
    
    int m_cooldown = 0;
    int m_score = 0;
    int m_currentFrame = 0;
    int m_lastEnemySpawnTime = 0;
    bool m_paused = false;
    bool m_running = true;

    std::shared_ptr<Entity> m_player;

    void init(const std::string & path){
        // TODO : read in config file here 
        //        use the premade PlayerConfig, EnemyConfig, BulletConfig structs

        if(!m_font.loadFromFile("./Arial.ttf")) {
            std::cout << "Error loading font" << std::endl;
        }

        m_text.setFont(m_font);
        m_text.setCharacterSize(24);
        m_text.setFillColor(sf::Color::White);
        m_text.setPosition(10, 10);

        // set up default window parameters
        m_window.create(sf::VideoMode(1280, 720), "Geometry Wars");
        m_window.setFramerateLimit(60);

       spawnPlayer();
    }

    void setPaused(bool paused) {
        m_paused = paused;
    }

    void sMovement() {
       for(auto& entity : m_entities.getEntities()) {
           if(entity->cTransform && entity->cShape && entity != m_player) {
               entity->cTransform->pos += entity->cTransform->speed;
               entity->cShape->circle.setPosition(entity->cTransform->pos.x, entity->cTransform->pos.y);
           }
       } 
       if(m_player->cInput->up) {
           m_player->cTransform->pos.y -= m_player->cTransform->speed.y;
       }
       if(m_player->cInput->down) {
           m_player->cTransform->pos.y += m_player->cTransform->speed.y;
       }
       if(m_player->cInput->left) {
           m_player->cTransform->pos.x -= m_player->cTransform->speed.x;
       }
       if(m_player->cInput->right) {
           m_player->cTransform->pos.x += m_player->cTransform->speed.x;
       }
    }
    void sUserInput() {

        sf::Event event;
        while(m_window.pollEvent(event)) {

            if(event.type == sf::Event::Closed) {
                m_running = false;
            }

            if(event.type == sf::Event::KeyPressed) {

                switch(event.key.code) {
                    case sf::Keyboard::Escape:
                        if(m_paused) {
                            setPaused(false);
                        }
                        else {
                            setPaused(true);
                        }
                        break;
                    case sf::Keyboard::W:
                        m_player->cInput->up = true;
                        break;
                    case sf::Keyboard::A:
                        m_player->cInput->left = true;
                        break;
                    case sf::Keyboard::S:
                        m_player->cInput->down = true;
                        break;
                    case sf::Keyboard::D:
                        m_player->cInput->right = true;
                        break;
                    default:
                        break;
                }
            }
            if(event.type == sf::Event::KeyReleased) {

                switch(event.key.code) {
                    case sf::Keyboard::W:
                        m_player->cInput->up = false;
                        break;
                    case sf::Keyboard::A:
                        m_player->cInput->left = false;
                        break;
                    case sf::Keyboard::S:
                        m_player->cInput->down = false;
                        break;
                    case sf::Keyboard::D:
                        m_player->cInput->right = false;
                        break;
                    default:
                        break;
                }
            }

            if(event.type == sf::Event::MouseButtonPressed) {

                if(event.mouseButton.button == sf::Mouse::Left) {
                   spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
                }
                if(event.mouseButton.button == sf::Mouse::Right) {

                    if(m_cooldown == 0) {
                        spawnSpecialWeapon(m_player, Vec2(event.mouseButton.x, event.mouseButton.y)); 
                        m_cooldown = 180;
                    }
                }

            }
        }
    }
    void sLifespan() {
        for(auto& entity : m_entities.getEntities()) {
            if(entity->cLifespan) {
                entity->cLifespan->remaining--;
                if(entity->cLifespan->remaining <= 0) {
                    entity->destroy();
                }
            }
        }
    }
    void sRender() {

        m_window.clear();
        
        for(auto& entity : m_entities.getEntities()) {
            if(entity->cShape) {
                m_window.draw(entity->cShape->circle);
            }
        }

        m_text.setString("Score: " + std::to_string(m_score));
        m_window.draw(m_text);

        m_window.display();
    }
    void sEnemySpawner() {
        int gametick = 60;
        if(m_score > 5000) {
            gametick = 45;
        }
        if(m_score > 10000) {
            gametick = 30;
        }
        if(m_score > 15000) {
            gametick = 15;
        }
        if(m_score > 20000) {
            gametick = 10;
        }


       
        if(m_currentFrame - m_lastEnemySpawnTime > gametick){
            spawnEnemy();
        }
    }
    void sCollision() {
        
        //Bullet Enemy Collision
        for(auto& b : m_entities.getEntities("bullet")) {
            for(auto& e : m_entities.getEntities("enemy")) {
                if(b->cCollision && e->cCollision) {
                    if(b->cTransform->pos.dist(e->cTransform->pos) < b->cCollision->radius + e->cCollision->radius) {

                        m_score += e->cScore->score;

                        spawnSmallEnemies(e);
                        b->destroy();
                        e->destroy();
                    }
                }
            }
        }

        //Player Enemy Collision
        for(auto& e : m_entities.getEntities("enemy")) {
            if(e->cCollision && m_player->cCollision) {
                if(e->cTransform->pos.dist(m_player->cTransform->pos) < e->cCollision->radius + m_player->cCollision->radius) {
                    m_player->cTransform->pos = Vec2(m_window.getSize().x/2.0f, m_window.getSize().y/2.0f);
                    if(m_score > 500) {
                        m_score -= 500;
                    }
                    else {
                        m_score = 0;
                    }
                }
            }
            //Enemy Wall Collision
            if(e->cTransform) {
                float radius = e->cCollision->radius;
                if(e->cTransform->pos.x-radius < 0 || e->cTransform->pos.x+radius > m_window.getSize().x) {
                    e->cTransform->speed.x *= -1;
                }
                if(e->cTransform->pos.y-radius < 0 || e->cTransform->pos.y+radius > m_window.getSize().y) {
                    e->cTransform->speed.y *= -1;
                }
            }
        }

        //Player Wall Collision
        if(m_player->cTransform) {
            float radius = m_player->cCollision->radius;
            if(m_player->cTransform->pos.x-radius < 0) {
                m_player->cInput->left = false;
            }
            if(m_player->cTransform->pos.x+radius > m_window.getSize().x) {
                m_player->cInput->right = false;
            }
            if(m_player->cTransform->pos.y-radius < 0) {
                m_player->cInput->up = false;
            }
            if(m_player->cTransform->pos.y+radius > m_window.getSize().y) {
                m_player->cInput->down = false;
            }
        }

    }

    void spawnPlayer() {
        
        //TODO: Finish adding properties to player with correct values from config file

        auto entity = m_entities.addEntity("player");

        float mx = m_window.getSize().x/2.0f;
        float my = m_window.getSize().y/2.0f;

        entity->cTransform = std::make_shared<CTransform>(Vec2(mx,my), Vec2(2.0f, 2.0f), 0);

        entity->cShape = std::make_shared<CShape>(32.0f, 8, sf::Color(10,10,10), sf::Color(255,0,0), 4.0f);

        entity->cInput = std::make_shared<CInput>();

        entity->cCollision = std::make_shared<CCollision>(32.0f);

        m_player = entity;
    }

    void spawnEnemy() {

        auto entity = m_entities.addEntity("enemy");
        float radius = rand() % 15 + 25.0f;
        int rad = radius;
        int courners = rand() % 7 + 3;

        entity->cTransform = std::make_shared<CTransform>(Vec2(rand() % (m_window.getSize().x- 2 * rad) + rad, rand() % (m_window.getSize().y - 2 * rad) + rad), Vec2(rand() % 5 -2.5, rand() % 5 -2.5), rand() % 360);


        entity->cShape = std::make_shared<CShape>(radius, courners, sf::Color(rand() % 255,rand () % 255,rand() % 255), sf::Color(255,0,0), 4.0f);

        entity->cCollision = std::make_shared<CCollision>(radius);

        entity->cScore = std::make_shared<CScore>(courners * 10);

        m_lastEnemySpawnTime = m_currentFrame;
    }

    void spawnSmallEnemies(std::shared_ptr<Entity> entity) {
        int points = entity->cShape->circle.getPointCount();
        for(int i = 0; i < points; i++) {
            auto enemy = m_entities.addEntity("small_enemy");

            float angle = 360.0/points;
            angle *= (i+1);

            Vec2 speed = Vec2(cos(angle), sin(angle));
            speed.normalize();

            enemy->cTransform = std::make_shared<CTransform>(entity->cTransform->pos,speed, rand() % 360);

            enemy->cShape = std::make_shared<CShape>(16.0f, points, sf::Color(10,10,10), sf::Color(255,0,0), 4.0f);

            enemy->cCollision = std::make_shared<CCollision>(16.0f);

            enemy->cLifespan = std::make_shared<CLifespan>(30);
        }
    }

    void spawnBullet(std::shared_ptr<Entity> entity, const Vec2& mousePos) {

        auto bullet = m_entities.addEntity("bullet");

        bullet->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, Vec2(1.0f, 1.0f), 0.0f);

        bullet->cShape = std::make_shared<CShape>(16.0f, 8, sf::Color(10,10,10), sf::Color(255,0,0), 4.0f);

        bullet->cCollision = std::make_shared<CCollision>(16.0f);

        bullet->cLifespan = std::make_shared<CLifespan>(60);

        Vec2 dir = mousePos - entity->cTransform->pos;
        dir.normalize();

        bullet->cTransform->speed = dir * Vec2(10.0f, 10.0f);

    }
    void spawnSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2& mousePos) {

        auto bullet = m_entities.addEntity("bullet");

        bullet->cTransform = std::make_shared<CTransform>(mousePos, Vec2(0, 0), 0.0f);

        bullet->cShape = std::make_shared<CShape>(16.0f, 20, sf::Color(10,10,10), sf::Color(255,0,0), 4.0f);

        bullet->cCollision = std::make_shared<CCollision>(16.0f);

        bullet->cLifespan = std::make_shared<CLifespan>(12);

    }
public:
    Game(const std::string & config) {
        init(config);
    }
    void run() {
        while(m_running) {

            if(!m_paused) {
            m_entities.update();

            sLifespan();
            sEnemySpawner();
            sCollision();
            sMovement();
            }
            sUserInput();
            sRender();

            if(m_cooldown > 0) {
                m_cooldown--;
            }

            m_currentFrame++;
        }
    }

};

int main (int argc, char *argv[]) {
    
    Game game("./config.txt");
    game.run();

    return 0;
}
