#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "mainwindow.h"
#include <QFile>
#include "qdom.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT //宏的展开将为我们的类提供信号槽机制、国际化机制以及 Qt 提供的不基于 C++ RTTI 的反射能力

public:
    explicit LoginDialog(QWidget *parent = nullptr);//explicit只能显示修饰构造函数
    ~LoginDialog();

private slots:
    void on_loginPushButton_clicked();//登录按钮单击事件
    void showWeiChatWindow();//登录成功与否再决定是否显示聊天窗口

private:
    Ui::LoginDialog *ui;
    MainWindow *weiChatWindow;//指向聊天窗口的指针
    QDomDocument mydoc;//QDomDocument 类表示整个 XML 文档。从概念上讲，它是文档树的根，并提供对文档数据的主要访问。
};

#endif // LOGINDIALOG_H
