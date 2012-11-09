#ifndef DARKWOWGUI_H
#define DARKWOWGUI_H

#include "ui_Gui.hpp"

#include <QtGui/QMainWindow>
#include <QPlainTextEdit>
#include <QSortFilterProxyModel>
#include <QTime>
#include <QThread>
#include <QString>

class ServerGui : public QMainWindow
{
    Q_OBJECT

public:
    ServerGui( QWidget* parent = 0, Qt::WFlags flags = 0 );
    ~ServerGui();

private:
    Ui::GuiClass ui;

private:
    void set_up_debug_widget();

private slots:
    void closeEvent(QCloseEvent *event);

    void open_scripting_console();

    void menu_test1();
    void menu_test2();
    
    void update_debug_widget();
};

#endif // DARKWOWGUI_H
