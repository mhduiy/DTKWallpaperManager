#ifndef DOWNLOADIMAGE_H
#define DOWNLOADIMAGE_H

#include <QObject>
#include <QRunnable>
#include <QImage>
#include "global.h"

class DownloadImage : public QObject, public QRunnable {
    Q_OBJECT
public:
    DownloadImage(const QString &url, int index);
    ~DownloadImage() override;
    void run() override;
signals:
    void downloadFinished(int index, QImage* img);
    void downloadFailed(int index);
private:
    int index;  //文件index
    QString url;   //文件路径
    QImage *img;
    bool isFinish = false;
};

#endif
