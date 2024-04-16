#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenuBar>
#include <QAction>
#include <QMenu>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QTimerEvent>
#include <QCloseEvent>
#include <QFileDialog>
#include "ftpconnectdlg.h"

void MainWindow::init_menu(){
    QMenu* menu = new QMenu(ui->list_file);
    auto cdupAction = menu->addAction(QString::fromLocal8Bit("返回上一级目录"));
    auto refAction = menu->addAction(QString::fromLocal8Bit("刷新"));
    auto putAction = menu->addAction(QString::fromLocal8Bit("上传"));
    auto getAction = menu->addAction(QString::fromLocal8Bit("下载"));
    auto delAction = menu->addAction(QString::fromLocal8Bit("删除"));

    //the widget emits the QWidget::customContextMenuRequested() signal
    ui->list_file->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->list_file, &QTabWidget::customContextMenuRequested, this, [=](const QPoint& pos){
        //保证右键后，在点击位置(pos)会显示菜单menu
        menu->exec(ui->list_file->viewport()->mapToGlobal(pos));
    });
    connect(cdupAction, &QAction::triggered, this, &MainWindow::cdupAction);
    connect(refAction, &QAction::triggered, this, &MainWindow::refAction);
    connect(putAction, &QAction::triggered, this, &MainWindow::putAction);
    connect(getAction, &QAction::triggered, this, &MainWindow::getAction);
    connect(delAction, &QAction::triggered, this, &MainWindow::delAction);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , ui_connect_status(QString::fromLocal8Bit("就绪"))
    , ui_pwd(QString::fromLocal8Bit(""))
    , ui_cur_time(cur_time())
{
    ui->setupUi(this);

   //创建菜单栏(menubar)
   //连接
   QMenu* menu_connect = menuBar()->addMenu(QString::fromLocal8Bit("连接(&C)"));
   QAction* act_connect = new QAction(QIcon(":/Icon/connect.png"), QString::fromLocal8Bit("开始连接"), this);
   act_connect->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
   menu_connect->addAction(act_connect);
   //退出
   QMenu* menu_disconnect = menuBar()->addMenu(QString::fromLocal8Bit("退出连接(&D)"));
   QAction* act_exit = new QAction(QIcon(":/Icon/disconnect.png"), QString::fromLocal8Bit("断开连接"), this);
   act_exit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
   menu_disconnect->addAction(act_exit);

   //设置状态栏
   ui->statusbar->addWidget(&ui_connect_status); //连接状态
   ui->statusbar->addWidget(&ui_pwd); //路径
   ui->statusbar->addPermanentWidget(&ui_cur_time); //时间

   connect(act_exit, &QAction::triggered, this, [=](){
       if(QMessageBox::question(this, QString::fromLocal8Bit("Exit dialog"), QString::fromLocal8Bit("你确定要退出连接吗？")) != QMessageBox::Yes){
           return;
       }else {
        clear_ui_list();
        ui_connect_status.setText(QString::fromLocal8Bit("就绪"));
        ui_pwd.clear();
        ftp.quit();
       }
   });

   connect(act_connect, &QAction::triggered, this, &MainWindow::onConnect);

   /*
    当计时器事件发生时(即startTimer())，使用QTimerEvent事件参数类调用虚拟timerEvent()函数。重新实现此函数以获取计时器事件。
    如果有多个计时器在运行，QTimerEvent::timerId()可以用来找出哪个计时器被激活了。
   */
   timer1 = startTimer(10); //每隔0.01秒调用timerEvent()函数

   ui->list_file->setColumnCount(7); //设置7列
   ui->list_file->setHorizontalHeaderLabels({QString::fromLocal8Bit("权限"),QString::fromLocal8Bit("链接数"),
                                             QString::fromLocal8Bit("属主"),QString::fromLocal8Bit("属组"),
                                             QString::fromLocal8Bit("大小"),QString::fromLocal8Bit("日期"),
                                             QString::fromLocal8Bit("文件名")});

   connect(ui->list_file, &QTableWidget::cellDoubleClicked, this, &MainWindow::cellDoubleClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::cur_time()
{
    return QTime::currentTime().toString();
}

//通过定时器事件处理函数，可以实现定时更新界面内容的功能
void MainWindow::timerEvent(QTimerEvent *e){
    if(e->timerId() != timer1) return;
    ui_cur_time.setText(cur_time());
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    qDebug() << "closeEvent";
    if(QMessageBox::question(this, QString::fromLocal8Bit("Exit") ,QString::fromLocal8Bit("你确定要退出吗？")) == QMessageBox::Yes) return;
    e->ignore();
}

void MainWindow::onConnect()
{
    FtpConnectDlg dlg;
    if(dlg.exec() != QDialog::Accepted) return;
    if(!ftp.login(dlg.ftp_data())){
        qDebug()<< QString::fromLocal8Bit("连接失败");
        QMessageBox::warning(this, QString::fromLocal8Bit("连接失败"),
                              QString::fromLocal8Bit("未连接上服务器！！！"));
    }
    ui_connect_status.setText(QString::fromLocal8Bit("连接成功：") + dlg.ftp_data().host);

    //连接成功后初始化
    init_menu();

    //清除所有已存在的信息，因为刚登录时不应该有信息
    clear_ui_list();
    //设置状态栏中的信息
    set_pwd();
    //将从ftp服务端得到的信息插入到每一行
    insert_info(ftp.dir());
}

//这个回调函数中的row,是系统自动识别双击的那一行的行号，并传递给槽函数的参数row
void MainWindow::cellDoubleClicked(int row)
{
    qDebug() << QString::fromLocal8Bit("我点击了第") << row << QString::fromLocal8Bit("行");

    //先检查行号是否为-1，如果是则直接返回，表示未双击有效单元格
    if(row == -1)return;

    QString file_name = ui->list_file->item(row,6)->text();

    //当权限不是d时，即不是文件时，一般就是l(符号链接),这种情况下文件名会是多个，
    //符号链接是一种特殊类型的文件，它包含了指向另一个文件或目录的路径信息，
    //所以此时它的文件名应该为文件名链表里的第一个结点，即：file_name.split(" ")[0]
    if(ui->list_file->item(row,0)->text()[0] != 'd') file_name = file_name.split(" ")[0];

    //表示file_name是文件的话，cd进入下一级目录，若是空目录就返回false.
    if(!ftp.cd(file_name)){
        QMessageBox::warning(this,this->windowTitle(),QString::fromLocal8Bit("%1切换失败，%1可能不是目录").arg(file_name));
        return;
    }
    //设置状态栏路径
    set_pwd();

    auto list = ftp.dir();
    if(list.empty()){
        QMessageBox::warning(this,this->windowTitle(),QString::fromLocal8Bit("%1切换失败，%1可能不是目录").arg(file_name));
        return;
    }

    //删除所有表中信息
    clear_ui_list();

    //插入所有表中信息
    insert_info(list);
}

void MainWindow::cdupAction()
{
    if(!ftp.cdup()) return;
    //设置状态栏路径
    set_pwd();
     //删除所有表中信息
    clear_ui_list();
    //插入所有表中信息
    insert_info(ftp.dir());
}

//刷新
void MainWindow::refAction()
{
    qDebug() << QString::fromLocal8Bit("刷新了页面");

    clear_ui_list();
    insert_info(ftp.dir());
}

void MainWindow::putAction()
{
    QFileDialog file(this,QString::fromLocal8Bit("上传文件"));
    file.setFileMode(QFileDialog::FileMode::ExistingFile);
    if(file.exec() != QDialog::Accepted) return;
    auto files = file.selectedFiles();
    if(files.size() > 1) {
        QMessageBox::warning(this,QString::fromLocal8Bit("选择文件过多！"),QString::fromLocal8Bit("最多选择一个文件"));
        return;
    }
    qDebug() << files[0];
    if(!ftp.put(files[0])){
        QMessageBox::warning(this,this->windowTitle(),QString::fromLocal8Bit("上传失败\n错误：") + ftp.error());
        return;
    }

    QMessageBox::information(this,this->windowTitle(),QString::fromLocal8Bit("上传成功！"));

    clear_ui_list();
    insert_info(ftp.dir());
}

void MainWindow::getAction()
{
    int row = ui->list_file->currentRow();
    if(row == -1){
        QMessageBox::warning(this,this->windowTitle(),QString::fromLocal8Bit("没有选择下载文件"));
        return;
    }
    QFileDialog file(this,QString::fromLocal8Bit("文件下载"));
    file.setFileMode(QFileDialog::FileMode::AnyFile);
    file.selectFile(ui->list_file->item(row,6)->text());
    if(file.exec() != QDialog::Accepted)return;

    //返回一个字符串的链表，内容是本地的路径+文件名
    auto files = file.selectedFiles();
    if(files.size() > 1) {
        QMessageBox::warning(this,QString::fromLocal8Bit("选择文件过多！"),QString::fromLocal8Bit("最多选择一个文件"));
        return;
    }
    qDebug() << files[0];
    //files[0]中不能包含中文路径，不然会出错，下载不下来的
    if(!ftp.get(files[0],ui->list_file->item(row,6)->text())){
        QMessageBox::warning(this,this->windowTitle(),QString::fromLocal8Bit("下载失败\n错误：") + ftp.error());
        return;
    }

    QMessageBox::information(this,this->windowTitle(),QString::fromLocal8Bit("下载成功！"));
}

//这个删除功能没有真正的删除！！！
void MainWindow::delAction()
{
    int row = ui->list_file->currentRow();
    if(row == -1){
        QMessageBox::warning(this,this->windowTitle(),QString::fromLocal8Bit("没有选择删除文件"));
        return;
    }
    if(!ftp.del(get_file_name(row)))
    {
        QMessageBox::warning(this,this->windowTitle(),QString::fromLocal8Bit("删除失败\n错误：") + ftp.error());
        return;
    }
    QMessageBox::information(this,this->windowTitle(),QString::fromLocal8Bit("删除成功！"));
    clear_ui_list();
    insert_info(ftp.dir());
}

QString MainWindow::get_file_name(int row)
{
    auto file_name = ui->list_file->item(row,6)->text();
    if(file_name.contains(" ") && ui->list_file->item(row,0)->text()[0] == 'l')
        file_name = file_name.split(" ")[0];
    return file_name;
}

void MainWindow::set_pwd()
{
    ui_pwd.setText(QString::fromLocal8Bit("当前路径：") + ftp.pwd());
}

void MainWindow::clear_ui_list()
{
    //当tablewidget的行数大于0，就删除表中第一行，一直循环直到删除完毕
    while(ui->list_file->rowCount() > 0) ui->list_file->removeRow(0);
}

void MainWindow::insert_info(const std::vector<FTP_FILE_INFO> &list)
{
    for(auto& info : list)
    {
        insert_row(ui->list_file->rowCount(),info);
    }
}

void MainWindow::insert_row(int row, const FTP_FILE_INFO &info)
{
    //将空行插入表的行
    ui->list_file->insertRow(row);

    insert_item(row,0,info.access);
    insert_item(row,1,info.link_cnt);
    insert_item(row,2,info.ower);
    insert_item(row,3,info.group);
    insert_item(row,4,info.size);
    insert_item(row,5,info.date);
    insert_item(row,6,info.file_name);
}

void MainWindow::insert_item(int row,int idx,QString item)
{
    //在row(行)与idx(列)的位置创建一个新表格项，并用item初始化，即将item的值填入
    ui->list_file->setItem(row,idx,new QTableWidgetItem(item));
    //设置每项的属性，这里是：可被选中的、可以被选择和不被选择、可与每项交互
    ui->list_file->item(row,idx)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
}
