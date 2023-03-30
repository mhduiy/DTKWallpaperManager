#ifndef GLOBAL_H
#define GLOBAL_H
#include <QString>
#include <QDir>

const QString CACHEPATH = QDir::homePath() + QDir::separator() + ".config/wallpaperDemo/cache/";


enum class imgType {
    LOCAL=0,    //本地图片
    ONLINE      //在线图片
};

#endif
