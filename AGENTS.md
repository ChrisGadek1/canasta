# Canasta - Card Game Implementation

## Project Overview

A 4-player Canasta card game implemented in C++ using Qt5 and OpenGL for rendering. The game supports one human player (Player 0, "Marek") and three AI-controlled opponents (Players 1-3: "Jedrek", "Andrzej", "Mariusz").

## Technology Stack

- **Language**: C++11
- **Framework**: Qt 5.6.3 (Core, Gui, Widgets, OpenGL)
- **Rendering**: OpenGL (legacy fixed-function pipeline via QGLWidget)
- **Build Systems**: 
  - CMake ([`CMakeLists.txt`](CMakeLists.txt))
  - qmake ([`canasta.pro`](canasta.pro))

## Architecture

### Core Classes

| Class | File | Description |
|-------|------|-------------|
| [`Card`](card.h) | [`card.h`](card.h:11), [`card.cpp`](card.cpp) | Represents a playing card with value, suit, position, and OpenGL texture |
| [`Player`](player.h) | [`player.h`](player.h:7) | Player entity (human or AI-controlled) |
| [`CardsDeck`](cardsdeck.h) | [`cardsdeck.h`](cardsdeck.h:9) | Collection of cards belonging to a player or game area |
| [`Meld`](meld.h) | [`meld.h`](meld.h:8) | Valid combination of cards placed on the table |
| [`Game`](game.h) | [`game.h`](game.h:9), [`game.cpp`](game.cpp) | Main game logic and state management |
| [`MainGLWidget`](mainglwidget.h) | [`mainglwidget.h`](mainglwidget.h:10), [`mainglwidget.cpp`](mainglwidget.cpp) | OpenGL rendering widget handling display and input |
| [`Position`](position.h) | [`position.h`](position.h:4) | 2D position representation for card placement |

### Enums

- [`CardValue`](cardvalue.h:4): TWO through ACE, plus JOKER
- [`CardSuit`](cardsuit.h:4): SPADES, HEARTS, DIAMONDS, CLUBS
- [`CardsDeckType`](cardsdecktype.h:4): PLAYERS_OWNED, HIDDEN, EXPOSED

## Game Mechanics

### Deck Composition (108 cards)
- Two copies of each card from 2 through King (4 suits × 13 values × 2 = 104 cards)
- Four jokers (2 red, 2 black)

### Initial Setup
- Each player receives 11 cards
- One card placed in exposed deck (cannot be a 3 of diamonds/hearts)
- Remaining cards form the hidden deck

### Player Positions
- **Player 0** (Human): Bottom of screen
- **Player 1**: Left side
- **Player 2**: Top
- **Player 3**: Right side

## Key Features

1. **Card Selection**: Click on own cards to select/deselect for melding
2. **Drawing Cards**: Click hidden deck to draw a card
3. **Melding**: Place valid combinations (same-value cards) on the table
4. **OpenGL Rendering**: All cards rendered with textures from [`images/cards/`](images/cards/)

## Meld Validation Rules

A valid meld must satisfy all of the following conditions:

1. **All non-magic cards have the same value** - Non-magic cards (3 through King) must share the same card value
2. **At least 2 non-magic cards** - The meld must contain at least 2 non-magic cards
3. **At least 3 total cards** - The combined count of magic and non-magic cards must be at least 3
4. **Magic cards not exceeding non-magic cards** - The number of magic cards (TWO or JOKER) cannot exceed the number of non-magic cards

### Magic Cards
- **TWO** (2): Can be used as a wildcard to substitute any card value
- **JOKER**: Universal wildcard that can substitute any card

See [`Meld::valid()`](meld.cpp:20-25) for implementation details.

## Build Instructions

### Using CMake
```bash
cmake -B build
cmake --build build
```

### Using qmake
```bash
qmake canasta.pro
mingw32-make
```

## File Structure

```
canasta/
├── CMakeLists.txt       # CMake build configuration
├── canasta.pro          # qmake project file
├── main.cpp             # Application entry point
├── mainwindow.*         # Qt MainWindow wrapper
├── mainglwidget.*       # OpenGL rendering widget
├── game.*               # Game logic
├── card.*               # Card representation
├── player.h             # Player class
├── cardsdeck.*          # Deck management
├── meld.*               # Meld (card combination) handling
├── position.*           # Position utilities
├── cardvalue.h          # Card value enum
├── cardsuit.h           # Card suit enum
├── cardsdecktype.h      # Deck type enum
├── resources.qrc        # Qt resource file
└── images/
    ├── background.png   # Table background
    └── cards/           # Card face textures
```

## Important Notes for Development

1. **Memory Management**: Cards are dynamically allocated with raw pointers; deletion handled in [`Card`](card.h:23) destructor via `glDeleteTextures`
2. **Coordinate System**: Normalized coordinates (-1 to 1), converted to pixels via [`Position::toPx()`](position.h:12)
3. **Card Linking**: Cards maintain `above` and `behind` pointers for stacking visualization
4. **Texture Loading**: Card textures follow naming pattern `{value}_of_{suit}.png` (see [`Card::texturePath()`](card.h:26))
