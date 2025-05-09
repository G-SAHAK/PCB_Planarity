# PCB Planarity Solver

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

This repository contains a C++ implementation of PCB (Printed Circuit Board) planarity analysis algorithms. The tool decomposes a non-planar graph into multiple planar subgraphs, which is particularly useful for PCB design applications where minimizing the number of layers is critical.

## Overview

PCB routing often involves dealing with non-planar graphs. This tool takes a non-planar graph as input and partitions it into multiple planar subgraphs (layers), each of which can be manufactured as a separate layer in a PCB. The goal is to minimize the number of layers while ensuring each layer is planar.

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

## Performance Considerations

- The **C+** algorithm generally offers good performance for medium-sized graphs
- The **BM** algorithm is faster but may produce more layers
- The **Exact** algorithm guarantees an optimal solution but might be too slow for large graphs

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

## License

This project is licensed under the MIT License - see below for details:

```
MIT License

Copyright (c) 2025 Sahak Gulakyan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Citation

If you use this tool in your research or academic work, please cite:

```
@software{gulakyan2025pcbplanarity,
  author = {Gulakyan, Sahak},
  title = {PCB Planarity: A Tool for Decomposing Non-Planar Graphs into Planar Subgraphs},
  year = {2025},
  url = {https://github.com/G-SAHAK/PCB_Planarity},
  note = {Part of Master's Thesis}
}
```
