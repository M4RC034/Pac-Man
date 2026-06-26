# Allegro Pac-Man

A classic Pac-Man game built in **C** using the [Allegro 5](https://liballeg.org/) multimedia library. 

---

## Table of Contents

- [Features](#-features)
- [Screenshots](#-screenshots)
- [Prerequisites](#-prerequisites)
- [Getting Started](#-getting-started)
  - [Clone the Repository](#1-clone-the-repository)
  - [Open in Visual Studio](#2-open-in-visual-studio)
  - [Restore NuGet Packages](#3-restore-nuget-packages)
  - [Build & Run](#4-build--run)
- [Controls](#-controls)
- [Project Structure](#-project-structure)
- [Map Customization](#-map-customization)
- [Architecture Overview](#-architecture-overview)
- [License](#-license)

---

## ✨ Features

| Feature | Description |
|---|---|
| **Scene Management System** | Modular scene architecture with Menu, Game, and Settings scenes. Scenes register their own event callbacks and can be swapped at runtime. |
| **Tile-Based Map Rendering** | Maps are defined as 2D character grids (`#` = wall, `.` = bean, `B` = ghost room, `P` = power pellet). Walls are rendered with intelligent neighbor-aware drawing for clean edges. |
| **Custom Map Support** | Load maps from external `.txt` files or use built-in hardcoded layouts (default open map, NTHU-themed map). |
| **Ghost AI with State Machine** | Ghosts operate under a multi-state system: `BLOCKED` → `GO_OUT` → `FREEDOM` → `FLEE` → `GO_IN`, each with unique movement behavior. |
| **BFS Shortest Path** | A breadth-first search pathfinding algorithm drives ghost navigation — used for chasing Pac-Man, fleeing, and returning to the ghost room. |
| **Power Pellet System** | Eating a power pellet triggers a timed power-up that causes ghosts to enter `FLEE` mode, reversing the hunter/hunted dynamic. |
| **Sprite Animation** | Pac-Man and ghosts use sprite sheets with frame-based animation tied to the game tick system. |
| **Audio System** | Background music (classic Pac-Man theme) and sound effects for movement, eating, death, and ghost interactions via Allegro's audio add-on. |
| **Collision Detection** | Rectangle-area overlap detection between Pac-Man and ghosts with a visual debug hitbox mode. |
| **Debug & Cheat Modes** | Toggle debug mode to visualize hitboxes. Cheat mode disables ghost collision for testing. |
| **Logging System** | Optional compile-time logging (`LOG_ENABLED`) outputs to both console and `log.txt` for debugging. |
| **60 FPS Game Loop** | Smooth event-driven game loop running at 60 FPS with a separate game tick timer for movement synchronization. |

---

## 📸 Screenshots

> *The game features a title screen with the classic Pac-Man logo, a bean-filled maze, animated Pac-Man and ghost sprites, and a "READY!" countdown before gameplay begins.*

---

## 📋 Prerequisites

- **Operating System:** Windows 10/11
- **IDE:** [Visual Studio 2019](https://visualstudio.microsoft.com/) or later (with C/C++ Desktop Development workload)
- **NuGet Packages** (automatically restored):
  - `Allegro` v5.2.7
  - `AllegroDeps` v1.12.0

> [!NOTE]
> This project uses Visual Studio's NuGet integration to manage the Allegro 5 library. No manual library installation is required.

---

## 🚀 Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/M4RC034/Pac-Man.git
cd Pac-Man
```

### 2. Open in Visual Studio

Open the solution file:

```
Allegro_pacman/Allegro_pacman.sln
```

Visual Studio will load the project with all configured build settings.

### 3. Restore NuGet Packages

The Allegro library is managed via NuGet. To restore packages:

1. In Visual Studio, go to **Tools → NuGet Package Manager → Package Manager Console**
2. Run:
   ```
   Update-Package -reinstall
   ```

Or simply **right-click the solution** in Solution Explorer → **Restore NuGet Packages**.

> [!IMPORTANT]
> If you see an error about missing NuGet packages on the first build, perform the restore step above and rebuild.

### 4. Build & Run

1. Select your build configuration:
   - **Debug | x86** or **Debug | x64** (recommended for development)
   - **Release | x86** or **Release | x64** (optimized build)
2. Press **`F5`** (Start Debugging) or **`Ctrl + F5`** (Start Without Debugging)

The game window (800×800) will open with the Pac-Man menu screen.

---

## 🎮 Controls

| Key | Action |
|---|---|
| `Enter` | Start game (from menu) |
| `W` / `↑` | Move Up |
| `A` / `←` | Move Left |
| `S` / `↓` | Move Down |
| `D` / `→` | Move Right |
| `C` | Toggle cheat mode (disable ghost collision) |
| `Esc` | Quit (from menu) |

> [!NOTE]
> Movement controls (`W/A/S/D`) are defined as hackathon TODO items in the source code. You need to uncomment the relevant key-handling code in `scene_game.c` to enable them.

---

## 📁 Project Structure

```
Pac-Man/
├── README.md
└── Allegro_pacman/
    ├── Allegro_pacman.sln              # Visual Studio solution
    └── Allegro_pacman/
        ├── Allegro_pacman.vcxproj      # Project file
        ├── packages.config             # NuGet dependencies
        ├── Assets/                     # Game resources
        │   ├── title.png               # Title screen image
        │   ├── pacman_move.png         # Pac-Man movement sprite sheet
        │   ├── pacman_die.png          # Pac-Man death animation
        │   ├── ghost_move_red.png      # Red ghost (Blinky) sprite
        │   ├── ghost_move_blue.png     # Blue ghost (Inky) sprite
        │   ├── ghost_move_pink.png     # Pink ghost (Pinky) sprite
        │   ├── ghost_move_orange.png   # Orange ghost (Clyde) sprite
        │   ├── ghost_flee.png          # Ghost flee-mode sprite
        │   ├── ghost_dead.png          # Ghost dead sprite
        │   ├── pacman_tile.png         # Tile sprites
        │   ├── settings.png            # Settings button icons
        │   ├── map_nthu.txt            # Custom NTHU map layout
        │   ├── *.ttf                   # Fonts (Minecraft, OpenSans)
        │   └── Music/                  # Audio files (.ogg)
        │       ├── original_theme.ogg
        │       ├── pacman-chomp.ogg
        │       ├── pacman_death.ogg
        │       ├── pacman_eatfruit.ogg
        │       ├── pacman_eatghost.ogg
        │       └── pacman_intermission.ogg
        └── Src/                        # Source code
            ├── main.c                  # Entry point
            ├── game.c / game.h         # Core engine, event loop, Allegro init
            ├── scene_menu.c / .h       # Menu scene (title screen)
            ├── scene_game.c / .h       # Gameplay scene (main loop)
            ├── scene_settings.c / .h   # Settings scene (placeholder)
            ├── scene_menu_object.c / .h# UI button components
            ├── pacman_obj.c / .h       # Pac-Man entity (movement, eating, drawing)
            ├── ghost.c / ghost.h       # Ghost entity (AI states, drawing)
            ├── ghost_red_move_script.c # Blinky's movement AI script
            ├── map.c / map.h           # Map loading, rendering, BFS pathfinding
            ├── utility.c / utility.h   # Asset loading, collision, math helpers
            └── shared.c / shared.h     # Shared resources (fonts, audio, globals)
```

---

## 🗺️ Map Customization

Maps are stored as plain text files. The format is:

```
<rows> <columns>
<map data using character tiles>
```

### Tile Legend

| Character | Meaning |
|---|---|
| `#` | Wall |
| `.` | Bean (dot) |
| `P` | Power Pellet |
| `B` | Ghost Room |
| ` ` (space) | Empty corridor |

### Example (`map_nthu.txt`)

```
30 36
####################################
#..................###.........#####
#.####.###########.....#######.....#
...
####################################
```

To load a custom map, update the `init()` function in `scene_game.c`:

```c
// Replace:
basic_map = create_map(NULL);

// With:
basic_map = create_map("Assets/your_map.txt");
```

> [!TIP]
> The file-reading code in `map.c` has TODO placeholders for `fopen`/`fscanf`. You need to complete the file I/O implementation (hackathon exercise) before custom `.txt` maps will load correctly.

---

## 🏗️ Architecture Overview

```
┌─────────────┐
│   main.c    │──▶ game_create()
└─────────────┘
       │
       ▼
┌─────────────────────────────────────────┐
│              game.c (Engine)            │
│  • Allegro init (display, timers,       │
│    audio, keyboard, mouse)              │
│  • Event loop (60 FPS redraw +          │
│    game tick updates)                   │
│  • Scene switching via function ptrs    │
│  • Logging & error handling             │
└──────────────────┬──────────────────────┘
                   │
         ┌─────────┴──────────┐
         ▼                    ▼
┌──────────────┐    ┌──────────────────┐
│ scene_menu.c │    │  scene_game.c    │
│  Title + BGM │    │  Core Gameplay   │
│  Enter→Game  │    │  • Map + Pacman  │
│  Esc→Quit    │    │  • Ghosts + AI   │
└──────────────┘    │  • Collision     │
                    │  • Score + Items  │
                    └──────┬───────────┘
                           │
              ┌────────────┼────────────┐
              ▼            ▼            ▼
        ┌──────────┐ ┌──────────┐ ┌──────────┐
        │ pacman   │ │ ghost.c  │ │  map.c   │
        │ _obj.c   │ │ + ghost  │ │ Loading  │
        │Movement  │ │ _red_    │ │ Render   │
        │Eat/Die   │ │ move.c   │ │ BFS path │
        └──────────┘ │ AI FSM   │ └──────────┘
                     └──────────┘
```

The engine uses a **function-pointer-based scene system**: each `Scene` struct holds pointers to `initialize`, `update`, `draw`, `destroy`, and input callbacks. Swapping scenes is as simple as calling `game_change_scene()` with a new `Scene` struct.

---

## 📄 License

This project was created for educational purposes as part of the I2P(I) 2021 Fall course at National Tsing Hua University (NTHU).
