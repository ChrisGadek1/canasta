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
    for(auto &deck: this->game.getDecks()) {
        if(deck.getType() == PLAYERS_OWNED) {
            for(auto &card: deck.getCards()) {
                int x = event->pos().x();
                int y = event->pos().y();
                Position cardPos = Position::toPx(card.position.left, card.position.down, card.position.top, card.position.right, width(), height());
                if(cardPos.down > y && cardPos.top < y && cardPos.left < x && cardPos.right > x) {
                    // TODO
                }
            }
        }

    }
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
}
