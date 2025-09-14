#include "mainglwidget.h"
#include <QDebug>
#include <QImage>
#include <QtOpenGL>
#include "card.h"
#include "player.h"
#include <vector>
#include "game.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>


MainGLWidget::MainGLWidget(QWidget *parent)
    : QGLWidget(parent), game(), textureId(0)
{
    this->game.initialize();
}

MainGLWidget::~MainGLWidget()
{
    if (textureId)
        glDeleteTextures(1, &textureId);
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
            card.loadTexture();
        }
    }
    Card::loadReverseTexture();
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

void MainGLWidget::mousePressEvent(QMouseEvent * event) {
    std::vector<Card*> foundCards;
    int x = event->pos().x();
    int y = event->pos().y();
    for(auto &deck: this->game.getDecks()) {
        if(deck.getType() == PLAYERS_OWNED) {
            for(auto &card: deck.getCards()) {
                Position cardPos = Position::toPx(card.position.left, card.position.down, card.position.top, card.position.right, width(), height());
                if(cardPos.down > y && cardPos.top < y && cardPos.left < x && cardPos.right > x) {
                    if(deck.getPlayer().getNumber() == 0) {
                        foundCards.push_back(&card);
                    }
                }
            }
        }
    }

    if(foundCards.size() == 2) {
        if(foundCards[0] == foundCards[1]->above) {
            foundCards[0]->setChosen(0);
            update();
        }
        else {
            foundCards[1]->setChosen(0);
            update();
        }
    }
    else if(foundCards.size() == 1) {
        foundCards[0]->setChosen(0);
        update();
    }
    Position buttonPos = Position::toPx(this->meldingButtonPosition.left, this->meldingButtonPosition.down, this->meldingButtonPosition.top, this->meldingButtonPosition.right, width(), height());
    if(buttonPos.down > y && buttonPos.top < y && buttonPos.left < x && buttonPos.right > x) {
        setMeldButtonColor(92.0f, 87.0f, 87.0f);
        update();
    }
}

#include <QFont>
#include <QFontMetrics>
#include <QString>

void MainGLWidget::drawButton(Position position, std::string text) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(this->meldButtonRcolor/255.0f, this->meldButtonGcolor/255.0f, this->meldButtonBcolor/255.0f);

    glBegin(GL_QUADS);
        glVertex2f(position.left,  position.down);
        glVertex2f(position.right, position.down);
        glVertex2f(position.right, position.top);
        glVertex2f(position.left,  position.top);
    glEnd();

    glColor3f(0.f, 0.f, 0.f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(position.left,  position.down);
        glVertex2f(position.right, position.down);
        glVertex2f(position.right, position.top);
        glVertex2f(position.left,  position.top);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1.f, 1.f, 1.f);

    float centerX = (position.left + position.right) / 2.0f;
    float centerY = (position.down + position.top) / 2.0f;

    int px = static_cast<int>( (centerX + 1.0f) * width()  * 0.5f );
    int py = static_cast<int>( (1.0f - centerY) * height() * 0.5f );

    QFont font("Arial", 14, QFont::Bold);
    QFontMetrics fm(font);

    QString qs = QString::fromStdString(text);
    int textWidth  = fm.width(qs);

    int ascent  = fm.ascent();
    int descent = fm.descent();
    int baseline = static_cast<int>(py + (ascent - descent) * 0.5f);

    int tx = px - textWidth / 2;
    int ty = baseline;

    renderText(tx, ty, qs, font);
}

void MainGLWidget::mouseMoveEvent(QMouseEvent *eventMove) {
    int x = eventMove->pos().x();
    int y = eventMove->pos().y();
    Position buttonPos = Position::toPx(this->meldingButtonPosition.left, this->meldingButtonPosition.down, this->meldingButtonPosition.top, this->meldingButtonPosition.right, width(), height());
    if(buttonPos.down > y && buttonPos.top < y && buttonPos.left < x && buttonPos.right > x) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
        setMeldButtonColor(97.0f, 78.0f, 78.0f);
        update();
    }
    else {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        setMeldButtonColor(82.0f, 57.0f, 55.0f);
        update();
    }
}

void MainGLWidget::mouseReleaseEvent(QMouseEvent *eventMove) {
    int x = eventMove->pos().x();
    int y = eventMove->pos().y();
    Position buttonPos = Position::toPx(this->meldingButtonPosition.left, this->meldingButtonPosition.down, this->meldingButtonPosition.top, this->meldingButtonPosition.right, width(), height());
    if(buttonPos.down > y && buttonPos.top < y && buttonPos.left < x && buttonPos.right > x) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
        setMeldButtonColor(97.0f, 78.0f, 78.0f);
        update();
    }
    else {
        setMeldButtonColor(82.0f, 57.0f, 55.0f);
        update();
    }
}

void MainGLWidget::setMeldButtonColor(float r, float g, float b) {
    this->meldButtonRcolor = r;
    this->meldButtonGcolor = g;
    this->meldButtonBcolor = b;
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
                card.draw(!deck.getPlayer().getIsHumanControlled(), angle);
            }
        }
    }
    drawButton(this->meldingButtonPosition, "Melduj");
}
