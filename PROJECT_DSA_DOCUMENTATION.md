# Advanced Data Structure Visualizer

## Project Overview

The **Advanced Data Structure Visualizer** is an interactive educational desktop application built with Qt Framework (C++) that helps students and developers understand complex data structures through beautiful visualizations, step-by-step animations, and detailed algorithmic explanations.

This project provides a comprehensive learning platform for visualizing and understanding four fundamental data structures with their operations and traversal algorithms.

---

## 🎯 Project Features

- **Interactive Visualizations**: Real-time animations showing data structure operations
- **Step-by-Step Tracing**: Detailed operation history with step-by-step explanations
- **Algorithm Display**: In-depth algorithm explanations with time/space complexity
- **Theory Pages**: Comprehensive theoretical background for each data structure
- **Modern UI**: Beautiful, user-friendly interface with gradient designs and smooth animations
- **Multiple Operations**: Support for Insert, Delete, Search, and Clear operations
- **Traversal Algorithms**: BFS and DFS implementations with visual animations

---

## 📊 Data Structures Implemented

### 1. Binary Search Tree (BST) [[DSA CONCEPT #1]]

**Location**: `treeinsertion.cpp`, `treeinsertion.h`

**DSA Concepts Used**:
- ✅ **Tree Data Structure**: Hierarchical node-based structure
- ✅ **Binary Tree Properties**: Each node has at most two children (left and right)
- ✅ **BST Property**: For any node, all nodes in left subtree < node < all nodes in right subtree
- ✅ **Tree Traversal Algorithms**:
  - **Breadth-First Search (BFS)**: Level-order traversal using Queue [[DSA CONCEPT #2]]
  - **Depth-First Search (DFS)**: Pre-order traversal using Stack/Recursion [[DSA CONCEPT #3]]
- ✅ **Binary Search**: Efficient search using BST property
- ✅ **Recursive Algorithms**: Tree operations using recursion
- ✅ **Tree Insertion**: Maintaining BST property during insertion
- ✅ **Tree Deletion**: Handling three cases (0, 1, or 2 children)
- ✅ **Inorder Successor Finding**: For node deletion with two children

**Operations Implemented**:
- Insert: O(log n) average, O(n) worst
- Search: O(log n) average, O(n) worst  
- Delete: O(log n) average, O(n) worst
- Clear: O(n)

**Key Functions**:
```cpp
insertNode(int value)          // Insert maintaining BST property
searchNode(int value)          // Binary search implementation
deleteNode(TreeNode* node, int value)  // Deletion with three cases
findMin(TreeNode* node)        // Find minimum (inorder successor)
performBFS()                   // Queue-based level-order traversal
performDFSRecursive(TreeNode* node)  // Stack-based DFS (preorder)
```

---

### 2. Red-Black Tree [[DSA CONCEPT #4]]

**Location**: `redblacktree.cpp`, `redblacktree.h`

**DSA Concepts Used**:
- ✅ **Self-Balancing Binary Search Tree**: Automatically maintains balance
- ✅ **Red-Black Tree Properties**:
  - Every node is either RED or BLACK
  - Root is always BLACK
  - No two consecutive RED nodes (RED parent cannot have RED child)
  - Every path from root to NULL has same number of BLACK nodes (black-height property)
- ✅ **Tree Rotations** [[DSA CONCEPT #5]]:
  - Left Rotation: Rebalancing operation
  - Right Rotation: Rebalancing operation
- ✅ **Violation Fixing Algorithms**: Fix Insert and Fix Delete
- ✅ **BST Insertion**: Standard BST insertion with color assignment
- ✅ **Tree Traversal Algorithms**:
  - **BFS (Breadth-First Search)**: Queue-based level-order [[DSA CONCEPT #2]]
  - **DFS (Depth-First Search)**: Recursive pre-order [[DSA CONCEPT #3]]
- ✅ **NIL Sentinel Node**: Special leaf node representation
- ✅ **Parent Pointer Maintenance**: For rotation operations

**Operations Implemented**:
- Insert: O(log n) guaranteed
- Search: O(log n) guaranteed
- Delete: O(log n) guaranteed
- Clear: O(n)

**Key Functions**:
```cpp
BSTInsert(RBNode* root, RBNode* node)    // Standard BST insertion
fixInsert(RBNode* node)                  // Fix Red-Black violations after insert
rotateLeftSync(RBNode* node)             // Left rotation for balancing
rotateRightSync(RBNode* node)            // Right rotation for balancing
deleteNodeHelper(RBNode* node, int value) // Delete with violation fixing
fixDelete(RBNode* node)                  // Fix violations after delete
findMin(RBNode* node)                    // Find minimum node
performBFS()                             // Level-order traversal
performDFSRecursive(RBNode* node)        // Preorder traversal
```

**Red-Black Tree Insertion Cases**:
1. **Case 1**: Uncle is RED → Recolor parent, uncle, and grandparent
2. **Case 2**: Uncle is BLACK (Triangle) → Rotate to convert to Case 3
3. **Case 3**: Uncle is BLACK (Line) → Rotate and recolor

---

### 3. Hash Table / Hash Map [[DSA CONCEPT #6]]

**Location**: `hashmap.cpp`, `hashmap.h`, `hashmapvisualization.cpp`, `hashmapvisualization.h`

**DSA Concepts Used**:
- ✅ **Hash Table Data Structure**: Array of buckets for key-value storage
- ✅ **Hash Function** [[DSA CONCEPT #7]]:
  - Uses `std::hash` for different data types (string, int, double, float, char)
  - Index calculation: `hash(key) % bucketCount`
- ✅ **Collision Resolution**: **Open Chaining** (Separate Chaining) [[DSA CONCEPT #8]]
  - Each bucket contains a linked list (std::forward_list) of key-value pairs
- ✅ **Dynamic Resizing / Rehashing** [[DSA CONCEPT #9]]:
  - Load factor calculation: `elements / bucketCount`
  - Automatic rehashing when load factor exceeds threshold (default 0.75)
  - Doubles bucket count during rehash
- ✅ **Linked List Operations**: Traversal, insertion, deletion in collision chains
- ✅ **Key-Value Pair Storage**: Generic storage using QVariant for multiple types
- ✅ **Type System**: Support for String, Integer, Double, Float, Char types

**Operations Implemented**:
- Insert/Put: O(1) average, O(n) worst (due to chaining)
- Search/Get: O(1) average, O(n) worst
- Delete: O(1) average, O(n) worst
- Clear: O(n)

**Key Functions**:
```cpp
indexFor(const QVariant &key, int bucketCount)  // Hash function implementation
emplaceOrAssign(key, value, assignIfExists)     // Insert/update logic
get(const QVariant &key)                        // Search with collision handling
erase(const QVariant &key)                      // Delete with chain maintenance
rehash(int newBucketCount)                      // Dynamic resizing algorithm
maybeGrow()                                     // Load factor check and rehash
```

**Hash Table Structure**:
- Bucket array: `std::vector<std::forward_list<Node>>`
- Each Node contains: key (QVariant), value (QVariant)
- Chaining: Collisions resolved by linked list in each bucket

---

### 4. Graph [[DSA CONCEPT #10]]

**Location**: `graphvisualization.cpp`, `graphvisualization.h`

**DSA Concepts Used**:
- ✅ **Graph Data Structure**: Non-linear structure with vertices and edges
- ✅ **Adjacency List Representation** [[DSA CONCEPT #11]]:
  - Uses `QHash<int, QSet<int>>` for efficient edge storage
  - Undirected graph (edges stored bidirectionally)
- ✅ **Graph Traversal Algorithms**:
  - **Breadth-First Search (BFS)** [[DSA CONCEPT #12]]:
    - Uses Queue data structure
    - Level-by-level traversal
    - Time: O(V + E), Space: O(V)
  - **Depth-First Search (DFS)** [[DSA CONCEPT #13]]:
    - Uses Stack data structure (iterative) or Recursion
    - Deep traversal before backtracking
    - Time: O(V + E), Space: O(V)
- ✅ **Visited Set**: QSet for tracking visited vertices during traversal
- ✅ **Graph Operations**: Add/Delete vertices and edges

**Operations Implemented**:
- Add Vertex: O(1)
- Add Edge: O(1)
- Delete Vertex: O(V + E) - must remove all connected edges
- Delete Edge: O(1)
- BFS Traversal: O(V + E)
- DFS Traversal: O(V + E)

**Key Functions**:
```cpp
addEdgeInternal(int u, int v)        // Add edge to adjacency list
removeVertexInternal(int u)          // Remove vertex and all edges
removeEdgeInternal(int u, int v)     // Remove edge from adjacency list
onStartBFS()                         // BFS implementation with Queue
onStartDFS()                         // DFS implementation with Stack
```

**Graph Representation**:
- Vertices: Stored in `QVector<GraphNode>`
- Edges: Adjacency list using `QHash<int, QSet<int>>`
- Undirected: Edge (u,v) implies edge (v,u) exists

---

## 🔄 Additional DSA Concepts Used

### Queue Data Structure [[DSA CONCEPT #2]]
**Used in**:
- BFS traversal in BST (`treeinsertion.cpp`)
- BFS traversal in Red-Black Tree (`redblacktree.cpp`)
- BFS traversal in Graph (`graphvisualization.cpp`)

**Implementation**: `QList<TreeNode*>` or `QList<int>` with `append()` (enqueue) and `pop_front()` (dequeue)

---

### Stack Data Structure [[DSA CONCEPT #3]]
**Used in**:
- DFS traversal (recursive call stack) in BST
- DFS traversal in Red-Black Tree
- DFS traversal in Graph (iterative: `QList<int>` with `append()` and `takeLast()`)

---

### Linked List [[DSA CONCEPT #14]]
**Used in**:
- Hash Map collision resolution (`std::forward_list<Node>` in each bucket)
- Chain traversal during hash operations

---

### Set Data Structure [[DSA CONCEPT #15]]
**Used in**:
- Graph adjacency list (`QSet<int>` for neighbor storage)
- Visited tracking in graph traversals (`QSet<int> visited`)
- Efficient duplicate checking

---

### Hash Function [[DSA CONCEPT #7]]
**Used in**:
- Hash Map implementation (`hashmap.cpp::indexFor()`)
- Supports multiple types: string, int, double, float, char
- Modular arithmetic for bucket indexing

---

## 📁 Project Structure

```
Advanced_DataStructure_Visualiser/
├── main.cpp                          # Application entry point
├── mainwindow.cpp/h                  # Main window management
├── homepage.cpp/h                    # Landing page
├── menupage.cpp/h                    # Data structure selection menu
├── theorypage.cpp/h                  # Theoretical information display
│
├── treeinsertion.cpp/h               # Binary Search Tree implementation
├── redblacktree.cpp/h                # Red-Black Tree implementation
├── hashmap.cpp/h                     # Hash Map core logic
├── hashmapvisualization.cpp/h        # Hash Map UI and visualization
├── graphvisualization.cpp/h          # Graph implementation and visualization
│
├── basevisualization.cpp/h           # Base visualization class
├── traversalvisualization.h          # Traversal visualization utilities
├── uiutils.cpp/h                     # UI utility functions
├── stylemanager.cpp/h                # Styling management
├── widgetmanager.cpp/h               # Widget lifecycle management
├── backbutton.cpp/h                  # Navigation button component
│
└── CMakeLists.txt                    # Build configuration
```

---

## 🛠️ Technology Stack

- **Framework**: Qt 6.8.2 (C++)
- **Language**: C++17
- **Build System**: CMake
- **UI Toolkit**: Qt Widgets
- **Graphics**: QPainter for custom drawings, QGraphicsScene/View for complex visualizations
- **Animations**: QTimer, QPropertyAnimation

---

## 📝 DSA Concepts Summary

| # | DSA Concept | Location | Description |
|---|-------------|----------|-------------|
| 1 | Binary Search Tree | `treeinsertion.cpp/h` | Hierarchical tree with BST property |
| 2 | Queue | `treeinsertion.cpp`, `redblacktree.cpp`, `graphvisualization.cpp` | Used in BFS traversals |
| 3 | Stack | `treeinsertion.cpp`, `redblacktree.cpp`, `graphvisualization.cpp` | Used in DFS traversals (recursive/iterative) |
| 4 | Red-Black Tree | `redblacktree.cpp/h` | Self-balancing BST with color properties |
| 5 | Tree Rotations | `redblacktree.cpp` | Left/Right rotations for balancing |
| 6 | Hash Table | `hashmap.cpp/h` | Key-value storage with hashing |
| 7 | Hash Function | `hashmap.cpp::indexFor()` | Computes bucket index from key |
| 8 | Open Chaining | `hashmap.cpp` | Collision resolution using linked lists |
| 9 | Rehashing | `hashmap.cpp::rehash()` | Dynamic resizing based on load factor |
| 10 | Graph | `graphvisualization.cpp/h` | Non-linear structure with vertices/edges |
| 11 | Adjacency List | `graphvisualization.cpp` | Graph representation using hash map of sets |
| 12 | BFS Algorithm | `graphvisualization.cpp`, `treeinsertion.cpp`, `redblacktree.cpp` | Level-order traversal |
| 13 | DFS Algorithm | `graphvisualization.cpp`, `treeinsertion.cpp`, `redblacktree.cpp` | Depth-first traversal |
| 14 | Linked List | `hashmap.cpp` | Collision chains in hash buckets |
| 15 | Set | `graphvisualization.cpp` | Adjacency list and visited tracking |

---

## 🚀 Building and Running

### Prerequisites
- Qt 6.8.2 or later
- CMake 3.16 or later
- C++17 compatible compiler (MSVC, GCC, Clang)
- Qt Creator (recommended) or command-line build tools

### Build Instructions

1. **Using Qt Creator**:
   - Open `CMakeLists.txt` in Qt Creator
   - Configure project
   - Build and Run

2. **Using Command Line**:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ./AdvDS  # or AdvDS.exe on Windows
   ```

---

## 📖 Usage Guide

1. **Launch Application**: Start the application to see the home page
2. **Select Data Structure**: Choose from Binary Search Tree, Red-Black Tree, Graph, or Hash Table
3. **Read Theory**: Review the theoretical background and complexity analysis
4. **Try It Yourself**: Click "Try It Yourself" to open the interactive visualization
5. **Perform Operations**: Use Insert, Delete, Search, and Clear operations
6. **View Algorithms**: Switch to the "Algorithm" tab to see step-by-step algorithms
7. **Traversal**: Use BFS/DFS buttons to visualize tree/graph traversals
8. **Track Steps**: Monitor the "Steps" tab for detailed operation history

---

## 🎨 UI Features

- **Gradient Backgrounds**: Beautiful color schemes for each page
- **Smooth Animations**: Fade-in effects and operation animations
- **Real-time Visualization**: Live updates during operations
- **Step-by-Step Tracing**: Detailed operation logs
- **Algorithm Display**: Comprehensive algorithm explanations
- **Responsive Design**: Adapts to different window sizes

---

## 📊 Time and Space Complexities

### Binary Search Tree
- **Insert**: O(log n) average, O(n) worst
- **Search**: O(log n) average, O(n) worst
- **Delete**: O(log n) average, O(n) worst
- **Space**: O(n)

### Red-Black Tree
- **Insert**: O(log n) guaranteed
- **Search**: O(log n) guaranteed
- **Delete**: O(log n) guaranteed
- **Space**: O(n)

### Hash Table
- **Insert**: O(1) average, O(n) worst
- **Search**: O(1) average, O(n) worst
- **Delete**: O(1) average, O(n) worst
- **Space**: O(n)

### Graph
- **Add Vertex**: O(1)
- **Add Edge**: O(1)
- **BFS/DFS**: O(V + E)
- **Space**: O(V + E)

---

## 🔍 Key Algorithms Implemented

1. **BST Insertion**: Maintains BST property during insertion
2. **BST Deletion**: Handles three cases (0, 1, 2 children)
3. **Red-Black Tree Insertion**: BST insert + violation fixing (3 cases)
4. **Red-Black Tree Deletion**: BST delete + violation fixing (4 cases)
5. **Tree Rotations**: Left and right rotations for balancing
6. **Hash Function**: Type-specific hash computation
7. **Rehashing**: Dynamic table resizing with element redistribution
8. **BFS Traversal**: Queue-based level-order traversal
9. **DFS Traversal**: Stack-based/recursive depth-first traversal
10. **Graph Representation**: Adjacency list with efficient edge operations

---

## 🎓 Educational Value

This project serves as an excellent educational tool for:
- Understanding data structure operations visually
- Learning algorithm implementations
- Grasping time/space complexity concepts
- Visualizing tree balancing mechanisms
- Understanding graph traversal algorithms
- Learning hash table collision resolution

---

## 📄 License

This project is developed for educational purposes as part of a DSA Lab course.

---

## 👨‍💻 Development Notes

- **Memory Management**: Proper cleanup using `deleteLater()` and widget managers
- **Animation System**: QTimer-based step-by-step visualizations
- **UI Consistency**: StyleManager for uniform styling across components
- **Error Handling**: Input validation and user-friendly error messages
- **Code Organization**: Modular design with separation of concerns

---

---

## 📝 PROMPT FOR CONSTRUCTING A PROJECT REPORT

**Use the following prompt with an AI assistant (like ChatGPT, Claude, or similar) to generate a comprehensive project report:**

---

**PROMPT:**

```
You are a technical documentation expert. Please create a comprehensive project report for the "Advanced Data Structure Visualizer" application based on the following information:

**PROJECT DETAILS:**
- Project Name: Advanced Data Structure Visualizer
- Technology: Qt Framework (C++), CMake build system
- Purpose: Interactive educational tool for visualizing data structures

**DATA STRUCTURES IMPLEMENTED:**

1. **Binary Search Tree (BST)**
   - Operations: Insert, Delete, Search, Clear
   - Traversals: BFS, DFS (Preorder)
   - Files: treeinsertion.cpp/h
   - Key algorithms: BST insertion, deletion with three cases, inorder successor finding

2. **Red-Black Tree**
   - Operations: Insert, Delete, Search, Clear
   - Traversals: BFS, DFS (Preorder)
   - Files: redblacktree.cpp/h
   - Key algorithms: Self-balancing, tree rotations (left/right), violation fixing (insert/delete cases)

3. **Hash Table / Hash Map**
   - Operations: Insert/Put, Delete, Search/Get, Clear
   - Collision Resolution: Open Chaining (Separate Chaining)
   - Files: hashmap.cpp/h, hashmapvisualization.cpp/h
   - Key algorithms: Hash function, rehashing, load factor management

4. **Graph**
   - Operations: Add/Delete Vertex, Add/Delete Edge, Clear
   - Representation: Adjacency List (QHash<int, QSet<int>>)
   - Traversals: BFS, DFS
   - Files: graphvisualization.cpp/h
   - Type: Undirected, unweighted

**ADDITIONAL DSA CONCEPTS:**
- Queue (for BFS traversals)
- Stack (for DFS traversals, recursive call stack)
- Linked List (for hash table collision chains)
- Set (for graph adjacency and visited tracking)
- Hash Functions (multiple type support)

**PROJECT FEATURES:**
- Interactive visualizations with real-time animations
- Step-by-step operation tracing
- Algorithm explanations with complexity analysis
- Theory pages for each data structure
- Modern UI with gradient designs
- Multiple data type support (Hash Map)

**REPORT REQUIREMENTS:**

Please create a professional project report with the following sections:

1. **Executive Summary**
   - Brief overview of the project
   - Objectives and goals
   - Target audience

2. **Introduction**
   - Problem statement
   - Motivation
   - Scope of the project

3. **System Design**
   - Architecture overview
   - Technology stack
   - Project structure
   - UI/UX design principles

4. **Data Structures Implementation**
   For each data structure (BST, Red-Black Tree, Hash Table, Graph):
   - Detailed description
   - DSA concepts used (mark them clearly)
   - Algorithms implemented
   - Code structure
   - Time/Space complexity analysis
   - Visual representation approach

5. **Algorithms and Operations**
   - Detailed algorithm explanations
   - Step-by-step procedures
   - Complexity analysis for each operation
   - Examples and use cases

6. **User Interface Design**
   - UI components
   - Navigation flow
   - Visualization techniques
   - User interaction patterns

7. **Implementation Details**
   - Key coding techniques
   - Design patterns used
   - Memory management
   - Animation system

8. **Testing and Validation**
   - How operations were tested
   - Edge cases handled
   - Performance considerations

9. **Results and Screenshots**
   - Feature demonstrations
   - Visualization examples
   - Operation flows

10. **Conclusion**
    - Project achievements
    - Learning outcomes
    - Future enhancements
    - References

**IMPORTANT INSTRUCTIONS:**
- Mark all DSA concepts clearly where they appear (use markers like [DSA CONCEPT #X])
- Include complexity analysis tables
- Provide algorithm pseudocode where relevant
- Include visual descriptions of data structure operations
- Make the report academic yet accessible
- Use proper technical terminology
- Include code snippets from the actual implementation
- Discuss the educational value of each feature
- Explain why specific algorithms were chosen

Please format the report professionally with proper headings, subheadings, bullet points, tables, and code blocks where appropriate.
```

---

**END OF README**

---

**Note to User**: 

1. Copy the prompt above (starting from "PROMPT:" and ending at "END OF README")
2. Paste it into an AI assistant of your choice
3. The AI will generate a comprehensive project report based on your codebase
4. You may need to provide additional details or screenshots if requested by the AI
5. Review and customize the generated report as needed for your specific requirements

---

