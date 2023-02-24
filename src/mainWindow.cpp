#include "mainWindow.h"
#include <QLayout>
#include <DTitlebar>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include "fileRead.h"

const QString LOCALWALLPAPERPATH = "/usr/share/wallpapers/deepin";
const QSize ICONBTNSIZE(300,200);

MainWindow::MainWindow(QWidget *parent) : DMainWindow (parent)
{
    //设置全局线程池
    pool = QThreadPool::globalInstance();
    //设置最大线程数
    pool->setMaxThreadCount(12);

    this->titlebar()->setTitle("DTK壁纸管理器");
    this->titlebar()->setIcon(QIcon(":/images/logo.svg"));
    initUI();
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
    //设置两个按钮默认隐藏
    returnBtn->hide();
    funcBtn->hide();
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

    //连接信号和槽，左边功能按钮和右边的界面
    connect(funcModLV, &DListView::clicked, [this](const QModelIndex &index){
        this->imgStacked->setCurrentIndex(index.row());
        this->resizeImgStackedWidget();
    });

    //返回按钮被按下
    connect(returnBtn, &DPushButton::clicked, this, [this](){
        this->mainStacked->setCurrentIndex(0);
        //隐藏
        this->returnBtn->hide();
        if(this->imgStacked->currentIndex() == 0){    //当前显示本地图片界面，隐藏功能按键
            this->funcBtn->hide();
        }
    });

    //mainStacked页面被切换
    connect(mainStacked, &DStackedWidget::currentChanged, [this](int index){
        if(index == 1) {    //大图界面
            this->funcBtn->setText("设置壁纸");
            this->funcBtn->show();
        }
        else if(index == 0 && this->imgStacked->currentIndex() == 1){   //在线图片界面
            this->funcBtn->setText("再来一组");
        }
        else {  //本地图片界面
            this->funcBtn->setText("再来一组");
            this->funcBtn->hide();
        }
    });

    //图片显示界面stackedwidget被切换
    connect(imgStacked, &DStackedWidget::currentChanged, [this](int index){
        if(index == 0) {
            this->funcBtn->hide();
        }
        else {
            this->funcBtn->setText("再来一组");
            this->funcBtn->show();
        }
    });

    connect(funcBtn, &DPushButton::clicked, this, [this](){
        if(this->mainStacked->currentIndex() == 1) {
            this->setWallPaper(this->imgDetailIndex, this->imgDetailType);
        }
    });


    QModelIndex index = funcModLV->model()->index(0,0);
    funcModLV->setCurrentIndex(index);

    //读取本地壁纸
    readLocalWallPaper();

}

void MainWindow::readImgFromFile(const QString &filePath, int index, imgType type)
{
    FileRead *myRead = new FileRead(filePath, index, type);
    //建立接受读取完成的连接
    connect(myRead, &FileRead::readFinished, this, &MainWindow::acceptReadFinish);
    //提交到线程池并开始执行
    pool->start(myRead);
}

void MainWindow::readLocalWallPaper()
{
    QDir dir(LOCALWALLPAPERPATH);
    //读取所有文件的文件名
    QStringList &&list = dir.entryList(QDir::Files);
    QString filePath;
    int index = 0;
    qDebug() << "本地图片数量：" << list.size();
    //前端先设置IconButton占位，再执行加载操作
    setCorrectBtnToUI(imgType::LOCAL, list.size());
    //建立子线程读取所有文件
    for(const QString &it:list) {
        filePath = LOCALWALLPAPERPATH + '/' + it;
        imgsLocalPath.push_back(filePath);
        readImgFromFile(filePath, index++, imgType::LOCAL);
    }
}

//void MainWindow::initOnlineImgs()
//{
//    QPushButton btn;
//    QImage img;
//    btn.setIcon(QIcon(QPixmap::fromImage(img)));
//}

void MainWindow::showImgs(const QList<QString> &filePaths, int currentIndex)
{
//    if(currentIndex == 0) {
////        imgFlowLocal-
////        //清除指针对象
////        for(int i = 0; i < imgsLocal.size(); i++) {
////            delete imgsLocal[i];
////            imgsLocal.remove(i);
////        }
//        for(const QString &filePath:filePaths) {
//            DIconButton *btn = new DIconButton();
//            btn->setFixedSize(300,200);
//            btn->setIcon(QIcon(filePath));
//            btn->setIconSize(btn->size());
//            imgsLocal.push_back(btn);
//            imgFlowLocal->addWidget(btn);
//        }
//    }
//    else {

    //    }
}

void MainWindow::showDetailImg(int index, imgType type)
{
    this->imgDetailType = type;
    this->imgDetailIndex = index;

    if(type == imgType::LOCAL) {
        imgViewer->setImage(*imgsLocalMap.find(index).value());
    }
    else {
        imgViewer->setImage(*imgsOnlineMap.find(index).value());
    }
    //设置默认缩放比例
    imgViewer->autoFitImage();
    imgViewer->update();
    qDebug() << imgViewer->size();
    mainStacked->setCurrentIndex(1);
    //显示返回按钮
    returnBtn->show();
}

bool MainWindow::setWallPaper(int index, imgType type)
{
    QStringList options;
    QString filePath;
    if(type == imgType::LOCAL) {
        filePath = imgsLocalPath[index];
    }
    else {
        filePath = imgsOnlinePath[index];
    }
    QString setWallpaper = QString("for screen in  `xrandr|grep ' connected'|awk '{print $1}'`; do dbus-send --dest=com.deepin.daemon.Appearance /com/deepin/daemon/Appearance --print-reply com.deepin.daemon.Appearance.SetMonitorBackground string:$screen string:'")
            .append(filePath).append("';done");
    options << "-c" << setWallpaper;
    QProcess::execute("/bin/bash",options);

    this->sendMessage(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), "壁纸设置成功");

    return true;
}

void MainWindow::removeAllImgs(imgType type)
{
    //移除所有显示
    //释放空间
    //大小设置为0
    if(type ==  imgType::LOCAL) {
        imgsLocalPath.clear();
        for (DIconButton *it : this->imgsLocal) {
            this->imgFlowLocal->removeWidget(it);   //前端删除
            delete it;  //释放空间
        }
        this->imgsLocal.resize(0);
    }
    else if(type == imgType::ONLINE) {
        imgsOnlinePath.clear();
        for (DIconButton *it : this->imgsOnline) {
            this->imgFlowOnline->removeWidget(it);   //前端删除
            delete it;  //释放空间
        }
        this->imgsOnline.resize(0);
    }
}

void MainWindow::test()
{
//    readLocalWallPaper();
//    for(int i = 0; i < 30; i++) {
//        DPushButton *btn = new DPushButton(QString("Ltest %1").arg(i));
//        btn->setFixedSize(300,200);
//        imgFlowLocal->addWidget(btn);
//    }

//    for(int i = 0; i < 30; i++) {
//        DPushButton *btn = new DPushButton(QString("Otest %1").arg(i));
//        btn->setFixedSize(300,200);
//        imgFlowOnline->addWidget(btn);
//    }
//    this->resizeImgStackedWidget();
}

void MainWindow::setCorrectBtnToUI(imgType type, int sum)
{
    if(sum <= 0) {
        qWarning() << "setCorrectBtnToUI: size not correct";
        exit(-1);
    }
    if(type == imgType::LOCAL) {
        imgsLocal.resize(sum);    //设置容器大小
        for(int i = 0; i < imgsLocal.size(); i++) {
            imgsLocal[i]= new DIconButton(); //开辟新空间
            imgsLocal[i]->setFixedSize(ICONBTNSIZE);  //设置按钮大小
            imgsLocal[i]->setIconSize(ICONBTNSIZE);   //设置按钮图标大小
            this->imgFlowLocal->addWidget(imgsLocal[i]);  //添加到对应流式布局

            connect(imgsLocal[i], &DIconButton::clicked, this, [this,i](){
                this->showDetailImg(i,imgType::LOCAL);
            });
        }
    }
    else if(type == imgType::ONLINE) {
        this->imgsOnline.resize(sum);
        for(int i = 0; i < imgsOnline.size(); i++) {
            imgsOnline[i] = new DIconButton();
            imgsOnline[i]->setFixedSize(ICONBTNSIZE);
            imgsOnline[i]->setIconSize(ICONBTNSIZE);
            this->imgFlowOnline->addWidget(imgsOnline[i]);

            connect(imgsOnline[i], &DIconButton::clicked, this, [this,i](){
                this->showDetailImg(i,imgType::ONLINE);
            });
        }
    }
    qDebug() << "scrolocal" << scrollareaLocal->size();
    qDebug() << "widget" << imgWidgetLocal->size();
    resizeImgStackedWidget();   //更新flow大小
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    resizeImgStackedWidget();
}

void MainWindow::MysetCurrentIndex(int index)
{
    qInfo() << index;
}

void MainWindow::acceptReadFinish(int index, imgType type, QImage* img)
{
    switch (type) {
        case imgType::LOCAL:
            this->imgsLocalMap.insert(index, img);  //存入 map
            if(imgsLocal.size() > index) {
                this->imgsLocal[index]->setIcon(QPixmap::fromImage(*img));  //显示到前端
            }
            break;
        case imgType::ONLINE:
            this->imgsOnlineMap.insert(index, img);
            if(imgsOnline.size() > index) {
                this->imgsOnline[index]->setIcon(QPixmap::fromImage(*img));  //显示到前端
            }
            break;
    }
}


void MainWindow::resizeImgStackedWidget()
{
    qDebug() << "1stack" << imgStacked->size();
    qDebug() << "1scrolocal" << scrollareaLocal->size();
    qDebug() << "1widget" << imgWidgetLocal->size();

    qDebug() << imgStacked->currentIndex();
    if(imgStacked->currentIndex() == 0) {
        int flowHeight = imgFlowLocal->heightForWidth(scrollareaLocal->width());
        if(flowHeight < 0) {    //过滤没有内容的情况
            return;
        }
        imgWidgetLocal->setFixedWidth(scrollareaLocal->width());
        imgWidgetLocal->setFixedHeight(flowHeight);
    }
    else {
        int flowHeight = imgFlowOnline->heightForWidth(scrollareaOnline->width());
        if(flowHeight < 0) {
            return;
        }
        imgWidgetOnline->setFixedWidth(scrollareaOnline->width());
        imgWidgetOnline->setFixedHeight(flowHeight);
    }
}
