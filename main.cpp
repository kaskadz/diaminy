#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <algorithm>

enum Entity {
    SHIP = '.',
    WALL = '#',
    HOLE = 'O',
    MINE = '*',
    DIAX = '+',
    VOID = ' '
};

enum Direction {
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

    Map(int height, int width, int maxMoves, char **map, Position shipPosition)
            : height(height), width(width), maxMoves(maxMoves), map(map), shipInitialPosition(shipPosition) {
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
        printf("m: %d\n", maxMoves);
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

    static Map *CreateFromInputStream(std::istream &stream);
};

Map *Map::CreateFromInputStream(std::istream &stream) {
    int height, width, maxMoves;
    char **map;

    stream >> height >> width >> maxMoves;
    map = new char *[height];

    Position shipPosition = Position(-1, -1);

    stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string line;
    stream >> std::noskipws;
    for (int i = height - 1; i >= 0; --i) {
        map[i] = new char[width];
        for (int j = 0; j < width; ++j) {
            stream >> map[i][j];
            if (map[i][j] == SHIP) {
                map[i][j] = VOID;
                shipPosition = Position(j, i);
            }
        }
        stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    if (shipPosition.x == -1 || shipPosition.y == -1) {
        throw "Ship not found";
    }

    return new Map(height, width, maxMoves, map, shipPosition);
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

int main(int argc, char *argv[]) {
    try {
        Map *map = (argc >= 2) ? ReadMapFromFile(argv[1]) : ReadMapFromStdin();
        map->print();
        MoveData moveData = map->move(map->shipInitialPosition, NE);
        printf("(%d,%d) %d", moveData.finalPosition.x, moveData.finalPosition.y, moveData.diamondsGathered->size());
        delete moveData.diamondsGathered;

        delete map;
    } catch (const char *e) {
        printf("[ERROR]: %s\n", e);
    }

    return 0;
}