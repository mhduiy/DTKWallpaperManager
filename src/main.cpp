#include <DApplication>
#include <DWidgetUtil>
#include <QLoggingCategory>
#include "mainWindow.h"

DWIDGET_USE_NAMESPACE //使用DWidget命名空间

int main(int argc, char *argv[]) {
    DApplication a(argc, argv);

    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);

    MainWindow w;

    w.resize(static_cast<int>(1920.0*0.6),static_cast<int>(1080*0.6));

    w.show();

    Dtk::Widget::moveToCenter(&w);
    a.exec();
}
