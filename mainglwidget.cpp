#include "mainglwidget.h"
#include <QDebug>
#include <QImage>
#include <QtOpenGL>
#include "card.h"
#include "player.h"
#include <vector>
#include "game.h"
#include <QDialog>
#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>


MainGLWidget::MainGLWidget(QWidget *parent)
    : QGLWidget(parent), game(), textureId(0), leftArrowTextureId(0), rightArrowTextureId(0), flipTextureId(0)
{
    this->game.initialize();
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

void MainGLWidget::clickOnPointsButton() {
    QDialog pointsDialog(this);
    pointsDialog.setWindowTitle(tr("Punkty"));
    pointsDialog.resize(600, 450);
    QFont pointsFont("Arial", 18);

    std::vector<Player>& players = this->game.getPlayers();
    std::vector<RoundResult> roundResults = this->game.loadRoundResults();
    int rowsCount = static_cast<int>(roundResults.size());
    QTableWidget* pointsTable = new QTableWidget(rowsCount + 1, 2, &pointsDialog);
    pointsTable->setFont(pointsFont);
    pointsTable->horizontalHeader()->setFont(pointsFont);
    pointsTable->verticalHeader()->setFont(pointsFont);
    pointsTable->setHorizontalHeaderLabels({
        QString::fromStdString(players[0].getName() + " + " + players[2].getName()),
        QString::fromStdString(players[1].getName() + " + " + players[3].getName())
    });
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

void MainGLWidget::clickOnMeldButton() {
    std::vector<Card*> chosenCards;
    std::vector<CardsDeck*> chosenDecks;
    for(auto &deck: this->game.getDecks()) {
        if(deck.getType() == PLAYERS_OWNED && deck.getPlayer().getNumber() == 0) {
            std::vector<Card*> cardsToCheck = deck.getCards();
            for(auto card: cardsToCheck) {
                if(card->isChosen()) {
                    chosenCards.push_back(card);
                    chosenDecks.push_back(&deck);
                }
            }
        }
    }
    Player* currentPlayer = nullptr;
    for(auto &player: this->game.getPlayers()){
        if(player.getNumber() == 0) {
            currentPlayer = &player;
        }
    }
    if(chosenCards.size() > 0) {
        auto firstNonMagicCard = std::find_if(chosenCards.begin(), chosenCards.end(), [](const Card* card) {
            return card->value != TWO && card->value != JOKER;
        });
        if (firstNonMagicCard == chosenCards.end()) return;
        if (this->game.hasMeldWithValue((*firstNonMagicCard)->value, *currentPlayer)) return;
        Player partner = this->game.getPlayers()[(currentPlayer->getNumber() + 2) % 4];
        std::vector<Player> meldOwners{(*currentPlayer), partner};
        Meld meld((*firstNonMagicCard)->value, meldOwners);
        for (int i = 0; i < chosenDecks.size(); i++) {
            meld.addCard(chosenCards[i]);
            std::vector<Card*>& cards = chosenDecks[i]->getCards();
            cards.erase(std::find(cards.begin(), cards.end(), chosenCards[i]));
        }
        this->game.addMeld(meld);
        this->game.initializeMeldCardPositions();
        this->game.initializeCardsPositions();
        update();
    }
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
        clickOnMeldButton();
    }
    if (mouseOverPointsButton(x, y)) {
        setPointsButtonColor(92.0f, 87.0f, 87.0f);
        update();
        clickOnPointsButton();
    }
    if (this->game.mouseOverHiddenDeck(width(), height(), x, y)) {
        this->game.handleClickOnHiddenDeck();
    }
    if (this->game.mouseOverExposedDeck(width(), height(), x, y) && this->game.canHumanPlayerDiscard()) {
        this->game.handleClickOnExposedDeck();
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
            this->game.addCardsToMeld(meldIndex);
            update();
        }
    }
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
    glColor3f(1.f, 1.f, 1.f);
    QFont font("Arial", 14, QFont::Bold);
    for (auto &player : this->game.getPlayers()) {
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
   if (this->game.mouseOverHiddenDeck(width(), height(), x, y) && !this->game.getHiddenDeck().getCards().empty() && !this->game.hasHumanPlayerDrawnThisTurn()) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
    }
    else if (this->game.mouseOverExposedDeck(width(), height(), x, y) && this->game.canHumanPlayerDiscard()) {
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
    for(auto &deck: this->game.getDecks()) {
        if(deck.getType() == PLAYERS_OWNED) {
            std::vector<Card*> cardsToRender;
            if (deck.getPlayer().getNumber() == 0) {
                cardsToRender = this->game.getVisibleCards(deck.getCards());
            } else {
                cardsToRender = deck.getCards();
            }
            for(auto &card: cardsToRender) {
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
                card->draw(!deck.getPlayer().getIsHumanControlled(), angle);
            }
        }
        else if (deck.getType() == EXPOSED) {
            for (auto &card: deck.getCards()) {
                card->draw(false, 0);
            }
        }
        else {
            for (auto &card: deck.getCards()) {
                card->draw(true, 0);
            }
        }
    }
    for (auto &meld : this->game.getMelds()) {
        if (meld.isOwnedBy(this->game.getPlayers()[0]) == showOpponentsMelds) continue;
        for(auto &card: meld.getCards()) {
            if (showOpponentsMelds) {
                Position originalPos = card->position;
                card->position = Position::rotateCW90(card->position);
                card->draw(false, 90);
                card->position = originalPos;
            } else {
                card->draw(false, 0);
            }
        }
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
