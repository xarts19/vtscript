#include "ScriptingConsole.hxx"

#include "ScriptEngine/ScriptParser.h"

#include <QDebug>

ScriptingConsole::ScriptingConsole(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags), running_script(NULL)
{
	ui.setupUi(this);

    connect(ui.btn_execute, SIGNAL(clicked()), this, SLOT(execute_script()));

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    ui.script_edit->setFont(font);
    ui.script_edit->setTabStopWidth( QFontMetrics(font).width( " " ) * TAB_SIZE );
    new Highlighter(ui.script_edit->document());
}

ScriptingConsole::~ScriptingConsole()
{
    if (running_script != NULL)
    {
        if (!running_script->is_finished())
        {
            qDebug() << "Waiting for scripts to finish";
            running_script->stop();
            running_script->wait();
        }
        delete running_script;
        running_script = NULL;
    }
}

void ScriptingConsole::execute_script()
{
    if (running_script == NULL || running_script->is_finished())
    {
        if (running_script && running_script->is_finished())
            delete running_script;

        VTScript::AST::Node* ast = VTScript::Parser::parse( ui.script_edit->toPlainText() );
        if (ast == NULL)
            return;
        running_script = new VTScript::Interpreter(ast);
        running_script->start();
    }
    else
        qDebug() << "Script already running";
}
