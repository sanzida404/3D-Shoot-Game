# 3D Shoot Game: SEU Corridor Explorer

A first-person 3D zombie shooter built with **C++ and OpenGL (GLUT)**. The player explores a university corridor with classrooms, a washroom and an elevator, and must shoot zombies to unlock the lift and clear all 3 floors.

## Features

- Fully enclosed 3D environment: 8 classrooms, a central corridor, a washroom with cubicles, and elevators
- Procedural textures (tile floor, brick walls, wood doors, metal) and two light sources
- Openable doors with real collision, rotating ceiling fans, and a window with an outdoor scene
- 6 zombies with patrol and chase behaviour, wall/furniture collision, and death animation
- First-person gun with muzzle flash and bullet tracers
- 3 floors: kill 3 / 4 / 5 zombies to unlock the lift and move up; clear floor 3 to win

## Controls

| Key / Mouse | Action |
|---|---|
| W A S D / Arrow keys | Move |
| Mouse | Look around |
| Shift | Run |
| Space | Shoot |
| Left click (near a door) | Open / close door, use the lift |
| R | Restart (after Game Over or Win) |
| ESC | Quit |

A zombie that reaches the player means instant death.

## Project Structure

| File | Contents | Owner |
|---|---|---|
| `environment.h` | Textures, walls, floor, classrooms, washroom, doors, elevator, window, `drawScene()` |
| `characters.h` | Zombie model and AI, gun and shooting, player movement, keyboard and mouse input |
| `common.h` | Shared globals, structs, constants and collision functions | 
| `main.cpp` | HUD, render loop, timer, OpenGL init, `main()` |

Only `main.cpp` is compiled. The `.h` files are included by it.

## Build and Run

### Linux (Ubuntu/Debian)

```bash
sudo apt install freeglut3-dev
g++ main.cpp -o seu_corridor -lglut -lGLU -lGL -lm
./seu_corridor
```

### Windows (MinGW + freeglut)

```bash
g++ main.cpp -o seu_corridor.exe -lfreeglut -lopengl32 -lglu32
```

### macOS

```bash
g++ main.cpp -o seu_corridor -framework GLUT -framework OpenGL -Wno-deprecated
```

> The game needs a display window, so it cannot run in a headless environment such as GitHub Codespaces.

## Team

| Name | ID | Part |
|---|---|---|
| Sanzida Islam Shormi | 2023100000447 | Environment |
| Razia Binte Alam Raya | 2023100000110 | Zombie and shooter movement |
| Sanzida Islam Shormi & Razia Binte Alam Raya | 2023100000447 & 2023100000110 | Common and main |
| Mst. Sumaiya Islam Zannat | 2023200000244 | Merging all parts and final integration |

## Course Info

Computer Graphics & Animation Lab | Summer 26/, Instructor: Tanjina Oriana
