#include "Builtin.h"

#include "Interpreter.h"
#include "ScriptParser.h"

#include <QDebug>
#include <QIODevice>
#include <QFile>

using namespace VTScript;

WS::SP_Object Builtin::print::operator()(WS::ObjectList& args)
{
    QStringList l;
    foreach(const WS::SP_Object& obj, args.values())
        l << obj->__str__();
    qDebug() << qPrintable(l.join(" "));

    return WS::SP_Object();
}

WS::SP_Object Builtin::help::operator()(WS::ObjectList& args)
{
    check_num(args, 1);
    qDebug() << qPrintable(args.at(0)->__repr__());

    return WS::SP_Object();
}

WS::SP_Object Builtin::_int::operator()(WS::ObjectList& args)
{
    WS::SP_Object obj = args.takeFirst();
    return obj->invoke("__int__", args);
}

WS::SP_Object Builtin::_double::operator()(WS::ObjectList& args)
{
    WS::SP_Object obj = args.takeFirst();
    return obj->invoke("__double__", args);
}

WS::SP_Object Builtin::_bool::operator()(WS::ObjectList& args)
{
    WS::SP_Object obj = args.takeFirst();
    return obj->invoke("__bool__", args);
}

WS::SP_Object Builtin::exec::operator()(WS::ObjectList& args)
{
    WS::check<WS::String>(args);

    QString filename = args.at<WS::String>(0)->value();
    QFile file( filename );
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
        throw InterpretError("Couldn't open file: " + filename);

    QString script = QTextStream(&file).readAll();
    
    VTScript::AST::Node* ast = VTScript::Parser::parse( script );
    if (ast == NULL)
        throw InterpretError("Couldn't parse script: " + filename);

    QThread* running_script = new VTScript::Interpreter(ast);
    running_script->start();
    running_script->wait();

    return WS::SP_Object( new WS::None() );
}