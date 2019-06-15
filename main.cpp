#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <algorithm>

class Map {
private:
    char **map;

    Map(int height, int width, int maxMoves, char **map)
            : height(height), width(width), maxMoves(maxMoves), map(map) {
    }

public:
    int const height;
    int const width;
    int const maxMoves;

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

    void print() {
        for (int i = height - 1; i >= 0; --i) {
            for (int j = 0; j < width; ++j) {
                std::cout << at(j, i);
            }
            std::cout << std::endl;
        }
    }

    static Map *CreateFromInputStream(std::istream &stream);
};

Map *Map::CreateFromInputStream(std::istream &stream) {
    int height, width, maxMoves;
    char **map;

    stream >> height >> width >> maxMoves;
    map = new char *[height];

    stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string line;
    stream >> std::noskipws;
    for (int i = height - 1; i >= 0; --i) {
        map[i] = new char[width];
        for (int j = 0; j < width; ++j) {
            stream >> map[i][j];
        }
        stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return new Map(height, width, maxMoves, map);
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
    } catch (const char *e) {
        printf("[ERROR]: %s\n", e);
    }

    return 0;
}