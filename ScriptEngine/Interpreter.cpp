#include "Interpreter.h"
#include "Errors.h"
#include "Builtin.h"

#include <QDebug>
#include <assert.h>

using namespace VTScript;

namespace
{
    SymbolTableManager::SymbolTable init_table_built_in()
    {
        SymbolTableManager::SymbolTable table;
        table["print"] = WS::SP_Object(new Builtin::print());
        table["help"] = WS::SP_Object(new Builtin::help());
        table["int"] = WS::SP_Object(new Builtin::_int());
        table["double"] = WS::SP_Object(new Builtin::_double());
        table["bool"] = WS::SP_Object(new Builtin::_bool());
        table["exec"] = WS::SP_Object(new Builtin::exec());
        return table;
    }
}

SymbolTableManager::SymbolTable SymbolTableManager::table_built_in = init_table_built_in();


WS::SP_Object SymbolTableManager::get_var(QString identifier) const
{
    for (int i = _tables.size() - 1; i >= 0; --i)
    {
        WS::SP_Object obj = _tables.at(i)[identifier];
        if (obj != NULL)
            return obj;
    }
    return WS::SP_Object();
}

void SymbolTableManager::modify_var(QString identifier, WS::SP_Object value)
{
    for (int i = _tables.size() - 1; i > read_only_threshold; --i)
    {
        WS::SP_Object& obj = _tables[i][identifier];
        if (obj != NULL)
        {
            obj = value;
            return;
        }
    }
    add_var(identifier, value);
}

void SymbolTableManager::add_var(QString identifier, WS::SP_Object value)
{
    _tables.top()[identifier] = value;
}


Interpreter::Interpreter(AST::Node* ast_root) :
        ast(ast_root),
        __return_value(),
        __is_set_break(false),
        __is_set_continue(false),
        __is_set_return(false),
        __is_terminated(false),
        __is_finished(false)
{
}

void Interpreter::run()
{
    qDebug() << "Interpreter:";
    try
    {
        stack.push("__main__");
        ast->accept(this);
        stack.pop();
        qDebug() << "Done";
    }
    catch (const InterpretError& e)
    {
        qDebug() << "Unhandled exception: " << e.what();
        while (!stack.isEmpty())
            qDebug() << "  In " << stack.pop();
    }
    catch (const InterruptError&)
    {
        qDebug() << "Script stopped";
    }

    __is_finished = true;
}

void Interpreter::visit(AST::Noop* node)
{
    if (__is_terminated) throw InterruptError();

    __return_value = WS::SP_Object(new WS::None());
}

void Interpreter::visit(AST::Leaf* node)
{
    if (__is_terminated) throw InterruptError();

    WS::SP_Object return_value;

    if (node->is_identifier())
    {
        WS::SP_Object obj = symbol_table_manager.get_var(node->name());
        if (obj != NULL)
            return_value = obj;
        else
            throw InterpretError(QString("Line %1: Identifier not found: '%2'").arg(node->line()).arg(node->name()));
    }
    else
    {
        return_value = node->object();
    }

    if (return_value == NULL)
        throw InterpretError(QString("Line %1: Unknown error").arg(node->line()));

    __return_value = return_value;
}

void Interpreter::visit(AST::FunctionCall* node)
{
    if (__is_terminated) throw InterruptError();

    node->function_object()->accept(this);
    WS::SP_Object obj = __return_value;

    // _func_object expression should yield WSFuncion descendant
    if (obj->__type__() != WSTypes::Function)
        throw InterpretError(QString("Line %1: Not a function: '%2'").arg(node->line()).arg(obj->__repr__()));

    QSharedPointer<WS::Function> fnc = obj.staticCast<WS::Function>();
    WS::ObjectList args;

    foreach(AST::Expression* expr, node->arguments_expressions())
    {
        expr->accept(this);
        args.append(__return_value);
    }

    if (!fnc->check_num_arguments( args.size() ))
        throw WrongNumberOfArgumentsError(args.size());

    stack.push( QString("Line %1: ").arg(node->line()) + fnc->__repr__() );
    WS::SP_Object res = (*fnc)(args);
    stack.pop();
    if (res == NULL)
        res = WS::SP_Object(new WS::None());
    __return_value = res;
}

void Interpreter::visit(AST::UnaryOperator* node)
{
    if (__is_terminated) throw InterruptError();

    node->argument()->accept(this);
    WS::SP_Object obj = __return_value;
    QString method_name = OperatorTypes::to_string(node->type());

    stack.push( QString("Line %1: ").arg(node->line()) + obj->__repr__() + " " + method_name );
    WS::SP_Object res = obj->invoke(method_name, WS::ObjectList()); 
    stack.pop();

    if (res == NULL)
        res = WS::SP_Object(new WS::None());

    __return_value = res;
}

void Interpreter::visit(AST::BinaryOperator* node)
{
    if (__is_terminated) throw InterruptError();

    if (node->type() == OperatorTypes::Assign)
    {
        AST::Leaf* name = static_cast<AST::Leaf*>(node->left());
        node->right()->accept(this);
        symbol_table_manager.modify_var(name->name(), __return_value);
        // __return_value is propagated further
    }
    else if (node->type() == OperatorTypes::Dot)
    {
        node->left()->accept(this);
        WS::SP_Object obj = __return_value;

        AST::FunctionCall* method = static_cast<AST::FunctionCall*>(node->right());
        AST::Leaf* method_name_node = static_cast<AST::Leaf*>(method->function_object());

        QString method_name = method_name_node->name();
        WS::ObjectList args;

        foreach(AST::Expression* expr, method->arguments_expressions())
        {
            expr->accept(this);
            args.append(__return_value);
        }

        stack.push( QString("Line %1: ").arg(node->line()) + obj->__repr__() + "." + method_name );
        WS::SP_Object res = obj->invoke(method_name, args); 
        stack.pop();

        if (res == NULL)
            res = WS::SP_Object(new WS::None());

        __return_value = res;
    }
    else
    {
        node->left()->accept(this);
        WS::SP_Object obj = __return_value;

        WS::ObjectList args;
        node->right()->accept(this);
        args.append(__return_value);

        stack.push( QString("Line %1: ").arg(node->line()) + obj->__repr__() + "." + OperatorTypes::to_string(node->type()) );
        WS::SP_Object res = obj->invoke(OperatorTypes::to_string(node->type()), args); 
        stack.pop();

        if (res == NULL)
            res = WS::SP_Object(new WS::None());

        __return_value = res;
    }
}

void Interpreter::visit(AST::Return* node)
{
    if (__is_terminated) throw InterruptError();

    node->expr()->accept(this);
    __is_set_return = true;
    //__return_value is propagated further from expression
}

void Interpreter::visit(AST::Continue* node)
{
    if (__is_terminated) throw InterruptError();

    __is_set_continue = true;
}

void Interpreter::visit(AST::Break* node)
{
    if (__is_terminated) throw InterruptError();

    __is_set_break = true;
}

void Interpreter::visit(AST::Block* node)
{
    if (__is_terminated) throw InterruptError();

    symbol_table_manager.push();

    foreach(AST::Node* stmt, node->values())
    {
        stmt->accept(this);
        if (__is_set_break || __is_set_continue || __is_set_return)
            break;
    }

    symbol_table_manager.pop();
}

void Interpreter::visit(AST::FunctionDeclaration* node)
{
    if (__is_terminated) throw InterruptError();

    node->fnc()->set_interpreter(this);
    symbol_table_manager.modify_var(node->name(), node->fnc());
}

void Interpreter::visit(AST::While* node)
{
    if (__is_terminated) throw InterruptError();

    while (true)
    {
        node->condition()->accept(this);
        WS::SP_Object cond_expr = __return_value;
        QSharedPointer<WS::Bool> condition = cond_expr->invoke("__bool__", WS::ObjectList()).staticCast<WS::Bool>();

        if (!condition->value())
            break;

        node->body()->accept(this);

        if (__is_set_continue)
            __is_set_continue = false;

        if (__is_set_break || __is_set_return)
        {
            __is_set_break = false;
            break;
        }
    }
}

void Interpreter::visit(AST::If* node)
{
    if (__is_terminated) throw InterruptError();

    node->condition()->accept(this);
    WS::SP_Object cond_expr = __return_value;
    QSharedPointer<WS::Bool> condition = cond_expr->invoke("__bool__", WS::ObjectList()).staticCast<WS::Bool>();

    if (condition->value())
        node->then_stmt()->accept(this);
    else
        node->else_stmt()->accept(this);
}

WS::SP_Object Interpreter::exec_user_fnc(WS::UserFunction* fnc, WS::ObjectList args)
{
    if (__is_terminated) throw InterruptError();

    WS::SP_Object ret;
    symbol_table_manager.push_fnc();

    const QStringList& params = fnc->parameters();
    
    for ( int i = 0; i < params.size(); ++i )
    {
        symbol_table_manager.modify_var(params.at(i), args.at(i));
    }

    foreach(AST::Node* stmt, fnc->body()->values())
    {
        stmt->accept(this);
        if (__is_set_return)
        {
            ret = __return_value;
            __is_set_return = false;
            break;
        }
    }

    symbol_table_manager.pop();

    return ret;
}