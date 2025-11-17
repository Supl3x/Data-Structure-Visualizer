# Advanced Data Structure Visualizer

Advanced Data Structure Visualizer is a Qt desktop app built for students (and curious developers) who learn best by switching between theory and practice. Read a quick refresher, hit “Try it yourself,” and you’re dropped into an animated workspace where inserts, searches, deletions, traversals, and balancing steps play out in real time.

## Highlights
- Multi-page Qt Widgets interface built with `QStackedWidget`, featuring a landing page, menu, theory portal, and visualization workspaces.
- Binary Search Tree lab with insertion/deletion/search walkthroughs, BFS/DFS traversal animations, random input generation, and a persistent step trace.
- Red-Black Tree studio that visualizes rotations, recoloring, structural fixes, traversals, and algorithm cards explaining each scenario.
- Graph sandbox for creating vertices/edges, random graphs, editing adjacency, and running animated BFS/DFS with frontier tracking.
- Hash Table playground with configurable key/value types, chained buckets, load-factor stats, hash index tracing, and algorithm notes.
- Shared UI utilities (style manager, back button component, widget manager) keep navigation consistent and prevent leaked widgets.

## Requirements
You only need a typical Qt desktop toolchain:

| Component | Minimum Version | Purpose |
|-----------|-----------------|---------|
| Qt (Widgets module) | Qt 6.5+ (Qt 5.15+ with minor tweaks) | UI toolkit, signal/slot system, animations |
| CMake | 3.16+ | Project configuration and build generation |
| C++ Compiler | C++17-compliant (MSVC 2019+, MinGW-w64 11+, Clang 12+) | Compiles the application |
| Ninja or Make (optional) | Latest | Faster builds when using `-G "Ninja"` |

> Windows tip: the Qt Online Installer (pick the MinGW or MSVC kit) brings along the compiler, Qt libraries, and Qt Creator so you can build immediately.

## Project Structure
- `main.cpp`: Starts `QApplication`, wires `HomePage → MenuPage → TheoryPage → Visualizers`, and keeps widget lifetimes tidy through `WidgetManager`.
- `homepage.*`: Full-bleed hero layout with animated panels and a single “Get Started” button.
- `menupage.*`: Four data-structure cards with emojis, gradients, and staggered fade-ins that emit the chosen structure.
- `theorypage.*`: Scrollable info cards (definition, complexity, applications, pros/cons) plus a `Try It Yourself` button that launches the right simulator.
- `treeinsertion.*`, `redblacktree.*`, `graphvisualization.*`, `hashmapvisualization.*`: The actual labs—each has its own canvas, control stack, history tabs, traversal controls, and animation timers.
- `hashmap.*`, `basevisualization.*`, `stylemanager.*`, `widgetmanager.*`, `uiutils.*`, `backbutton.*`: Support code (data logic, shared styling, navigation controls, safe `deleteLater` helpers).

## Building the Application
1. **Configure Qt paths**  
   Ensure `cmake` can locate Qt. The typical approach is to set `CMAKE_PREFIX_PATH` to the Qt kit directory (folder containing `lib/cmake/Qt6`).

2. **Generate build files**
   ```powershell
   cd E:\Saad\Third Semester\DSA\Project\Second_Attempt
   cmake -S . -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:\Qt\6.7.2\mingw_64"
   ```
   Replace the generator and Qt path with the kit you installed (e.g., `-G "MinGW Makefiles"` or `-G "Visual Studio 17 2022"` on MSVC).

3. **Compile**
   ```powershell
   cmake --build build
   ```

4. **Run**
   ```powershell
   build\AdvDS.exe
   ```
   On Linux/macOS the executable will be under `build/AdvDS`.

## Running from Qt Creator
1. Open the folder in Qt Creator.
2. Choose the kit that matches your Qt install (e.g., Desktop Qt 6.7.2 MinGW 64-bit).
3. Hit **Configure Project**, then **Build & Run**. Qt Creator manages `CMAKE_PREFIX_PATH` and deployment automatically.

## Usage Guide
1. Launch the app, enjoy the landing page, and tap **Get Started**.
2. Choose a data-structure card; the app opens the matching theory page.
3. Skim the cards, then click **Try It Yourself** to jump into the lab.
4. In the lab:
   - Enter values to insert/search/delete.
   - Use BFS/DFS buttons to watch each node highlight in sequence.
   - Hit **Randomize** when you want a messy dataset to explore.
   - Check the right-side tabs for narrated steps and pseudocode.
5. Tap the floating back button any time to return to theory or the main menu.

## Deployment Notes
- **Windows**: When distributing, copy the Qt runtime using `windeployqt build\AdvDS.exe`.
- **macOS**: Use `macdeployqt AdvDS.app`.
- **Linux**: Ensure target machines have the same Qt version or bundle libraries via `linuxdeployqt`.

## Troubleshooting
- *“Could NOT find Qt6Widgets”*: Double-check `CMAKE_PREFIX_PATH` or set `Qt6_DIR` to the kit’s `lib/cmake/Qt6` folder.
- *Linker errors mentioning Qt::Widgets*: The Widgets module wasn’t installed—rerun the Qt installer and add it.
- *Fonts look off*: The UI falls back gracefully, but installing Segoe UI or Poppins restores the intended typography.

---
This README walks through the non-Markdown parts of the repo and the tools you need so you can build, run, and tweak the Advanced Data Structure Visualizer on any desktop OS that Qt supports. Have fun experimenting!

