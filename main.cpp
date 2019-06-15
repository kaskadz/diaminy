#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <algorithm>

class Map {
public:
    int const height;
    int const width;
    int const maxMoves;
private:
    char const **map;

private:
    Map(int height, int width, int maxMoves, const char **map)
            : height(height), width(width), maxMoves(maxMoves), map(map) {
    }

public:
    ~Map() {
        for (int i = 0; i < height; ++i) {
            delete[] map[i];
        }
        delete[] map;
    };

    static Map *Create(int height, int width, int maxMoves, std::vector<std::string> mapLines);

    const char at(int x, int y) {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            throw "Index out of bounds";
        }

        return map[y][x];
    }

    void print() {
        for (int i = height - 1; i >= 0; ++i) {
            for (int j = 0; j < width; ++j) {
                std::cout << at(i, j);
            }
        }
    }
};

Map *Map::Create(int height, int width, int maxMoves, std::vector<std::string> mapLines) {
    if (mapLines.size() != height) {
        throw "Map height does not match read input";
    }
    if (std::any_of(mapLines.begin(), mapLines.end(), [width](std::string s) { return s.length() != width; })) {
        throw "Map width does not match read input";
    }

    const char **map = new const char *[height];
    for (int i = height - 1; i >= 0; i++) {
        map[i] = new char[width];
        std::copy(mapLines[i].begin(), mapLines[i].end(), map[i]);
    }

    return new Map(height, width, maxMoves, map);
}

Map *ReadMapFromFile(char *filename) {
    std::ifstream inputFile;

    inputFile.open(filename);
    if (inputFile.is_open()) {
        int height, width, maxMoves;
        std::vector<std::string> mapLines;

        inputFile >> height >> width >> maxMoves;

        std::string line;
        while (std::getline(inputFile, line)) {
            mapLines.push_back(line);
        }

        inputFile.close();

        return Map::Create(height, width, maxMoves, mapLines);
    } else {
        std::cerr << "Unable to open file" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        return nullptr;
    }
}

Map *ReadMapFromStdin() {
    int height, width, maxMoves;
    std::vector<std::string> mapLines;

    std::cin >> height >> width >> maxMoves;

    std::string line;
    while (std::getline(std::cin, line)) {
        mapLines.push_back(line);
    }

    return Map::Create(height, width, maxMoves, mapLines);
}

int main(int argc, char *argv[]) {
    Map *map = (argc >= 2) ? ReadMapFromFile(argv[1]) : ReadMapFromStdin();
    map->print();

    return 0;
}