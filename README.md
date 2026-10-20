# Octree

A generic C++20 octree implementation.

## Overview

**Octree** is a C++20 project focused on the implementation of a flexible, hierarchical tree data structure for spatial organization.

The main goal is the tree itself: its internal structure, node relationships, references, traversal mechanisms, and type abstractions. The implementation is organized into distinct components to separate tree infrastructure from the details of individual use cases.

The project is currently undergoing refactoring. Interfaces and implementation details may evolve as the design develops.

## Design and Structure

The implementation is organized around several core components:

- **Nodes** — The structural building blocks of the tree, including branches, base nodes, and subtrees.
- **References** — Abstractions for expressing relationships between tree elements, including ID-based and parent-ID-based references.
- **Iterators** — Traversal-related abstractions for accessing tree elements.
- **Traits** — Type properties and configuration abstractions used by the tree implementation.
- **Tree infrastructure** — Shared implementation components and identifiers that support the overall data structure.

The intention is to keep the tree's underlying mechanisms distinct from application-specific algorithms and data.

## Project Structure

```text
src/
├── Octree/
│   ├── Octree.h
│   └── Implementation/
│       ├── Buffer.hpp
│       ├── Implementation.h
│       ├── TreeBase.hpp
│       ├── TreeID.hpp
│       ├── Iterator/
│       ├── Nodes/
│       ├── References/
│       └── Traits/
├── Init.cpp
├── Init.h
├── SimulatorV0.cpp
├── SimulatorV0.h
└── main.cpp
```

The `Octree` directory contains the core data structure and its implementation components. The remaining source files currently provide initialization and an experimental application using the tree.

## Requirements

- C++20-compatible compiler
- CMake 3.20 or newer
- OpenMP
- Ninja (recommended)

## Build

Configure and build with CMake:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

OpenMP must be available to the compiler and CMake configuration.

## Example Application

The repository currently includes an experimental application in `SimulatorV0`.

It serves as a practical test case for exercising the tree implementation. It is not the primary focus of the project, and its design and behavior may change independently of the core tree.

## Development Status

**Work in progress — active refactoring.**

The internal architecture and interfaces are still evolving. The current focus is on the tree implementation and the relationships between its components.

## License

No license has been specified yet.

