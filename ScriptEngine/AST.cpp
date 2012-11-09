#include "AST.h"

#include <QStringList>

using namespace VTScript;
using namespace VTScript::AST;
using namespace VTScript::ASTTools;

void Noop                ::accept(ASTTools::NodeVisitor* visitor) { visitor->visit(this); }
void Leaf                ::accept(ASTTools::NodeVisitor* visitor) { visitor->visit(this); }
void FunctionCall        ::accept(ASTTools::NodeVisitor* visitor) { visitor->visit(this); }
void UnaryOperator       ::accept(ASTTools::NodeVisitor* visitor) { visitor->visit(this); }
void BinaryOperator      ::accept(ASTTools::NodeVisitor* visitor) { visitor->visit(this); }
void Return              ::accept(ASTTools::NodeVisitor* visitor) { visitor->visit(this); }
void Continue            ::accept(ASTTools::NodeVisitor* visitor) { visitor->visit(this); }
void Break               ::accept(ASTTools::NodeVisitor* visitor) { visitor->visit(this); }
void Block               ::accept(ASTTools::NodeVisitor* visitor) { visitor->visit(this); }
void FunctionDeclaration ::accept(ASTTools::NodeVisitor* visitor) { visitor->visit(this); }
void While               ::accept(ASTTools::NodeVisitor* visitor) { visitor->visit(this); }
void If                  ::accept(ASTTools::NodeVisitor* visitor) { visitor->visit(this); }


QString PrintNodeVisitor::print(Node* root)
{
    result_string_list.clear();
    indents.clear();
    indents.push(0);
    root->accept(this);
    return result_string_list.join("");
}

void PrintNodeVisitor::visit(AST::Noop* node)
{
    result_string_list << QString(indents.top() * TAB_SIZE, ' ') << "Noop";
}

void PrintNodeVisitor::visit(AST::Leaf* node)
{
    result_string_list << QString(indents.top() * TAB_SIZE, ' ') << node->name();
}

//  <indent>  <function_name>(<arg1>, <arg2>, ...)
void PrintNodeVisitor::visit(AST::FunctionCall* node)
{
    result_string_list << QString(indents.top() * TAB_SIZE, ' ');
    indents.push(0);
    node->function_object()->accept(this);
    result_string_list << "( ";

    foreach ( Expression* arg, node->arguments_expressions() )
    {
        arg->accept(this);
        result_string_list << ", ";
    }

    if (node->arguments_expressions().size() > 0) result_string_list.pop_back();
    result_string_list << " )";
    indents.pop();
}

// <indent>(<operator> <argument>)
void PrintNodeVisitor::visit(AST::UnaryOperator* node)
{
    result_string_list << QString(indents.top() * TAB_SIZE, ' ');
    indents.push(0);
    result_string_list << "(" << VTScript::OperatorTypes::to_string(node->type()) << " ";
    node->_argument->accept(this);
    result_string_list << ")";
    indents.pop();
}

// <indent>(<argument> <operator> <argument>)
void PrintNodeVisitor::visit(AST::BinaryOperator* node)
{
    result_string_list << QString(indents.top() * TAB_SIZE, ' ');
    indents.push(0);
    result_string_list << "(";
    node->left()->accept(this);
    result_string_list << " " << VTScript::OperatorTypes::to_string(node->type()) << " ";
    node->right()->accept(this);
    result_string_list << ")";
    indents.pop();
}

// <indent>return <expression>
void PrintNodeVisitor::visit(AST::Return* node)
{
    result_string_list << QString(indents.top() * TAB_SIZE, ' ');
    indents.push(0);
    result_string_list << "return ";
    node->expr()->accept(this);
    indents.pop();
}

// <indent> continue
void PrintNodeVisitor::visit(AST::Continue* node)
{
    result_string_list << QString(indents.top() * TAB_SIZE, ' ');
    result_string_list << "continue";
}

// <indent> break
void PrintNodeVisitor::visit(AST::Break* node)
{
    result_string_list << QString(indents.top() * TAB_SIZE, ' ');
    result_string_list << "break";
}

// <indent>{
//              statements;
// <indent>}
void PrintNodeVisitor::visit(AST::Block* node)
{
    result_string_list << QString(indents.top() * TAB_SIZE, ' ') << "{\n";
    indents.push(indents.top() + 1);

    foreach (Node* stmt, node->_statements)
    {
        stmt->accept(this);
        result_string_list << "\n";
    }

    indents.pop();
    result_string_list << QString(indents.top() * TAB_SIZE, ' ') << "}\n";
}


void PrintNodeVisitor::visit(AST::FunctionDeclaration* node)
{
    QStringList params;

    foreach ( QString arg, node->parameters() )
        params << arg;

    result_string_list << QString("%1%2( %3 )\n").arg( QString(indents.top() * TAB_SIZE, ' ') ).arg(node->name()).arg(params.join(", "));
    node->body()->accept(this);
}

void PrintNodeVisitor::visit(AST::While* node)
{
    result_string_list << QString(indents.top() * TAB_SIZE, ' ') << "while (";
    indents.push(0);
    node->_condition->accept(this);
    result_string_list << ")\n";
    indents.pop();
    if (dynamic_cast<AST::Block*>(node->_body) == 0)
        indents.push(indents.top() + 1);
    else
        indents.push(indents.top());
    node->_body->accept(this);
    indents.pop();
}

void PrintNodeVisitor::visit(AST::If* node)
{
    result_string_list << QString(indents.top() * TAB_SIZE, ' ') << "if (";
    indents.push(0);
    node->_condition->accept(this);
    result_string_list << ")\n";
    indents.pop();

    if (dynamic_cast<AST::Block*>(node->_then) == 0)
        indents.push(indents.top() + 1);
    else
        indents.push(indents.top());
    node->_then->accept(this);
    indents.pop();

    result_string_list << QString(indents.top() * TAB_SIZE, ' ') << "else\n";

    if (dynamic_cast<AST::Block*>(node->_else) == 0)
        indents.push(indents.top() + 1);
    else
        indents.push(indents.top());
    node->_else->accept(this);
    indents.pop();
}
