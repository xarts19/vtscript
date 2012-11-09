#ifndef SCRIPTINGCONSOLE_H
#define SCRIPTINGCONSOLE_H

#include "Highlighter.hxx"
#include "ScriptEngine/Interpreter.h"

#include "ui_ScriptingConsole.hpp"

#include <QtGui/QMainWindow>

class ScriptingConsole : public QMainWindow
{
	Q_OBJECT

public:
	ScriptingConsole(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ScriptingConsole();

private:
	Ui::ScriptingConsoleClass ui;
    VTScript::Interpreter* running_script;

private slots:
    void execute_script();

};

#endif // SCRIPTINGCONSOLE_H
