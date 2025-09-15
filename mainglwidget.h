#ifndef MAINGLWIDGET_H
#define MAINGLWIDGET_H

#include <QGLWidget>
#include <QImage>
#include "card.h"
#include <vector>
#include "game.h"

class MainGLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit MainGLWidget(QWidget *parent = nullptr);
    ~MainGLWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *eventMove) override;
    void mouseReleaseEvent(QMouseEvent *eventMove) override;

private:
    Game game;
    GLuint textureId;
    QImage loadTexture(const QString &path);
    void drawButton(Position position, std::string text);
    float meldButtonRcolor = 82.0f;
    float meldButtonGcolor = 57.0f;
    float meldButtonBcolor = 55.0f;
    Position meldingButtonPosition = Position(0.7, -0.8, -0.7, 0.9);
    void setMeldButtonColor(float r, float g, float b);
    void clickOnMeldButton();
};

#endif // MAINGLWIDGET_H
