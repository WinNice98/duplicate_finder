#include "helpwindow.h"
#include "ui_helpwindow.h"

helpwindow::helpwindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::helpwindow)
{
    ui->setupUi(this);
    QString label_1_text
        = "<h2>Duplicate Finder</h2>\n<p>Программа предназначена для поиска и анализа дубликатов "
          "файлов на вашем компьютере.</p>\n<ul>\n<li>Поддержка алгоритмов: <b>MD5</b>, "
          "<b>SHA1</b>, <b>SHA256</b></li>\n<li>Работа с подкаталогами</li>\n<li>Автоматическое "
          "выделение старых дубликатов</li>\n<li>Удаление ненужных копий</li>\n</ul>";
    ui->label_1->setText(label_1_text);
    QString label_2_text
        = "<h3>Как пользоваться программой</h3>\n<ol>\n<li>Выберите папку для анализа (При выборе "
          "папки при помощи ручного ввода пути не забудьте нажать Enter для применения "
          "изменений).</li>\n<li>Укажите метод хэширования (MD5, SHA1 или "
          "SHA256).</li>\n<li>Нажмите кнопку <b>«Начать поиск»</b>.</li>\n<li>После завершения "
          "сканирования появится список найденных дубликатов.</li>\n<li>Вы можете выделить "
          "ненужные файлы и удалить их.</li>\n</ol>\n<p><i>Совет:</i> используйте сортировку по "
          "дате, чтобы определить, какой файл оставить.</p>\n";
    ui->label_2->setText(label_2_text);
}

helpwindow::~helpwindow()
{
    delete ui;
}
