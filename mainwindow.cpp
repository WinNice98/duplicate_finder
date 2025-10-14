#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "hashworker.h"
#include <QFileDialog>
#include <QDirIterator>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>

void MainWindow::initDatabase() {
    QString dbPath = "duplicates.db";

    if (QFile::exists(dbPath)) {
        if (!QFile::remove(dbPath))
            qWarning() << "Не удалось удалить старую базу данных!";
    }
    // Создаём или открываем базу в корне программы
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qWarning() << "Ошибка при открытии базы:" << db.lastError().text();
        return;
    }

    QSqlQuery query;
    QString createTable = R"(
        CREATE TABLE IF NOT EXISTS files (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            filename TEXT,
            path TEXT UNIQUE,
            hash TEXT,
            size INTEGER,
            modified DATETIME
        )
    )";

    if (!query.exec(createTable)) {
        qWarning() << "Ошибка при создании таблицы:" << query.lastError().text();
    } else {
        qDebug() << "База успешно инициализирована.";
    }
}

void MainWindow::resetDatabase(const QString &dbPath)
{
    // Закрываем соединение
    {
        db = QSqlDatabase::database();
        if (db.isOpen())
            db.close();
    }

    // Удаляем соединение
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

    // Удаляем файл
    if (QFile::exists(dbPath)) {
        if (QFile::remove(dbPath))
            qDebug() << "База данных удалена:" << dbPath;
        else
            qWarning() << "Не удалось удалить базу данных!";
    }
}

void MainWindow::startHashing(const QStringList &files, HashMethod method)
{
    HashWorker *worker = new HashWorker(files, method, this);

    // Прогресс-бар
    connect(worker, &HashWorker::progress, this, [=](int current, int total){
        ui->progressBar->setMaximum(total);
        ui->progressBar->setValue(current);
    });

    // Запись в базу (GUI-поток)
    connect(worker, &HashWorker::fileHashed, this, [=](const QString &filename, const QString &path, const QString &hash, qint64 size, QDateTime modified){
        QSqlQuery query(db); // db — соединение GUI-потока
        query.prepare("INSERT INTO files (filename, path, hash, size, modified) VALUES (?, ?, ?, ?, ?)");
        query.addBindValue(filename);
        query.addBindValue(path);
        query.addBindValue(hash);
        query.addBindValue(size);
        query.addBindValue(modified.toString(Qt::ISODate));
        query.exec();
        updateDuplicatesView();
    });

    // Удаляем поток после завершения
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);

    worker->start();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window
                   | Qt::WindowMinMaxButtonsHint
                   | Qt::WindowSystemMenuHint
                   | Qt::WindowCloseButtonHint);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui->start_button->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_set_path_button_clicked()
{
    resetDatabase("duplicate.db");
    QString dir = QFileDialog::getExistingDirectory(
        this,                              // родительское окно
        "Выбери папку для анализа",        // заголовок диалога
        QDir::homePath(),                  // стартовая директория
        QFileDialog::ShowDirsOnly          // только каталоги
    );
    set_current_dir(dir);
    ui->current_path_edit->setText(get_current_dir());
    ui->start_button->setEnabled(true);
    initDatabase();
}

void MainWindow::updateDuplicatesView() {
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"Select", "File", "Modified", "File Path"});

    // 1️⃣ Сначала читаем все дубликаты в структуру
    QSqlQuery query(db);
    query.exec("SELECT filename, hash, path, modified FROM files "
               "WHERE hash IN ("
               "    SELECT hash "
               "    FROM files "
               "    GROUP BY hash "
               "    HAVING COUNT(*) > 1"
               ")");

    // hash -> список файлов (имя, (дата, путь))
    QMap<QString, QList<QPair<QString, QPair<QDateTime, QString>>>> filesByHash;

    while (query.next()) {
        QString filename = query.value(0).toString();
        QString hash = query.value(1).toString();
        QString path = query.value(2).toString();

        // Попытка прочитать дату из разных форматов
        QString dateStr = query.value(3).toString();
        QDateTime dt = QDateTime::fromString(dateStr, Qt::ISODate);
        if (!dt.isValid()) dt = QDateTime::fromString(dateStr, "yyyy-MM-dd HH:mm:ss");
        if (!dt.isValid()) dt = QDateTime::currentDateTime(); // запасной вариант

        filesByHash[hash].append({filename, {dt, path}});
    }

    // 2️⃣ Построение дерева и чекбоксов
    for (auto it = filesByHash.begin(); it != filesByHash.end(); ++it) {
        QString hash = it.key();
        auto &fileList = it.value();

        // находим самую новую дату
        QDateTime newest = QDateTime();
        for (auto &f : fileList) {
            if (!newest.isValid() || f.second.first > newest) newest = f.second.first;
        }

        // создаём элемент хэша
        QStandardItem *hashItem = new QStandardItem(hash);
        hashItem->setFlags(hashItem->flags() & ~Qt::ItemIsEditable);
        model->appendRow(hashItem);

        // добавляем дочерние элементы с чекбоксами
        for (auto &f : fileList) {
            QStandardItem *checkItem = new QStandardItem();
            checkItem->setFlags(checkItem->flags() | Qt::ItemIsUserCheckable);
            checkItem->setFlags(checkItem->flags() & ~Qt::ItemIsEditable);
            checkItem->setData(f.second.first < newest ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);

            QStandardItem *fileItem = new QStandardItem(f.first);
            fileItem->setFlags(fileItem->flags() & ~Qt::ItemIsEditable);

            QStandardItem *dateItem = new QStandardItem(f.second.first.toString("dd.MM.yyyy HH:mm:ss"));
            dateItem->setFlags(dateItem->flags() & ~Qt::ItemIsEditable);

            QStandardItem *pathItem = new QStandardItem(f.second.second);
            pathItem->setFlags(pathItem->flags() & ~Qt::ItemIsEditable);

            hashItem->appendRow({checkItem, fileItem, dateItem, pathItem});
        }
    }

    // 3️⃣ Подключение модели к TreeView
    ui->treeView->setModel(model);
    ui->treeView->expandAll();

    // разрешаем сортировку
    ui->treeView->setSortingEnabled(true);
    ui->treeView->sortByColumn(2, Qt::DescendingOrder); // сортировка по дате
}

void MainWindow::on_start_button_clicked()
{

    if (QDir(current_dir).exists()) {
        qDebug() << "Папка существует!";
        QStringList filePaths;
        QDirIterator it(get_current_dir(), QDir::Files,ui->subcatalog_checker->isChecked() ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);

        while (it.hasNext()) {
            QString path;
            path = it.next();
            //qDebug() << path;
            filePaths << path;
        }
        ui->progressBar->setValue(0);
        ui->progressBar->setMaximum(filePaths.size());
        switch (ui->comboBox->currentIndex()) {
        case 0:
            qDebug() << "MD5";
            startHashing(filePaths, HashMethod::MD5);
            break;
        case 1:
            qDebug() << "SHA-1";
            startHashing(filePaths, HashMethod::SHA1);
            break;
        case 2:
            qDebug() << "SHA-256";
            startHashing(filePaths, HashMethod::SHA256);
            break;
        case 3:
            qDebug() << "Побитово";
            break;
        default:
            break;
        }
        ui->start_button->setEnabled(false);
    } else {
        qDebug() << "Папка не найдена!";
    }


}


void MainWindow::on_current_path_edit_returnPressed()
{
    set_current_dir(ui->current_path_edit->text());
}


void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    // Получаем путь к файлу из второго столбца ("File")
    QString filePath = index.siblingAtColumn(3).data().toString();

    if (!filePath.isEmpty() && QFile::exists(filePath)) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    }
}


void MainWindow::on_delete_selected_button_clicked()
{
    if (!ui->treeView->model()) return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->treeView->model());
    if (!model) return;

    QList<QString> filesToDelete;

    // проходим по всем хэш-группам
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem *hashItem = model->item(i, 0);
        if (!hashItem) continue;

        // проходим по дочерним элементам
        for (int j = 0; j < hashItem->rowCount(); ++j) {
            QStandardItem *checkItem = hashItem->child(j, 0); // чекбокс
            QStandardItem *pathItem  = hashItem->child(j, 3); // путь к файлу

            if (!checkItem || !pathItem) continue;

            if (checkItem->data(Qt::CheckStateRole) == Qt::Checked) {
                filesToDelete.append(pathItem->text());
            }
        }
    }

    if (filesToDelete.isEmpty()) {
        QMessageBox::information(this, "Удаление", "Нет выделенных файлов для удаления.");
        return;
    }

    // Подтверждение
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Удаление",
        QString("Вы действительно хотите удалить %1 выбранных файлов?").arg(filesToDelete.size()),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes) return;

    // Удаление файлов и записи из базы
    for (const QString &filePath : filesToDelete) {
        QFile file(filePath);
        if (file.exists()) {
            if (!file.remove()) {
                QMessageBox::warning(this, "Ошибка", "Не удалось удалить файл:\n" + filePath);
                continue;
            }
        }

        // удаляем запись из базы
        QSqlQuery query(db);
        query.prepare("DELETE FROM files WHERE path = :path");
        query.bindValue(":path", filePath);
        query.exec();
    }

    // Обновляем TreeView
    updateDuplicatesView();
}

