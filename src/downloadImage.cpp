#include "downloadImage.h"
#include <QEventLoop>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QDebug>
#include <QFile>

DownloadImage::DownloadImage(const QString &url, int index)
{
    setAutoDelete(true);
    this->url = url;
    this->index = index;
}

DownloadImage::~DownloadImage()
{
    qDebug() << "销毁成功";
}

void DownloadImage::run()
{
    qDebug() << "开始下载" << index << url;
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    QNetworkAccessManager *manager = new QNetworkAccessManager;
    QNetworkReply *reply = manager->get(request);
    //设置一个局部事件循环
    QEventLoop eventLoop;
    //下载完成，事件停止
    connect(reply, &QNetworkReply::finished,[&](){
        isFinish = true;
        eventLoop.quit();
    });
    QTimer *timer = new QTimer;
    //超时，事件停止
    connect(timer, &QTimer::timeout, &eventLoop, [&](){
        isFinish = false;
        eventLoop.quit();
    });
    //设置不循环
    timer->setSingleShot(true);
    //超时时间为8s
    timer->start(16000);
    //开始事件循环
    eventLoop.exec();

    if(isFinish) {
        QByteArray bytes = reply->readAll();
        img = new QImage();
        qDebug() << "下载成功" << index << "::" << url;
        img->loadFromData(bytes);
        emit downloadFinished(index, img);

    }
    else {
        emit downloadFailed(index);
        qDebug() << "下载失败" << index << "::" << url;
    }

    //释放内存
    timer->stop();
    timer->deleteLater();
    reply->close();
    reply->deleteLater();
}
