#include "mainWindow.h"
#include <QLayout>
#include <DTitlebar>
#include <QDebug>
#include <QDir>

const QString LOCALWALLPAPERPATH = "/usr/share/wallpapers/deepin";

MainWindow::MainWindow(QWidget *parent) : DMainWindow (parent)
{
    this->titlebar()->setTitle("DTK壁纸管理器");
    this->titlebar()->setIcon(QIcon(":/images/logo.svg"));
    initUI();
    test();
}

MainWindow::~MainWindow()
{

}

void MainWindow::initUI()
{
    //设置titleBar部分
    returnBtn = new DPushButton("返回");
    funcBtn = new DPushButton("再来一组");
    returnBtn->setFixedWidth(50);
    funcBtn->setFixedWidth(100);
    //设置这两个按钮的对齐方式
    this->titlebar()->addWidget(returnBtn, Qt::AlignLeft);
    this->titlebar()->addWidget(funcBtn, Qt::AlignRight);
    //最外层stackedWidget
    mainStacked = new DStackedWidget(this);
    //主界面窗口
    mainWidget = new DWidget();
    //设置主界面的layout
    QHBoxLayout *mainLayout = new QHBoxLayout(mainWidget);

    //左侧功能模块按钮区
    funcModLV  = new DListView();
    model = new QStandardItemModel();
    model->appendRow(new DStandardItem("本地壁纸"));
    model->appendRow(new DStandardItem("在线壁纸"));
    funcModLV->setModel(model);
    funcModLV->setFixedWidth(150);

    //右侧图片流显示区域
    scrollareaLocal = new DScrollArea();
    scrollareaOnline = new DScrollArea();

    imgStacked  = new DStackedWidget();
    imgWidgetLocal = new DWidget();
    imgWidgetOnline = new DWidget();
    imgFlowLocal = new DFlowLayout();
    imgFlowOnline = new DFlowLayout();

    //设置流式布局属性
    imgFlowLocal->setSpacing(20);
    imgFlowOnline->setSpacing(20);
    imgFlowLocal->setFlow(QListView::Flow::LeftToRight);

    //设置流式布局
    imgWidgetLocal->setLayout(imgFlowLocal);
    imgWidgetOnline->setLayout(imgFlowOnline);

    //将两个流式布局添加到DScrollArea中
    scrollareaLocal->setWidget(imgWidgetLocal);
    scrollareaOnline->setWidget(imgWidgetOnline);

    //将两个DScrollArea添加到imgStacked中
    imgStacked->addWidget(scrollareaLocal);
    imgStacked->addWidget(scrollareaOnline);

    //将左侧功能区和右侧图片流区域添加在mainLayout
    mainLayout->addWidget(funcModLV);
    mainLayout->addWidget(imgStacked);

    //将主界面和显示图片大图的界面添加到最外层StackedWidget
    mainStacked->addWidget(mainWidget);
    imgViewer = new DImageViewer();
    mainStacked->addWidget(imgViewer);

    this->setCentralWidget(mainStacked);

    //连接信号和槽
    connect(funcModLV, &DListView::clicked, [this](const QModelIndex &index){
        this->imgStacked->setCurrentIndex(index.row());
        this->resizeImgStackedWidget();
    });

    QModelIndex index = funcModLV->model()->index(0,0);
    funcModLV->setCurrentIndex(index);
}

void MainWindow::initLocalImgs()
{
    QDir dir(LOCALWALLPAPERPATH);
    QStringList &&list = dir.entryList(QDir::Files);
    QStringList filePaths;
    for(const QString &it:list) {
        filePaths.append(LOCALWALLPAPERPATH + '/' + it);
    }
    showImgs(filePaths, 0);
}

void MainWindow::initOnlineImgs()
{
    QPushButton btn;
    QImage img;
    btn.setIcon(QIcon(QPixmap::fromImage(img)));



}

void MainWindow::showImgs(const QList<QString> &filePaths, int currentIndex)
{
    if(currentIndex == 0) {
//        imgFlowLocal-
//        //清除指针对象
//        for(int i = 0; i < imgsLocal.size(); i++) {
//            delete imgsLocal[i];
//            imgsLocal.remove(i);
//        }
        for(const QString &filePath:filePaths) {
            DIconButton *btn = new DIconButton();
            btn->setFixedSize(300,200);
            btn->setIcon(QIcon(filePath));
            btn->setIconSize(btn->size());
            imgsLocal.push_back(btn);
            imgFlowLocal->addWidget(btn);
        }
    }
    else {

    }
}

void MainWindow::showlargeImg(const QString &filePath)
{

}

void MainWindow::test()
{
    initLocalImgs();
    for(int i = 0; i < 30; i++) {
        DPushButton *btn = new DPushButton(QString("Ltest %1").arg(i));
        btn->setFixedSize(300,200);
        imgFlowLocal->addWidget(btn);
    }

    for(int i = 0; i < 30; i++) {
        DPushButton *btn = new DPushButton(QString("Otest %1").arg(i));
        btn->setFixedSize(300,200);
        imgFlowOnline->addWidget(btn);
    }
    this->resizeImgStackedWidget();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    resizeImgStackedWidget();
}

void MainWindow::MysetCurrentIndex(int index)
{
    qInfo() << index;
}


void MainWindow::resizeImgStackedWidget()
{
    qDebug() << imgStacked->currentIndex();
    if(imgStacked->currentIndex() == 0) {
        int flowHeight = imgFlowLocal->heightForWidth(scrollareaLocal->width());
        imgWidgetLocal->setFixedWidth(scrollareaLocal->width());
        imgWidgetLocal->setFixedHeight(flowHeight);
    }
    else {
        int flowHeight = imgFlowOnline->heightForWidth(scrollareaOnline->width());
        imgWidgetOnline->setFixedWidth(scrollareaOnline->width());
        imgWidgetOnline->setFixedHeight(flowHeight);
    }
}
