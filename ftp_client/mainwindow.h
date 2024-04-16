#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "ftp.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void timerEvent(QTimerEvent *e);
    void closeEvent(QCloseEvent* e);

public slots:
    void cdupAction();
    void refAction();
    void putAction();
    void getAction();
    void delAction();
    void onConnect();
    void cellDoubleClicked(int row);

    static QString cur_time();

private:
    //初始化菜单
    void init_menu();

    QString get_file_name(int row);
    void set_pwd();
    void clear_ui_list();
    void insert_info(const std::vector<FTP_FILE_INFO>& list);
    void insert_row(int row,const FTP_FILE_INFO& info);
    void insert_item(int row,int idx,QString item);

private:
    Ui::MainWindow *ui;
    QLabel ui_connect_status;
    QLabel ui_pwd;
    QLabel ui_cur_time;
    int timer1;
    Ftp ftp;
};
#endif // MAINWINDOW_H
