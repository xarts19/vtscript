#pragma once

#include "AST.h"

namespace VTScript
{
    
    class Checker : public ASTTools::NodeVisitor
    {
    public:
        enum State 
        {
            InWhile,
            InIf,
            InFunction,
            ContinueIsSet,
            BreakIsSet,
            ReturnIsSet
        };
        
        Checker(AST::Node* ast_root);
        ~Checker() {}
        void run();

        VISITOR_METHODS

    private:
        bool is_inside_function();
        bool is_inside_loop();

    private:
        AST::Node* ast;
        QStack<State> state_stack;

    };

};