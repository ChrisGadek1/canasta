#ifndef MAINGLWIDGET_H
#define MAINGLWIDGET_H

#include <QGLWidget>
#include <QImage>
#include "card.h"
#include <vector>
#include "game.h"

// Arrow button constants
static constexpr float ARROW_SIZE = 0.1f;
static constexpr float ARROW_Y_DOWN = -0.8f;
static constexpr float ARROW_Y_TOP = -0.7f;
static constexpr float ARROW_ALIGNMENT = 0.72f;

// Melding button constants
static constexpr float MELDING_BUTTON_LEFT = 0.75f;
static constexpr float MELDING_BUTTON_DOWN = -0.95f;
static constexpr float MELDING_BUTTON_TOP = -0.85f;
static constexpr float MELDING_BUTTON_RIGHT = 0.95f;

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
    GLuint leftArrowTextureId;
    GLuint rightArrowTextureId;
    QImage loadTexture(const QString &path);
    void drawMeldingButton();
    void drawLeftArrowButton();
    void drawRightArrowButton();
    float meldButtonRcolor = 82.0f;
    float meldButtonGcolor = 57.0f;
    float meldButtonBcolor = 55.0f;
    void setMeldButtonColor(float r, float g, float b);
    void clickOnMeldButton();
};

#endif // MAINGLWIDGET_H
