#include "mainglwidget.h"
#include <QDebug>
#include <QImage>
#include <QtOpenGL>
#include "card.h"
#include "player.h"
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>


MainGLWidget::MainGLWidget(QWidget *parent)
    : QGLWidget(parent), textureId(0)
{
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

    Player player(true, "name");

    std::vector<Card> cards;

    Card c1(CardValue::ACE, CardSuit::SPADES, player);
    Card c2(CardValue::TEN, CardSuit::HEARTS, player);
    cards.push_back(c1);
    cards.push_back(c2);

    this->cards = cards;

    for (auto &card : this->cards) {
        card.loadTexture();
    }
}

void MainGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1); // ortograficzna projekcja z NDC
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void MainGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (textureId) {
        glBindTexture(GL_TEXTURE_2D, textureId);

        glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex2f(-1.0f, -1.0f);
            glTexCoord2f(1, 0); glVertex2f(1.0f, -1.0f);
            glTexCoord2f(1, 1); glVertex2f(1.0f, 1.0f);
            glTexCoord2f(0, 1); glVertex2f(-1.0f, 1.0f);
        glEnd();
    }

    float x = -0.8f;
    for (const auto &card : cards) {
        card.draw(x, -0.5f, 0.4f, 0.6f);
        x += 0.45f;
    }
}
