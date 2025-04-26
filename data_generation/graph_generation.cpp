#include <fstream>
#include <iostream>
#include <random>
#include <vector>

std::vector<std::vector<int>> available_edges;
std::vector<std::vector<int>> graph;  // number of edges
std::vector<int> vertices_degrees;
std::mt19937 generator;

void FillAvailableEdges() {
    available_edges = std::vector<std::vector<int>>(vertices_degrees.size(),
                                                    std::vector<int>(vertices_degrees.size(), 0));
    for (size_t i = 0; i < vertices_degrees.size(); ++i) {
        for (size_t j = 0; j < i; ++j) {
            available_edges[i][j] = std::min(vertices_degrees[i], vertices_degrees[j]);
            available_edges[j][i] = available_edges[i][j];
        }
    }
}

std::vector<int> GetCurrentDegrees() {
    std::vector<int> degrees(graph.size(), 0);
    for (size_t i = 0; i < degrees.size(); ++i) {
        for (auto edges_number : graph[i]) {
            degrees[i] += edges_number;
        }
    }
    return degrees;
}

std::vector<std::pair<int, size_t>> GetCurrentDegreesAndIndices() {
    std::vector<std::pair<int, size_t>> degrees(graph.size(), {0, 0});
    for (size_t i = 0; i < degrees.size(); ++i) {
        for (auto edges_number : graph[i]) {
            degrees[i].first += edges_number;
        }
        degrees[i].second = i;
    }
    return degrees;
}

bool NoAvailableEdges() {
    for (auto& edges_list : available_edges) {
        for (auto edges_number : edges_list) {
            if (edges_number > 0) {
                return false;
            }
        }
    }
    return true;
}

bool IsFound() {
    auto actual = GetCurrentDegrees();
    return vertices_degrees == actual;
}

bool NoSenseToCheckFurther() {
    auto degrees = GetCurrentDegrees();
    for (size_t i = 0; i < vertices_degrees.size(); ++i) {
        if (degrees[i] > vertices_degrees[i]) {
            return true;
        }
    }
    return false;
}

bool NoAvailableEdgesForVertex(size_t vertex) {
    for (size_t j = 0; j < graph.size(); ++j) {
        if (available_edges[vertex][j] != 0) {
            return false;
        }
    }
    return true;
}

bool FindGraph() {
    if (NoSenseToCheckFurther()) return false;
    if (NoAvailableEdges()) return false;
    if (IsFound()) return true;

    auto degrees = GetCurrentDegreesAndIndices();
    std::sort(degrees.begin(), degrees.end());
    size_t min_degree_available_vertex = degrees[0].second;
    size_t current_vertex_index = 0;
    while (NoAvailableEdgesForVertex(min_degree_available_vertex)) {
        current_vertex_index += 1;
        if (current_vertex_index == degrees.size()) {
            return false;
        }
        min_degree_available_vertex = degrees[current_vertex_index].second;
    }

    auto weights = vertices_degrees;
    for (size_t j = 0; j < vertices_degrees.size(); ++j) {
        if (available_edges[min_degree_available_vertex][j] == 0) {
            weights[j] = 0;
        } else {
            weights[j] -= graph[min_degree_available_vertex][j];
        }
    }
    std::discrete_distribution<int> vertex_dist(weights.begin(), weights.end());
    int next_vertex = vertex_dist(generator);

    // right branch: add edge
    graph[min_degree_available_vertex][next_vertex] += 1;
    graph[next_vertex][min_degree_available_vertex] += 1;
    if (FindGraph()) {
        return true;
    }
    graph[min_degree_available_vertex][next_vertex] -= 1;
    graph[next_vertex][min_degree_available_vertex] -= 1;

    // left branch: forbid this edge
    available_edges[min_degree_available_vertex][next_vertex] -= 1;
    available_edges[next_vertex][min_degree_available_vertex] -= 1;
    if (FindGraph()) {
        return true;
    }
    available_edges[min_degree_available_vertex][next_vertex] += 1;
    available_edges[next_vertex][min_degree_available_vertex] += 1;
    return false;
}

int main(int argc, char** argv) {
    int seed = 123;
    if (argc == 2) {
        seed = std::stoi(argv[1]);
    }
    generator.seed(seed);

    std::ifstream read("vertices_degrees.txt");
    int airports_number;
    read >> airports_number;
    vertices_degrees = std::vector<int>(airports_number);
    for (auto& degree : vertices_degrees) {
        read >> degree;
    }

    std::sort(vertices_degrees.begin(), vertices_degrees.end());
    FillAvailableEdges();
    graph = std::vector<std::vector<int>>(airports_number, std::vector<int>(airports_number, 0));

    if (!FindGraph()) {
        std::cout << "not found\n";
    } else {
        std::ofstream write("graph.txt");
        for (auto& edges_list : graph) {
            for (auto edges_number : edges_list) {
                write << edges_number << ' ';
            }
            write << '\n';
        }
        write.flush();
    }
    return 0;
}
