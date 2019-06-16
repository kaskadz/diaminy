#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <queue>

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
};

struct MoveData {
    Position finalPosition;
    std::vector<Position> *diamondsGathered;
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
        auto *diamondsGathered = new std::vector<Position>;
        while (true) {
            Position nextPosition = currentPosition.move(direction);
            switch (at(nextPosition)) {
                case DIAX:
                    diamondsGathered->push_back(nextPosition);
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
        };
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

    if (shipPosition.x == -1 || shipPosition.y == -1) {
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

struct Edge {
    int weight;
    int direction;
    Position to;
};

class Graph {
private:
    Map *map;
public:
    std::vector<Edge> *neighbours;
    int const size;

private:
    explicit Graph(std::vector<Edge> *vertex, int size, Map *map) : neighbours(vertex), size(size), map(map) {
    }

public:
    static Graph *Generate(Map *map) {
        auto *neighbours = new std::vector<Edge>[map->abs_positions()];

        std::queue<Position> positions;
        positions.push(map->shipInitialPosition);
        while (!positions.empty()) {
            Position pos = positions.front();
            positions.pop();
            if (neighbours[map->abs_position(pos)].empty()) {
                for (int d = 0; d < 8; ++d) {
                    MoveData md = map->move(pos, d);
                    if (md.finalPosition.x != pos.x || md.finalPosition.y != pos.y) {
                        int weight = md.diamondsGathered->size();
                        neighbours[map->abs_position(pos)].push_back(Edge{weight, d, md.finalPosition});
                        if (neighbours[map->abs_position(md.finalPosition)].empty()) {
                            positions.push(md.finalPosition);
                        }
                    }
                    delete md.diamondsGathered;
                }
            }
        }

        return new Graph(neighbours, map->abs_positions(), map);
    }

    ~Graph() {
        delete neighbours;
    }

    void print() {
        for (int i = 0; i < size; ++i) {
            if (!neighbours[i].empty()) {
                Position position = map->rel_position(i);
                printf("(%d,%d): ", position.x, position.y);
                for (Edge e : neighbours[i]) {
                    printf("{(%d,%d), %d, %d} ", e.to.x, e.to.y, e.weight, e.direction);
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
                for (Edge e : neighbours[i]) {
                    printf("\t\"(%d,%d)\" -> \"(%d,%d)\" [label=%d];\n",
                           position.x, position.y, e.to.x, e.to.y, e.weight);
                }
            }
        }
        printf("\t\"(%d,%d)\" [color=gold]\n", map->shipInitialPosition.x, map->shipInitialPosition.y);
        printf("}\n");
    }
};

int main(int argc, char *argv[]) {
    try {
        Map *map = (argc >= 2) ? ReadMapFromFile(argv[1]) : ReadMapFromStdin();
        map->print();

        Graph *graph = Graph::Generate(map);
        graph->print_dot();

        delete map;
    } catch (const char *e) {
        printf("[ERROR]: %s\n", e);
    }

    return 0;
}