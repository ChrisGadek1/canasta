# Canasta - Card Game Implementation

## Project Overview

A 4-player Canasta card game implemented in C++ using Qt5 and OpenGL for rendering. One human player (Player 0, "Marek") plays against three AI-controlled players (Players 1-3: "Jedrek", "Andrzej", "Mariusz"). The project is work in progress: drawing, selecting, discarding and melding work for the human player; AI players play full turns with heuristic rules (see [AI players](#ai-players)).

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
| [`AiPlayer`](aiplayer.h) | [`aiplayer.h`](aiplayer.h), [`aiplayer.cpp`](aiplayer.cpp) | Plays the turns of a non-human player through the player-generic `Game` actions |
| [`AiProbability`](aiprobability.h) | [`aiprobability.h`](aiprobability.h), [`aiprobability.cpp`](aiprobability.cpp) | Per-decision snapshot of unseen cards for one AI player with draw / opponent-pair probabilities |
| [`CardMemory`](cardmemory.h) | [`cardmemory.h`](cardmemory.h), [`cardmemory.cpp`](cardmemory.cpp) | Public knowledge about the round: cards known to be in each hand (`knownInHand`) and values each player discarded (`discardedBy`) |
| [`Position`](position.h) | [`position.h`](position.h), [`position.cpp`](position.cpp) | Rectangle `left, down, top, right` in normalized coords |

### Enums

- [`CardValue`](cardvalue.h): `INVALID_CARD_VALUE = -1`, `TWO` ... `KING`, `ACE`, `JOKER`
- [`CardSuit`](cardsuit.h): `SPADES`, `HEARTS`, `DIAMONDS`, `CLUBS`
- [`CardsDeckType`](cardsdecktype.h): `PLAYERS_OWNED`, `HIDDEN`, `EXPOSED`

### Game State (`Game`)

- `players` - 4 `Player` objects, index == player number
- `decks` - order after `generateDecks()`: 4 `PLAYERS_OWNED` hands (players 0-3), then `EXPOSED`, then `HIDDEN`. Decks are looked up with `getHumanPlayerDeck()`, `getExposedDeck()`, `getHiddenDeck()` (`std::find_if` by type, throw `std::logic_error` if missing)
- `melds` - all melds of both partnerships
- `currentPlayerIndex` - player whose turn it is; advanced by `finishCurrentPlayerTurn()` (after a discard, or via `finishTurnIfCurrentPlayerHasNoCards()` after melding all hand cards) to the next player, with `currentPlayerHasDrawnThisTurn` reset. The current player's name is drawn in yellow
- `currentPlayerHasDrawnThisTurn` - set after drawing from the hidden deck (drawn red 3s are melded and replaced immediately in the same draw) or taking the exposed deck; blocks further draws in `drawFromHiddenDeck()`; required by `canCreateMeld()`, `canAddCardsToMeld()` and `canDiscard()`
- `teamHasInitialMeld`, `teamInitialMeldMinimumPoints`, `teamHasMeldedInPreviousTurns` - `std::array` per partnership, indexed by `getTeamIndex(playerNumber)` (`playerNumber % 2`)
- **Player-generic actions** (for any player, with an explicit card list; each checks `currentPlayerIndex` and the game rules itself): `drawFromHiddenDeck()`, `canTakeExposedDeck()`/`takeExposedDeck()`, `canCreateMeld()`/`createMeld()`, `canAddCardsToMeld()`/`addCardsToMeld()`, `canDiscard()`/`discard()`, plus team queries (`getTeamMeldsPoints()`, `hasTeamCanasta()`, `hasTeamInsufficientInitialMelds()`, `wouldFinishWithoutCanasta()`, ...). The human methods (`canHumanPlayer...`, `handleClickOn...`, `meldHumanPlayerChosenCards()`, `canAddCardsToMeld(meldIndex)`) are thin wrappers passing player `0` and `getHumanPlayerChosenCards()`. AI code should use the generic ones
- `canHumanPlayerDiscard()` allows a discard only after a draw, with exactly one card selected; discarding ends the turn
- `humanPlayerCardPage` - current page of the human hand

## Game Mechanics

### Deck Composition
108 cards ([`Game::generateDecks()`](game.cpp:58)): 4 suits × 13 values (TWO-ACE) × 2 = 104 cards + 4 jokers (2 × `DIAMONDS` = red, 2 × `SPADES` = black). Shuffled with `std::default_random_engine` seeded from `system_clock`.

### Initial Setup
- Each player receives 11 cards (dealt round-robin from the back of the shuffled vector)
- One card goes face-up to the exposed deck; while it is a red 3 (THREE of DIAMONDS/HEARTS) or a magic card, it stays in the exposed deck (freezing it) and another card is put on top
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
3. **Discard** - with exactly one selected card, click the exposed deck → `Game::handleClickOnExposedDeck()` → `discard()`; an empty exposed deck is clickable at `Game::exposedDeckPosition()`
3a. **Take exposed deck** - before drawing, click the exposed deck with a top card that is neither magic nor a black 3 (stop card) and either selected cards that contain at least two naturals of the top card's value and form a valid meld with it (a new meld must reach the initial meld minimum if the team has no initial meld yet), or no selected cards and an existing team meld of that value (not allowed when the deck is frozen - `isExposedDeckFrozen()`, i.e. contains a magic card or a red 3) → `takeExposedDeck()`: the top card (+ the pair) go to a new or existing team meld (`getTeamMeld()`), the rest of the pile goes to the hand; counts as the turn's draw
4. **Create meld** - "Melduj" button (enabled when `Game::canHumanPlayerMeld()`) → `MainGLWidget::clickOnMeldButton()` → `Game::meldHumanPlayerChosenCards()` → `createMeld()` creates a `Meld` owned by player 0 and partner 2 from the selected cards
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
5. **No black 3s** (`hasNoBlackThrees`) - black 3s can never be melded; a player has to discard them to empty the hand

Each failing check prints a `printf` debug message.

### Additional rules
- **Magic cards**: `TWO` and `JOKER` (`Meld::isMagicCard`)
- `Meld::addCard()` always re-sorts so magic cards come first (`moveMagicCardsToFront`)
- `Meld::canAddCards()` validates by building a temporary meld from existing + new cards
- Each partnership may have only one meld per value (`Game::hasMeldWithValue(value, player)` checks melds owned by the player)
- Meld base value = value of the first non-magic selected card, or `JOKER` for a meld of only magic cards (`Meld::baseValue()`)
- **Magic cards meld** - a meld of only 2s and jokers is valid only as a complete canasta (at least 7 cards, laid down at once); more magic cards can be added later. Scores 700 (`Meld::isMagicCardsCanasta()`)
- **Canasta** - meld of at least `Meld::CANASTA_SIZE` (7) cards (`Meld::isCanasta()`). Emptying the hand (finishing) is allowed only if the team has a canasta or the move completes one; otherwise the move is blocked and `MainGLWidget::showNoCanastaDialog()` is shown. Checked by `Game::wouldMeldFinishWithoutCanasta()` ("Melduj" with all hand cards), `wouldAddingCardsToMeldFinishWithoutCanasta()` (adding all hand cards to a meld) and `wouldClickOnExposedDeckFinishWithoutCanasta()` (discarding the last card, or taking the exposed deck with all hand cards when the rest of the pile is only red 3s or empty)
- **Initial meld**: until the team's `teamHasInitialMeld` is set, the team's melds must together reach its `teamInitialMeldMinimumPoints` (computed for both teams in `initialize()` from the team's total in `results.json`: < 0 → 15, < 1500 → 50, < 3000 → 90, else 120; `Meld::getPoints()` sums `Card::getPoints()`). Discarding with insufficient melds opens a modal (`MainGLWidget::showInsufficientInitialMeldsDialog()`) with "Usuń ogony" (`Game::removeHumanTeamMelds()` returns the cards to the hand) and "Anuluj". The flag is set when a turn ends with team melds on the table or after taking the exposed deck. Finishing the turn by melding all hand cards (`wouldMeldingChosenCardsFinishWithInsufficientInitialMelds()`) also requires the minimum
- **Round end** - when the current player's hand is empty at the end of a turn, `finishCurrentPlayerTurn()` sets `roundFinishingPlayerNumber` (`isRoundFinished()`) and `roundFinishedConcealed` (no team melds in previous turns). `MainGLWidget::mousePressEvent()` then shows `showRoundEndDialog()` ("Koniec rundy", per-team breakdown from `Game::calculateTeamRoundScore()`: magic cards canasta 700 / natural 500 / mixed 300, red 3s 100 each or 800 for all four (negative without melds), going out 100 / concealed 200, melded cards, minus hand cards). Closing it calls `Game::finishRound()` (appends the totals to `results.json`, deletes all cards and re-runs `initialize()`) and reloads card textures (`loadCardsTextures()`, needs `makeCurrent()`)
- **Empty hidden deck** - `roundFinishedByEmptyHiddenDeck` (also `isRoundFinished()`, no going-out bonus) is set by `finishCurrentPlayerTurn()` when a card is discarded while the hidden deck is empty, or by `handleClickOnHiddenDeck()` when the last hidden card drawn is a red 3
- Melds are owned by a partnership: `owners = {player, partner}`; `Meld::isOwnedBy()` compares player numbers

## AI players

- `MainGLWidget` keeps one `AiPlayer` per player (`aiPlayers`, index == player number). After every human click (`mousePressEvent()`) and every AI step, `scheduleAiStep()` starts a `QTimer::singleShot(AI_STEP_DELAY_MS)` if the current player is not human; `playAiStep()` calls `AiPlayer::startTurn()` (when `!isPlayingTurn()`) and `AiPlayer::step()`, repaints and shows the round end dialog when needed
- One `step()` = one visible action. Turn phases (`AiTurnPhase`): `DRAW` → `MELD` (repeated while a meld action succeeds) → `DISCARD` → `DONE` (also set when the round finishes or the turn passes on)
- `AiStrategy` is chosen in `startTurn()` and again after drawing (first match): `DEFENSIVE` (an opponent has ≤ 3 cards, the partner has ≤ 2 cards and the team has a canasta, or the hidden deck has < 10), `OPENING` (no team initial meld), `GOING_OUT` (team canasta and ≤ 6 cards in hand), `BUILDING`. `DEFENSIVE` and `GOING_OUT` dump cards (`canDumpCards()`)
- All weights are `static constexpr` members of `AiPlayer`
- **Draw** (`playDrawPhase()`): candidates for taking the exposed deck are no own cards (team meld exists, pile not frozen), all naturals of the top value, and - without a team initial meld - those plus up to 2 wild cards; a candidate is skipped when it is illegal, would finish without a canasta or leave < 2 cards without a canasta. `scoreTakingExposedDeck()`: 15 per non-red-3 pile card, +40 per useful card (top card, red 3, a value with a team meld or ≥ 2 naturals after taking), +25 per wild card, −10 per dead junk card, +60 when it makes the initial meld, +50 when the meld reaches a canasta, −30 per own wild card used, ×0.5 in `GOING_OUT` with mostly junk. Takes the pile when the pile has ≥ 7 cards or the score beats `AiProbability::expectedDrawGain()`, otherwise draws from the hidden deck
- **Opening** (`planOpeningMelds()`, after the draw while the team is below the minimum): greedily ≥ 3 naturals by points, then pairs + 1 wild card, then extra wild cards into chosen melds (≤ 2 wild cards in total), keeping 2 cards in hand; played meld by meld only if the whole plan reaches the minimum
- **Melding** (`findMeldActions()` + `scoreMeldAction()`, best action per step): naturals to team melds; wild cards to a team meld only to complete a canasta now, or any valid amount when dumping; new melds from ≥ 3 naturals, or a pair + wild card when dumping; a magic cards canasta from ≥ 7 wild cards. Score = card points (×2 in `DEFENSIVE`) + 10 × resulting meld size + 700 / 500 / 300 when it completes a canasta − 20 per wild card in `BUILDING`. Keeps at least 2 cards in hand (1 with a team canasta while dumping, so the discard goes out)
- **Discard** (`playDiscardPhase()`): the card with the lowest keep value + danger. Keep value: black 3 0, wild card 100 (−50 when freezing), pair 30 + 15 per extra card, single 10, dead single 0, minus points / 2 in `DEFENSIVE`. Danger for the next player: 200 if their team has that meld and the pile is not frozen, 150 / 60 for 2 / 1 known cards, otherwise `opponentHasPairProbability()` × 100; −30 if they discarded that value, ×0.5 when frozen, × (1 + pile size / 5). A wild card freezes the pile (`shouldFreezeExposedDeck()`) when it has ≥ 5 cards, is not frozen, the hand has no black 3 and every natural card's danger is ≥ 50
- Every decision is logged with `printf` (`P<number> ...`)
- **Card memory** - `Game::cardMemory` (`getCardMemory()`, reset in `initialize()`) is updated only from public events: `takeExposedDeck()` (pile cards go to `knownInHand`, red 3s skipped), `removeHumanTeamMelds()` (returned cards), `createMeld()`/`addCardsToMeld()`/`takeExposedDeck()` (melded hand cards are forgotten), `discard()` (forgotten + `discardedBy`). `Game::getUnseenCardsCounts(playerNumber)` returns, per `CardValue` index, how many cards the player cannot see: 8 per value (4 jokers) minus own hand, exposed pile, all melds and other players' `knownInHand`. AI code must use only these and never read other players' hands
- **Probabilities** - `AiProbability(game, playerNumber)` is built per decision (cheap, 14 values): `drawProbability(v)` = unseen[v] / total unseen; `opponentHasPairProbability(opp, v)` = 1 with 2 known cards, otherwise a binomial estimate over the opponent's unknown hand cards (×0.3 if they discarded `v`); `cardGain(v)` (0 for threes, 25 magic, 40 extends a team meld, 30 makes a pair, 5 single); `redThreeDrawProbability()` (4 minus red 3s visible in melds and the exposed pile, over total unseen); `expectedDrawGain()` = Σ drawProbability × cardGain + redThreeDrawProbability × 50 (logged with the draw); `isDeadValue(v)` - a single natural in hand with no unseen cards left

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
- Cards freezing the exposed deck (`Card::freezesExposedDeck()` - magic cards and red 3s) are placed horizontally (`Game::exposedDeckPosition(true)`) and drawn with angle 90, so they stick out from under the pile

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

Call `initializeCardsPositions()` / `initializeMeldCardPositions()` after any change of hand or meld contents, then `update()` on the widget. `initializeCardsPositions()` also sorts every hand (`CardsDeck::sortCards()`: aces down to threes, then twos, then jokers; ties by suit). After drawing from the hidden deck, `humanPlayerCardPage` switches to the page with the drawn card.

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
├── aiplayer.*           # AI turn phases, strategy modes and move rules
├── aiprobability.*      # Probabilities of AI decisions (unseen cards, draws, opponents' pairs)
├── cardmemory.*         # Publicly known cards in hands and discards (used by the AI)
├── position.*           # Rectangle + coordinate conversion
├── roundresult.h        # Points of both teams in one round
├── teamroundscore.h     # One team's round score breakdown (canastas, red 3s, going out, melded cards, hand cards)
├── results.json         # Round results shown in the "Punkty" popup (read by Game::loadRoundResults(), appended by Game::finishRound(); working directory)
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

- AI difficulty levels and weight tuning (weights are constants in `AiPlayer`)
- End of the whole game (5000 points), dealer rotation (player 0 always starts)

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
