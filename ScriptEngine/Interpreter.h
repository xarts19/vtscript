#pragma once

#include "AST.h"
#include "Objects.h"

#include <QSharedPointer>
#include <QThread>
#include <QHash>
#include <QList>
#include <QVector>
#include <QString>

namespace VTScript
{
    class SymbolTableManager
    {
    public:
        typedef QHash< QString, WS::SP_Object > SymbolTable;

        SymbolTableManager() : read_only_threshold(0) { _tables.push(table_built_in); }

        WS::SP_Object get_var(QString identifier) const;

        // adds to local scope if not found
        void modify_var(QString identifier, WS::SP_Object value);
        inline void modify_var(QString identifier, WS::Object* value) { modify_var(identifier, WS::SP_Object(value)); }

        void push() { _tables.push(SymbolTable()); }
        void pop() { _tables.pop(); }

        void push_fnc() { read_only_threshold = _tables.size() - 1; _tables.push(SymbolTable()); }

    private:
        void add_var(QString identifier, WS::SP_Object value);

    private:
        QStack<SymbolTable> _tables;
        int read_only_threshold;

        static SymbolTable table_built_in;
    };


    class Interpreter : public ASTTools::NodeVisitor, public QThread
    {
    public:
        Interpreter(AST::Node* ast_root);
        ~Interpreter() { delete ast; }
        void run();

        bool is_finished() { return __is_finished; }
        void stop() { __is_terminated = true; }

        VISITOR_METHODS

        WS::SP_Object exec_user_fnc(WS::UserFunction* fnc, WS::ObjectList args);

    private:
        AST::Node* ast;
        SymbolTableManager symbol_table_manager;
        QStack<QString> stack;

        WS::SP_Object __return_value;
        bool __is_set_break;
        bool __is_set_continue;
        bool __is_set_return;
        bool __is_terminated;
        bool __is_finished;
    };

};