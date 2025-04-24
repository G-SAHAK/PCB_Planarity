#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/planarity/MaximalPlanarSubgraphSimple.h>
#include <ogdf/planarity/PlanarSubgraphCactus.h>
#include <ogdf/planarity/PlanarSubgraphBoyerMyrvold.h>
#include <ogdf/planarity/MaximumPlanarSubgraph.h>
#include <ogdf/basic/extended_graph_alg.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <windows.h>
#include <Psapi.h>
using namespace ogdf;

struct PlaneInfo {
    std::string filename;
    int numNodes;
    int numEdges;
};

static std::vector<PlaneInfo> solve_Cplus(const Graph& G, GraphAttributes& GA, const std::string& outputDir) {
    std::vector<PlaneInfo> planes;
    int planeCount = 0;
    GraphCopy H(G);

    while (H.numberOfEdges() > 0) {
        NodeArray<int> component(H);
        int numComponents = connectedComponents(H, component);
        List<edge> selectedEdges;

        for (int comp = 0; comp < numComponents; ++comp) {
            Graph subgraph;
            NodeArray<node> mapHtoSub(H, nullptr);
            EdgeArray<edge> mapSubToH(subgraph, nullptr);

            for (node v : H.nodes) {
                if (component[v] == comp) {
                    mapHtoSub[v] = subgraph.newNode();
                }
            }

            for (edge e : H.edges) {
                node s = e->source();
                node t = e->target();
                if (component[s] == comp && component[t] == comp) {
                    node s_sub = mapHtoSub[s];
                    node t_sub = mapHtoSub[t];
                    edge e_sub = subgraph.newEdge(s_sub, t_sub);
                    mapSubToH[e_sub] = e;
                }
            }

            PlanarSubgraphCactus<int> psc;
            MaximalPlanarSubgraphSimple<int> mps(psc);
            List<edge> delEdgesSub;
            mps.call(subgraph, delEdgesSub);

            for (edge e_sub : subgraph.edges) {
                if (!delEdgesSub.search(e_sub).valid()) {
                    edge e_H = mapSubToH[e_sub];
                    if (e_H != nullptr) {
                        selectedEdges.pushBack(e_H);
                    }
                }
            }
        }

        Graph P_full;
        NodeArray<node> mapGtoP(G, nullptr);
        for (node v_G : G.nodes) {
            mapGtoP[v_G] = P_full.newNode();
        }

        for (edge e : selectedEdges) {
            node s_H = e->source();
            node t_H = e->target();
            node s_G = H.original(s_H);
            node t_G = H.original(t_H);
            if (s_G != nullptr && t_G != nullptr) {
                P_full.newEdge(mapGtoP[s_G], mapGtoP[t_G]);
            }
        }

        GraphAttributes PA_full(P_full, GA.attributes());
        for (node v_G : G.nodes) {
            node v_P = mapGtoP[v_G];
            PA_full.x(v_P) = GA.x(v_G);
            PA_full.y(v_P) = GA.y(v_G);
            PA_full.width(v_P) = GA.width(v_G);
            PA_full.height(v_P) = GA.height(v_G);
            if (GA.has(GraphAttributes::nodeLabel)) {
                PA_full.label(v_P) = GA.label(v_G);
            }
            if (GA.has(GraphAttributes::nodeWeight)) {
                PA_full.weight(v_P) = GA.weight(v_G);
            }
        }

        if (P_full.numberOfEdges() > 0) {
            std::string filename = outputDir + "/plane" + std::to_string(planeCount) + ".graphml";
            std::ofstream os(filename);
            if (!os) {
                std::cerr << "[ERROR] Failed to open " << filename << " for writing!\n";
                continue;
            }
            if (GraphIO::writeGraphML(PA_full, os)) {
                planes.push_back({ filename, P_full.numberOfNodes(), P_full.numberOfEdges() });
                planeCount++;
            }
            else {
                std::cerr << "[ERROR] Failed to write to " << filename << "\n";
            }
        }

        for (edge e : selectedEdges) {
            H.delEdge(e);
        }
    }
    return planes;
}

static std::vector<PlaneInfo> solve_BM(const Graph& G, GraphAttributes& GA, const std::string& outputDir) {
    std::vector<PlaneInfo> planes;
    int planeCount = 0;
    GraphCopy H(G);
    while (H.numberOfEdges() > 0) {
        PlanarSubgraphBoyerMyrvold psbm;
        List<edge> delEdges;
        psbm.call(H, delEdges);

        Graph S;
        NodeArray<node> nodeMap(H, nullptr);
        NodeArray<node> inverseMap(S, nullptr);
        for (node v : H.nodes) {
            node v_S = S.newNode();
            nodeMap[v] = v_S;
            inverseMap[v_S] = v;
        }
        for (edge e : H.edges) {
            if (!delEdges.search(e).valid()) {
                S.newEdge(nodeMap[e->source()], nodeMap[e->target()]);
            }
        }

        GraphAttributes SA(S, GA.attributes());
        for (node v : S.nodes) {
            node v_H = inverseMap[v];
            node v_G = H.original(v_H);
            SA.x(v) = GA.x(v_G);
            SA.y(v) = GA.y(v_G);
            SA.width(v) = GA.width(v_G);
            SA.height(v) = GA.height(v_G);
            if (GA.has(GraphAttributes::nodeWeight)) {
                SA.weight(v) = GA.weight(v_G);
            }
        }

        if (S.numberOfEdges() > 0) {
            std::string filename = outputDir + "/plane" + std::to_string(planeCount) + ".graphml";
            std::ofstream os(filename);
            if (!os) {
                std::cerr << "Error opening output file: " << filename << std::endl;
                continue;
            }
            if (GraphIO::writeGraphML(SA, os)) {
                planes.push_back({ filename, S.numberOfNodes(), S.numberOfEdges() });
                planeCount++;
            }
            else {
                std::cerr << "Error writing to file: " << filename << std::endl;
            }
        }

        for (edge e : S.edges) {
            node s_H = inverseMap[e->source()];
            node t_H = inverseMap[e->target()];
            edge origEdge = H.searchEdge(s_H, t_H);
            if (origEdge) H.delEdge(origEdge);
        }
    }
    return planes;
}

static std::vector<PlaneInfo> solve_Exact(const Graph& G, GraphAttributes& GA, const std::string& outputDir) {
    std::vector<PlaneInfo> planes;
    int planeCount = 0;
    GraphCopy H(G);

    while (H.numberOfEdges() > 0) {
        MaximumPlanarSubgraph<int> mps;
        List<edge> delEdges;
        mps.call(H, delEdges);

        Graph S;
        NodeArray<node> nodeMap(H, nullptr);
        NodeArray<node> inverseMap(S, nullptr);
        for (node v : H.nodes) {
            node v_S = S.newNode();
            nodeMap[v] = v_S;
            inverseMap[v_S] = v;
        }
        for (edge e : H.edges) {
            if (!delEdges.search(e).valid()) {
                S.newEdge(nodeMap[e->source()], nodeMap[e->target()]);
            }
        }

        GraphAttributes SA(S, GA.attributes());
        for (node v : S.nodes) {
            node v_H = inverseMap[v];
            node v_G = H.original(v_H);
            SA.x(v) = GA.x(v_G);
            SA.y(v) = GA.y(v_G);
            SA.width(v) = GA.width(v_G);
            SA.height(v) = GA.height(v_G);
            if (GA.has(GraphAttributes::nodeWeight)) {
                SA.weight(v) = GA.weight(v_G);
            }
        }

        if (S.numberOfEdges() > 0) {
            std::string filename = outputDir + "/plane" + std::to_string(planeCount) + ".graphml";
            std::ofstream os(filename);
            if (!os) {
                std::cerr << "Error opening output file: " << filename << std::endl;
                continue;
            }
            if (GraphIO::writeGraphML(SA, os)) {
                planes.push_back({ filename, S.numberOfNodes(), S.numberOfEdges() });
                planeCount++;
            }
            else {
                std::cerr << "Error writing to file: " << filename << std::endl;
            }
        }

        for (edge e : S.edges) {
            node s_H = inverseMap[e->source()];
            node t_H = inverseMap[e->target()];
            edge origEdge = H.searchEdge(s_H, t_H);
            if (origEdge) H.delEdge(origEdge);
        }
    }
    return planes;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input.graphml> <algorithm> <output_dir>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string algorithm = argv[2];
    std::string outputDir = argv[3];

    std::string baseName = std::filesystem::path(inputFile).stem().string();
    std::string fullOutputDir = outputDir + "/" + baseName + "/" + algorithm;
    if (!std::filesystem::exists(fullOutputDir)) {
        std::filesystem::create_directories(fullOutputDir);
    }

    Graph G;
    GraphAttributes GA(G, GraphAttributes::nodeGraphics | GraphAttributes::edgeGraphics | GraphAttributes::nodeWeight);
    std::ifstream is(inputFile);
    if (!is) {
        std::cerr << "Error opening input file: " << inputFile << std::endl;
        return 1;
    }
    if (!GraphIO::readGraphML(GA, G, is)) {
        std::cerr << "Error reading input file: " << inputFile << std::endl;
        return 1;
    }

    if (!isSimpleUndirected(G)) {
        std::cerr << "Input graph is not simple undirected." << std::endl;
        return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<PlaneInfo> planes;
    if (algorithm == "C+") {
        planes = solve_Cplus(G, GA, fullOutputDir);
    }
    else if (algorithm == "BM") {
        planes = solve_BM(G, GA, fullOutputDir);
    }
    else if (algorithm == "Exact") {
        planes = solve_Exact(G, GA, fullOutputDir);
    }
    else {
        std::cerr << "Unknown algorithm: " << algorithm << ". Use 'C+', 'BM', or 'Exact'." << std::endl;
        return 1;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Number of GraphML files created: " << planes.size() << std::endl;
    for (const auto& plane : planes) {
        std::cout << "File: " << plane.filename << " - Vertices: " << plane.numNodes << ", Edges: " << plane.numEdges << std::endl;
    }

    std::cout << "Time taken: " << elapsed.count() << " seconds" << std::endl;
    PROCESS_MEMORY_COUNTERS memCounter;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter))) {
        std::cout << "Peak memory used: " << memCounter.PeakWorkingSetSize / 1024 << " KB" << std::endl;
    }
    return 0;
}