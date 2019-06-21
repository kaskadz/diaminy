#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <queue>
#include <stack>
#include <deque>
#include <unordered_set>
#include <bits/unordered_set.h>

bool DebugMode;

enum Entity {
    SHIP = '.',
    WALL = '#',
    HOLE = 'O',
    MINE = '*',
    DIAX = '+',
    VOID = ' '
};

enum Direction : int {
    N = 0,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    NW
};

class Position {
public:
    int x;
    int y;

    Position(int x, int y) : x(x), y(y) {}

    Position move(Direction direction) {
        switch (direction) {
            case N:
                return {x, y + 1};
            case NE:
                return {x + 1, y + 1};
            case E:
                return {x + 1, y};
            case SE:
                return {x + 1, y - 1};
            case S:
                return {x, y - 1};
            case SW:
                return {x - 1, y - 1};
            case W:
                return {x - 1, y};
            case NW:
                return {x - 1, y + 1};
        }
    }

    bool operator==(const Position &rhs) const {
        return x == rhs.x &&
               y == rhs.y;
    }

    bool operator!=(const Position &rhs) const {
        return !(rhs == *this);
    }
};

namespace std {
    template<>
    struct hash<Position> {
        size_t operator()(const Position &pt) const {
            return ((hash<int>()(pt.x) ^ (hash<int>()(pt.y) << 1)) >> 1);
        }
    };
}

struct MoveData {
    Position finalPosition;
    std::unordered_set<Position> *diamondsGathered;
};

class Map {
private:
    char **map;

    Map(int height, int width, int maxMoves, char **map, Position shipPosition, int targetScore)
            : height(height), width(width), maxMoves(maxMoves), map(map), shipInitialPosition(shipPosition),
              targetScore(targetScore) {
    }

    void set(Position position, Entity e) {
        set(position.x, position.y, e);
    }

    void set(int x, int y, Entity e) {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            throw "Index out of bounds";
        }

        map[y][x] = e;
    }

public:
    int const height;
    int const width;
    int const maxMoves;
    int const targetScore;
    const Position shipInitialPosition;

    ~Map() {
        for (int i = 0; i < height; ++i) {
            delete[] map[i];
        }
        delete[] map;
    };

    const char at(int x, int y) {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            throw "Index out of bounds";
        }

        return map[y][x];
    }

    const char at(Position position) {
        return at(position.x, position.y);
    }

    void print() {
        printf("h: %d w: %d\n", height, width);
        printf("m: %d t: %d\n", maxMoves, targetScore);
        printf("s: (%d, %d)\n", shipInitialPosition.x, shipInitialPosition.y);

        for (int i = height - 1; i >= 0; --i) {
            for (int j = 0; j < width; ++j) {
                std::cout << at(j, i);
            }
            std::cout << std::endl;
        }
    }

    MoveData move(Position initial, Direction direction) {
        Position currentPosition = initial;
        auto *diamondsGathered = new std::unordered_set<Position>;
        while (true) {
            Position nextPosition = currentPosition.move(direction);
            switch (at(nextPosition)) {
                case DIAX:
                    diamondsGathered->insert(nextPosition);
                case VOID:
                case SHIP:
                    currentPosition = nextPosition;
                    break;
                case HOLE:
                    return {nextPosition, diamondsGathered};
                case WALL:
                    return {currentPosition, diamondsGathered};
                case MINE:
                    return {initial, diamondsGathered};
            }
        }
    }

    MoveData move(Position initial, int direction) {
        if (direction < 0 || direction >= 8) {
            throw "Enum out of range";
        }

        return move(initial, (Direction) direction);
    }

    int abs_position(Position position) {
        return width * position.y + position.x;
    }

    int abs_positions() {
        return width * height;
    }

    Position rel_position(int pos) {
        return {pos % width, pos / width};
    }

    std::unordered_set<Position> *get_diamonds() {
        auto diamonds = new std::unordered_set<Position>;
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (at(j, i) == DIAX) {
                    diamonds->insert(Position(i, j));
                }
            }
        }
        return diamonds;
    }

    static Map *CreateFromInputStream(std::istream &stream);
};

Map *Map::CreateFromInputStream(std::istream &stream) {
    int height, width, maxMoves;
    char **map;

    stream >> height >> width >> maxMoves;
    map = new char *[height];

    Position shipPosition = Position(-1, -1);
    int targetScore = 0;

    stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string line;
    stream >> std::noskipws;
    for (int i = height - 1; i >= 0; --i) {
        map[i] = new char[width];
        for (int j = 0; j < width; ++j) {
            stream >> map[i][j];
            if (map[i][j] == DIAX) {
                targetScore++;
            } else if (map[i][j] == SHIP) {
                map[i][j] = VOID;
                shipPosition = Position(j, i);
            }
        }
        stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    if (shipPosition == Position(-1, -1)) {
        throw "Ship not found";
    }

    return new Map(height, width, maxMoves, map, shipPosition, targetScore);
}

Map *ReadMapFromFile(char *filename) {
    std::ifstream inputFile;

    inputFile.open(filename);
    if (inputFile.is_open()) {
        Map *map = Map::CreateFromInputStream(inputFile);

        inputFile.close();

        return map;
    } else {
        std::cerr << "Unable to open file" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        return nullptr;
    }
}

Map *ReadMapFromStdin() {
    return Map::CreateFromInputStream(std::cin);
}

class Edge {
public:
    std::unordered_set<Position> *diamonds;
    int direction;
    Position from;
    Position to;

    Edge(std::unordered_set<Position> *diamonds, int direction, Position from, Position to)
            : diamonds(diamonds), direction(direction), from(from), to(to) {}

    ~Edge() {
        delete diamonds;
    }

    bool is_reverse(Edge *e) const {
        return this->to == e->from && this->from == e->to;
    }

    bool operator==(const Edge &rhs) const {
        return from == rhs.from &&
               to == rhs.to;
    }

    bool operator!=(const Edge &rhs) const {
        return !(rhs == *this);
    }
};

namespace std {
    template<>
    struct hash<Edge> {
        size_t operator()(const Edge &e) const {
            return ((hash<Position>()(e.from) ^ (hash<Position>()(e.to) << 1)) >> 1);
        }
    };
}

class Graph {
private:
    Map *map;
public:
    std::vector<Edge *> *neighbours;
    int const size;
    std::unordered_set<Position> *diamonds;

private:
    explicit Graph(std::vector<Edge *> *vertex, int size, Map *map, std::unordered_set<Position> *diamonds)
            : neighbours(vertex), size(size), map(map), diamonds(diamonds) {
    }

public:

    static Graph *Generate(Map *map) {
        auto neighbours = new std::vector<Edge *>[map->abs_positions()];
        auto diamonds = map->get_diamonds();

        std::queue<Position> positions;
        positions.push(map->shipInitialPosition);
        while (!positions.empty()) {
            Position currentPosition = positions.front();
            positions.pop();
            if (neighbours[map->abs_position(currentPosition)].empty()) {
                for (int d = 0; d < 8; ++d) {
                    MoveData md = map->move(currentPosition, d);
                    if (md.finalPosition != currentPosition) {
                        Edge *e = new Edge(md.diamondsGathered, d, currentPosition, md.finalPosition);
                        neighbours[map->abs_position(currentPosition)].push_back(e);

                        if (neighbours[map->abs_position(md.finalPosition)].empty()) {
                            positions.push(md.finalPosition);
                        }
                    }
                }
            }
        }

        return new Graph(neighbours, map->abs_positions(), map, diamonds);
    }

    ~Graph() {
        delete neighbours;
    }

    void print() {
        for (int i = 0; i < size; ++i) {
            if (!neighbours[i].empty()) {
                Position position = map->rel_position(i);
                printf("(%d,%d): ", position.x, position.y);
                for (Edge *e : neighbours[i]) {
                    printf("{(%d,%d), %d, %d} ", e->to.x, e->to.y, e->diamonds->size(), e->direction);
                }
                printf("\n");
            }
        }
    }

    void print_visited_map() {
        for (int i = map->height - 1; i >= 0; --i) {
            for (int j = 0; j < map->width; ++j) {
                auto pos = Position(j, i);
                if (neighbours[map->abs_position(pos)].empty()) {
                    std::cout << map->at(pos);
                } else {
                    std::cout << 'X';
                }
            }
            std::cout << std::endl;
        }
    }

    void print_dot() {
        printf("digraph diaminy {\n");
        printf("\trankdir=TOP \n");
        printf("\tnode [style=filled, shape=circle, color=lightgreen]; \n");
        for (int i = 0; i < size; ++i) {
            if (!neighbours[i].empty()) {
                Position position = map->rel_position(i);
                for (Edge *e: neighbours[i]) {
                    printf("\t\"(%d,%d)\" -> \"(%d,%d)\" [label=%d];\n",
                           position.x, position.y, e->to.x, e->to.y, e->diamonds->size());
                }
            }
        }
        printf("\t\"(%d,%d)\" [color=gold]\n", map->shipInitialPosition.x, map->shipInitialPosition.y);
        printf("}\n");
    }

    void print_dot_path(std::vector<Edge *> *edges) {
        printf("digraph diaminy {\n");
        printf("\trankdir=TOP \n");
        printf("\tnode [style=filled, shape=circle, color=lightgreen]; \n");
        for (int i = 0; i < size; ++i) {
            if (!neighbours[i].empty()) {
                Position position = map->rel_position(i);
                for (Edge *e: neighbours[i]) {
                    bool is_path = std::any_of(edges->begin(), edges->end(), [e](Edge *x) { return *x == *e; });
                    printf("\t\"(%d,%d)\" -> \"(%d,%d)\" [label=%d%s];\n",
                           position.x, position.y, e->to.x, e->to.y, e->diamonds->size(),
                           is_path ? ", style=bold, color=tomato" : "");
                }
            }
        }
        for (Edge *e : *edges) {
            printf("\t\"(%d,%d)\" [color=lightskyblue]\n", e->to.x, e->to.y);
        }
        printf("\t\"(%d,%d)\" [color=gold]\n", map->shipInitialPosition.x, map->shipInitialPosition.y);
        printf("}\n");
    }

    std::vector<Edge *>
    *traversal1_1(Position v, std::vector<Edge *> *edges_visited, std::unordered_set<Position> *diamonds_gathered,
                  int max_diamonds, int max_leaps) {
        if (diamonds_gathered->size() > max_diamonds) throw "Too much diamonds";
        if (edges_visited->size() > max_leaps) throw "Too much leaps";

        if (diamonds_gathered->size() == max_diamonds) {
            delete diamonds_gathered;
            return edges_visited;
        }

        if (edges_visited->size() == max_leaps) {
            delete edges_visited;
            delete diamonds_gathered;
            return new std::vector<Edge *>();
        }

        for (Edge *e : neighbours[map->abs_position(v)]) {
            if (std::all_of(edges_visited->begin(), edges_visited->end(), [e](Edge *x) { return *x != *e; })) {
                auto new_edges_visited = new std::vector<Edge *>(*edges_visited);
                new_edges_visited->push_back(e);

                auto new_diamonds_gathered = new std::unordered_set<Position>(*diamonds_gathered);
                for (Position diax : *(e->diamonds)) {
                    new_diamonds_gathered->insert(diax);
                }

                auto result = traversal1_1(e->to, new_edges_visited, new_diamonds_gathered, max_diamonds, max_leaps);

                if (result->empty()) {
                    delete result;
                } else {
                    delete diamonds_gathered;
                    delete edges_visited;
                    return result;
                }
            }
        }

        delete diamonds_gathered;
        delete edges_visited;
        return new std::vector<Edge *>();
    }

    void traversal1(int maxLeaps) {
        auto result = traversal1_1(map->shipInitialPosition, new std::vector<Edge *>(),
                                   new std::unordered_set<Position>(),
                                   diamonds->size(), maxLeaps);
        if (result->empty()) {
            printf("BRAK\n");
            delete result;
        } else {
            for (const Edge *e : *result) {
                printf("%d", e->direction);
            }
            if (DebugMode) {
                printf("\n");
                this->print_dot_path(result);
            }
            delete result;
        }
    }
};

int main(int argc, char *argv[]) {
    try {
        DebugMode = argc >= 2;
        Map *map = (argc >= 2) ? ReadMapFromFile(argv[1]) : ReadMapFromStdin();
        if (DebugMode) map->print();

        Graph *graph = Graph::Generate(map);
        if (DebugMode) graph->print_dot();

        graph->traversal1(map->maxMoves);

        delete map;
    } catch (const char *e) {
        printf("[ERROR]: %s\n", e);
    }

    return 0;
}