# Ray Caster

A Wolfenstein-style 3D raycaster built from scratch in C++ using SFML. Features a real-time minimap, a complex 16×12 maze, and a startup instruction panel.

![C++](https://img.shields.io/badge/C++-GCC%2015.2-blue) ![SFML](https://img.shields.io/badge/SFML-3.x-green)

---

## Features

- **3D rendering** — Wolfenstein-style first-person wall rendering using the DDA raycasting algorithm
- **Fisheye correction** — perpendicular distance calculation for accurate wall proportions
- **Wall shading** — north/south faces are darker than east/west for a depth effect
- **Real-time minimap** — top-down view showing the map, player position, direction arrow, and cast rays
- **16×12 maze** — corridors, rooms, and dead ends to explore
- **Per-axis collision detection** — smooth sliding along walls instead of hard stops
- **Splash screen** — control instructions shown on startup, dismissed with any key
- **60 FPS cap** — consistent frame rate via SFML's frame limiter

---

## Requirements

- Windows
- SFML 3.x (included as DLLs in the release)
- No installation needed — just download and run

---

## Running

Download the latest release, extract the zip, and run:

```
raycaster.exe
```

All required DLLs are bundled in the zip.

---

## Controls

| Input | Action |
|---|---|
| W / S | Move forward / backward |
| A / D | Strafe left / right |
| Left / Right | Rotate camera |
| Escape | Quit |

---

## Building from Source

Requires GCC and SFML 3 installed via MSYS2 UCRT64:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-sfml
```

Then compile:

```bash
g++ main.cpp -o raycaster -lsfml-graphics -lsfml-window -lsfml-system
```

---

## How It Works

The renderer uses the **DDA (Digital Differential Analysis)** algorithm to cast one ray per vertical column of the screen. Each ray marches through the map tile by tile until it hits a wall. The perpendicular distance to that wall determines the height of the vertical slice drawn on screen — closer walls produce taller slices, farther walls produce shorter ones. This creates the illusion of 3D depth from a 2D grid map.

---

## Project Structure

```
raycaster/
├── main.cpp        # entire application, single file
└── *.dll           # SFML runtime dependencies (in release zip)
```

---

## License

MIT
