# Watopoly 🎓🎲

A University of Waterloo themed Monopoly-style board game implemented in C++20.

## 📋 Description

Watopoly is a digital recreation of the classic Monopoly board game with a unique University of Waterloo twist. Players move around a board featuring iconic campus locations and academic buildings, buying properties, collecting rent, and trying to bankrupt their opponents.

The game features familiar UW locations like:
- **Arts Buildings**: AL (Arts Lecture), ML (Modern Languages), ECH, PAS, HH
- **Engineering**: RCH, DWE, CPH  
- **Health Sciences**: LHI, BMH, OPT
- **Environment**: EV1, EV2, EV3
- **Science**: PHYS, B1, B2, EIT, ESC, C2
- **Mathematics**: MC (Math & Computer), DC (Davis Centre)

## ✨ Features

- 🎮 **2-6 Player Support**: Play with friends locally
- 🎯 **Testing Mode**: Debug and test game mechanics
- 💾 **Save/Load Games**: Continue your games later
- 🎲 **Dice Rolling**: Realistic dice mechanics with doubles detection
- 🏠 **Property Management**: Buy, improve, and collect rent on properties
- 🎪 **Multiple Game Pieces**: Choose from Goose, GRT Bus, Tim Hortons Doughnut, Professor, Student, Money, Laptop, or Pink Tie
- 🏛️ **Academic Building System**: Build improvements on your properties to increase rent

## 🔧 Requirements

- **Compiler**: GCC with C++20 support and modules (`-fmodules-ts`)
- **OS**: Linux/Unix-like system (tested on Ubuntu/Debian)
- **Make**: GNU Make build system

## 🚀 Build Instructions

### Method 1: Using Make
```bash
cd watopoly/
make clean
make
```

### Method 2: Using the Compile Script
```bash
cd watopoly/
chmod +x compile
./compile
```

### Method 3: Manual Compilation
```bash
cd watopoly/
g++ -std=c++20 -fmodules-ts -Wall -g -c Declarations.cc
g++ -std=c++20 -fmodules-ts -Wall -g -c Implementations.cc  
g++ -std=c++20 -fmodules-ts -Wall -g -c harness.cc
g++ *.o -o watopoly
```

## 🎮 How to Play

### Starting a New Game
```bash
./watopoly
```
You'll be prompted to enter the number of players (2-6).

### Testing Mode
```bash
./watopoly -testing
```
Enables debugging features and controlled dice rolling.

### Loading a Saved Game
```bash
./watopoly -load <filename>
```
Example: `./watopoly -load save1.txt`

### Quick Start with Make
```bash
make run        # Start new game
make test       # Start in testing mode  
make load       # Load a saved game (prompts for filename)
```

## 🎯 Game Rules

Watopoly follows classic Monopoly rules with UW theming:

1. **Movement**: Roll dice to move around the board
2. **Properties**: Land on unowned properties to purchase them
3. **Rent**: Pay rent when landing on opponents' properties  
4. **Monopolies**: Own all properties in a color group to build improvements
5. **Improvements**: Build up to 5 levels on monopolized properties
6. **Special Squares**: 
   - **Goose Nesting**: Start position
   - **GO TO TIMS**: Go directly to Tim Hortons
   - **CIF**: Chance-style events
   - **SLC**: Student Life Centre events

## 📁 File Structure

```
watopoly/
├── Declarations.cc      # Class declarations and interfaces
├── Implementations.cc   # Game logic implementations  
├── harness.cc          # Main program and game loop
├── Makefile            # Build configuration
├── compile             # Alternative build script
├── board.txt           # ASCII art game board layout
├── boardTileOrder.txt  # Board tile sequence configuration
├── watopoly_data.csv   # Property data (costs, rents, etc.)
├── pieces.csv          # Available game pieces
├── save1.txt          # Example save file
├── saved_game.txt     # Another save file
└── safeFile.txt       # Backup save file
```

## 🏗️ Architecture

The game uses modern C++20 modules with the following key components:

- **Game**: Main game controller and state management
- **Player**: Player data and actions
- **Tile/Property**: Board position implementations  
- **AcademicBuilding**: Improvable properties with rent progression
- **Bank**: Game economy management
- **Dice**: Random number generation with testing support
- **CommandInterpreter**: User input processing


## 🐛 Troubleshooting

### Compilation Issues
- Ensure you have GCC 11+ with C++20 modules support
- Try the alternative `./compile` script if Make fails
- Check that all source files are present

### Runtime Issues  
- Verify all data files (CSV, TXT) are in the same directory as the executable
- Check file permissions for save/load operations
- Use `-testing` mode to debug game logic

## 📜 License

This project is an educational implementation created for learning purposes.

## 🎓 Credits

Created as a University of Waterloo computer science project, featuring beloved campus locations and UW culture.

---

*Ready to dominate the University of Waterloo campus? Good luck, and may the best Warrior win!* 🦆
