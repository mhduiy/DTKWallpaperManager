#include "fileRead.h"
#include <QFile>
#include <QDebug>
FileRead::FileRead(const QString &filePath, int index, imgType type)
{
    this->setAutoDelete(true);  //设置自动析构
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
    QImage *img = new QImage();
    if(!img->load(filePath)) {  //如果读取失败
        emit readFailed(index, this->type);
        delete img; //释放指针指向内存
        return;
    }
    else {
        emit readFinished(index, this->type, img);   //读取成功
    }
}
