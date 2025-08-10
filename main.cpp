#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include "card.h"
#include <QDebug>
#include <QString>

int main(int argc, char *argv[])
{
    Player player = Player(true, "name");
    Card card = Card(CardValue::JACK, CardSuit::SPADES, player);
    qDebug() << QString::fromStdString(card.texturePath());

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
