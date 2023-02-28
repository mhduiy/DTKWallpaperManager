#include "mainWindow.h"
#include <QLayout>
#include <DTitlebar>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkReply>
#include <QJsonObject>
#include "fileRead.h"
#include "downloadImage.h"
#include <DFileDialog>
#include <QTimer>

//本地壁纸路径
const QString LOCALWALLPAPERPATH = "/usr/share/wallpapers/deepin";
//设置DIconButton及其Icon(界面的图片按钮)的默认大小
const QSize ICONBTNSIZE(300,200);
//DIconButton 的样式
const QString BTNSTYLE = "background-color: rgba(0, 0, 0, 0.055);border: 0px;";
//填写client_id
const QString client_id = "1bld7WzB18JZCImZ2lTUhbXxI8_zrA9ju36e9HmKgWc";

MainWindow::MainWindow(QWidget *parent) : DMainWindow (parent)
{
    //设置全局线程池
    pool = QThreadPool::globalInstance();
    //设置最大线程数
    pool->setMaxThreadCount(12);
    //设置主界面的title和icon
    this->titlebar()->setTitle("DTK壁纸管理器");
    this->titlebar()->setIcon(QIcon(":/images/logo.svg"));
    //初始化ui
    initUI();
}

MainWindow::~MainWindow()
{
    //退出前释放申请的内存
    removeAllImgs(imgType::LOCAL);
    removeAllImgs(imgType::ONLINE);
}

void MainWindow::initUI()
{
    //设置titleBar部分
    returnBtn = new DPushButton();
    funcBtn = new DPushButton("再来一组");
    spinner = new DSpinner();
    saveBtn = new DPushButton("保存壁纸");
    //设置加入titlebar各个组件的默认大小
    returnBtn->setFixedWidth(40);
    saveBtn->setFixedWidth(80);
    funcBtn->setFixedWidth(80);
    spinner->setFixedSize(30,30);

    returnBtn->setIcon(QIcon(":/images/left-arrow.png"));
    returnBtn->setIconSize(QSize(20,20));

    //将设置的组件默认隐藏
    returnBtn->hide();
    funcBtn->hide();
    spinner->hide();
    saveBtn->hide();
    //添加组件到titlebar并设置对齐方式
    this->titlebar()->addWidget(returnBtn, Qt::AlignLeft);
    this->titlebar()->addWidget(spinner, Qt::AlignRight);
    this->titlebar()->addWidget(saveBtn, Qt::AlignRight);
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
    //向功能区添加两个项目
    model->appendRow(new DStandardItem("本地壁纸"));
    model->appendRow(new DStandardItem("在线壁纸"));
    funcModLV->setModel(model);
    funcModLV->setFixedWidth(150);

    //右侧图片流显示区域
    //新建滚动区域
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
    //设置flow方向
    imgFlowLocal->setFlow(QListView::Flow::LeftToRight);
    imgFlowOnline->setFlow(QListView::Flow::LeftToRight);

    //将本地和在线的图片窗口设置为流式布局
    imgWidgetLocal->setLayout(imgFlowLocal);
    imgWidgetOnline->setLayout(imgFlowOnline);

    //将两个流式布局的窗口添加到滚动区域中
    scrollareaLocal->setWidget(imgWidgetLocal);
    scrollareaOnline->setWidget(imgWidgetOnline);

    //将两个滚动区域添加到imgStacked中
    imgStacked->addWidget(scrollareaLocal);
    imgStacked->addWidget(scrollareaOnline);

    //将左侧功能区和右侧图片流区域添加在mainLayout
    mainLayout->addWidget(funcModLV);
    mainLayout->addWidget(imgStacked);

    //将主界面和显示图片大图的界面添加到最外层StackedWidget
    mainStacked->addWidget(mainWidget);
    imgViewer = new DImageViewer();
    mainStacked->addWidget(imgViewer);

    //设置中心界面
    this->setCentralWidget(mainStacked);

    //新建一个网络访问对象，用于api获取url
    networkAccessManager = new QNetworkAccessManager;

    //连接信号和槽，左边功能按钮和右边的界面
    connect(funcModLV, &DListView::clicked, [this](const QModelIndex &index){
        //实现点击左侧功能按钮右边界面切换
        this->imgStacked->setCurrentIndex(index.row());
        //切换后刷新流式布局窗口大小，始终正确显示到界面上
        this->resizeImgStackedWidget();
    });

    //返回按钮被按下，此处从大图界面切换回图片浏览界面
    connect(returnBtn, &DPushButton::clicked, this, [this](){
        //实现返回图片浏览界面
        this->mainStacked->setCurrentIndex(0);
        //隐藏返回按钮
        this->returnBtn->hide();
        //当前显示本地图片界面，隐藏功能按键
        if(this->imgStacked->currentIndex() == 0){
            this->funcBtn->hide();
        }
    });

    //mainStacked页面被切换，也就是切换浏览壁纸和查看大图的界面
    connect(mainStacked, &DStackedWidget::currentChanged, [this](int index){
        //切换后刷新流式布局窗口大小，始终正确显示到界面上
        this->resizeImgStackedWidget();
        //如果当前在大图界面
        if(index == 1) {
            //设置大图页面的对应功能按钮文字，并显示功能按钮和保存按钮
            this->funcBtn->setText("设置壁纸");
            this->funcBtn->show();
            this->saveBtn->show();
        }
        //如果当前在线图片界面
        else if(index == 0 && this->imgStacked->currentIndex() == 1){
            //功能按钮设置为再来一组并隐藏保存按钮
            this->funcBtn->setText("再来一组");
            this->saveBtn->hide();
        }
        //本地图片界面，与上面同理，本地不需要再刷新，直接隐藏功能按钮
        else {
            this->funcBtn->setText("再来一组");
            this->funcBtn->hide();
            this->saveBtn->hide();
        }
    });

    //图片显示界面stackedwidget被切换，也就是浏览本地壁纸和在线壁纸的界面被切换
    connect(imgStacked, &DStackedWidget::currentChanged, [this](int index){
        //本地壁纸界面，隐藏功能按钮
        if(index == 0) {
            this->funcBtn->hide();
        }
        //在线壁纸界面，显示功能按钮
        else {
            this->funcBtn->setText("再来一组");
            this->funcBtn->show();
            //如果是第一次点击在线壁纸界面，则直接加载在线图片，并将保存第一次的变量设置为false
            if(this->isFirstOnline) {
                this->readOnlineWallPaper();
                this->isFirstOnline = false;
            }
        }
        //刷新设置流式布局的窗口大小，防止界面显示不正确
        this->resizeImgStackedWidget();
    });

    //功能按键被按下【再来一组或者保存壁纸】
    connect(funcBtn, &DPushButton::clicked, this, [this](){
        //如果当前在查看大图界面，则功能按钮为设置壁纸
        if(this->mainStacked->currentIndex() == 1) {
            //调用设置壁纸函数
            this->setWallPaper(this->imgDetailIndex, this->imgDetailType);
        }
        //在线壁纸界面功能按钮为再来一组，加载在线图片
        else if(this->mainStacked->currentIndex() == 0 && this->imgStacked->currentIndex() == 1){
            //调用加载在线图片的函数
            this->readOnlineWallPaper();
        }
    });

    //保存壁纸按钮被按下
    connect(saveBtn, &DPushButton::clicked, this, [this](){
        //向用户打开选择保存位置的对话框
        //四个参数分别是 父窗口 标题 默认文件名 文件过滤条件
        QString outPath = DFileDialog::getSaveFileName(this, "请选择保存路径", "out.jpg", "JPG(*.jpg)");
        //设置一个变量 是否保存成功  默认为否
        bool saveSuccess = false;
        //如果用户没有选择路径，或点击取消，则直接退出函数，不做响应
        if(outPath.isEmpty()) {
            return;
        }
        //若当前大图展示界面为本地图片，这里的变量在用户点击图片展示大图后就已经设置
        if(imgDetailType == imgType::LOCAL) {
            //从map中找到图片并保存为jpg，并接收返回值，判断是否保存成功
            saveSuccess = imgsLocalMap.find(imgDetailIndex).value()->save(outPath, "jpg");
        }
        else {
            saveSuccess = imgsOnlineMap.find(imgDetailIndex).value()->save(outPath, "jpg");
        }
        //保存成功或者失败都会从主界面发送消息展示给用户
        if(saveSuccess) {
            this->sendMessage(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), "保存成功");
        }
        else {
            this->sendMessage(QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning), "保存失败");
        }

    });

    //设置进入主界面左侧功能按键区域默认选中在本地壁纸
    QModelIndex index = funcModLV->model()->index(0,0);
    funcModLV->setCurrentIndex(index);

    //进入主界面读取本地壁纸
    readLocalWallPaper();
}

//在本地读取图片，这里使用多线程读取
void MainWindow::readImgFromFile(const QString &filePath, int index, imgType type)
{
    //将读取操作的参数设置到子线程
    FileRead *myRead = new FileRead(filePath, index, type);
    //建立接受读取完成的连接
    connect(myRead, &FileRead::readFinished, this, &MainWindow::acceptReadFinish);
    //提交到线程池并开始执行
    pool->start(myRead);
}

//读取本地所有壁纸
void MainWindow::readLocalWallPaper()
{
    //设置一个dir读取指定文件夹下的所有文件
    QDir dir(LOCALWALLPAPERPATH);
    //读取所有文件的文件名
    QStringList &&list = dir.entryList(QDir::Files);
    QString filePath;
    //该变量用于为各个文件建立索引
    int index = 0;
    //前端先设置IconButton占位，再执行加载操作
    setCorrectBtnToUI(imgType::LOCAL, list.size());
    //子线程读取所有文件
    for(const QString &it:list) {
        //设置文件绝对路径并开始读取
        filePath = LOCALWALLPAPERPATH + '/' + it;
        readImgFromFile(filePath, index++, imgType::LOCAL);
    }
}

//读取在线壁纸
void MainWindow::readOnlineWallPaper()
{
    //读取前先将已经存放的壁纸内存释放
    removeAllImgs(imgType::ONLINE);

    //设置读取url的状态：设置功能按钮文字，设置功能按钮无法点击
    funcBtn->setText("获取url...");
    funcBtn->setEnabled(false);
    //显示加载控件，并开始动画
    spinner->show();
    spinner->start();

    //拼接api的url
    QString url = "https://api.unsplash.com/photos/?";

    url.append("client_id=").append(client_id);
    url.append("&per_page=30");
    QNetworkRequest request(url);

    //
    networkAccessManager->get(request);

    //get完成，成功返回，开始读取图片url
    connect(networkAccessManager,&QNetworkAccessManager::finished,[&](QNetworkReply * reply){

        //功能按钮还原，加载控件隐藏
        funcBtn->setText("再来一组");
        funcBtn->setEnabled(true);
        spinner->stop();
        spinner->hide();
        //获取成功后，进行json解析，获取图片url
        QJsonArray array = QJsonDocument::fromJson(reply->readAll()).array();
         //未知原因，该connect会被调用两次，这里过滤无效的一次
        if(array.size() <= 0) {
            return;
        }

        //更加图片url的数量，前端设置对于数量的DIconButton占位，等待加载图片
        setCorrectBtnToUI(imgType::ONLINE, array.size());

        //处理返回的json数据，提取图片url
        for (int i = 0; i < array.size(); i++) {
            if (array.at(i).isObject()) {
                QJsonObject object = array.at(i).toObject();
                if (object.contains("urls")) {
                    //将图片url作特殊处理
                    QJsonObject urlObject = object.value("urls").toObject();
                    QString url = urlObject.value("regular").toString();
                    url = url.left(url.indexOf("&q="));
                    url = url.right(url.size()-28);
                    url = QString("https://dogefs.s3.ladydaily.com/~/source/unsplash/") + url;
                    url.append(QString("&q=100&w=%1&h=%2").arg(priScreenRect.width()).arg(priScreenRect.height()));
                    //子线程中下载，将信息传送过去
                    DownloadImage *runner = new DownloadImage(url, i);
                    //建立子线程下载成功的连接
                    connect(runner, &DownloadImage::downloadFinished, this, [this](int index, QImage* img){
                        //调用接收数据的函数
                        this->acceptReadFinish(index, imgType::ONLINE, img);
                    });
                    //在子线程开始执行
                    pool->start(runner);
                }
            }
        }
    });

}

//显示大图
void MainWindow::showDetailImg(int index, imgType type)
{
    //将用户点击的图片信息保存，用于设置壁纸和保存壁纸
    this->imgDetailType = type;
    this->imgDetailIndex = index;

    //根据点击信息判断从本地壁纸map中寻找img还是在线壁纸map中寻找img
    if(type == imgType::LOCAL) {
        if(imgsLocalMap.find(index) == imgsLocalMap.end()) {
            return;
        }
        //将找到的img放到imgViewer中显示
        imgViewer->setImage(*imgsLocalMap.find(index).value());
    }
    else {
        if(imgsOnlineMap.find(index) == imgsOnlineMap.end()) {
            return;
        }
        imgViewer->setImage(*imgsOnlineMap.find(index).value());
    }
    //设置默认缩放比例
    imgViewer->autoFitImage();
    imgViewer->update();
    //切换到大图界面
    mainStacked->setCurrentIndex(1);
    //显示返回按钮
    returnBtn->show();
}


//设置壁纸
bool MainWindow::setWallPaper(int index, imgType type)
{
    //保存参数
    QStringList options;
    //设置缓存路径
    QString filePath = QString(CACHEPATH) + "temp.jpg";
    QFile file(filePath);
    //如果图片缓存存在则先删除
    if(file.exists()) {
        file.remove();
    }
    //判定是本地壁纸还是在线壁纸
    if(type == imgType::LOCAL)  {
         //将img保存到本地
        imgsLocalMap.find(index).value()->save(filePath,"jpg",100);
    }
    else {
        imgsOnlineMap.find(index).value()->save(filePath,"jpg",100);
    }

    //dbus设置壁纸
    QString setWallpaperCmd = QString("dbus-send --dest=com.deepin.daemon.Appearance /com/deepin/daemon/Appearance --print-reply com.deepin.daemon.Appearance.SetMonitorBackground string:%1 string:'%2'")
            .arg(this->priScreenName)
            .arg(filePath);
    //设置参数
    options << "-c" << setWallpaperCmd;
    //调用外部程序，设置壁纸
    QProcess::execute("/bin/bash",options);

    //设置完成 删除缓存
    if(file.exists()) {
        file.remove();
    }
    file.close();
    //主界面发送设置成功的消息
    this->sendMessage(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), "壁纸设置成功");
    return true;
}


//释放所有的图片内存
void MainWindow::removeAllImgs(imgType type)
{
    //移除所有显示
    //释放空间
    //大小设置为0

    //本地
    if(type ==  imgType::LOCAL) {
        //析构显示图片的按钮
        for (int i = 0; i<imgsLocal.size(); i++) {
            //删除前端按钮
            this->imgFlowLocal->removeWidget(imgsLocal[i]);
            //释放空间
            if(nullptr != imgsLocal[i])
                delete imgsLocal[i];
        }
        //析构map存放的image
        for(auto it = imgsLocalMap.begin(); it != imgsLocalMap.end(); it++) {
            if(nullptr != it.value())
                delete it.value();
        }
        //重新设置大小
        imgsLocalMap.clear();
        this->imgsLocal.resize(0);
    }

    //在线，与本地同理
    else if(type == imgType::ONLINE) {
        for (int i = 0; i<imgsOnline.size(); i++) {
            this->imgFlowOnline->removeWidget(imgsOnline[i]);   //前端删除
            if(nullptr != imgsOnline[i])
                delete imgsOnline[i];  //释放空间
        }
        for(auto it = imgsOnlineMap.begin(); it != imgsOnlineMap.end(); it++) {
            if(nullptr != it.value())
                delete it.value();
        }
        imgsOnlineMap.clear();
        this->imgsOnline.resize(0);
    }
}

//前端设置DIconButton占位
void MainWindow::setCorrectBtnToUI(imgType type, int sum)
{

    //传递的数量有误
    if(sum <= 0) {
        qWarning() << "setCorrectBtnToUI: size not correct";
        exit(-1);
    }

    //本地
    if(type == imgType::LOCAL) {
        //设置容器大小
        imgsLocal.resize(sum);
        for(int i = 0; i < imgsLocal.size(); i++) {
            //开辟新空间
            imgsLocal[i]= new DIconButton();
            //设置按钮大小
            imgsLocal[i]->setFixedSize(ICONBTNSIZE);
            //设置按钮图标大小
            imgsLocal[i]->setIconSize(ICONBTNSIZE);
            //设置按钮样式
            imgsLocal[i]->setStyleSheet(BTNSTYLE);
            //添加到对应流式布局
            this->imgFlowLocal->addWidget(imgsLocal[i]);

            //建立图片被点击的连接
            connect(imgsLocal[i], &DIconButton::clicked, this, [this,i](){
                this->showDetailImg(i,imgType::LOCAL);
            });
        }
    }
    //在线，与本地同理
    else if(type == imgType::ONLINE) {
        this->imgsOnline.resize(sum);
        for(int i = 0; i < imgsOnline.size(); i++) {
            imgsOnline[i] = new DIconButton();
            //设置按钮样式
            imgsOnline[i]->setStyleSheet(BTNSTYLE);
            imgsOnline[i]->setFixedSize(ICONBTNSIZE);
            imgsOnline[i]->setIconSize(ICONBTNSIZE);
            this->imgFlowOnline->addWidget(imgsOnline[i]);

            connect(imgsOnline[i], &DIconButton::clicked, this, [this,i](){
                this->showDetailImg(i,imgType::ONLINE);
            });
        }
    }

    //设置一个局部的事件循环，等待1ms，让ui加载完成后进行更新flow大小的操作，这似乎是一个bug

    QEventLoop eventLoop;
    //设置计时器
    QTimer timer;
    //计时器只响应一次
    timer.setSingleShot(true);
    //计时器停止计时则事件停止
    connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    //开始计时
    timer.start(1);
    //开始事件循环
    eventLoop.exec();
    //更新窗口大小
    resizeImgStackedWidget();
}

void MainWindow::setScreenInfo(const QRect &rect, const QString &name)
{
    this->priScreenRect = rect;
    this->priScreenName = name;
}

//主窗口大小发生变化时，更新设置流式布局的窗口大小
void MainWindow::resizeEvent(QResizeEvent *event) {
    resizeImgStackedWidget();
}

//接收成功读取的信号，包括本地的（读取成功）和在线的（下载成功）
void MainWindow::acceptReadFinish(int index, imgType type, QImage* img)
{
    switch (type) {
        case imgType::LOCAL:
            //存入 map
            this->imgsLocalMap.insert(index, img);
            //显示到前端
            if(imgsLocal.size() > index) {
                this->imgsLocal[index]->setIcon(QPixmap::fromImage(*img));
            }
            break;
            //与上面同理
        case imgType::ONLINE:
            this->imgsOnlineMap.insert(index, img);
            if(imgsOnline.size() > index) {
                this->imgsOnline[index]->setIcon(QPixmap::fromImage(*img));  //显示到前端
            }
            break;
    }
}

//接收下载失败的信号
void MainWindow::acceptDLFailed(int index)
{
    DStyle *style = new DStyle;
    this->imgsOnline[index]->setIcon(style->standardIcon(DStyle::SP_CloseButton));
}


//更新设置流式布局的窗口大小
void MainWindow::resizeImgStackedWidget()
{
    //如果是本地壁纸界面
    if(imgStacked->currentIndex() == 0) {
        //根据外层（滚动界面）宽度获取界面的应有高度
        int flowHeight = imgFlowLocal->heightForWidth(scrollareaLocal->width());
        if(flowHeight < 0) {    //过滤没有内容的情况
            return;
        }
        //设置窗口固定大小
        imgWidgetLocal->setFixedWidth(scrollareaLocal->width());
        imgWidgetLocal->setFixedHeight(flowHeight);
    }
    //在线壁纸界面，与本地同理
    else {
        int flowHeight = imgFlowOnline->heightForWidth(scrollareaOnline->width());
        if(flowHeight < 0) {
            return;
        }
        imgWidgetOnline->setFixedWidth(scrollareaOnline->width());
        imgWidgetOnline->setFixedHeight(flowHeight);
    }
}
