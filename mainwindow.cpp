#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainglwidget.h"
#include <QActionGroup>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    MainGLWidget *glWidget = new MainGLWidget(this);
    setCentralWidget(glWidget);

    QAction *newGameAction = menuBar()->addAction(tr("Nowa gra"));
    connect(newGameAction, &QAction::triggered, glWidget, &MainGLWidget::startNewGame);
    QAction *pointsAction = menuBar()->addAction(tr("Punkty"));
    connect(pointsAction, &QAction::triggered, glWidget, &MainGLWidget::clickOnPointsButton);

    QMenu *difficultyMenu = menuBar()->addMenu(tr("Poziom trudności"));
    QActionGroup *difficultyGroup = new QActionGroup(difficultyMenu);
    std::vector<std::pair<QString, AiWeights>> difficulties{
        {tr("Łatwy"), AiWeights::easy()},
        {tr("Normalny"), AiWeights::normal()},
        {tr("Trudny"), AiWeights::hard()}
    };
    for (int i = 0; i < static_cast<int>(difficulties.size()); i++) {
        QAction *difficultyAction = difficultyMenu->addAction(difficulties[i].first);
        difficultyAction->setCheckable(true);
        difficultyAction->setChecked(i == 1);
        difficultyGroup->addAction(difficultyAction);
        AiWeights aiWeights = difficulties[i].second;
        connect(difficultyAction, &QAction::triggered, [glWidget, aiWeights]() { glWidget->setAiWeights(aiWeights); });
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
