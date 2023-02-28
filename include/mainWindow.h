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
#include <QThreadPool>
#include <QNetworkAccessManager>
#include <DSpinner>
#include "global.h"

DWIDGET_USE_NAMESPACE //使用DWidget命名空间

class MainWindow : public DMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void initUI();  //初始化ui
    void readImgFromFile(const QString &filePath, int index, imgType type);    //读取本地图片文件
    void readLocalWallPaper();          //读取本地壁纸文件
    void readOnlineWallPaper();         //读取在线壁纸
    void removeAllImgs(imgType type);   //移除所有图片内存
    void setCorrectBtnToUI(imgType type, int sum);  //提前设置按钮在前端显示
    void setScreenInfo(const QRect &rect, const QString &name);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void acceptReadFinish(int index, imgType type, QImage* img);    //接收读取成功的消息
    void acceptDLFailed(int index);                 //接收下载失败的信号
    void showDetailImg(int index, imgType type);    //显示大图
    bool setWallPaper(int index, imgType type);     //设置壁纸

private:
    void resizeImgStackedWidget();

private:
    QThreadPool *pool;                  //全局线程池
    DListView *funcModLV;               //在线壁纸和本地壁纸的功能模块
    DStackedWidget *mainStacked;        //最外层stack，一个显示主界面，一个显示图片详情
    DStackedWidget *imgStacked;         //图片流界面deepin
    DWidget *mainWidget;                //主界面
    DImageViewer *imgViewer;            //图片显示
    QStandardItemModel *model;          //左侧功能区model
    DFlowLayout *imgFlowOnline;         //在线壁纸flow
    DFlowLayout *imgFlowLocal;          //本地壁纸flow
    DWidget *imgWidgetOnline;           //在线壁纸窗口
    DWidget *imgWidgetLocal;            //本地壁纸窗口
    DScrollArea *scrollareaLocal;       //本地壁纸滚动区域
    DScrollArea *scrollareaOnline;      //在线壁纸滚动区域

    DWidget *imgfixWidgetLocal;         //本地壁纸外层窗口，在scrollArea内一层，用于居中其内部的Widget(该widget为流布局)，实现流布局的居中
    DWidget *imgfixWidgetOnline;        //在线壁纸外层窗口，...

    QNetworkAccessManager  *networkAccessManager;   //网络连接管理

    QVector<DIconButton*> imgsLocal;    //本地图片
    QVector<DIconButton*> imgsOnline;   //在线图片

    QHash<int, QImage*> imgsLocalMap;   //从硬盘读取的本地图片
    QHash<int, QImage*> imgsOnlineMap;  //从硬盘中读取的在线图片

    DPushButton *returnBtn;             //返回按键
    DPushButton *funcBtn;               //功能按键（刷新页面，设置壁纸）
    DPushButton *saveBtn;               //保存壁纸按钮

    DSpinner *spinner;                  //加载spinner

    int imgDetailIndex;                 //当前显示大图图片的index
    imgType imgDetailType;              //当前显示大图图片的类型
    bool isFirstOnline = true;          //是否是第一次浏览在线图片
    QRect priScreenRect;                //当前显示器的分辨率
    QString priScreenName;              //当前的显示器的name
};

#endif // MAINWINDOWS_H
