#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <DListView>
#include <DStackedWidget>
#include <DImageViewer>
#include <DStandardItem>
#include <QStandardItemModel>
#include <QVector>
#include <DPushButton>
#include <dflowlayout.h>
#include <DIconButton>
#include <DScrollArea>
#include <QImage>


DWIDGET_USE_NAMESPACE //使用DWidget命名空间

class MainWindow : public DMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void initUI();  //初始化ui
    void initLocalImgs();    //准备本地图片
    void initOnlineImgs();  //准备在线图片
    void showImgs(const QList<QString> &filePaths, int currentIndex);  //显示图片在流式布局中
    void showlargeImg(const QString &filePath); //显示大图
    void test();

protected:
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void MysetCurrentIndex(int index);

private:
    void resizeImgStackedWidget();

private:
    DListView *funcModLV;    //在线壁纸和本地壁纸的功能模块
    DStackedWidget *mainStacked;   //最外层stack，一个显示主界面，一个显示图片详情
    DStackedWidget *imgStacked;      //图片流界面deepin
    DWidget *mainWidget;    //主界面
    DImageViewer *imgViewer; //图片显示
    QStandardItemModel *model;  //左侧功能区model
    DFlowLayout *imgFlowOnline; //在线壁纸flow
    DFlowLayout *imgFlowLocal;  //本地壁纸flow
    DWidget *imgWidgetOnline;   //在线壁纸窗口
    DWidget *imgWidgetLocal;    //本地壁纸窗口
    DScrollArea *scrollareaLocal;
    DScrollArea *scrollareaOnline;

    QVector<DIconButton*> imgsLocal;    //本地图片
    QVector<DIconButton*> imgsOnline;   //在线图片
    //加载图片步骤：先创建button占位，在前端显示，然后根据index通知子线程从文件中加载图片，子线程读取完毕发送信号和index，前端收到信号，从Image中加载图片到前端
    QHash<int, QImage*> imgsLocalMap;   //从硬盘读取的本地图片
    QHash<int, QImage*> imgsOnlineMap;  //从硬盘中读取的在线图片

    DPushButton *returnBtn; //返回按键
    DPushButton *funcBtn;   //功能按键（刷新页面，设置壁纸）
};

#endif // MAINWINDOWS_H
