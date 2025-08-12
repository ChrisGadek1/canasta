#ifndef MAINGLWIDGET_H
#define MAINGLWIDGET_H

#include <QGLWidget>
#include <QImage>
#include "card.h"
#include <vector>

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

private:
    GLuint textureId;
    QImage loadTexture(const QString &path);
    std::vector<Card> cards;
};

#endif // MAINGLWIDGET_H
