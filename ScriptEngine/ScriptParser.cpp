#include "ScriptParser.h"
#include "Errors.h"
#include "Checker.h"

#include <QRegExp>
#include <QDebug>

#include <exception>

using namespace VTScript;
using namespace AST;

const Statics Parser::statics;


Statics::Statics()
{
    keywords << "while" << "if" << "else" << "def" << "true" << "false"
        << "return" << "break" << "continue" << "or" << "and" << "not" << "None";

    regexps[Token::Identifier] = QRegExp( "^(" "[_A-Za-z][_A-Za-z0-9]*"
                                           "|" "operator.+(?=\\()"
                                           ")" );
    regexps[Token::Rational] = QRegExp( "^\\d*\\.\\d+" );
    regexps[Token::Integral] = QRegExp( "^\\d+" );
    regexps[Token::Literal] = QRegExp( "^("    "\"[^\"\\r\\n]*\""
                                           "|" "\'[^\'\\r\\n]*\'"
                                       ")" );

    regexps[Token::Operator] = QRegExp( "^("    "="
                                            "|" "<"
                                            "|" ">"
                                            "|" "<="
                                            "|" ">="
                                            "|" "=="
                                            "|" "!="
                                            "|" "\\+"
                                            "|" "-"
                                            "|" "\\*"
                                            "|" "/"
                                            "|" "%"
                                            "|" "\\("
                                            "|" "\\)"
                                            "|" "\\{"
                                            "|" "\\}"
                                            "|" "\\["
                                            "|" "\\]"
                                            "|" ";"
                                            "|" "\\."
                                            "|" ","
                                        ")" );

    regexps[Token::Whitespace] = QRegExp( "^[ \t]*" );
    regexps[Token::Newline] = QRegExp( "^(\\n|\\r\\n)" );
    regexps[Token::Comment] = QRegExp( "^//[^\\n\\r]*" );

    precedence_to_operators[3] = QSet<QString>() << "*" << "/" << "%";
    precedence_to_operators[4] = QSet<QString>() << "+" << "-";
    precedence_to_operators[5] = QSet<QString>() << ">" << "<" << "<=" << ">=";
    precedence_to_operators[6] = QSet<QString>() << "==" << "!=";
    precedence_to_operators[7] = QSet<QString>() << "and";
    precedence_to_operators[8] = QSet<QString>() << "or";

    string_to_oper_type["="] = OperatorTypes::Assign;
    string_to_oper_type["[]"] = OperatorTypes::Subscript;
    string_to_oper_type["."] = OperatorTypes::Dot;
    string_to_oper_type["unary_not"] = OperatorTypes::Not;
    string_to_oper_type["+"] = OperatorTypes::Plus;
    string_to_oper_type["unary_+"] = OperatorTypes::UnaryPlus;
    string_to_oper_type["-"] = OperatorTypes::Minus;
    string_to_oper_type["unary_-"] = OperatorTypes::UnaryMinus;
    string_to_oper_type["*"] = OperatorTypes::Mult;
    string_to_oper_type["/"] = OperatorTypes::Div;
    string_to_oper_type["%"] = OperatorTypes::Mod;
    string_to_oper_type["<"] = OperatorTypes::Less;
    string_to_oper_type[">"] = OperatorTypes::Greater;
    string_to_oper_type["<="] = OperatorTypes::LessEq;
    string_to_oper_type[">="] = OperatorTypes::GreaterEq;
    string_to_oper_type["=="] = OperatorTypes::Equal;
    string_to_oper_type["!="] = OperatorTypes::NotEqual;
    string_to_oper_type["and"] = OperatorTypes::And;
    string_to_oper_type["or"] = OperatorTypes::Or;
}


TokenStream::TokenStream( QVector<Token> tokens, int position ) : tokens( tokens ), position ( position )
{
    if ( 0 > position || position >= tokens.size() )
        throw std::logic_error( "Starting position of a TokenStream is out of range" );
}

void TokenStream::advance()
{
    if ( ++position > tokens.size() )
        throw std::logic_error( "Advanced past the end of the stream" );
}

void TokenStream::match( const Token& token, bool only_try_to )
{
    if ( current() == token )
    {
        advance();
        return;
    }

    if ( !only_try_to )
        throw ParseError( QString("Expected \"%1\", but got \"%2\" instead")
                              .arg(token.to_string(), current().to_string()) );
}

void TokenStream::match( const QString& value, bool only_try_to )
{
    if ( current().data() == value )
    {
        advance();
        return;
    }

    if ( !only_try_to )
        throw ParseError( QString("Expected \"%1\", but got \"%2\" instead")
                              .arg( value, current().data() ) );
}

void TokenStream::match( const Token::Type& type, bool only_try_to )
{
    if ( current().type() == type )
    {
        advance();
        return;
    }

    if ( !only_try_to )
        throw ParseError( QString("Expected \"%1\", but got \"%2\" instead")
                              .arg( Token::type_toString(type), Token::type_toString(current().type()) ) );
}

Node* Parser::parse( QString script )
{
    try
    {
        QVector<Token> tokens = _lex( script );

        /*
        qDebug() << "Lexer:";
        for ( int i = 0; i < tokens.size(); ++i )
            qDebug() << qPrintable(tokens[i].to_string());
        */

        Node* root = _parse( tokens );
        if (root == NULL)
            return NULL;

        /*
        qDebug() << "Parser:";
        ASTTools::PrintNodeVisitor print_visitor;
        qDebug() << qPrintable(print_visitor.print(root));
        */

        //qDebug() << "Checker:";
        Checker checker(root);
        checker.run();

        return root;
    }
    catch (const LexError& e)
    {
        qDebug() << "Lexer error:" << e.what();
    }
    catch (const ParseError& e)
    {
        qDebug() << "Parser error:" << e.what();
    }
    catch (const CheckerError& e)
    {
        qDebug() << "Checker error:" << e.what();
    }

    return NULL;
}

QVector<Token> VTScript::Parser::_lex( QString source )
{
    QVector<Token> tokens;
    ulong line = 1;
    int position = 0;

    tokens << Token(Token::Operator, "{", line);

    while ( position < source.size() )
    {
        int max_len = 0;
        Token::Type type;

        for ( QHash<Token::Type, QRegExp>::const_iterator it = statics.regexps.begin(); it != statics.regexps.end(); ++it )
        {
            if ( -1 != it.value().indexIn( source, position, QRegExp::CaretAtOffset ) )
            {
                int len = it.value().matchedLength();
                QString data = it.value().cap( 0 );

                if ( len > max_len )
                {
                    max_len = len;
                    type = it.key();
                }
            }
        }

        if ( max_len == 0 )
        {
            throw LexError( QString("Couldn't match any token in line %1").arg(line) );
        }

        if ( type == Token::Comment || type == Token::Whitespace )
        {
            //skip
        }
        else if ( type == Token::Newline )
        {
            line++;
        }
        else
        {
            QString value = statics.regexps[type].cap( 0 );

            if ( type == Token::Literal )
                value = value.mid(1, value.size()-2);

            if ( type == Token::Identifier )
            {
                if ( statics.keywords.contains( value ) )
                    type = Token::Keyword;
            }

            tokens << Token( type, value, line );
        }

        position += max_len;
    }

    tokens << Token(Token::Operator, "}", line);
    tokens << Token(Token::Eof, "", line);
    return tokens;
}

Node* Parser::_parse( QVector<Token> tokens )
{
    TokenStream tstream( tokens );

    Flags flags(Flags::NoSemicolon);

    try
    {
        //main block is just a list of statements (block without curly braces)
        return parse_block( tstream, flags );
    }
    catch ( const ParseError& e )
    {
        throw ParseError( QString("Parser: Error in token: %1 ;  %2").arg(tstream.current().to_string()).arg(e.what()) );
    }
}

/*
    STATEMENT ->  FUNCTION
               |  WHILE
               |  IF
               |  BLOCK
               |  RETURN
               |  BREAK
               |  CONTINUE
               |  EXPRESSION [;]
*/
Node* Parser::parse_statement( TokenStream& tstream, Flags flags )
{
    if ( tstream.is_at("def") )
        return parse_function_declaration(tstream,flags);

    else if ( tstream.is_at("while") )
        return parse_while(tstream, flags);

    else if ( tstream.is_at("if") )
        return parse_if(tstream, flags);

    else if ( tstream.is_at("{") )
        return parse_block(tstream, flags);

    else if ( tstream.is_at("return") )
        return parse_return(tstream, flags);

    else if ( tstream.is_at("break") )
        return parse_break(tstream, flags);

    else if ( tstream.is_at("continue") )
        return parse_continue(tstream, flags);

    else
    {
        Expression* expr = parse_expression_root(tstream, flags);
        tstream.match( ";", flags.is_set(Flags::NoSemicolon) );
        return expr;
    }
}

/*
    BLOCK ->  "{" STMT STMT ... "}"
*/
Block* Parser::parse_block( TokenStream& tstream, Flags flags)
{
    ulong line = tstream.current().line();
    QList<Node*> statements;

    tstream.match( "{" );

    while ( true )
    {
        if (tstream.is_at("}"))
            break;

        statements << parse_statement( tstream, flags );
    }

    tstream.match( "}" );

    return new Block(line, statements);
}

/*
    FUNCTION ->  "def" <Identifier> "(" <Identifier> "," <Identifier> "," ... ")" BLOCK
*/
FunctionDeclaration* Parser::parse_function_declaration( TokenStream& tstream, Flags flags )
{
    ulong line = tstream.current().line();

    QString name;
    QStringList params;
    Block* body;

    tstream.match("def");
    tstream.match(Token::Identifier);
    name = tstream.previous().data();
    tstream.match("(");

    while ( true )
    {
        if (tstream.is_at(")"))
            break;

        tstream.match(Token::Identifier);
        params << tstream.previous().data();

        if (tstream.is_at(")"))
            break;

        tstream.match(",");
    }

    tstream.match( ")" );
    body = parse_block(tstream, flags);

    return new FunctionDeclaration(line, name, params, body);
}

/*
    WHILE ->  "while" "(" EXPRESSION ")" STATEMENT
     * here EXPRESSION has to actually be convertible to bool
*/
While* Parser::parse_while( TokenStream& tstream, Flags flags )
{
    ulong line = tstream.current().line();

    tstream.match("while");
    tstream.match("(");
    Expression* cond = parse_expression_root(tstream, flags);
    tstream.match(")");
    Node* body = parse_statement(tstream, flags);
    
    return new While(line, cond, body);
}

/*
    IF ->  "if" "(" EXPRESSION ")" STATEMENT [ "else" STATEMENT ]
     * here EXPRESSION has to actually be convertible to bool
     * "else STATEMENT" is filled with Noop if not present
*/
If* Parser::parse_if( TokenStream& tstream, Flags flags )
{
    ulong line = tstream.current().line();

    tstream.match("if");
    tstream.match("(");
    Expression* cond = parse_expression_root(tstream, flags);
    tstream.match(")");
    Node* then_ = parse_statement(tstream, flags);
    Node* else_ = NULL;

    if ( tstream.is_at("else") )
    {
        tstream.advance();
        else_ = parse_statement(tstream, flags);
    }
    else
        else_ = new Noop(line);

    return new If(line, cond, then_, else_);
}

/*
    RETURN ->  "return" EXPRESSION [;]
     * here EXPRESSION has to evaluate to some object (as ALL expressions do, I guess...)
     * EXPRESSION should always be present
*/
Return* Parser::parse_return( TokenStream& tstream, Flags flags )
{
    ulong line = tstream.current().line();

    tstream.match("return");
    Expression* expr = parse_expression_root(tstream, flags);
    tstream.match( ";", flags.is_set(Flags::NoSemicolon) );

    return new Return(line, expr);
}

/*
    BREAK ->  "break" [;]
*/
Break* Parser::parse_break( TokenStream& tstream, Flags flags )
{
    ulong line = tstream.current().line();

    tstream.match("break");
    tstream.match( ";", flags.is_set(Flags::NoSemicolon) );

    return new Break(line);
}

/*
    CONTINUE ->  "continue" [;]
*/
Continue* Parser::parse_continue( TokenStream& tstream, Flags flags )
{
    ulong line = tstream.current().line();

    tstream.match("continue");
    tstream.match( ";", flags.is_set(Flags::NoSemicolon) );

    return new Continue(line);
}

/*
    Precedence levels and association for operators (listed top to bottom, in descending precedence):
                                    
                                        left-to-right
  I     Fucntion call           ()
        Array subscripting      []
        Element selection       .
                                        right-to-left
  II    Logical NOT             not
        Unary plus and minus    + -
                                        left-to-right
  III   Multiplication          *
        Division                /
        Remainder               %

  IV    Addition                +
        Subtraction             -

  V     Relational operators    < > <= >=

  VI    Relational operators    == !=

  VII   Logical AND             and

  VIII  Logical OR              or
                                        right-to-left
  IX    Assignment              =
*/

Expression* Parser::parse_expression_root( TokenStream& tstream, Flags flags )
{
    return parse_expression<9>( tstream, flags );
}


/*
    LVL_IX -> LVL_VIII "=" LVL_IX
        
        Luckily, we can leave this as it is.

*/
template<>
Expression* Parser::parse_expression<9>( TokenStream& tstream, Flags flags )
{
    Expression* left_branch = parse_expression<8>( tstream, flags );
    if ( tstream.is_at("=") )
    {
        OperatorType type = statics.string_to_oper_type["="];
        ulong line = tstream.current().line();

        tstream.advance();

        Leaf* name = dynamic_cast<Leaf*>(left_branch);

        if (name == NULL || !name->is_identifier())
            throw ParseError("Not a valid lvalue for assignment");

        Expression* right = parse_expression<9>(tstream, flags);
        return new BinaryOperator( line, left_branch, right, type );
    }
    else
    {
        return left_branch;
    }
}


/*
Left-recursive version:
    LVL_III -> LVL_III "*" LVL_II
             | LVL_III "/" LVL_II
             | LVL_III "%" LVL_II
             | LVL_II

    LVL_IV -> LVL_IV "+" LVL_III
            | LVL_IV "-" LVL_III
            | LVL_III

    LVL_V -> LVL_V ">" LVL_IV
           | LVL_V "<" LVL_IV
           | LVL_V ">=" LVL_IV
           | LVL_V "<=" LVL_IV
           | LVL_IV

    LVL_VI -> LVL_VI "==" LVL_V
            | LVL_VI "!=" LVL_V
            | LVL_V

    LVL_VII -> LVL_VII "and" LVL_VI
             | LVL_VI

    LVL_VIII -> LVL_VIII "or" LVL_VII
              | LVL_VII

   Sadly, we must convert this version to right-recursive to be able to implement it in top-down parser

Right-recursive version:
    LVL_III -> LVL_II SUB_LVL_III

    SUB_LVL_III -> "*" LVL_II SUB_LVL_III
                 | "/" LVL_II SUB_LVL_III
                 | "%" LVL_II SUB_LVL_III
                 | e

     and so on ...

        * Left branch becomes a child of right branch (left-to-right association)
*/
template<int precedence>
Expression* Parser::parse_expression( TokenStream& tstream, Flags flags )
{
    Expression* left_branch = parse_expression<precedence-1>( tstream, flags );
    Expression* root = parse_expression_subtree<precedence>( left_branch, tstream, flags );

    if ( root == NULL )
        root = left_branch;

    return root;
}

// SUB_LVL_PRECEDENCE
template<int precedence>
Expression* Parser::parse_expression_subtree( AST::Expression* left_branch, TokenStream& tstream, Flags flags )
{
    Expression* result;
    if ( ( tstream.current().type() == Token::Operator || tstream.current().type() == Token::Keyword )
        && statics.precedence_to_operators[precedence].contains(tstream.current().data()) )
    {
        OperatorType type = statics.string_to_oper_type[tstream.current().data()];
        ulong line = tstream.current().line();

        tstream.advance();
        Expression* right = parse_expression<precedence-1>(tstream, flags);
        result = new BinaryOperator( line, left_branch, right, type );
    }
    else
        return NULL;

    // Parse next "same-level" expression in chain
    Expression* root = parse_expression_subtree<precedence>( result, tstream, flags );

    if ( root == NULL )
        root = result;

    return root;
}


/*
    LVL_II -> not LVL_II
            | + LVL_II
            | - LVL_II
            | LVL_I

    Luckily, we can leave this as it is.

     * (right-to-left association)
*/
template<>
Expression* Parser::parse_expression<2>( TokenStream& tstream, Flags flags )
{
    if ( tstream.is_at("not") || tstream.is_at("+") || tstream.is_at("-") )
    {
        OperatorType type = statics.string_to_oper_type["unary_" + tstream.current().data()];
        ulong line = tstream.current().line();

        tstream.advance();
        Expression* arg = parse_expression<2>(tstream, flags);
        return new UnaryOperator(line, arg, type);
    }

    return parse_expression<1>(tstream, flags);   //fall-through
}


/*
Left-recursive version:
    LVL_I -> LVL_I "(" EXPRESSION "," EXPRESSION "," ... ")"
           | LVL_I "[" EXPRESSION "]"
           | LVL_I "." LVL_0
           | LVL_0

   Sadly, we must convert this version to right-recursive to be able to implement it in top-down parser

Right-recursive version:
    LVL_I -> LVL_0 SUB_LVL_I

    SUB_LVL_I -> "(" EXPRESSION "," EXPRESSION "," ... ")" SUB_LVL_I    // unary operator
               | "[" EXPRESSION "]" SUB_LVL_I                           // unary operator
               | "." LVL_0 SUB_LVL_I                                    // binary operator
               | e                                                      // empty token

        * Left branch becomes a child of right branch (left-to-right association)
*/
template<>
Expression* Parser::parse_expression<1>( TokenStream& tstream, Flags flags )
{
    Expression* left_branch = parse_expression_leaf( tstream, flags );
    Expression* root = parse_expression_subtree<1>( left_branch, tstream, flags );

    if ( root == NULL )
        root = left_branch;

    return root;
}

// SUB_LVL_I
template<>
Expression* Parser::parse_expression_subtree<1>( AST::Expression* left_branch, TokenStream& tstream, Flags flags )
{
    Expression* result;
    ulong line = tstream.current().line();

    if (tstream.is_at("("))             // function call
    {
        tstream.match("(");
        QList<Expression*> args;

        while ( true )
        {
            if (tstream.is_at(")"))
                break;

            args << parse_expression_root(tstream, flags);

            if (tstream.is_at(")"))
                break;

            tstream.match(",");
        }

        tstream.match(")");
        result = new FunctionCall(line, left_branch, args);
    }
    else if (tstream.is_at("["))        // Subscription
    {
        OperatorType type = statics.string_to_oper_type["[]"];
        tstream.match("[");
        Expression* right = parse_expression_root(tstream, flags);
        tstream.match("]");
        result = new BinaryOperator(line, left_branch, right, type);
    }
    else if (tstream.is_at("."))        // Element selection
    {
        OperatorType type = statics.string_to_oper_type["."];
        tstream.match(".");
        Expression* right = parse_expression_root(tstream, flags);
        result = new BinaryOperator(line, left_branch, right, type);
    }
    else                                        // e
        return NULL;

    // Parse next "same-level" expression in chain (SUB_LVL_I)
    Expression* root = parse_expression_subtree<1>( result, tstream, flags );

    if ( root == NULL )
        root = result;

    return root;
}


/*
    LVL_0 -> <Identifier>
           | <Number>
           | <Literal>
           | "true"
           | "false"
           | "(" EXPRESSION ")"
*/
Expression* Parser::parse_expression_leaf( TokenStream& tstream, Flags flags )
{
    if ( tstream.is_at("(") )
    {
        tstream.match("(");
        Expression* expr = parse_expression_root( tstream, flags );
        tstream.match(")");
        return expr;
    }
    else if ( tstream.is_at(Token::Identifier) ||
              tstream.is_at(Token::Integral) ||
              tstream.is_at(Token::Rational) ||
              tstream.is_at(Token::Literal) ||
              tstream.is_at("true") ||
              tstream.is_at("false") ||
              tstream.is_at("None"))
    {
        ulong line = tstream.current().line();
        Token token = tstream.current();
        WS::SP_Object object;

        // create WSObjects for the token, so we don't need to do it in interpreter
        switch (tstream.current().type())
        {
        case Token::Rational:
            {
                bool ok;
                double d = tstream.current().data().toDouble(&ok);
                if (ok)
                    object = WS::SP_Object(new WS::Rational(d));
                else
                    throw ParseError(QString("Line %1: Error interpreting rational number: '%2'").arg(line).arg(tstream.current().data()));
            }
            break;
        case Token::Integral:
            {
                bool ok;
                long long l = tstream.current().data().toLongLong(&ok);
                if (ok)
                    object = WS::SP_Object(new WS::Integral(l));
                else
                    throw ParseError(QString("Line %1: Error interpreting integral number: '%2'").arg(line).arg(tstream.current().data()));
            }
            break;
        case Token::Literal:
            {
                object = WS::SP_Object(new WS::String(tstream.current().data()));
            }
            break;
        case Token::Keyword:
            {
                if (tstream.current().data() == "true")
                    object = WS::SP_Object(new WS::Bool(true));
                else if (tstream.current().data() == "false")
                    object = WS::SP_Object(new WS::Bool(false));
                else if (tstream.current().data() == "None")
                    object = WS::SP_Object(new WS::None());
            }
            break;
        }

        tstream.advance();
        return new Leaf(line, token, object);
    }

    throw ParseError( QString("Unexpected token \"%1\" while parsing expression leaf")
                          .arg( tstream.current().to_string() ) );
}
