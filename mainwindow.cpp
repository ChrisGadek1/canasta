#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainglwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    MainGLWidget *glWidget = new MainGLWidget(this);
    setCentralWidget(glWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}
