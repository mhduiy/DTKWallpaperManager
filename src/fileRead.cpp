#include "fileRead.h"
#include <QFile>
#include <QDebug>
FileRead::FileRead(const QString &filePath, int index, imgType type)
{
    //设置线程完成自动析构
    this->setAutoDelete(true);

    //保存参数
    this->index = index;
    this->filePath  = filePath;
    this->type = type;
}

FileRead::~FileRead()
{
    qDebug() << "线程销毁";
}

void FileRead::run()
{
    //创建一个QImage对象，保存图片
    QImage *img = new QImage();
    //如果读取失败
    if(!img->load(filePath)) {
        //发送读取失败的信号
        emit readFailed(index, this->type);
        //释放内存并退出函数
        delete img;
        return;
    }
    //读取成功，发送读取成功信号和图片内存地址
    else {
        emit readFinished(index, this->type, img);   //读取成功
    }
}
