#include <DApplication>
#include <DWidgetUtil>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QScreen>
#include "mainWindow.h"
#include "global.h"

DWIDGET_USE_NAMESPACE //使用DWidget命名空间

int main(int argc, char *argv[]) {
    DApplication a(argc, argv);

    a.setApplicationName("DTK壁纸管理器");
    a.setApplicationVersion("1.0");
    a.setProductIcon(QIcon(":/images/logo.svg"));
    a.setApplicationDescription("一个可以浏览本地壁纸和设置在线壁纸的工具");

    //注册类型
    qRegisterMetaType<imgType>("imgType");

    //开启Debug调试
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);
    //获取主屏name和分辨率
    QRect mRect = QGuiApplication::primaryScreen()->geometry();
    QString priScreenName = QGuiApplication::primaryScreen()->name();


    MainWindow w;
    w.show();

    w.setScreenInfo(mRect, priScreenName);  //设置系统主要显示器的信息

    w.resize(static_cast<int>(mRect.width()*0.6),static_cast<int>(mRect.height()*0.6));

    Dtk::Widget::moveToCenter(&w);
    a.exec();
}
