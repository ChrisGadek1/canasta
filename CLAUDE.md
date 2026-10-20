# Canasta - Card Game Implementation

## Project Overview

A 4-player Canasta card game implemented in C++ using Qt5 and OpenGL for rendering. One human player (Player 0, "Marek") plays against three AI-controlled players (Players 1-3: "Jedrek", "Andrzej", "Mariusz"). The project is work in progress: drawing, selecting, discarding and melding work for the human player; AI turns, turn rotation and scoring are **not implemented yet**.

## Technology Stack

- **Language**: C++11
- **Framework**: Qt 5.6.3 (Core, Gui, Widgets, OpenGL), MinGW 4.9 32-bit
- **Rendering**: OpenGL legacy fixed-function pipeline (`glBegin`/`glEnd`, `GL_QUADS`) via `QGLWidget`; text via `QGLWidget::renderText`
- **Build Systems**:
  - CMake ([`CMakeLists.txt`](CMakeLists.txt)) - hardcodes `CMAKE_PREFIX_PATH` to `D:\QT_5.6.3\5.6.3\mingw49_32`, Win32 platform, links `opengl32`
  - qmake ([`canasta.pro`](canasta.pro)) - links `-lopengl32`
- **Platform**: Windows (`windows.h` included under `_WIN32`)

## Architecture

### Core Classes

| Class | File | Description |
|-------|------|-------------|
| [`Card`](card.h) | [`card.h`](card.h), [`card.cpp`](card.cpp) | Card value, suit, `Position`, OpenGL texture, `chosen` flag, `above`/`behind` links |
| [`Player`](player.h) | [`player.h`](player.h), [`player.cpp`](player.cpp) | Name, number (0-3), `isHumanControlled` |
| [`CardsDeck`](cardsdeck.h) | [`cardsdeck.h`](cardsdeck.h), [`cardsdeck.cpp`](cardsdeck.cpp) | `std::vector<Card*>` with a `CardsDeckType` and (for hands) an owning `Player` |
| [`Meld`](meld.h) | [`meld.h`](meld.h), [`meld.cpp`](meld.cpp) | Cards on the table with a base `CardValue` and a list of owners (a partnership) |
| [`Game`](game.h) | [`game.h`](game.h), [`game.cpp`](game.cpp) | Game state, dealing, card layout, hit-testing, draw/discard/meld logic, hand pagination |
| [`MainGLWidget`](mainglwidget.h) | [`mainglwidget.h`](mainglwidget.h), [`mainglwidget.cpp`](mainglwidget.cpp) | Owns `Game`; renders everything, handles mouse input, buttons, meld creation |
| [`MainWindow`](mainwindow.h) | [`mainwindow.h`](mainwindow.h), [`mainwindow.cpp`](mainwindow.cpp), [`mainwindow.ui`](mainwindow.ui) | `QMainWindow` that sets `MainGLWidget` as its central widget |
| [`Position`](position.h) | [`position.h`](position.h), [`position.cpp`](position.cpp) | Rectangle `left, down, top, right` in normalized coords |

### Enums

- [`CardValue`](cardvalue.h): `INVALID_CARD_VALUE = -1`, `TWO` ... `KING`, `ACE`, `JOKER`
- [`CardSuit`](cardsuit.h): `SPADES`, `HEARTS`, `DIAMONDS`, `CLUBS`
- [`CardsDeckType`](cardsdecktype.h): `PLAYERS_OWNED`, `HIDDEN`, `EXPOSED`

### Game State (`Game`)

- `players` - 4 `Player` objects, index == player number
- `decks` - order after `generateDecks()`: 4 `PLAYERS_OWNED` hands (players 0-3), then `EXPOSED`, then `HIDDEN`. Decks are looked up with `getHumanPlayerDeck()`, `getExposedDeck()`, `getHiddenDeck()` (`std::find_if` by type, throw `std::logic_error` if missing)
- `melds` - all melds of both partnerships
- `currentPlayerIndex` - always 0 at the moment (never advanced)
- `humanPlayerHasDrawnThisTurn` - set after drawing a card other than a red 3 (a drawn red 3 is melded and another card may be drawn); blocks further draws in `handleClickOnHiddenDeck()`; required by `canHumanPlayerMeld()`, `canAddCardsToMeld()` and `canHumanPlayerDiscard()`
- `humanPlayerHasDiscardedThisTurn` - set after discarding (never reset yet); `canHumanPlayerDiscard()` allows a discard only after a draw, once, with exactly one card selected
- `humanPlayerCardPage` - current page of the human hand

## Game Mechanics

### Deck Composition
108 cards ([`Game::generateDecks()`](game.cpp:58)): 4 suits × 13 values (TWO-ACE) × 2 = 104 cards + 4 jokers (2 × `DIAMONDS` = red, 2 × `SPADES` = black). Shuffled with `std::default_random_engine` seeded from `system_clock`.

### Initial Setup
- Each player receives 11 cards (dealt round-robin from the back of the shuffled vector)
- One card goes face-up to the exposed deck; while it is a red 3 (THREE of DIAMONDS/HEARTS), it stays in the exposed deck and another card is put on top
- Remaining cards form the hidden deck

### Player Positions and Partnerships
| Player | Name | Screen side | Hand rendered | Partner |
|--------|------|-------------|---------------|---------|
| 0 | Marek (human) | Bottom | Face up, angle 0, paginated | 2 |
| 1 | Jedrek | Left | Reverse, angle 270 | 3 |
| 2 | Andrzej | Top | Reverse, angle 180 | 0 |
| 3 | Mariusz | Right | Reverse, angle 90 | 1 |

Partner of player `n` is `(n + 2) % 4`.

### Human Player Actions ([`MainGLWidget::mousePressEvent()`](mainglwidget.cpp:193))
1. **Select card** - click a visible own card; `Card::setChosen(0)` toggles `chosen` and shifts it up by `0.06`
2. **Draw** - click hidden deck → `Game::handleClickOnHiddenDeck()` moves the top card (`back()`) to the human hand
3. **Discard** - with exactly one selected card, click the exposed deck → `Game::handleClickOnExposedDeck()`
4. **Create meld** - "Melduj" button (enabled when `Game::canHumanPlayerMeld()`) → `MainGLWidget::clickOnMeldButton()` creates a `Meld` owned by player 0 and partner 2 from the selected cards
5. **Add to meld** - click an own-team meld with selected cards → `Game::canAddCardsToMeld()` / `Game::addCardsToMeld()`
6. **Paginate hand** - arrows shown when hand > `Game::CARDS_PER_PAGE` (15)
7. **Flip melds view** - flip button toggles `showOpponentsMelds` (own team vs. opponents' melds)

All actions only work when `currentPlayerIndex == 0`. Selection state spans all pages (checks iterate the full hand, not only visible cards).

## Meld Rules

### Validation ([`Meld::valid()`](meld.cpp:30))
1. **All non-magic cards have the same value** (`allNonMagicCardsHaveTheSameValue`)
2. **At least 2 non-magic cards** (`hasAtLeastTwoNonMagicCards`)
3. **At least 3 total cards** (`hasAtLeastThreeTotalCards`)
4. **Magic cards not exceeding non-magic cards** (`magicCardsNotExceedingNonMagicCards`)

Each failing check prints a `printf` debug message.

### Additional rules
- **Magic cards**: `TWO` and `JOKER` (`Meld::isMagicCard`)
- `Meld::addCard()` always re-sorts so magic cards come first (`moveMagicCardsToFront`)
- `Meld::canAddCards()` validates by building a temporary meld from existing + new cards
- Each partnership may have only one meld per value (`Game::hasMeldWithValue(value, player)` checks melds owned by the player)
- Meld base value = value of the first non-magic selected card
- Melds are owned by a partnership: `owners = {player, partner}`; `Meld::isOwnedBy()` compares player numbers

## Rendering

### Coordinate System
- OpenGL ortho projection `(-1, 1, -1, 1)`; all layout in normalized coords, origin at center, Y up
- **`Position` constructor order is `(left, down, top, right)`** - not the usual `(l, t, r, b)`
- `Position::toPx(left, down, top, right, vw, vh)` converts to Qt pixel coords (Y flipped, so in px `down > top`)
- Hit-test pattern used everywhere: `pos.down > y && pos.top < y && pos.left < x && pos.right > x`
- `Position::rotateCW90()` - used to draw opponents' melds rotated

### Card Geometry
- `Card::cardHeight = 0.3`, `Card::ratio = 0.68870523416` (width = height × ratio)
- `Card::draw(reverse, angleDegrees)` - draws a textured quad at `position`, rotating **texture coordinates** (not vertices) by the angle; `reverse` uses the shared `Card::reverseTextureId`
- For side players (1, 3) position rectangles are already horizontal; rotation is applied only to the texture

### Layout ([`Game::initializeCardsPositions()`](game.cpp:399), [`Game::initializeMeldCardPositions()`](game.cpp:326))
| Element | Location |
|---------|----------|
| Human hand | y `-0.9`..`-0.6`, spread x `-0.6`..`0.6` |
| Partner hand (2) | y `0.6`..`0.9`, spread x `-0.6`..`0.6` |
| Left/right hands (1/3) | x `-0.9`..`-0.6` / `0.6`..`0.9`, spread y `-0.6`..`0.6` |
| Hidden deck | top-left, right edge `-0.65`, bottom `0.65` |
| Exposed deck | top-right, left edge `Game::exposedLeftPos = 0.65`, bottom `Game::exposedBottomPos = 0.65` |
| Melds | columns from x `-0.5`, cards stacked upward from y `-0.5` with `0.1` gap |
| UI buttons | constants in [`mainglwidget.h`](mainglwidget.h): `MELDING_BUTTON_*` (bottom right), `POINTS_BUTTON_*` (bottom left), `FLIP_BUTTON_*`, `ARROW_*`, `PLAYER_NAME_*` |

Call `initializeCardsPositions()` / `initializeMeldCardPositions()` after any change of hand or meld contents, then `update()` on the widget.

### Paint order ([`MainGLWidget::paintGL()`](mainglwidget.cpp:424))
Background → decks (hands, exposed, hidden) → melds (own team or opponents, depending on `showOpponentsMelds`) → arrows → flip button → "Melduj" button → "Punkty" button → player names. Buttons are drawn with `MainGLWidget::drawButton()`.

### Input feedback
`mouseMoveEvent` / `mouseReleaseEvent` change the cursor (`QApplication::setOverrideCursor`) to a pointing hand over interactive elements and change the meld button color (`setMeldButtonColor`, RGB 0-255).

## Resources

- [`resources.qrc`](resources.qrc) with prefix `/images`, so paths are `:/images/images/<file>` (e.g. `:/images/images/cards/ace_of_hearts.png`)
- [`images/`](images/): `background.png`, `arrow_left.png`, `arrow_right.png`, `flip.png`
- [`images/cards/`](images/cards/): `{value}_of_{suit}.png` (value: `2`-`10`, `jack`, `queen`, `king`, `ace`), `black_joker.png`, `red_joker.png`, `card_reverse.png`; `*2.png` variants exist but are unused
- Jokers: black if suit is SPADES/CLUBS, red otherwise ([`Card::texturePath()`](card.cpp:16))
- New images must be added to `resources.qrc` (and to the resource list in `CMakeLists.txt`)
- Textures are loaded in `MainGLWidget::initializeGL()` (requires a GL context); every card calls `Card::loadTexture()`

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

New source files must be added to **both** `CMakeLists.txt` (`add_executable`) and `canasta.pro` (`SOURCES`/`HEADERS`).

## File Structure

```
canasta/
├── CMakeLists.txt       # CMake build configuration
├── canasta.pro          # qmake project file
├── main.cpp             # Application entry point
├── mainwindow.*         # Qt MainWindow wrapper (+ mainwindow.ui)
├── mainglwidget.*       # OpenGL rendering widget, input handling, UI buttons
├── game.*               # Game logic, layout, pagination
├── card.*               # Card representation and drawing
├── player.*             # Player class
├── cardsdeck.*          # Deck management
├── meld.*               # Meld handling and validation
├── position.*           # Rectangle + coordinate conversion
├── roundresult.h        # Points of both teams in one round
├── results.json         # Round results shown in the "Punkty" popup (read by Game::loadRoundResults() from the working directory)
├── cardvalue.h          # Card value enum
├── cardsuit.h           # Card suit enum
├── cardsdecktype.h      # Deck type enum
├── resources.qrc        # Qt resource file
└── images/
    ├── background.png   # Table background
    ├── arrow_left.png, arrow_right.png, flip.png
    └── cards/           # Card face textures and card_reverse.png
```

## Code Conventions

- Class per file pair (`name.h` / `name.cpp`), lowercase filenames, `#ifndef NAME_H` include guards
- camelCase for methods and members, PascalCase for classes, UPPER_SNAKE_CASE for enum values and UI constants
- Frequent explicit `this->` member access
- Short getters/setters and constructors defined inline in headers
- Mouse hit-test methods named `mouseOver<Element>(...)`, click handlers `handleClickOn<Element>()` / `clickOn<Element>()`
- Debug output via `printf` (game logic) and `qDebug()` (Qt/texture loading)
- Some strings and comments are in Polish ("Melduj" button, texture load error, layout comments like `dół`, `góra`)

## Not Implemented Yet

- AI logic, turn rotation (`currentPlayerIndex` stays 0; `resetHumanPlayerDrawFlag()` is never called, so after one draw the human cannot draw again)
- Scoring, canastas, red-3 handling, picking up the discard pile

## Important Notes for Development

1. **Memory Management**: Cards are dynamically allocated raw pointers shared between decks and melds; `Game::~Game()` deletes all cards (non-copyable `Game`), and [`Card`](card.h) destructor frees its texture. `MainGLWidget::~MainGLWidget()` calls `makeCurrent()` so GL textures are freed with a valid context
2. **Coordinate System**: Normalized coordinates (-1 to 1), converted to pixels via [`Position::toPx()`](position.cpp:3)
3. **Card Linking**: Cards maintain `above` and `behind` pointers for stacking visualization (set in hands, exposed deck and melds)
4. **Texture Loading**: Card textures follow naming pattern `{value}_of_{suit}.png` (see [`Card::texturePath()`](card.cpp:16))
5. **Human-only checks**: many methods hardcode player number `0` as the human player

## Important Notes for Agents

1. Agents should not try to run this game, because it's not a terminal based game
2. Agents should ALWAYS try to follow existing conventions in the project, regarding variable names, functions, creating logic etc.
3. After adding changes, agents should ALWAYS get through the changes they made and check if they're meeting requirements and don't need to be refactored
4. Agents should NOT add unnecessary comments to the code and only add them, when the code is not self-describing, which should be an EXCEPTION
5. Agents should be as precise as possible - not writing too much in responses to the user and always pointing the most important part of a problem
6. If a work could be split into smaller tasks, then these tasks should be run by separate agents (using **Agent** tool)
7. If agent finds it useful, may create a new file for specific functions instead of editing existing
