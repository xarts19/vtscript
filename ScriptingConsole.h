#ifndef SCRIPTINGCONSOLE_H
#define SCRIPTINGCONSOLE_H

#include "Highlighter.h"
#include "ScriptEngine/Interpreter.h"

#include <QMainWindow>

namespace Ui {
    class ScriptingConsoleClass;
}

class ScriptingConsole : public QMainWindow
{
	Q_OBJECT

public:
    ScriptingConsole(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~ScriptingConsole();

private:
    Ui::ScriptingConsoleClass* ui;
    VTScript::Interpreter* running_script;

private slots:
    void execute_script();

};

#endif // SCRIPTINGCONSOLE_H
