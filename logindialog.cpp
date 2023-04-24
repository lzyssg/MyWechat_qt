#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
//    ui->pwdLineEdit->setFocus();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_loginPushButton_clicked()
{
    showWeiChatWindow();
}

void LoginDialog::showWeiChatWindow()
{
    QFile file(":/userlog.xml");
    file.open(QIODevice::ReadOnly);
    mydoc.setContent(&file);
    file.close();
    QDomElement root = mydoc.documentElement();//获取xml文件的根元素
    if(root.hasChildNodes()){
        QDomNodeList userlist = root.childNodes();//获取根元素所有用户节点
        bool exist=false;//用户是否存在
        for(int i=0;i<userlist.count();i++){//遍历所有用户节点
            QDomNode user=userlist.at(i);//根据索引获取用户节点
            QDomNodeList record=user.childNodes();//用户的账户和密码
            QString uname=record.at(0).toElement().text();
            QString pword=record.at(1).toElement().text();

            if(uname==ui->usrLineEdit->text()){
                exist=true;//用户名正确说明用户存在
                if(!(pword==ui->pwdLineEdit->text())){
                    QMessageBox::warning(0,QObject::tr("提示"),"密码错误！");
                    ui->pwdLineEdit->clear();
                    ui->pwdLineEdit->setFocus();//聚焦
                    return ;
                }
            }

        }
        if(!exist){//遍历完所有用户都没找到相同用户名
            QMessageBox::warning(0,QObject::tr("提示"),"用户名不存在！");
            ui->usrLineEdit->clear();
            ui->pwdLineEdit->clear();
            ui->usrLineEdit->setFocus();
            return;
        }

        //密码正确
        weiChatWindow = new MainWindow();
        weiChatWindow->setWindowTitle(ui->usrLineEdit->text());
        weiChatWindow->show();
    }

}
