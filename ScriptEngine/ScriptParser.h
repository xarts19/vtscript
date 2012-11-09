#ifndef _WOW_SCRIPT_PARSER_H_
#define _WOW_SCRIPT_PARSER_H_

#include "AST.h"
#include "Token.h"

#include <QString>
#include <QVector>
#include <QHash>
#include <QSet>

namespace VTScript
{
    /* Used by parser to iterate, query and check tokens */
    class TokenStream
    {
    public:
        TokenStream( QVector<Token> tokens, int position = 0 );

        void advance();
        void match( const Token& token, bool only_try_to = false );
        void match( const QString& value, bool only_try_to = false );
        void match( const Token::Type& type, bool only_try_to = false );

        inline bool is_at(const QString& value) const { return current().data() == value; }
        inline bool is_at(const Token::Type& type) const { return current().type() == type; }

        const Token& current() const { return tokens[position]; }
        const Token& previous() const { return tokens[position-1]; }

    private:
        QVector<Token> tokens;
        int position;
    };

    /* Constant data structures, used by parser and lexer; never change */
    struct Statics
    {
        Statics();
        ~Statics() {}

        QSet<QString> keywords;
        QHash<Token::Type, QRegExp> regexps;
        QHash<int, QSet<QString> > precedence_to_operators;
        QHash<QString, OperatorTypes::OperatorType> string_to_oper_type;
    };

    /* Options, used by parsing routines */
    class Flags
    {
    public:
        enum _Flags
        {
            NoSemicolon = 1
            /*
            2, 4, 8, 16, ...
            */
        };

        Flags() : parse_flags(0) {}
        Flags(int flags) : parse_flags(flags) {}

        bool is_set( int flag ) const { return ( ( parse_flags & flag ) == flag ); }
        void add_flags( int flags ) { parse_flags |= flags; }
        void remove_flags( int flags ) { parse_flags &= ~flags; }

    private:
        int parse_flags;
    };

#define PARSE_ARGUMENTS TokenStream& tstream, Flags flags

    class Parser
    {
    public:

        /*
            TODO: add parsing for lists
        */
        // returns NULL on failure
        static AST::Node* parse( QString script );

    private:
        static AST::Node* parse_statement( PARSE_ARGUMENTS );
        static AST::Block* parse_block( PARSE_ARGUMENTS );
        static AST::FunctionDeclaration* parse_function_declaration( PARSE_ARGUMENTS );
        static AST::While* parse_while( PARSE_ARGUMENTS );
        static AST::If* parse_if( PARSE_ARGUMENTS );
        static AST::Return* parse_return( PARSE_ARGUMENTS );
        static AST::Break* parse_break( PARSE_ARGUMENTS );
        static AST::Continue* parse_continue( PARSE_ARGUMENTS );

        static AST::Expression* parse_expression_root( PARSE_ARGUMENTS );
        static AST::Expression* parse_expression_leaf( PARSE_ARGUMENTS );

        template<int precedence>
        static AST::Expression* parse_expression( PARSE_ARGUMENTS );
        template<>
        static AST::Expression* parse_expression<1>( PARSE_ARGUMENTS );
        template<>
        static AST::Expression* parse_expression<2>( PARSE_ARGUMENTS );
        template<>
        static AST::Expression* parse_expression<9>( PARSE_ARGUMENTS );

        template<int precedence>
        static AST::Expression* parse_expression_subtree( AST::Expression* left_branch, PARSE_ARGUMENTS );
        template<>
        static AST::Expression* parse_expression_subtree<1>( AST::Expression* left_branch, PARSE_ARGUMENTS );

    public:
        static const Statics statics;

    private:
        // throws LexError
        static QVector<Token> _lex( QString source );

        // throws ParseError
        static AST::Node* _parse( QVector<Token> tokens );
    };

}
#endif//_WOW_SCRIPT_PARSER_H_