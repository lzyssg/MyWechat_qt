#include "filesrvdlg.h"
#include "ui_filesrvdlg.h"

FileSrvDlg::FileSrvDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileSrvDlg)
{
    ui->setupUi(this);

    //服务器初始化
    myTcpSrv = new QTcpServer(this);
    mySrvPort=5555;
    connect(myTcpSrv,SIGNAL(newConnection()),this,SLOT(sndChatMsg()));//有新连接就发送信号
    myTcpSrv->close();
    myTotalBytes=0;//总字节
    mySendBytes=0;//发送字节
    myBytesTobeSend=0;//剩余字节
    myPayloadSize=64*1024;//有效载荷
    ui->sendProgressBar->reset();//进度条重置
    ui->openFilePushButton->setEnabled(true);
    ui->sendFilePushButton->setEnabled(false);
}

FileSrvDlg::~FileSrvDlg()
{
    delete ui;
}

//发送数据
void FileSrvDlg::sndChatMsg(){
    ui->sendFilePushButton->setEnabled(false);//设置不可用
    mySrvSocket=myTcpSrv->nextPendingConnection();
    connect(mySrvSocket,SIGNAL(bytesWritten(qint64)),this,SLOT(refreshProgress(qint64)));
    myLocPathFile=new QFile(myPathFile);
    myLocPathFile->open((QFile::ReadOnly));
    myTotalBytes=myLocPathFile->size();
    QDataStream sendOut(&myOutputBlock,QIODevice::WriteOnly);//把发送缓存myoutblock封装在sendout
    sendOut.setVersion(QDataStream::Qt_6_4);
    mytime=QTime::currentTime();
    QString curFile=myPathFile.right(myPathFile.size()-myPathFile.lastIndexOf('/')-1);//获取没路径的文件名
    sendOut<<qint64(0)<<qint64(0)<<curFile;//两个0作用是构造一个临时文件头
    myTotalBytes+=myOutputBlock.size();//总字节加=上一步写入的
    sendOut.device()->seek(0);//从队列头开始读，覆盖之前的两个0
    sendOut<<myTotalBytes<<qint64((myOutputBlock.size()-sizeof(qint64)*2));//写入实际总长度和文件长度(-上面写入的两个int)
    myBytesTobeSend=myTotalBytes-mySrvSocket->write(myOutputBlock);//余下字节数(待发送)
    myOutputBlock.resize(0);//清空缓存留着下次使用
}

//更新进度条
void FileSrvDlg::refreshProgress(qint64 bynum){
    qApp->processEvents();//传输大文件使页面不会冻结
    mySendBytes +=(int)bynum;
    if(myBytesTobeSend>0){
        myOutputBlock=myLocPathFile->read(qMin(myBytesTobeSend,myPayloadSize));//缓存发送的数据不能大于负载
        myBytesTobeSend-=(int)mySrvSocket->write(myOutputBlock);
        myOutputBlock.resize(0);
    }else{
        myLocPathFile->close();
    }

    ui->sendProgressBar->setMaximum(myTotalBytes);
    ui->sendProgressBar->setValue(mySendBytes);
    ui->sfileSizeLineEdit->setText(tr("%1").arg(myTotalBytes/(1024*1024))+"MB");//文件总进度
    ui->sendSizeLineEdit->setText(tr("%1").arg(mySendBytes/(1024*1024))+"MB");//已发送栏
    if(mySendBytes==myTotalBytes){//发送完毕关闭服务
        myLocPathFile->close();
        myTcpSrv->close();
        QMessageBox::information(0,QObject::tr("完毕"),"文件传输完成！");
    }

}

//...按钮
void FileSrvDlg::on_openFilePushButton_clicked()
{
    myPathFile=QFileDialog::getOpenFileName(this);
    if(!myPathFile.isEmpty()){
        myFileName=myPathFile.right(myPathFile.size()-myPathFile.lastIndexOf('/')-1);//获取文件名
        ui->sfileNameLineEdit->setText(tr("%1").arg(myFileName));
        ui->sendFilePushButton->setEnabled(true);
        ui->openFilePushButton->setEnabled(false);

    }
}

//发送按钮
void FileSrvDlg::on_sendFilePushButton_clicked()
{
    // 监听：QHostAddress::Any 表示当前网卡的所有 ip 地址
    if(!myTcpSrv->listen(QHostAddress::Any,mySrvPort)){//开始监听
        QMessageBox::warning(0,QObject::tr("异常"),"打开tcp端口出错，请重新连接");
        close();//关窗口
        return ;
    }
    emit sendFileName(myFileName);
}

//停止按钮
void FileSrvDlg::on_srvClosePushButton_clicked()
{
    if(myTcpSrv->isListening()){//先关闭服务器-打开的文件句柄-网络套接字
        myTcpSrv->close();
        myLocPathFile->close();
        mySrvSocket->abort();//取消侦听
    }
    close();
}

void FileSrvDlg::closeEvent(QCloseEvent *){
    on_srvClosePushButton_clicked();
}

void FileSrvDlg::cntRefused(){
    myTcpSrv->close();
    QMessageBox::warning(0,QObject::tr("提示"),"对方拒绝接受！");
}
