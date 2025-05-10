# Graph Planarity Solver

This repository contains a C++ implementation of Graph planarity analysis algorithms.

## Algorithms

The tool implements three different algorithms for planar decomposition:

1. **C+ (Cactus)**: Implements a heuristic approach using the PlanarSubgraphCactus algorithm in combination with MaximalPlanarSubgraphSimple. This provides a good balance between solution quality and performance.

2. **BM (Boyer-Myrvold)**: Uses the PlanarSubgraphBoyerMyrvold algorithm, which is a linear-time planarity testing algorithm adapted for planar subgraph extraction.

3. **Exact**: Employs the MaximumPlanarSubgraph algorithm, which provides an exact solution but may be computationally intensive for large graphs.

## Dependencies

- [OGDF (Open Graph Drawing Framework)](https://ogdf.uos.de/)
- C++17 compatible compiler
- Windows environment (for memory usage reporting)

## Building the Project

This project requires OGDF to be installed. Follow these steps to build:

1. Install OGDF according to their [installation instructions](https://ogdf.github.io/doc/ogdf/md_doc_2build.html)
2. Build using your preferred build system. For example, with CMake:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage

Run the executable with the following parameters:

```
PCB-Planarity <input.graphml> <algorithm> <output_dir>
```

Where:
- `<input.graphml>`: Path to the input graph in GraphML format
- `<algorithm>`: One of "C+", "BM", or "Exact"
- `<output_dir>`: Directory where the output files will be stored

Example:
```
PCB-Planarity input_graph.graphml BM ./results
```

The output will be stored in `./results/<input_basename>/<algorithm>/plane<N>.graphml`, where `N` is the index of the planar layer.

## Input Format

The input should be a simple undirected graph in GraphML format. The graph should include node positions and dimensions (width and height). Node weights are optional.

## Output

For each planar layer identified, the program generates a GraphML file. Each file represents a planar subgraph of the original graph. The program also outputs:

- Number of GraphML files created (number of planes/layers)
- For each plane: vertices count and edges count
- Execution time
- Peak memory usage

## Example Output

```
Number of GraphML files created: 3
File: ./results/test_graph/BM/plane0.graphml - Vertices: 125, Edges: 310
File: ./results/test_graph/BM/plane1.graphml - Vertices: 125, Edges: 51
File: ./results/test_graph/BM/plane2.graphml - Vertices: 125, Edges: 12
Time taken: 0.875 seconds
Peak memory used: 15428 KB
```

## Technical Details

The tool works by:
1. Reading the input graph using OGDF
2. Extracting a maximal planar subgraph using the specified algorithm
3. Saving this subgraph as a separate layer
4. Removing the extracted edges from the original graph
5. Repeating until the original graph has no more edges
