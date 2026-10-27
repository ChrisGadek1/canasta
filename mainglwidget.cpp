#include "mainglwidget.h"
#include <QDebug>
#include <algorithm>
#include <QImage>
#include <QtOpenGL>
#include "card.h"
#include "player.h"
#include <vector>
#include "game.h"
#include <QDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>


MainGLWidget::MainGLWidget(QWidget *parent)
    : QGLWidget(parent), game(), textureId(0), leftArrowTextureId(0), rightArrowTextureId(0), flipTextureId(0)
{
    this->game.initialize();
    for (auto &player : this->game.getPlayers()) {
        this->aiPlayers.push_back(AiPlayer(player.getNumber(), AiWeights::normal()));
    }
    connect(&animationTimer, &QTimer::timeout, this, &MainGLWidget::animateCards);
}

void MainGLWidget::startAnimation() {
    if (animationTimer.isActive()) return;
    animationClock.start();
    animationTimer.start(ANIMATION_FRAME_MS);
}

void MainGLWidget::animateCards() {
    qint64 elapsedMs = std::min<qint64>(animationClock.restart(), MAX_ANIMATION_FRAME_STEP_MS);
    float progressDelta = static_cast<float>(elapsedMs) / CARD_ANIMATION_DURATION_MS;
    bool anyCardAnimating = false;
    for (auto &deck : this->game.getDecks()) {
        for (auto *card : deck.getCards()) {
            anyCardAnimating |= card->advanceAnimation(card->position, progressDelta);
        }
    }
    for (auto &meld : this->game.getMelds()) {
        bool opponentsMeld = !meld.isOwnedBy(this->game.getPlayers()[0]);
        for (auto *card : meld.getCards()) {
            anyCardAnimating |= card->advanceAnimation(opponentsMeld ? Position::rotateCW90(card->position) : card->position, progressDelta);
        }
    }
    update();
    if (anyCardAnimating) return;
    animationTimer.stop();
    if (this->game.isRoundFinished()) {
        showRoundEndDialog();
    } else {
        scheduleAiStep();
    }
}

void MainGLWidget::scheduleAiStep() {
    if (aiStepScheduled || animationTimer.isActive() || this->game.isRoundFinished() || this->game.getCurrentPlayer()->getIsHumanControlled()) return;
    aiStepScheduled = true;
    QTimer::singleShot(AI_STEP_DELAY_MS, this, &MainGLWidget::playAiStep);
}

void MainGLWidget::playAiStep() {
    aiStepScheduled = false;
    if (this->game.isRoundFinished() || this->game.getCurrentPlayer()->getIsHumanControlled()) return;
    AiPlayer& aiPlayer = this->aiPlayers[this->game.getCurrentPlayerIndex()];
    if (!aiPlayer.isPlayingTurn()) {
        aiPlayer.startTurn(this->game);
    }
    aiPlayer.step(this->game);
    startAnimation();
}

MainGLWidget::~MainGLWidget()
{
    makeCurrent();
    if (textureId)
        glDeleteTextures(1, &textureId);
    if (leftArrowTextureId)
        glDeleteTextures(1, &leftArrowTextureId);
    if (rightArrowTextureId)
        glDeleteTextures(1, &rightArrowTextureId);
    if (flipTextureId)
        glDeleteTextures(1, &flipTextureId);
    if (Card::reverseTextureId)
        glDeleteTextures(1, &Card::reverseTextureId);
}

void MainGLWidget::loadCardsTextures() {
    for(auto &deck : this->game.getDecks()) {
        for(auto &card : deck.getCards()) {
            card->loadTexture();
        }
    }
    for(auto &meld : this->game.getMelds()) {
        for(auto &card : meld.getCards()) {
            card->loadTexture();
        }
    }
}

QImage MainGLWidget::loadTexture(const QString &path)
{
    QImage img;
    if (!img.load(path)) {
        qDebug() << "Nie udało się wczytać tekstury:" << path;
        return QImage();
    }
    return QGLWidget::convertToGLFormat(img);
}

void MainGLWidget::initializeGL()
{
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    QImage texture = loadTexture(":/images/images/background.png");
    if (!texture.isNull()) {
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     texture.width(), texture.height(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    loadCardsTextures();
    Card::loadReverseTexture();

    QImage leftArrowImg = loadTexture(":/images/images/arrow_left.png");
    if (!leftArrowImg.isNull()) {
        glGenTextures(1, &leftArrowTextureId);
        glBindTexture(GL_TEXTURE_2D, leftArrowTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     leftArrowImg.width(), leftArrowImg.height(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, leftArrowImg.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    QImage rightArrowImg = loadTexture(":/images/images/arrow_right.png");
    if (!rightArrowImg.isNull()) {
        glGenTextures(1, &rightArrowTextureId);
        glBindTexture(GL_TEXTURE_2D, rightArrowTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     rightArrowImg.width(), rightArrowImg.height(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, rightArrowImg.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    QImage flipImg = loadTexture(":/images/images/flip.png");
    if (!flipImg.isNull()) {
        glGenTextures(1, &flipTextureId);
        glBindTexture(GL_TEXTURE_2D, flipTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     flipImg.width(), flipImg.height(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, flipImg.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    setMouseTracking(true);
    startAnimation();
}

void MainGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

bool MainGLWidget::shouldShowLeftArrow() const {
    return game.canGoPrevious();
}

bool MainGLWidget::shouldShowRightArrow() const {
    CardsDeck& humanDeck = const_cast<Game&>(game).getHumanPlayerDeck();
    return game.canGoNext(humanDeck.getCards());
}

Position MainGLWidget::getLeftArrowPosition() const {
    return Position::toPx(-ARROW_ALIGNMENT, ARROW_Y_DOWN, ARROW_Y_TOP, -ARROW_ALIGNMENT + ARROW_SIZE, width(), height());
}

Position MainGLWidget::getRightArrowPosition() const {
    return Position::toPx(ARROW_ALIGNMENT - ARROW_SIZE, ARROW_Y_DOWN, ARROW_Y_TOP, ARROW_ALIGNMENT, width(), height());
}

bool MainGLWidget::mouseOverLeftArrow(int x, int y) const {
    Position pos = getLeftArrowPosition();
    return pos.down > y && pos.top < y && pos.left < x && pos.right > x;
}

bool MainGLWidget::mouseOverRightArrow(int x, int y) const {
    Position pos = getRightArrowPosition();
    return pos.down > y && pos.top < y && pos.left < x && pos.right > x;
}

bool MainGLWidget::mouseOverFlipButton(int x, int y) const {
    Position pos = Position::toPx(FLIP_BUTTON_LEFT, FLIP_BUTTON_DOWN, FLIP_BUTTON_TOP, FLIP_BUTTON_RIGHT, width(), height());
    return pos.down > y && pos.top < y && pos.left < x && pos.right > x;
}

bool MainGLWidget::mouseOverPointsButton(int x, int y) const {
    Position pos = Position::toPx(POINTS_BUTTON_LEFT, POINTS_BUTTON_DOWN, POINTS_BUTTON_TOP, POINTS_BUTTON_RIGHT, width(), height());
    return pos.down > y && pos.top < y && pos.left < x && pos.right > x;
}

QStringList MainGLWidget::getTeamsNames() {
    std::vector<Player>& players = this->game.getPlayers();
    return {
        QString::fromStdString(players[0].getName() + " + " + players[2].getName()),
        QString::fromStdString(players[1].getName() + " + " + players[3].getName())
    };
}

void MainGLWidget::showRoundEndDialog() {
    QDialog roundEndDialog(this);
    roundEndDialog.setWindowTitle(tr("Koniec rundy"));
    roundEndDialog.resize(600, 450);
    QFont pointsFont("Arial", 18);

    std::vector<Player>& players = this->game.getPlayers();
    std::vector<TeamRoundScore> teamsScores{this->game.calculateTeamRoundScore(players[0]), this->game.calculateTeamRoundScore(players[1])};
    QStringList rowLabels{tr("Kanasty"), tr("Czerwone trójki"), tr("Zejście"), tr("Wyłożone karty"), tr("Karty w ręce"), tr("Suma")};

    QTableWidget* scoreTable = new QTableWidget(rowLabels.size(), 2, &roundEndDialog);
    scoreTable->setFont(pointsFont);
    scoreTable->horizontalHeader()->setFont(pointsFont);
    scoreTable->verticalHeader()->setFont(pointsFont);
    scoreTable->setHorizontalHeaderLabels(getTeamsNames());
    scoreTable->setVerticalHeaderLabels(rowLabels);
    scoreTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    scoreTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    scoreTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QFont sumFont = pointsFont;
    sumFont.setBold(true);
    for (int team = 0; team < 2; team++) {
        const TeamRoundScore& score = teamsScores[team];
        std::vector<int> points{score.getCanastasPoints(), score.getRedThreesPoints(), score.getGoingOutPoints(), score.getMeldedCardsPoints(), score.getHandCardsPoints(), score.getTotalPoints()};
        for (int row = 0; row < static_cast<int>(points.size()); row++) {
            scoreTable->setItem(row, team, new QTableWidgetItem(QString::number(points[row])));
        }
        scoreTable->item(rowLabels.size() - 1, team)->setFont(sumFont);
    }

    QVBoxLayout* layout = new QVBoxLayout(&roundEndDialog);
    layout->addWidget(scoreTable);

    roundEndDialog.exec();
    makeCurrent();
    this->game.finishRound();
    loadCardsTextures();
    startAnimation();
}

void MainGLWidget::clickOnPointsButton() {
    QDialog pointsDialog(this);
    pointsDialog.setWindowTitle(tr("Punkty"));
    pointsDialog.resize(600, 450);
    QFont pointsFont("Arial", 18);

    std::vector<RoundResult> roundResults = this->game.loadRoundResults();
    int rowsCount = static_cast<int>(roundResults.size());
    QTableWidget* pointsTable = new QTableWidget(rowsCount + 1, 2, &pointsDialog);
    pointsTable->setFont(pointsFont);
    pointsTable->horizontalHeader()->setFont(pointsFont);
    pointsTable->verticalHeader()->setFont(pointsFont);
    pointsTable->setHorizontalHeaderLabels(getTeamsNames());
    pointsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    pointsTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    pointsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QStringList rowLabels;
    int team02Sum = 0;
    int team13Sum = 0;
    for (int i = 0; i < rowsCount; i++) {
        rowLabels << QString::number(i + 1);
        pointsTable->setItem(i, 0, new QTableWidgetItem(QString::number(roundResults[i].getTeam02Points())));
        pointsTable->setItem(i, 1, new QTableWidgetItem(QString::number(roundResults[i].getTeam13Points())));
        team02Sum += roundResults[i].getTeam02Points();
        team13Sum += roundResults[i].getTeam13Points();
    }
    rowLabels << tr("Suma");
    pointsTable->setVerticalHeaderLabels(rowLabels);

    QFont sumFont = pointsFont;
    sumFont.setBold(true);
    QTableWidgetItem* team02SumItem = new QTableWidgetItem(QString::number(team02Sum));
    QTableWidgetItem* team13SumItem = new QTableWidgetItem(QString::number(team13Sum));
    team02SumItem->setFont(sumFont);
    team13SumItem->setFont(sumFont);
    pointsTable->setItem(rowsCount, 0, team02SumItem);
    pointsTable->setItem(rowsCount, 1, team13SumItem);

    QVBoxLayout* layout = new QVBoxLayout(&pointsDialog);
    layout->addWidget(pointsTable);

    pointsDialog.exec();
    setPointsButtonColor(82.0f, 57.0f, 55.0f);
    update();
}

void MainGLWidget::showInsufficientInitialMeldsDialog() {
    QMessageBox insufficientMeldsDialog(this);
    insufficientMeldsDialog.setWindowTitle(tr("Wyłożenie się"));
    insufficientMeldsDialog.setFont(QFont("Arial", 18));
    insufficientMeldsDialog.setText(tr("Wyłożone ogony nie posiadają wymaganej ilości punktów (%1) do wyłożenia się.").arg(this->game.getHumanTeamInitialMeldMinimumPoints()));
    QPushButton* removeMeldsButton = insufficientMeldsDialog.addButton(tr("Usuń ogony"), QMessageBox::AcceptRole);
    insufficientMeldsDialog.addButton(tr("Anuluj"), QMessageBox::RejectRole);
    insufficientMeldsDialog.exec();
    if (insufficientMeldsDialog.clickedButton() == removeMeldsButton) {
        this->game.removeHumanTeamMelds();
    }
}

void MainGLWidget::showNoCanastaDialog() {
    QMessageBox noCanastaDialog(this);
    noCanastaDialog.setWindowTitle(tr("Koniec gry"));
    noCanastaDialog.setFont(QFont("Arial", 18));
    noCanastaDialog.setText(tr("Nie można skończyć gry - brak wyłożonej kanasty"));
    noCanastaDialog.addButton(tr("Ok"), QMessageBox::AcceptRole);
    noCanastaDialog.exec();
}

void MainGLWidget::clickOnMeldButton() {
    this->game.meldHumanPlayerChosenCards();
    update();
}

void MainGLWidget::mousePressEvent(QMouseEvent * event) {
    std::vector<Card*> foundCards;
    int x = event->pos().x();
    int y = event->pos().y();
    Card* foundTopCard = nullptr;
    for(auto &deck: this->game.getDecks()) {
        if(deck.getType() == PLAYERS_OWNED) {
            std::vector<Card*> cardsToCheck;
            if (deck.getPlayer().getNumber() == 0) {
                cardsToCheck = this->game.getVisibleCards(deck.getCards());
            } else {
                cardsToCheck = deck.getCards();
            }
            for(auto &card: cardsToCheck) {
                Position cardPos = Position::toPx(card->position.left, card->position.down, card->position.top, card->position.right, width(), height());
                if(cardPos.down > y && cardPos.top < y && cardPos.left < x && cardPos.right > x) {
                    if(deck.getPlayer().getNumber() == 0) {
                        foundTopCard = card;
                    }
                }
            }
        }
    }
    if (foundTopCard) {
        foundTopCard->setChosen(0);
        update();
    }
    Position buttonPos = Position::toPx(MELDING_BUTTON_LEFT, MELDING_BUTTON_DOWN, MELDING_BUTTON_TOP, MELDING_BUTTON_RIGHT, width(), height());
    if(buttonPos.down > y && buttonPos.top < y && buttonPos.left < x && buttonPos.right > x && this->game.canHumanPlayerMeld()) {
        setMeldButtonColor(92.0f, 87.0f, 87.0f);
        update();
        if (this->game.wouldMeldingChosenCardsFinishWithInsufficientInitialMelds()) {
            showInsufficientInitialMeldsDialog();
        } else if (this->game.wouldMeldFinishWithoutCanasta()) {
            showNoCanastaDialog();
        } else {
            clickOnMeldButton();
        }
    }
    if (mouseOverPointsButton(x, y)) {
        setPointsButtonColor(92.0f, 87.0f, 87.0f);
        update();
        clickOnPointsButton();
    }
    if (this->game.mouseOverHiddenDeck(width(), height(), x, y)) {
        this->game.handleClickOnHiddenDeck();
    }
    if (this->game.mouseOverExposedDeck(width(), height(), x, y) && (this->game.canHumanPlayerDiscard() || this->game.canHumanPlayerTakeExposedDeck())) {
        if (this->game.canHumanPlayerDiscard() && this->game.hasHumanTeamInsufficientInitialMelds()) {
            showInsufficientInitialMeldsDialog();
        } else if (this->game.wouldClickOnExposedDeckFinishWithoutCanasta()) {
            showNoCanastaDialog();
        } else {
            this->game.handleClickOnExposedDeck();
        }
        update();
    }
    // Handle left arrow button click (only if visible)
    if (shouldShowLeftArrow() && mouseOverLeftArrow(x, y)) {
        this->game.previousPage();
        this->game.initializeCardsPositions();
        update();
    }
    // Handle right arrow button click (only if visible)
    if (shouldShowRightArrow() && mouseOverRightArrow(x, y)) {
        this->game.nextPage();
        this->game.initializeCardsPositions();
        update();
    }
    // Handle flip button click
    if (mouseOverFlipButton(x, y)) {
        showOpponentsMelds = !showOpponentsMelds;
        update();
    }
    // Handle click on existing meld to add cards
    {
        int meldIndex = -1;
        if (!showOpponentsMelds && this->game.mouseOverMeld(width(), height(), x, y, meldIndex) && this->game.canAddCardsToMeld(meldIndex)) {
            if (this->game.wouldMeldingChosenCardsFinishWithInsufficientInitialMelds()) {
                showInsufficientInitialMeldsDialog();
            } else if (this->game.wouldAddingCardsToMeldFinishWithoutCanasta(meldIndex)) {
                showNoCanastaDialog();
            } else {
                this->game.addCardsToMeld(meldIndex);
            }
            update();
        }
    }
    startAnimation();
}

#include <QFont>
#include <QFontMetrics>
#include <QString>

void MainGLWidget::drawMeldingButton() {
    drawButton(MELDING_BUTTON_LEFT, MELDING_BUTTON_DOWN, MELDING_BUTTON_TOP, MELDING_BUTTON_RIGHT,
               this->meldButtonRcolor, this->meldButtonGcolor, this->meldButtonBcolor, tr("Melduj"));
}

void MainGLWidget::drawPointsButton() {
    drawButton(POINTS_BUTTON_LEFT, POINTS_BUTTON_DOWN, POINTS_BUTTON_TOP, POINTS_BUTTON_RIGHT,
               this->pointsButtonRcolor, this->pointsButtonGcolor, this->pointsButtonBcolor, tr("Punkty"));
}

void MainGLWidget::drawButton(float left, float down, float top, float right, float r, float g, float b, const QString& caption) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(r/255.0f, g/255.0f, b/255.0f);

    glBegin(GL_QUADS);
        glVertex2f(left,  down);
        glVertex2f(right, down);
        glVertex2f(right, top);
        glVertex2f(left,  top);
    glEnd();

    glColor3f(0.f, 0.f, 0.f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(left,  down);
        glVertex2f(right, down);
        glVertex2f(right, top);
        glVertex2f(left,  top);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1.f, 1.f, 1.f);

    float centerX = (left + right) / 2.0f;
    float centerY = (down + top) / 2.0f;

    drawCenteredText(caption, centerX, centerY, QFont("Arial", 14, QFont::Bold));
}

void MainGLWidget::drawCenteredText(const QString& text, float centerX, float centerY, const QFont& font) {
    int px = static_cast<int>( (centerX + 1.0f) * width()  * 0.5f );
    int py = static_cast<int>( (1.0f - centerY) * height() * 0.5f );

    QFontMetrics fm(font);
    int textWidth  = fm.width(text);

    int ascent  = fm.ascent();
    int descent = fm.descent();
    int baseline = static_cast<int>(py + (ascent - descent) * 0.5f);

    renderText(px - textWidth / 2, baseline, text, font);
}

void MainGLWidget::drawPlayersNames() {
    QFont font("Arial", 14, QFont::Bold);
    for (auto &player : this->game.getPlayers()) {
        if (player.getNumber() == this->game.getCurrentPlayerIndex()) {
            glColor3f(1.f, 0.85f, 0.f);
        } else {
            glColor3f(1.f, 1.f, 1.f);
        }
        float centerX, centerY;
        switch(player.getNumber()) {
            case 0:
                centerX = 0.f;
                centerY = PLAYER_NAME_BOTTOM_Y;
                break;
            case 1:
                centerX = -PLAYER_NAME_SIDE_X;
                centerY = PLAYER_NAME_SIDE_Y;
                break;
            case 2:
                centerX = 0.f;
                centerY = PLAYER_NAME_TOP_Y;
                break;
            default:
                centerX = PLAYER_NAME_SIDE_X;
                centerY = PLAYER_NAME_SIDE_Y;
                break;
        }
        drawCenteredText(QString::fromStdString(player.getName()), centerX, centerY, font);
    }
    glColor3f(1.f, 1.f, 1.f);
}

void MainGLWidget::mouseMoveEvent(QMouseEvent *eventMove) {
    int x = eventMove->pos().x();
    int y = eventMove->pos().y();
    Position buttonPos = Position::toPx(MELDING_BUTTON_LEFT, MELDING_BUTTON_DOWN, MELDING_BUTTON_TOP, MELDING_BUTTON_RIGHT, width(), height());
    bool setToHand = false;
    if(buttonPos.down > y && buttonPos.top < y && buttonPos.left < x && buttonPos.right > x && this->game.canHumanPlayerMeld()) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
        setToHand = true;
        setMeldButtonColor(97.0f, 78.0f, 78.0f);
        update();
    }
    else {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        setMeldButtonColor(82.0f, 57.0f, 55.0f);
        update();
    }
    if (mouseOverPointsButton(x, y)) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
        setToHand = true;
        setPointsButtonColor(97.0f, 78.0f, 78.0f);
    }
    else {
        setPointsButtonColor(82.0f, 57.0f, 55.0f);
    }
   if (this->game.mouseOverHiddenDeck(width(), height(), x, y) && !this->game.getHiddenDeck().getCards().empty() && this->game.canHumanPlayerDraw()) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
    }
    else if (this->game.mouseOverExposedDeck(width(), height(), x, y) && (this->game.canHumanPlayerDiscard() || this->game.canHumanPlayerTakeExposedDeck())) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
    }
    else {
        // Check if mouse is over arrow buttons or flip button
        bool overArrow = false;
        if (shouldShowLeftArrow() && mouseOverLeftArrow(x, y)) {
            overArrow = true;
        }
        else if (shouldShowRightArrow() && mouseOverRightArrow(x, y)) {
            overArrow = true;
        }

        if (overArrow || mouseOverFlipButton(x, y)) {
            QApplication::setOverrideCursor(Qt::PointingHandCursor);
        }
        else if (!setToHand) {
            int meldIndex = -1;
            if (!showOpponentsMelds && this->game.mouseOverMeld(width(), height(), x, y, meldIndex) && this->game.canAddCardsToMeld(meldIndex)) {
                QApplication::setOverrideCursor(Qt::PointingHandCursor);
            } else {
                QApplication::setOverrideCursor(Qt::ArrowCursor);
            }
        }
    }
}

void MainGLWidget::mouseReleaseEvent(QMouseEvent *eventMove) {
    int x = eventMove->pos().x();
    int y = eventMove->pos().y();
    Position buttonPos = Position::toPx(MELDING_BUTTON_LEFT, MELDING_BUTTON_DOWN, MELDING_BUTTON_TOP, MELDING_BUTTON_RIGHT, width(), height());
    if(buttonPos.down > y && buttonPos.top < y && buttonPos.left < x && buttonPos.right > x && this->game.canHumanPlayerMeld()) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
        setMeldButtonColor(97.0f, 78.0f, 78.0f);
        update();
    }
    else {
        setMeldButtonColor(82.0f, 57.0f, 55.0f);
        bool overArrow = (shouldShowLeftArrow() && mouseOverLeftArrow(x, y)) ||
                         (shouldShowRightArrow() && mouseOverRightArrow(x, y));
        if (overArrow || mouseOverFlipButton(x, y) || mouseOverPointsButton(x, y)) {
            QApplication::setOverrideCursor(Qt::PointingHandCursor);
        } else {
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        }
        update();
    }
}

void MainGLWidget::setMeldButtonColor(float r, float g, float b) {
    this->meldButtonRcolor = r;
    this->meldButtonGcolor = g;
    this->meldButtonBcolor = b;
}

void MainGLWidget::setPointsButtonColor(float r, float g, float b) {
    this->pointsButtonRcolor = r;
    this->pointsButtonGcolor = g;
    this->pointsButtonBcolor = b;
}

void MainGLWidget::drawLeftArrowButton() {
    if (!leftArrowTextureId) return;
    glBindTexture(GL_TEXTURE_2D, leftArrowTextureId);

    float left = -ARROW_ALIGNMENT;
    float right = left + ARROW_SIZE;

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(left,  ARROW_Y_DOWN);
        glTexCoord2f(1, 0); glVertex2f(right, ARROW_Y_DOWN);
        glTexCoord2f(1, 1); glVertex2f(right, ARROW_Y_TOP);
        glTexCoord2f(0, 1); glVertex2f(left,  ARROW_Y_TOP);
    glEnd();
}

void MainGLWidget::drawRightArrowButton() {
    if (!rightArrowTextureId) return;
    glBindTexture(GL_TEXTURE_2D, rightArrowTextureId);

    float right = ARROW_ALIGNMENT;
    float left = right - ARROW_SIZE;

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(left,  ARROW_Y_DOWN);
        glTexCoord2f(1, 0); glVertex2f(right, ARROW_Y_DOWN);
        glTexCoord2f(1, 1); glVertex2f(right, ARROW_Y_TOP);
        glTexCoord2f(0, 1); glVertex2f(left,  ARROW_Y_TOP);
    glEnd();
}

void MainGLWidget::drawFlipButton() {
    if (!flipTextureId) return;
    glBindTexture(GL_TEXTURE_2D, flipTextureId);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(FLIP_BUTTON_LEFT,  FLIP_BUTTON_DOWN);
        glTexCoord2f(1, 0); glVertex2f(FLIP_BUTTON_RIGHT, FLIP_BUTTON_DOWN);
        glTexCoord2f(1, 1); glVertex2f(FLIP_BUTTON_RIGHT, FLIP_BUTTON_TOP);
        glTexCoord2f(0, 1); glVertex2f(FLIP_BUTTON_LEFT,  FLIP_BUTTON_TOP);
    glEnd();
}


void MainGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (textureId) {
        glBindTexture(GL_TEXTURE_2D, textureId);

        glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex2f(-1, -1);
            glTexCoord2f(1, 0); glVertex2f(1, -1);
            glTexCoord2f(1, 1); glVertex2f(1, 1);
            glTexCoord2f(0, 1); glVertex2f(-1, 1);
        glEnd();
    }
    struct CardDrawing {
        Card* card;
        bool reverse;
        float angle;
    };
    std::vector<CardDrawing> animatingCards;
    auto drawCard = [&animatingCards](Card* card, bool reverse, float angle, bool visible) {
        if (card->isAnimating()) {
            animatingCards.push_back({card, reverse, angle});
        } else if (visible) {
            card->draw(reverse, angle);
        }
    };
    for(auto &deck: this->game.getDecks()) {
        if(deck.getType() == PLAYERS_OWNED) {
            std::vector<Card*> visibleCards;
            if (deck.getPlayer().getNumber() == 0) {
                visibleCards = this->game.getVisibleCards(deck.getCards());
            } else {
                visibleCards = deck.getCards();
            }
            for(auto &card: deck.getCards()) {
                int angle;
                switch(deck.getPlayer().getNumber()) {
                    case 0:
                        angle = 0;
                        break;
                    case 1:
                        angle = 270;
                        break;
                    case 2:
                        angle = 180;
                        break;
                    default:
                        angle = 90;
                        break;
                }
                bool visible = std::find(visibleCards.begin(), visibleCards.end(), card) != visibleCards.end();
                drawCard(card, !deck.getPlayer().getIsHumanControlled(), angle, visible);
            }
        }
        else if (deck.getType() == EXPOSED) {
            for (auto &card: deck.getCards()) {
                drawCard(card, false, card->freezesExposedDeck() ? 90 : 0, true);
            }
        }
        else {
            for (auto &card: deck.getCards()) {
                drawCard(card, true, 0, true);
            }
        }
    }
    for (auto &meld : this->game.getMelds()) {
        bool opponentsMeld = !meld.isOwnedBy(this->game.getPlayers()[0]);
        for(auto &card: meld.getCards()) {
            drawCard(card, false, opponentsMeld ? 90 : 0, opponentsMeld == showOpponentsMelds);
        }
    }
    for (auto &cardDrawing : animatingCards) {
        cardDrawing.card->draw(cardDrawing.reverse, cardDrawing.angle);
    }
    if (shouldShowLeftArrow())
        drawLeftArrowButton();
    if (shouldShowRightArrow())
        drawRightArrowButton();
    drawFlipButton();
    drawMeldingButton();
    drawPointsButton();
    drawPlayersNames();
}
