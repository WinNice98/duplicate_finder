#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

enum class HashMethod { MD5, SHA1, SHA256 };

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:

public:
    MainWindow(QWidget *parent = nullptr);
    void set_current_dir(QString dir){
        current_dir = dir;
    }
    QString get_current_dir(){
        return current_dir;
    }
    ~MainWindow();

private slots:

    void on_set_path_button_clicked();

    void on_start_button_clicked();

    void on_current_path_edit_returnPressed();

    void on_treeView_doubleClicked(const QModelIndex &index);

    void on_delete_selected_button_clicked();

    void on_about_button_clicked();

private:
    QString current_dir;
    void initDatabase();
    void updateDuplicatesView();
    void resetDatabase(const QString &dbPath);
    void startHashing(const QStringList &files, HashMethod method);
    QSqlDatabase db;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
