#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <optional>

const int   WINDOW_W  = 1200;
const int   WINDOW_H  = 600;
const int   VIEW_W    = WINDOW_W / 2;
const int   MAP_W     = WINDOW_W / 2;
const int   MAP_ROWS  = 12;
const int   MAP_COLS  = 16;
const float TILE      = 37.0f;
const float MOVE_SPD  = 3.0f;
const float ROT_SPD   = 2.0f;
const float FOV       = 1.0472f;
const int   NUM_RAYS  = VIEW_W;
const float PI        = 3.14159265f;

const std::vector<std::string> MAP = {
    "################",
    "#..............#",
    "#.###.##.####..#",
    "#.#.....#....#.#",
    "#.#.###.#.##.#.#",
    "#...#.#...#....#",
    "#.###.#.###.##.#",
    "#.#...#.#......#",
    "#.#.###.#.####.#",
    "#...#...#......#",
    "#.###...########",
    "################",
};

bool isWall(float x, float y) {
    int col = (int)x;
    int row = (int)y;
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) return true;
    return MAP[row][col] == '#';
}

struct Player {
    float x = 1.5f, y = 1.5f, angle = 0.0f;

    void update(float dt, bool fwd, bool back, bool sl, bool sr, bool rl, bool rr) {
        if (rl) angle -= ROT_SPD * dt;
        if (rr) angle += ROT_SPD * dt;
        if (angle < 0)       angle += 2.0f * PI;
        if (angle > 2.0f*PI) angle -= 2.0f * PI;

        float dx = std::cos(angle), dy = std::sin(angle);
        float sx = std::cos(angle - PI/2.0f), sy = std::sin(angle - PI/2.0f);
        float nx = x, ny = y;

        if (fwd)  { nx += dx*MOVE_SPD*dt; ny += dy*MOVE_SPD*dt; }
        if (back) { nx -= dx*MOVE_SPD*dt; ny -= dy*MOVE_SPD*dt; }
        if (sl)   { nx += sx*MOVE_SPD*dt; ny += sy*MOVE_SPD*dt; }
        if (sr)   { nx -= sx*MOVE_SPD*dt; ny -= sy*MOVE_SPD*dt; }

        if (!isWall(nx, y)) x = nx;
        if (!isWall(x, ny)) y = ny;
    }
};

struct Hit { float dist; bool ns; };

Hit castRay(const Player& p, float ang) {
    float rdx = std::cos(ang), rdy = std::sin(ang);
    int mx = (int)p.x, my = (int)p.y;
    float ddx = (rdx == 0) ? 1e30f : std::abs(1.0f/rdx);
    float ddy = (rdy == 0) ? 1e30f : std::abs(1.0f/rdy);
    int sx, sy;
    float sdx, sdy;

    if (rdx < 0) { sx = -1; sdx = (p.x - mx) * ddx; }
    else         { sx =  1; sdx = (mx + 1.0f - p.x) * ddx; }
    if (rdy < 0) { sy = -1; sdy = (p.y - my) * ddy; }
    else         { sy =  1; sdy = (my + 1.0f - p.y) * ddy; }

    bool ns = false;
    while (true) {
        if (sdx < sdy) { sdx += ddx; mx += sx; ns = false; }
        else           { sdy += ddy; my += sy; ns = true;  }
        if (isWall((float)mx, (float)my)) break;
    }
    float dist = ns ? sdy - ddy : sdx - ddx;
    return { dist, ns };
}

void draw3D(sf::RenderTarget& t, const Player& p) {
    for (int col = 0; col < NUM_RAYS; col++) {
        float ang = p.angle - FOV/2.0f + FOV*(col/(float)NUM_RAYS);
        Hit h = castRay(p, ang);

        float sh = WINDOW_H / (h.dist + 0.0001f);
        if (sh > WINDOW_H) sh = (float)WINDOW_H;
        float top = (WINDOW_H - sh) / 2.0f;
        float x = (float)(MAP_W + col);

        sf::RectangleShape ceil({ 1.0f, top });
        ceil.setPosition({ x, 0.0f });
        ceil.setFillColor(sf::Color(40, 40, 60));
        t.draw(ceil);

        uint8_t br = h.ns ? 160 : 220;
        sf::RectangleShape wall({ 1.0f, sh });
        wall.setPosition({ x, top });
        wall.setFillColor(sf::Color(br, br, br));
        t.draw(wall);

        float fh = WINDOW_H - top - sh;
        if (fh > 0) {
            sf::RectangleShape floor({ 1.0f, fh });
            floor.setPosition({ x, top + sh });
            floor.setFillColor(sf::Color(60, 40, 40));
            t.draw(floor);
        }
    }
}

void drawMap(sf::RenderTarget& t, const Player& p) {
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            sf::RectangleShape tile({ TILE-1, TILE-1 });
            tile.setPosition({ col*TILE, row*TILE });
            tile.setFillColor(MAP[row][col]=='#'
                ? sf::Color(80,80,100) : sf::Color(20,20,30));
            t.draw(tile);
        }
    }

    for (int i = 0; i < NUM_RAYS; i += NUM_RAYS/60) {
        float ang = p.angle - FOV/2.0f + FOV*(i/(float)NUM_RAYS);
        Hit h = castRay(p, ang);
        float ex = p.x + std::cos(ang)*h.dist;
        float ey = p.y + std::sin(ang)*h.dist;

        sf::Vertex line[2];
        line[0].position = { p.x*TILE, p.y*TILE };
        line[0].color    = sf::Color(255,220,0,80);
        line[1].position = { ex*TILE, ey*TILE };
        line[1].color    = sf::Color(255,220,0,80);
        t.draw(line, 2, sf::PrimitiveType::Lines);
    }

    sf::CircleShape dot(5.0f);
    dot.setFillColor(sf::Color(0,220,100));
    dot.setOrigin({ 5.0f, 5.0f });
    dot.setPosition({ p.x*TILE, p.y*TILE });
    t.draw(dot);

    sf::Vertex arrow[2];
    arrow[0].position = { p.x*TILE, p.y*TILE };
    arrow[0].color    = sf::Color(0,255,120);
    arrow[1].position = { p.x*TILE + std::cos(p.angle)*15.0f,
                          p.y*TILE + std::sin(p.angle)*15.0f };
    arrow[1].color    = sf::Color(0,255,120);
    t.draw(arrow, 2, sf::PrimitiveType::Lines);

    sf::RectangleShape div({ 2.0f, (float)WINDOW_H });
    div.setPosition({ (float)MAP_W, 0.0f });
    div.setFillColor(sf::Color(60,60,80));
    t.draw(div);
}

void drawSplash(sf::RenderTarget& t, const sf::Font& font) {
    sf::RectangleShape overlay({ (float)WINDOW_W, (float)WINDOW_H });
    overlay.setPosition({ 0.0f, 0.0f });
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    t.draw(overlay);

    float pw = 460.0f, ph = 280.0f;
    float px = (WINDOW_W - pw) / 2.0f;
    float py = (WINDOW_H - ph) / 2.0f;

    sf::RectangleShape panel({ pw, ph });
    panel.setPosition({ px, py });
    panel.setFillColor(sf::Color(20, 20, 35));
    panel.setOutlineColor(sf::Color(80, 80, 140));
    panel.setOutlineThickness(2.0f);
    t.draw(panel);

    sf::Text title(font, "RAY CASTER", 28);
    title.setFillColor(sf::Color(100, 180, 255));
    title.setStyle(sf::Text::Bold);
    auto tb = title.getLocalBounds();
    title.setOrigin({ tb.size.x / 2.0f, 0.0f });
    title.setPosition({ (float)WINDOW_W / 2.0f, py + 20.0f });
    t.draw(title);

    sf::RectangleShape line({ pw - 40.0f, 1.0f });
    line.setPosition({ px + 20.0f, py + 62.0f });
    line.setFillColor(sf::Color(80, 80, 140));
    t.draw(line);

    const std::vector<std::pair<std::string, std::string>> controls = {
        { "W / S",        "Move forward / backward" },
        { "A / D",        "Strafe left / right"     },
        { "Left / Right", "Rotate camera"           },
        { "Escape",       "Quit"                    },
    };

    float rowY = py + 80.0f;
    for (const auto& [key, desc] : controls) {
        sf::Text keyText(font, key, 18);
        keyText.setFillColor(sf::Color(255, 220, 80));
        keyText.setPosition({ px + 30.0f, rowY });
        t.draw(keyText);

        sf::Text descText(font, desc, 18);
        descText.setFillColor(sf::Color(200, 200, 200));
        descText.setPosition({ px + 200.0f, rowY });
        t.draw(descText);

        rowY += 36.0f;
    }

    sf::Text prompt(font, "Press any key to start", 16);
    prompt.setFillColor(sf::Color(120, 120, 120));
    auto pb = prompt.getLocalBounds();
    prompt.setOrigin({ pb.size.x / 2.0f, 0.0f });
    prompt.setPosition({ (float)WINDOW_W / 2.0f, py + ph - 36.0f });
    t.draw(prompt);
}

int main() {
    sf::RenderWindow window(
        sf::VideoMode({ (unsigned)WINDOW_W, (unsigned)WINDOW_H }),
        "Ray Caster",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
        if (!font.openFromFile("C:/Windows/Fonts/consola.ttf"))
            font.openFromFile("C:/Windows/Fonts/cour.ttf");

    Player player;
    sf::Clock clock;
    bool showSplash = true;

    while (window.isOpen()) {
        while (const std::optional<sf::Event> ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) window.close();
            if (const auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
                if (kp->scancode == sf::Keyboard::Scan::Escape) window.close();
                else showSplash = false;
            }
        }

        float dt = clock.restart().asSeconds();

        if (!showSplash) {
            player.update(dt,
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W),
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S),
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A),
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D),
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left),
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)
            );
        }

        window.clear(sf::Color(10,10,15));
        draw3D(window, player);
        drawMap(window, player);
        if (showSplash) drawSplash(window, font);
        window.display();
    }

    return 0;
}
