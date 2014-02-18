#pragma once

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QSortFilterProxyModel>
#include <QTime>
#include <QThread>
#include <QString>

namespace Ui {
    class GuiClass;
}

class Gui : public QMainWindow
{
    Q_OBJECT

public:
    Gui( QWidget* parent = 0, Qt::WindowFlags flags = 0 );
    ~Gui();

private:
    Ui::GuiClass* ui;

private:
    void set_up_debug_widget();

private slots:
    void closeEvent(QCloseEvent *event);

    void open_scripting_console();

    void menu_test1();
    void menu_test2();
    
    void update_debug_widget();
};
