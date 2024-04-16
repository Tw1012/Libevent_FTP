#include "ftpconnectdlg.h"
#include "ui_ftpconnectdlg.h"
#include <QMessageBox>

FtpConnectDlg::FtpConnectDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FtpConnectDlg)
{
    ui->setupUi(this);
    ui->lineEdit_host->setText("10.200.87.129");
    ui->lineEdit_user->setText("Tw");
    ui->lineEdit_password->setText("122427");
    connect(ui->tbn_connect, &QPushButton::clicked, this, &FtpConnectDlg::onConnect);
}

FtpConnectDlg::~FtpConnectDlg()
{
    delete ui;
}

void FtpConnectDlg::onConnect(){
    if(ui->lineEdit_host->text() == "10.200.87.129" && ui->lineEdit_password->text() == "122427"){
        QDialog::accept();
    }else{
        QMessageBox::warning(this, "error", QString::fromLocal8Bit("您的密码或者IP输入错误！！！"));
    }

}

FTP_LOGIN_DATA FtpConnectDlg::ftp_data(){
    FTP_LOGIN_DATA data;
    data.host = ui->lineEdit_host->text();
    data.user = ui->lineEdit_user->text();
    data.pass = ui->lineEdit_password->text();
    return data;
}
