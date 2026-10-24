# AI implementation plan: scored moves (#2), turn phases (#4), probabilities (#5), card memory (#6)

The plan has 6 steps. Step 1 makes the game logic work for any player; steps 2–6 add the AI on top. Each step can be a separate task or agent.

---

## Step 1: Make `Game` work for any player (required first)

Today most game logic only works for player 0 or reads the `chosen` flags. The AI needs the same actions with an explicit player and explicit card list.

**State changes in `game.h`**

| Now | After |
|---|---|
| `humanPlayerHasDrawnThisTurn` | `currentPlayerHasDrawnThisTurn` |
| `humanTeamHasInitialMeld` | `std::array<bool, 2> teamHasInitialMeld` (index = `playerNumber % 2`) |
| `humanTeamInitialMeldMinimumPoints` | `std::array<int, 2> teamInitialMeldMinimumPoints` |
| `humanTeamHasMeldedInPreviousTurns` | `std::array<bool, 2> teamHasMeldedInPreviousTurns` |

**New actions, each taking a player and an explicit card list:**

```cpp
CardsDeck& getPlayerDeck(int playerNumber);
void drawFromHiddenDeck(int playerNumber);                  // red 3 handling moves here
bool canTakeExposedDeck(int playerNumber, const std::vector<Card*>& cards);
void takeExposedDeck(int playerNumber, const std::vector<Card*>& cards);
bool canCreateMeld(int playerNumber, const std::vector<Card*>& cards);
void createMeld(int playerNumber, const std::vector<Card*>& cards);   // moved from MainGLWidget::clickOnMeldButton
bool canAddCardsToMeld(int playerNumber, int meldIndex, const std::vector<Card*>& cards) const;
void addCardsToMeld(int playerNumber, int meldIndex, const std::vector<Card*>& cards);
bool canDiscard(int playerNumber, Card* card);
void discard(int playerNumber, Card* card);
int getTeamMeldsPoints(int playerNumber) const;
bool hasTeamCanasta(int playerNumber) const;
bool wouldFinishWithoutCanasta(int playerNumber, int remainingCardsCount, int resultingMeldSize) const;
```

**Rewire the existing code:**

- The current `canHumanPlayer…`, `handleClickOn…` and `wouldClickOn…` methods become thin wrappers that call these actions with `0` and `getHumanPlayerChosenCards()`. This keeps `MainGLWidget` almost unchanged.
- `finishCurrentPlayerTurn()`:
  - Remove the loop that skips AI players; just go to `(currentPlayerIndex + 1) % 4`.
  - Set `teamHasInitialMeld` and `teamHasMeldedInPreviousTurns` for the **current player's** team.
  - Compute `roundFinishedConcealed` from that team's flag.
- `initialize()` computes the initial meld minimum for **both** teams.
- Only `drawFromHiddenDeck()` for player 0 updates `humanPlayerCardPage`.

---

## Step 2: Turn driver and strategy modes (#4)

**New files `aiplayer.h/.cpp`** (add them to `CMakeLists.txt` and `canasta.pro`):

```cpp
enum class AiTurnPhase { DRAW, MELD, DISCARD, DONE };
enum class AiStrategy { OPENING, BUILDING, GOING_OUT, DEFENSIVE };

class AiPlayer {
    int playerNumber;
    AiTurnPhase phase;
    AiStrategy strategy;
    AiWeights weights;
    void chooseStrategy(Game& game);
    void playDrawPhase(Game& game);
    bool playMeldStep(Game& game);       // one meld action per call, false = nothing to do
    void playDiscardPhase(Game& game);
public:
    void startTurn(Game& game);
    bool step(Game& game);               // one visible action, false = turn finished
};
```

**How turns run in `MainGLWidget`:**

- After any human action, if the current player is not human, start `QTimer::singleShot(AI_STEP_DELAY_MS, this, &MainGLWidget::playAiStep)` (about 600 ms).
- `playAiStep()` calls `aiPlayers[currentPlayerIndex].step(game)`, then `update()`.
- Then it checks `isRoundFinished()` and shows `showRoundEndDialog()` if so. Otherwise it schedules the next step.
- The AI does one action per tick: draw, then each meld, then the discard. The human can follow the moves, and the CPU sits idle between steps.
- Human input is already blocked while `currentPlayerIndex != 0`.

**Strategy mode, chosen once at the start of each turn** (first matching rule wins):

| Mode | Condition |
|---|---|
| `DEFENSIVE` | Any opponent has ≤ 3 cards in hand, or the hidden deck has < 10 cards |
| `OPENING` | The team has no initial meld |
| `GOING_OUT` | The team has a canasta and the AI's hand has ≤ 6 cards |
| `BUILDING` | Otherwise |

The mode selects which rules and weights apply below.

---

## Step 3: Card memory (#6)

**New files `cardmemory.h/.cpp`.** It uses **public information only**: the AI never looks into other players' hands. One instance lives in `Game` and is reset in `initialize()`.

```cpp
class CardMemory {
    int knownInHand[4][14];        // cards known to be in a player's hand (taken from the pile)
    int discardedBy[4][14];        // what each player discarded this round
public:
    void onExposedDeckTaken(int playerNumber, const std::vector<Card*>& pileCards);
    void onCardsMelded(int playerNumber, const std::vector<Card*>& cards);  // decrements knownInHand
    void onCardDiscarded(int playerNumber, Card* card);                     // decrements knownInHand
    int getKnownInHand(int playerNumber, CardValue value) const;
    bool hasDiscarded(int playerNumber, CardValue value) const;
};
```

`Game` calls these hooks from `takeExposedDeck`, `createMeld`, `addCardsToMeld` and `discard`.

**Unseen cards:** for a given player, count the cards of each value that player hasn't seen:

`unseen[v] = total[v] − ownHand[v] − inMelds[v] − inExposedPile[v] − Σ knownInHand[other][v]`

- `total[v]` is 8 for normal values and 4 for jokers.
- This is recomputed at every decision. It's 14 values, so the cost is negligible.

**Easy difficulty:** the memory forgets entries older than N turns (`AiWeights::memoryTurns`).

---

## Step 4: Probabilities (#5)

Put these in `aiplayer.cpp` or a separate `aiprobability.h/.cpp`.

- **`totalUnseen`** = hidden deck size + the other players' hand sizes − all `knownInHand` entries.
- **`pDraw(v)`** = `unseen[v] / totalUnseen`: the chance the next drawn card has value v.
- **`pOpponentHasPair(opp, v)`:**
  - If `knownInHand[opp][v] ≥ 2`, it is 1.
  - Otherwise use a binomial approximation with `p = unseen[v] / totalUnseen` and `n = opp hand size − knownInHand total`, and account for `knownInHand[opp][v]` (0 or 1): `P(≥2 − known)`.
  - If `hasDiscarded(opp, v)`, multiply the result by 0.3, because players rarely discard from a pair.
- **`evDraw`** = `Σ_v pDraw(v) × cardGain(v)`, where `cardGain(v)` is:
  - 40 if it extends a team meld;
  - 30 if it makes a pair in hand;
  - 25 for a wild card;
  - 5 for a single;
  - 0 for a black 3.
- **`isDeadValue(v)`:** `unseen[v] == 0` and the AI holds exactly 1 card of value v. That card can never become a pair, so it's a good discard.

---

## Step 5: Decision rules (#2)

Every rule produces a score, and all scores come from `AiWeights`. The numbers below are starting values to be tuned by playing.

### A. Draw phase: take the pile or draw from the hidden deck

**Ways to take the pile, as the current code allows:**

1. Without selected cards: the team has a meld of the top card's value and the pile isn't frozen.
2. With 2 or more naturals of the top card's value from the hand, plus wild cards only if needed for the initial meld. This creates a new meld or extends a team meld.

In `OPENING`, the new meld built from the pile **alone** must reach the minimum; that's how `canHumanPlayerTakeExposedDeck` works today.

**Hard rules, applied in this order:**

1. A take is illegal under `canTakeExposedDeck` → draw.
2. The take would empty the hand without a canasta → don't take.
3. The pile has ≥ 7 cards and the take is legal → **take**.

**Otherwise compare scores** and take if `scoreTake > evDraw`:

```
scoreTake = 15 × (number of non-red-3 cards in the pile)
          + 40 × (pile cards that match a team meld or a value in hand)
          + 25 × (wild cards in the pile)
          + 60 if this fulfils the initial meld
          + 50 if the meld reaches 7 cards (canasta)
          − 10 × (cards from the pile that end up as dead singles in hand)
          − 30 × (wild cards the AI must add)
```

In `GOING_OUT` mode, apply `× 0.5` if most of the pile is junk.

### B. Meld phase: repeat while any candidate scores > 0

**Candidates:**

1. **New meld of value v:** all naturals of v, plus k wild cards where k ≤ the number of naturals.
2. **Add naturals to a team meld.** This always scores > 0, because it cuts the hand penalty and grows the meld toward a canasta.
3. **Add wild cards to a team meld.**
4. **Magic cards canasta:** only if the hand holds ≥ 7 wild cards.

**Hard rules:**

- In `OPENING`, a set of new melds is played **only together, and only if it reaches the team minimum**.
  - Choose the set greedily: sort the possible melds by points and add them until the minimum is reached.
  - If still short, add one wild card to the best pair, then a second one. Use at most 2 wild cards in total for the opening.
  - If it is still short, don't meld at all.
- Never leave the hand empty unless going out is legal: the team has a canasta or this move completes one.
- Keep at least 1 card for the discard, unless every card can be melded and going out is allowed.
- Black 3s never take part in melds.

**Wild-card rules** (candidate 3 is allowed only if one applies):

- The wild card **completes a canasta right now** (the meld is at 5 or 6 cards).
- It reaches the initial meld minimum.
- It enables going out.
- The mode is `DEFENSIVE`: dump wild cards, since one costs 20–50 points if left in hand.

**When to hold back** (in `BUILDING`):

- A **pair** of naturals with no team meld is kept in hand rather than melded with a wild card. The pair lets the AI take the pile later.
- Triples (3 or more naturals) are melded.

**Going out** (in `GOING_OUT` or `DEFENSIVE`): if the AI can meld everything except one discardable card, or meld everything, and the team has a canasta → do it.

**Scoring a candidate:**

```
+ points of the melded cards
+ 300 (mixed) / 500 (natural) if it completes a canasta
+ 10 × meld size after the move
− 20 per wild card used (in BUILDING only)
```

In `DEFENSIVE` mode, multiply the card-points part by 2.

### C. Discard phase: discard the card with the lowest `keepValue + danger`

**keepValue** (how much the AI wants to keep the card):

| Card | Value |
|---|---|
| Black 3 | 0, so it's the preferred discard. It blocks the next player from taking the pile |
| Wild card | 100. Exception: **freezing the pile** — if the pile has ≥ 5 cards and the next opponent can likely take it (`pOpponentHasPair > 0.5` or the opponent team has a meld of that value), it gets −50, so the AI throws the wild card to freeze the pile |
| Part of a pair or triple | 30 + 15 per extra card of the same value |
| Single | 10 |
| Dead single (`isDeadValue`) | 0 |
| In `DEFENSIVE` mode | − card points ÷ 2 (dump high-value cards first) |

**danger** (risk of feeding the next opponent `(p + 1) % 4`), multiplied by `(1 + pileSize / 5)`:

| Situation | Value |
|---|---|
| The opponent team has a meld of value v and the pile isn't frozen | +200 |
| `knownInHand[next][v] ≥ 2` | +150 |
| `knownInHand[next][v] == 1` | +60 |
| Otherwise | `pOpponentHasPair(next, v) × 100` |
| The next opponent has already discarded v | −30 |
| The pile is frozen and v only works for the opponent with a natural pair | × 0.5 |

**Hard rules:**

- The discard must not empty the hand without a canasta.
- Never discard while the team is below the initial meld minimum with melds on the table. Melding all-or-nothing in `OPENING` prevents that state.

### D. Partner coordination (cheap)

- Team melds are shared, including those of the human partner (player 0 for player 2). All "team meld" rules already cover the partner's melds.
- If the partner has ≤ 2 cards, the team has a canasta and the AI is not in `DEFENSIVE` mode, apply `× 0.5` to the melding scores. The AI keeps its cards and lets the partner go out, instead of dumping points.

---

## Step 6: Weights, difficulty and debugging

- **`AiWeights`** goes in `aiweights.h`, as a plain struct with every constant from step 5. It has preset factories `easy()`, `normal()` and `hard()`.
  - `easy`: memory limited to the last 3 turns, ±30% random noise on scores, never freezes the pile.
  - `hard`: full memory, no noise.
- **Debug log:** each AI decision prints a `printf` line, for example `P1 DRAW take=145 ev=62 -> take`. This makes tuning possible without a debugger.
- **Performance:** each decision is about 14 values × ~15 cards × a few melds, so microseconds even on a Pentium III. There is no heap allocation in the hot paths beyond the small `std::vector<Card*>` candidate lists.

---

## Order of work

1. Step 1 refactor. The human gameplay must work exactly as before; verify by playing.
2. `CardMemory` and the `Game` hooks.
3. `AiPlayer` with only the hard rules: always draw from the hidden deck, meld triples, discard the lowest single. This tests the turn loop and timers end to end.
4. Scored draw and discard decisions using the probabilities and memory.
5. Opening logic, wild-card rules, going out, `DEFENSIVE` mode.
6. Weight presets and tuning from the logs.
