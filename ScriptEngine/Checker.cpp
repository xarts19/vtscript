#include "Checker.h"
#include "Errors.h"

#include <QDebug>
#include <assert.h>

using namespace VTScript;


Checker::Checker(AST::Node* ast_root) :
        ast(ast_root)
{
}

void Checker::run()
{
    ast->accept(this);
}

bool Checker::is_inside_function()
{
    return (state_stack.indexOf(InFunction) != -1);
}

bool Checker::is_inside_loop()
{
    for (int i = state_stack.size() - 1; i >= 0; --i)
    {
        if (state_stack[i] == InWhile)
            return true;
        if (state_stack[i] == InFunction)   //left the function scope but didn't find enclosing loop
            return false;
    }

    return false;
}

void Checker::visit(AST::Noop* /*node*/)
{
}

void Checker::visit(AST::Leaf* /*node*/)
{
}

void Checker::visit(AST::FunctionCall* node)
{
    node->function_object()->accept(this);

    foreach(AST::Expression* expr, node->arguments_expressions())
        expr->accept(this);
}

void Checker::visit(AST::UnaryOperator* node)
{
    node->argument()->accept(this);
}

void Checker::visit(AST::BinaryOperator* node)
{
    if (node->type() == OperatorTypes::Assign)
    {
        AST::Leaf* name = dynamic_cast<AST::Leaf*>(node->left());
        if (name == NULL || !name->is_identifier())
            throw CheckerError(QString("Line %1: Left branch of assignment should be lvalue; checked in parser").arg(node->line()));

        node->right()->accept(this);
    }
    else if (node->type() == OperatorTypes::Dot)
    {
        node->left()->accept(this);

        // right branch should look exactly like function call
        AST::FunctionCall* method = dynamic_cast<AST::FunctionCall*>(node->right());
        if (method == NULL)
            throw CheckerError(QString("Line %1: Not a method to the right of the dot").arg(node->line()));

        AST::Leaf* method_name_node = dynamic_cast<AST::Leaf*>(method->function_object());
        if (method_name_node == NULL)
            throw CheckerError(QString("Line %1: Not a method name after the dot").arg(node->line()));

        foreach(AST::Expression* expr, method->arguments_expressions())
            expr->accept(this);
    }
    else
    {
        node->left()->accept(this);
        node->right()->accept(this);
    }
}

void Checker::visit(AST::Return* node)
{
    if (!is_inside_function())
        throw CheckerError(QString("Line %1: 'return' statement not allowed outside of function scope").arg(node->line()));

    node->expr()->accept(this);
}

void Checker::visit(AST::Continue* node)
{
    if (!is_inside_loop())
        throw CheckerError(QString("Line %1: 'continue' statement not allowed outside of loop scope").arg(node->line()));
}

void Checker::visit(AST::Break* node)
{
    if (!is_inside_loop())
        throw CheckerError(QString("Line %1: 'break' statement not allowed outside of loop scope").arg(node->line()));
}

void Checker::visit(AST::Block* node)
{
    foreach(AST::Node* stmt, node->values())
        stmt->accept(this);
}

void Checker::visit(AST::FunctionDeclaration* node)
{
    state_stack.push(InFunction);
    node->body()->accept(this);
}

void Checker::visit(AST::While* node)
{
    state_stack.push(InWhile);
    // condition is bool
    node->condition()->accept(this);
    node->body()->accept(this);
}

void Checker::visit(AST::If* node)
{
    state_stack.push(InIf);
    // condition is bool
    node->condition()->accept(this);
    node->then_stmt()->accept(this);
    node->else_stmt()->accept(this);
}

