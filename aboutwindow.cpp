#include "aboutwindow.h"
#include <QMovie>
#include "ui_aboutwindow.h"

aboutwindow::aboutwindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::aboutwindow)
{
    ui->setupUi(this);
    QMovie *movie = new QMovie(
        ":/gif/source/about.gif"); // можно и обычный путь "C:/путь/к/файлу.gif"
    ui->gif_label->setMovie(movie);
    movie->start();
    ui->program_name->setStyleSheet(R"(
        QLabel {
            color: white;
            font-family: "Segoe UI";
            font-weight: bold;
            font-size: 22px;
            padding: 10px;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #3A3A3A, stop:1 #232323);
            border-bottom: 2px solid #4C4C4C;
            border-top-left-radius: 10px;
            border-top-right-radius: 10px;
            text-shadow: 0px 0px 5px rgba(255,255,255,0.3);
        })");
}

aboutwindow::~aboutwindow()
{
    delete ui;
}
