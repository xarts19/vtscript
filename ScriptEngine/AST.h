#ifndef _AST_H_
#define _AST_H_

#include "Token.h"
#include "Objects.h"

#include <QSet>
#include <QList>
#include <QStringList>
#include <QStack>
#include <QSharedPointer>

#define TAB_SIZE 4

#define VISITOR_METHODS \
    void visit(AST::Noop* node);    \
    void visit(AST::Leaf* node);    \
    void visit(AST::FunctionCall* node);    \
    void visit(AST::UnaryOperator* node);   \
    void visit(AST::BinaryOperator* node);  \
    void visit(AST::Return* node);  \
    void visit(AST::Continue* node);    \
    void visit(AST::Break* node);   \
    void visit(AST::Block* node);   \
    void visit(AST::FunctionDeclaration* node); \
    void visit(AST::While* node);   \
    void visit(AST::If* node);

namespace VTScript
{

    // forward declaration
    namespace ASTTools
    {
        class NodeVisitor;
    };

    namespace AST
    {
        /*
            Abstract. Base for all AST nodes
        */
        class Node
        {
        public:
            Node(ulong line = 0) : _line(line) {}
            virtual ~Node() {}
            virtual void accept(ASTTools::NodeVisitor* visitor) = 0;

            virtual ulong line() const { return _line; }
            virtual void set_line(ulong line) { _line = line; }
            
        private:
            ulong _line;
        };


        /*
            Abstract. Base for all expressions
        */
        class Expression : public Node
        {
        public:
            Expression(ulong line) : Node(line) {}
            virtual ~Expression() {}
        };


        class Noop : public Expression
        {
        public:
            Noop(ulong line) : Expression(line) {}
            void accept(ASTTools::NodeVisitor* visitor);
        };


        /*
        Holds either identifier ( is_identifier() == true:  data = QString data() ) 
                  or object ( is_identifier() == false: obj = SP_WSObject object() )
        */
        class Leaf : public Expression
        {
        public:
            Leaf(ulong line, VTScript::Token token, VTScript::WS::SP_Object object) : 
                    Expression(line), _value(token), _obj(object) {}
            void accept(ASTTools::NodeVisitor* visitor);

            // for interpreter
            inline const bool is_identifier() const { return _value.type() == VTScript::Token::Identifier; }
            inline const QString& name() const { return _value.data(); }
            inline const VTScript::WS::SP_Object object() const { return _obj; }

        private:
            VTScript::Token _value;
            VTScript::WS::SP_Object _obj;
        };


        class FunctionCall : public Expression
        {
        public:
            FunctionCall(ulong line, Expression* fnc_obj, QList<Expression*> args) : 
                    Expression(line), _func_object(fnc_obj), _arguments(args) {}
            ~FunctionCall()
            {
                delete _func_object;
                foreach(Expression* expr, _arguments)
                    delete expr;
            }
            void accept(ASTTools::NodeVisitor* visitor);

            // for interpreter
            inline Expression* function_object() const { return _func_object; }
            inline const QList<Expression*>& arguments_expressions() const { return _arguments; }
            
        private:
            Expression* _func_object;
            QList<Expression*> _arguments;
        };


        class UnaryOperator : public Expression
        {
        public:
            UnaryOperator( ulong line, Expression* arg, VTScript::OperatorType t ) : 
                   Expression(line), _argument(arg), _type(t) {}
            ~UnaryOperator() { delete _argument; }
            void accept(ASTTools::NodeVisitor* visitor);

            // for interpreter
            inline Expression* argument() const { return _argument; }
            inline VTScript::OperatorType type() const { return _type; }
            
        public:
            Expression* _argument;
            VTScript::OperatorType _type;
        };


        class BinaryOperator : public Expression
        {
        public:
            BinaryOperator( ulong line, Expression* left, Expression* right, VTScript::OperatorType t ) : 
                    Expression(line), _left_branch(left), _right_branch(right), _type(t) {}
            ~BinaryOperator() { delete _left_branch; delete _right_branch; }
            void accept(ASTTools::NodeVisitor* visitor);

            // for interpreter
            inline Expression* right() const { return _right_branch; }
            inline Expression* left() const { return _left_branch; }
            inline VTScript::OperatorType type() const { return _type; }
            
        public:
            Expression* _left_branch;
            Expression* _right_branch;
            VTScript::OperatorType _type;
        };


        class Return : public Node
        {
        public:
            Return(ulong line, Expression* expr) : Node(line), _expr( expr ) {}
            ~Return() { delete _expr; }
            void accept(ASTTools::NodeVisitor* visitor);
        
            inline Expression* expr() const { return _expr; }

        public:
            Expression* _expr;
        };


        class Continue : public Node
        {
        public:
            Continue(ulong line) : Node(line) {}
            void accept(ASTTools::NodeVisitor* visitor);
        };


        class Break : public Node
        {
        public:
            Break(ulong line) : Node(line) {}
            void accept(ASTTools::NodeVisitor* visitor);
        };


        class Block : public Node
        {
        public:
            Block(ulong line, QList<Node*> stmts) : Node(line), _statements(stmts) {}
            ~Block() { foreach(Node* stmt, _statements) delete stmt; }
            void accept(ASTTools::NodeVisitor* visitor);

            inline const QList<Node*>& values() const { return _statements; }

        public:
            QList<Node*> _statements;
        };


        class FunctionDeclaration : public Node
        {
        public:
            FunctionDeclaration(ulong line, QString name, QStringList params, Block* body) :
                Node(line), _fnc( new VTScript::WS::UserFunction(name, params, body) ) {}
            ~FunctionDeclaration() { delete _fnc->body(); }
            void accept(ASTTools::NodeVisitor* visitor);

            inline QSharedPointer<VTScript::WS::UserFunction> fnc() const { return _fnc; }
            inline const QString& name() const { return fnc()->name(); }
            inline const QStringList& parameters() const { return fnc()->parameters(); }
            inline Block* body() const { return fnc()->body(); }
            
        public:
            QSharedPointer<VTScript::WS::UserFunction> _fnc;
        };


        class While : public Node
        {
        public:
            While(ulong line, Expression* cond, Node* body) : Node(line), _condition( cond ), _body( body ) {}
            ~While() { delete _condition; delete _body; }
            void accept(ASTTools::NodeVisitor* visitor);

            inline Expression* condition() const { return _condition; }
            inline Node* body() const { return _body; }

        public:
            Expression* _condition;
            Node* _body;
        };

        class If : public Node
        {
        public:
            If(ulong line, Expression* cond, Node* then_, Node* else_) : 
                    Node(line), _condition( cond ), _then( then_ ), _else( else_ ) {}
            ~If() { delete _condition; delete _then; delete _else; }
            void accept(ASTTools::NodeVisitor* visitor);

            inline Expression* condition() const { return _condition; }
            inline Node* then_stmt() const { return _then; }
            inline Node* else_stmt() const { return _else; }
            
        public:
            Expression* _condition;
            Node* _then;
            Node* _else;
        };
        
    }

    namespace ASTTools
    {
        class NodeVisitor
        {
        public:
            virtual ~NodeVisitor() {}

            virtual void visit(AST::Noop* node) = 0;
            virtual void visit(AST::Leaf* node) = 0;
            virtual void visit(AST::FunctionCall* node) = 0;
            virtual void visit(AST::UnaryOperator* node) = 0;
            virtual void visit(AST::BinaryOperator* node) = 0;
            virtual void visit(AST::Return* node) = 0;
            virtual void visit(AST::Continue* node) = 0;
            virtual void visit(AST::Break* node) = 0;
            virtual void visit(AST::Block* node) = 0;
            virtual void visit(AST::FunctionDeclaration* node) = 0;
            virtual void visit(AST::While* node) = 0;
            virtual void visit(AST::If* node) = 0;
        };

        class PrintNodeVisitor : public NodeVisitor
        {
        public:
            QString print(AST::Node* root);

            VISITOR_METHODS

        private:
            QStack<int> indents;
            QStringList result_string_list;
        };
    };

};

#endif//_AST_H_