#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-exception-baseclass"

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <array>
#include <algorithm>
#include <queue>
#include <stack>
#include <deque>
#include <unordered_set>
#include <bits/unordered_set.h>
#include <map>

//region GLOBAL VARIABLES

bool DebugMode;

//endregion

//region STATS

struct stats {
public:
    std::string case_name;
    unsigned int height = 0;
    unsigned int width = 0;
    unsigned int max_leaps = 0;
    unsigned int diamonds = 0;
    unsigned int non_empty_nodes = 0;
    unsigned int edges = 0;
    unsigned int edges_visited = 0;
    unsigned int diamonds_gathered = 0;
    unsigned long long int iterations = 0;
    unsigned long long int gu_leap_limit = 0;
    unsigned long long int gu_no_path = 0;

private:
    static bool exists(const std::string &filename) {
        std::ifstream ifile(filename);
        return ifile.is_open();
    }

public:
    void save(const std::string &filename) {
        char sep = ',';
        bool needs_header_init = !exists(filename);

        std::ofstream log_file;
        log_file.open(filename, std::ios_base::out | std::ios_base::app);
        if (log_file.is_open()) {
            if (needs_header_init)
                log_file << "case_name" << sep << "height" << sep << "width" << sep << "max_leaps" << sep << "diamonds"
                         << sep << "non_empty_nodes" << sep << "edges" << sep << "edges_visited" << sep
                         << "diamonds_gathered" << sep << "iterations" << sep << "gu_leap_limit" << sep << "gu_no_path"
                         << std::endl;

            log_file << case_name << sep << height << sep << width << sep << max_leaps << sep << diamonds
                     << sep << non_empty_nodes << sep << edges << sep << edges_visited << sep
                     << diamonds_gathered << sep << iterations << sep << gu_leap_limit << sep << gu_no_path
                     << std::endl;
            log_file.close();
        } else {
            std::cerr << "Unable to open log file" << std::endl;
            std::cerr << strerror(errno) << std::endl;
        }
    }
} Stats;

//endregion

//region ENUMS

enum Entity : char {
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

//endregion

//region DATA TYPES

class Map;

class Position;

class Edge;

class Vertex;

class Graph;

struct MoveData;

//endregion

//region DATA STRUCTURES

class Position {
public:
    int x;
    int y;

    Position(int x, int y);

    Position move(Direction direction);

    int absoluteOn(Map *map) const;

    bool operator==(const Position &rhs) const;

    bool operator!=(const Position &rhs) const;
};

struct MoveData {
    Position finalPosition;
    std::unordered_set<Position> *diamondsGathered;
};

class Map {
private:
    char **map; // TODO: Linearise this array

    Map(int height, int width, int maxMoves, char **map, Position shipPosition, int targetScore);

    void set(Position position, Entity e);

    void set(int x, int y, Entity e);

public:
    int const height;
    int const width;
    int const maxMoves;
    int const targetScore;
    const Position shipInitialPosition;

    ~Map();;

    const char at(int x, int y);

    const char at(Position position);

    void print();

    MoveData move(Position initial, Direction direction);

    MoveData move(Position initial, int direction);

    std::vector<Position> *traverse(char *stringPath);

    Position relativePosition(int pos);

    std::unordered_set<Position> *getDiamonds();

    void printToOutputStream(std::ostream &stream);

    void save(const std::string &filePath);

    static Map *CreateFromInputStream(std::istream &stream);
};

class Edge {
public:
    std::unordered_set<Position> *diamonds;
    int direction;
    Position from;
    Position to;

    Edge(std::unordered_set<Position> *diamonds, int direction, Position from, Position to);

    ~Edge();

    bool isReverse(Edge *e) const;

    bool operator==(const Edge &rhs) const;

    bool operator!=(const Edge &rhs) const;
};

class Vertex {
public:
    std::map<Direction, Edge *> edges{};
    int outDeg;
    int inDeg;

    Vertex() : outDeg(0), inDeg(0) {}
};

class Graph {
private:
    Map *map;
public:
    std::unordered_set<Position> *diamonds;
    std::map<int, Vertex *> *vertices;

private:
    explicit Graph(std::map<int, Vertex *> *vertices, Map *map, std::unordered_set<Position> *diamonds);

public:

    static Graph *Generate(Map *map);

    ~Graph();

    void print();

    void printVisitedMap(std::ostream &stream = std::cout);

    void printDot(std::ostream &stream = std::cout);

    void save(const std::string &filePath);

    void printDotPath(std::vector<Edge *> *edges, std::ostream &stream = std::cout);

    std::vector<Edge *>
    *traversalSub(Position v, std::vector<Edge *> *edgesVisited, std::unordered_set<Position> *diamondsGathered,
                  int maxDiamonds, int maxLeaps);

    void traversal(int maxLeaps);
};

//endregion

//region FUNCTIONS DECLARATION

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

Map *ReadMapFromStdin();

void CheckPath(Map *map, char *pathName);

void PrintPathNumbers(std::vector<Edge *> &edges, std::ostream &stream = std::cout);

void Solve(Map *map);

//endregion

//region HASHING FUNCTIONS

namespace std {
    template<>
    struct hash<Position> {
        size_t operator()(const Position &pt) const {
            return ((hash<int>()(pt.x) ^ (hash<int>()(pt.y) << 1)) >> 1);
        }
    };

    template<>
    struct hash<Edge> {
        size_t operator()(const Edge &e) const {
            return ((hash<Position>()(e.from) ^ (hash<Position>()(e.to) << 1)) >> 1);
        }
    };
}

//endregion

//region EDGE IMPLEMENTATION

Edge::Edge(std::unordered_set<Position> *diamonds, int direction, Position from, Position to)
        : diamonds(diamonds), direction(direction), from(from), to(to) {}

Edge::~Edge() {
    delete diamonds;
}

bool Edge::isReverse(Edge *e) const {
    return this->to == e->from && this->from == e->to;
}

bool Edge::operator==(const Edge &rhs) const {
    return from == rhs.from &&
           to == rhs.to;
}

bool Edge::operator!=(const Edge &rhs) const {
    return !(rhs == *this);
}

//endregion

//region POSITION IMPLEMENTATION

Position::Position(int x, int y) : x(x), y(y) {}

Position Position::move(Direction direction) {
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

bool Position::operator==(const Position &rhs) const {
    return x == rhs.x &&
           y == rhs.y;
}

bool Position::operator!=(const Position &rhs) const {
    return !(rhs == *this);
}

int Position::absoluteOn(Map *map) const {
    return map->width * y + x;
}

//endregion

//region MAP IMPLEMENTATION

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
                map[i][j] = HOLE;
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

void Map::save(const std::string &filePath) {
    std::ofstream output_file;

    output_file.open(filePath);
    if (output_file.is_open()) {
        output_file << height << ' ' << width << std::endl;
        output_file << maxMoves << std::endl;
        printToOutputStream(output_file);
        output_file.close();
    } else {
        std::cerr << "Unable to open file" << std::endl;
        std::cerr << strerror(errno) << std::endl;
    }
}

void Map::printToOutputStream(std::ostream &stream) {
    for (int i = height - 1; i >= 0; --i) {
        for (int j = 0; j < width; ++j) {
            stream << at(j, i);
        }
        stream << std::endl;
    }
}

std::unordered_set<Position> *Map::getDiamonds() {
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

Position Map::relativePosition(int pos) {
    return {pos % width, pos / width};
}

std::vector<Position> *Map::traverse(char *stringPath) {
    auto path = new std::vector<Position>();
    Position current = this->shipInitialPosition;
    for (int i = 0; stringPath[i] != '\0'; ++i) {
        if (stringPath[i] < 48 || stringPath[i] > 57)
            throw "Wrong path";
        MoveData md = this->move(current, stringPath[i] - 48);
        delete md.diamondsGathered;
        if (md.finalPosition == current)
            return path;
        path->push_back(md.finalPosition);
        current = md.finalPosition;
    }
    return path;
}

MoveData Map::move(Position initial, int direction) {
    if (direction < 0 || direction >= 8) {
        throw "Enum out of range";
    }

    return move(initial, (Direction) direction);
}

MoveData Map::move(Position initial, Direction direction) {
    Position currentPosition = initial;
    auto *diamondsGathered = new std::unordered_set<Position>;
    while (true) {
        Position nextPosition = currentPosition.move(direction);
        switch (at(nextPosition)) {
            case DIAX:
                diamondsGathered->insert(nextPosition);
            case VOID:
                currentPosition = nextPosition;
                break;
            case SHIP:
            case HOLE:
                return {nextPosition, diamondsGathered};
            case WALL:
                return {currentPosition, diamondsGathered};
            case MINE:
                return {initial, diamondsGathered};
        }
    }
}

void Map::print() {
    printf("h: %d w: %d\n", height, width);
    printf("m: %d t: %d\n", maxMoves, targetScore);
    printf("s: (%d, %d)\n", shipInitialPosition.x, shipInitialPosition.y);

    printToOutputStream(std::cout);
}

const char Map::at(Position position) {
    return at(position.x, position.y);
}

const char Map::at(int x, int y) {
    if (x < 0 || y < 0 || x >= width || y >= height) {
        throw "Index out of bounds";
    }

    return map[y][x];
}

Map::Map(int height, int width, int maxMoves, char **map, Position shipPosition, int targetScore)
        : height(height), width(width), maxMoves(maxMoves), map(map), shipInitialPosition(shipPosition),
          targetScore(targetScore) {
}

void Map::set(Position position, Entity e) {
    set(position.x, position.y, e);
}

void Map::set(int x, int y, Entity e) {
    if (x < 0 || y < 0 || x >= width || y >= height) {
        throw "Index out of bounds";
    }

    map[y][x] = e;
}

Map::~Map() {
    for (int i = 0; i < height; ++i) {
        delete[] map[i];
    }
    delete[] map;
}

//endregion

//region GRAPH IMPLEMENTATION

void Graph::traversal(int maxLeaps) {
    auto result = traversalSub(map->shipInitialPosition, new std::vector<Edge *>(),
                               new std::unordered_set<Position>(),
                               diamonds->size(), maxLeaps);
    if (result->empty()) {
        std::cout << ("BRAK");
        delete result;
    } else {
        PrintPathNumbers(*result);
        if (DebugMode) {
            std::cout << std::endl;
            this->printDotPath(result);

            std::ofstream output_dot_file;
            output_dot_file.open("sol.dot");
            if (output_dot_file.is_open()) {
                this->printDotPath(result, output_dot_file);
                output_dot_file.close();
            } else {
                std::cerr << "Unable to open dot output file" << std::endl;
                std::cerr << strerror(errno) << std::endl;
            }

            std::ofstream output_path_file;
            output_path_file.open("sol.txt");
            if (output_path_file.is_open()) {
                PrintPathNumbers(*result, output_path_file);
                output_path_file.close();
            } else {
                std::cerr << "Unable to open path output file" << std::endl;
                std::cerr << strerror(errno) << std::endl;
            }
        }
        delete result;
    }
}

std::vector<Edge *> *
Graph::traversalSub(Position v, std::vector<Edge *> *edgesVisited, std::unordered_set<Position> *diamondsGathered,
                    int maxDiamonds, int maxLeaps) {
    if (diamondsGathered->size() > maxDiamonds) throw "Too much diamonds";
    if (edgesVisited->size() > maxLeaps) throw "Too much leaps";
    if (vertices->count(v.absoluteOn(map)) == 0) throw "Encountered a non existing vertex";

    if (DebugMode) {
        Stats.iterations++;
    }

    if (diamondsGathered->size() == maxDiamonds) {
        delete diamondsGathered;
        return edgesVisited;
    }

    if (edgesVisited->size() == maxLeaps) {
        if (DebugMode) {
            Stats.gu_leap_limit++;
        }
        delete edgesVisited;
        delete diamondsGathered;
        return new std::vector<Edge *>();
    }

    for (auto kv : vertices->at(v.absoluteOn(map))->edges) {
//        if (std::all_of(edgesVisited->begin(), edgesVisited->end(),
//                        [e = kv.second](Edge *x) { return *x != *e; })) {
        auto new_edges_visited = new std::vector<Edge *>(*edgesVisited);
        new_edges_visited->push_back(kv.second);

        auto new_diamonds_gathered = new std::unordered_set<Position>(*diamondsGathered);
        for (Position diax : *(kv.second->diamonds)) {
            new_diamonds_gathered->insert(diax);
        }

        auto result = traversalSub(kv.second->to, new_edges_visited, new_diamonds_gathered, maxDiamonds,
                                   maxLeaps);

        if (result->empty()) {
            delete result;
        } else {
            delete diamondsGathered;
            delete edgesVisited;
            return result;
        }
//        }
    }

    if (DebugMode) {
        Stats.gu_no_path++;
    }

    delete diamondsGathered;
    delete edgesVisited;
    return new std::vector<Edge *>();
}

void Graph::printDotPath(std::vector<Edge *> *edges, std::ostream &stream) {
    stream << "digraph diaminy {" << std::endl;
    stream << "\trankdir=TOP" << std::endl;
    stream << "\tnode [style=filled, shape=circle, color=lightgreen];" << std::endl;
    for (auto vkv : *vertices) {
        Position position = map->relativePosition(vkv.first);
        for (auto ekv: vkv.second->edges) {
            bool is_path = std::any_of(edges->begin(), edges->end(),
                                       [e = ekv.second](Edge *x) { return *x == *e; });
            stream << "\t\"(" << position.x << "," << position.y << ")\" -> \"("
                   << ekv.second->to.x << "," << ekv.second->to.y << ")\" [label=" << ekv.second->diamonds->size()
                   << (is_path ? ", style=bold, color=tomato" : "") << "];" << std::endl;
        }
    }
    for (Edge *e : *edges) {
        stream << "\t\"(" << e->to.x << "," << e->to.y << ")\" [color=lightskyblue]" << std::endl;
    }
    stream << "\t\"(" << map->shipInitialPosition.x << "," << map->shipInitialPosition.y << ")\" [color=gold]"
           << std::endl;
    stream << "}" << std::endl;
}

void Graph::save(const std::string &filePath) {
    std::ofstream output_file;

    output_file.open(filePath);
    if (output_file.is_open()) {
        printDot(output_file);
        output_file.close();
    } else {
        std::cerr << "Unable to open file" << std::endl;
        std::cerr << strerror(errno) << std::endl;
    }
}

void Graph::printDot(std::ostream &stream) {
    stream << "digraph diaminy {" << std::endl;
    stream << "\trankdir=TOP" << std::endl;
    stream << "\tnode [style=filled, shape=circle, color=lightgreen];" << std::endl;
    for (auto vkv : *vertices) {
        Position position = map->relativePosition(vkv.first);
        for (auto ekv: vkv.second->edges) {
            stream << "\t\"(" << position.x << "," << position.y << ")\" -> \"(" << ekv.second->to.x << ","
                   << ekv.second->to.y << ")\" [label=" << ekv.second->diamonds->size() << "];" << std::endl;
        }
    }
    stream << "\t\"(" << map->shipInitialPosition.x << "," << map->shipInitialPosition.y << ")\" [color=gold]"
           << std::endl;
    stream << "}" << std::endl;
}

void Graph::printVisitedMap(std::ostream &stream) {
    for (int i = map->height - 1; i >= 0; --i) {
        for (int j = 0; j < map->width; ++j) {
            auto pos = Position(j, i);
            if (vertices->count(pos.absoluteOn(map)) == 0) {
                stream << map->at(pos);
            } else {
                stream << 'X';
            }
        }
        stream << std::endl;
    }
}

void Graph::print() {
    for (auto vkv : *vertices) {
        Position position = map->relativePosition(vkv.first);
        printf("(%d,%d): ", position.x, position.y);
        for (auto ekv : vkv.second->edges) {
            printf("{(%d,%d), %d, %d} ", ekv.second->to.x, ekv.second->to.y, ekv.second->diamonds->size(),
                   ekv.second->direction);
        }
        printf("\n");
    }
}

Graph *Graph::Generate(Map *map) { // TODO: Move generation to Graph's constructor
    auto vertices = new std::map<int, Vertex *>();
    auto diamonds = map->getDiamonds();

    std::queue<Position> positions;
    positions.push(map->shipInitialPosition);
    vertices->insert(std::pair<int, Vertex *>(map->shipInitialPosition.absoluteOn(map), new Vertex()));
    while (!positions.empty()) {
        Position currentPosition = positions.front();
        positions.pop();
        int current_abs_position = currentPosition.absoluteOn(map);

        Vertex *&currentVertex = vertices->at(current_abs_position);
        if (currentVertex == nullptr) {
            throw "Some vertex was not initialized!";
        }
        for (int d = 0; d < 8; ++d) {
            MoveData md = map->move(currentPosition, d);
            if (md.finalPosition != currentPosition) {
                Edge *e = new Edge(md.diamondsGathered, d, currentPosition, md.finalPosition);
                currentVertex->edges.insert(std::pair<Direction, Edge *>((Direction) d, e));
                currentVertex->outDeg++;

                int final_abs_position = md.finalPosition.absoluteOn(map);
                if (vertices->count(final_abs_position) == 0) {
                    vertices->insert(std::pair<int, Vertex *>(final_abs_position, new Vertex()));
                    positions.push(md.finalPosition);
                }
                vertices->at(final_abs_position)->inDeg++;
            }
        }
    }

    return new Graph(vertices, map, diamonds);
}

Graph::Graph(std::map<int, Vertex *> *vertices, Map *map, std::unordered_set<Position> *diamonds)
        : vertices(vertices), map(map), diamonds(diamonds) {
}

Graph::~Graph() {
    for (const auto &kv : *vertices) {
        for (auto ekv : kv.second->edges) {
            delete ekv.second;
        }
    }
    delete vertices;
}

//endregion

//region FUNCTIONS IMPLEMENTATION

void PrintPathNumbers(std::vector<Edge *> &edges, std::ostream &stream) {
    for (const Edge *e : edges) {
        stream << e->direction;
    }
}

void CheckPath(Map *map, char *pathName) {
    auto path = map->traverse(pathName);
    std::ofstream output_path_file; // TODO: Choose naming convention for local variables: `snake_case` or `javaCase`
    output_path_file.open("path.txt");
    if (output_path_file.is_open()) {
        for (Position p : *path) {
            output_path_file << p.x << "," << p.y << std::endl;
        }
        output_path_file.close();
    } else {
        std::cerr << "Unable to open path file" << std::endl;
        std::cerr << strerror(errno) << std::endl;
    }
    delete path;
}


void Solve(Map *map) {
    Graph *graph = Graph::Generate(map);
    if (DebugMode) {
        graph->printDot();
        graph->save("graph.dot");
        Stats.non_empty_nodes = graph->vertices->size();
        Stats.diamonds = graph->diamonds->size();
        for (const auto &kv : *graph->vertices) {
            Stats.edges += kv.second->outDeg;
        }
    }

    graph->traversal(map->maxMoves);
    if (DebugMode) Stats.save("log.csv");
    delete graph;
}

Map *ReadMapFromStdin() {
    return Map::CreateFromInputStream(std::cin);
}

//endregion

//region MAIN FUNCTION

int main(int argc, char *argv[]) {
    try {
        DebugMode = argc > 1;
        Map *map = (argc > 1) ? ReadMapFromFile(argv[1]) : ReadMapFromStdin();
        if (DebugMode) {
            map->print();
            Stats.case_name = argv[1];
            Stats.height = map->height;
            Stats.width = map->width;
            Stats.max_leaps = map->maxMoves;
        }

        if (argc > 2) {
            CheckPath(map, argv[2]);
        } else {
            Solve(map);
        }

        delete map;
    } catch (const char *e) {
        printf("[ERROR]: %s\n", e);
    }

    return 0;
}

//endregion

#pragma clang diagnostic pop