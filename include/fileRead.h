#ifndef FILEREAD_H
#define FILEREAD_H

#include <QObject>
#include <QRunnable>
#include <QImage>
#include "global.h"

class FileRead : public QObject, public QRunnable {
    Q_OBJECT
public:
    FileRead(const QString &filePath, int index, imgType type);
    ~FileRead() override;
    void run() override;
signals:
    void readFinished(int index, imgType type, QImage* img);
    void readFailed(int index, imgType type);
private:
    int index;  //文件index
    QString filePath;   //文件路径
    imgType type;
};

#endif
