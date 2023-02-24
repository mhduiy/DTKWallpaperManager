#include <DApplication>
#include <DWidgetUtil>
#include <QLoggingCategory>
#include "mainWindow.h"
#include "global.h"

DWIDGET_USE_NAMESPACE //使用DWidget命名空间

int main(int argc, char *argv[]) {
    DApplication a(argc, argv);

    qRegisterMetaType<imgType>("imgType");

    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);

    MainWindow w;
    w.show();
    w.resize(static_cast<int>(1920.0*0.6),static_cast<int>(1080*0.6));

    Dtk::Widget::moveToCenter(&w);
    a.exec();
}
