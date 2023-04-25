#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filesrvdlg.h"
#include "filecntdlg.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initMainWindow()
{
    myUdpSocket = new QUdpSocket(this);
    myUdpPort = 23232;
    myUdpSocket->bind(myUdpPort, QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint);
    connect(myUdpSocket, SIGNAL(readyRead()), this, SLOT(recvAndProcessChatMsg()));
    myfsrv = new FileSrvDlg(this);
    connect(myfsrv, SIGNAL(sendFileName(QString)), this, SLOT(getSfileName(QString)));
}

void MainWindow::on_sendPushButton_clicked()
{
    sendChatMsg(ChatMsg);
}

void MainWindow::sendChatMsg(ChatMsgType msgType, QString rmtName)
{
    QByteArray qba;
    QDataStream write(&qba, QIODevice::WriteOnly);
    QString locHostIp = getLocHostIp();
    QString locChatMsg = getLocChatMsg();
    write << msgType << myname;
    switch (msgType)
    {
    case ChatMsg:
        write << locHostIp << locChatMsg;
        break;
    case OnLine:
        write << locHostIp;
        break;
    case OffLine:
        break;
    case SfileName:
        write << locHostIp << rmtName << myFileName;
        break;
    case RefFile:
        write << locHostIp << rmtName;
        break;
    }
    myUdpSocket->writeDatagram(qba, qba.length(), QHostAddress::Broadcast, myUdpPort);
}

void MainWindow::recvAndProcessChatMsg()
{
    while (myUdpSocket->hasPendingDatagrams())
    {
        QByteArray qba;
        qba.resize(myUdpSocket->pendingDatagramSize());
        myUdpSocket->readDatagram(qba.data(), qba.size());
        QDataStream read(&qba, QIODevice::ReadOnly);
        int msgType;
        read >> msgType;
        QString name, hostip, chatmsg, rname, fname;
        QString curtime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        switch (msgType)
        {
        case ChatMsg: {
            read >> name >> hostip >> chatmsg;
            ui->chatTextBrowser->setTextColor(Qt::darkGreen);
            ui->chatTextBrowser->setCurrentFont(QFont("Times New Roman", 14));
            ui->chatTextBrowser->append("【" + name + "】" + curtime);
            ui->chatTextBrowser->append(chatmsg);
            break;
        }
        case OnLine:
            read >> name >> hostip;
            onLine(name, curtime);
            break;
        case OffLine:
            read >> name;
            offLine(name, curtime);
            break;
        case SfileName:
            read >> name >> hostip >> rname >> fname;
            recvFileName(name, hostip, rname, fname);
            break;
        case RefFile:
            read >> name >> hostip >> rname;
            if (myname == rname) myfsrv->cntRefused();
            break;
        }
    }
}

void MainWindow::onLine(QString name, QString time)
{
    bool notExist = ui->userListTableWidget->findItems(name, Qt::MatchExactly).isEmpty();
    if (notExist)
    {
        QTableWidgetItem *newuser = new QTableWidgetItem(name);
        ui->userListTableWidget->insertRow(0);
        ui->userListTableWidget->setItem(0, 0, newuser);
        ui->chatTextBrowser->setTextColor(Qt::gray);
        ui->chatTextBrowser->setCurrentFont(QFont("Times New Roman", 12));
        ui->chatTextBrowser->append(tr("%1 %2 上线！").arg(time).arg(name));
        sendChatMsg(OnLine);
    }
}

void MainWindow::offLine(QString name, QString time)
{
    int row = ui->userListTableWidget->findItems(name, Qt::MatchExactly).first()->row();
    ui->userListTableWidget->removeRow(row);
    ui->chatTextBrowser->setTextColor(Qt::gray);
    ui->chatTextBrowser->setCurrentFont(QFont("Times New Roman", 12));
    ui->chatTextBrowser->append(tr("%1 %2 离线！").arg(time).arg(name));
}

QString MainWindow::getLocHostIp()
{
    QList<QHostAddress> addrlist = QNetworkInterface::allAddresses();
    foreach (QHostAddress addr, addrlist)
    {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol) return addr.toString();
    }
    return 0;
}

QString MainWindow::getLocChatMsg()
{
    QString chatmsg = ui->chatTextEdit->toHtml();
    ui->chatTextEdit->clear();
    ui->chatTextEdit->setFocus();
    return chatmsg;
}

void MainWindow::on_searchPushButton_clicked()
{

    sendChatMsg(ChatMsg);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    sendChatMsg(OffLine);
}
