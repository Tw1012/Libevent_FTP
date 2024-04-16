#include "ftp.h"
#include <QFile>
#include <QDebug>
#include <QFileInfo>

Ftp::Ftp() : ftp(new ftplib()) {
    ftp->SetConnmode(ftplib::port); //这里设置了主动传输模式port
}

Ftp::~Ftp(){

}

bool Ftp::login(const FTP_LOGIN_DATA &data)
{
    if(!ftp->Connect(data.host.toStdString().c_str())){
        qDebug() << QString::fromLocal8Bit("连接IP失败");
        return false;
    }
    return ftp->Login(data.user.toStdString().c_str(),data.pass.toStdString().c_str());
}

QString Ftp::pwd()
{
    QByteArray arr(0x100,'\0');
    //将此时的路径传给arr.
    ftp->Pwd(arr.data(),arr.size());
    return arr;
}

std::vector<FTP_FILE_INFO> Ftp::dir()
{
    //ftp->Dir("dir.txt",cur_path.c_str());
    //获取FTP服务器上名为"dir.txt"的文件列表。如果这个操作失败（返回false），则会直接返回空列表
    if(!ftp->Dir("dir.txt",pwd().toStdString().c_str())) return {};
    //(R"(dir.txt)")是C++11引入的原始字符串字面量，用于表示字符串字面量中的反斜杠不需要转义
    QFile file(R"(dir.txt)");
    file.open(QFile::ReadOnly);
    QString list = file.readAll();
    file.close();

    std::vector<FTP_FILE_INFO> _list;
    QStringList lines = list.split("\r\n"); // 以\r\n划分每个文件或目录为一个元素
    for(auto& line : lines)
    {
        auto args = line.split(" "); //以空格划分每个元素
        args.removeAll(""); //删除所有空字符串    千万别是：" ",因为这是删除空格！！！

        //这里为什么是9呢，因为linux命令(ls -al)显示出来一般每行8个字符串，
        //最后一列是文件名，文件名可能会有大于1个的，所有会有超过8个字符串
        if(args.size() < 9) continue;

        QString date = args[5] + " " + args[6] +  " " + args[7];
        QString file_name = args[8];

        for(int i{9};i < args.size();i++) file_name +=  " " + args[i];

        FTP_FILE_INFO info;
        info.access = args[0];
        info.link_cnt = args[1];
        info.ower = args[2];
        info.group = args[3];
        info.size = args[4];
        info.date = date;
        info.file_name = file_name;

        _list.push_back(info);
    }
    return _list;
}

bool Ftp::cd(const QString &path)
{
    //从path这个路径进入子目录中.
     return ftp->Chdir(path.toStdString().c_str());

}

//返回上一级目录
bool Ftp::cdup()
{
    return ftp->Cdup();
}

bool Ftp::put(const QString &put_file)
{
    //    return ftp->Put(put_file.toStdString().c_str(),"test.txt",ftplib::image);
    return ftp->Put(put_file.toStdString().c_str(), QFileInfo(put_file).fileName().toStdString().c_str(),ftplib::image);
}

bool Ftp::get(const QString &get_file, const QString& remote_file)
{
    //get_file中不能包含中文路径，不然会出错，下载不下来的
    return ftp->Get(get_file.toStdString().c_str(),remote_file.toStdString().c_str(), ftplib::image);
}

bool Ftp::del(const QString &file)
{
    bool ret = ftp->Delete(file.toStdString().c_str());
    if(ret) return true;
    return ftp->Rmdir(file.toStdString().c_str());
}

void Ftp::quit(){
    if(ftp->Quit()){
        qDebug() << QString::fromLocal8Bit("退出连接成功");
    }else{
        qDebug() << QString::fromLocal8Bit("退出连接失败");
    }

}

QString Ftp::error()
{
    QString err = ftp->LastResponse();
    return err;
}
