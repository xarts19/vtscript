#pragma once

#include "Enums.h"

#include <QString>

namespace VTScript
{
    struct Token
    {
        enum Type
        {
            Identifier,
            Integral,
            Rational,
            Literal,
            Operator,
            Whitespace,
            Newline,
            Comment,
            Keyword,
            Eof,
            Error
        };

        inline static const char* Token::type_toString( Type type )
        {
            switch ( type )
            {
            case Identifier : return "Identifier";
            case Integral   : return "Integral";
            case Rational   : return "Rational";
            case Literal    : return "Literal";
            case Operator   : return "Operator";
            case Whitespace : return "Whitespace";
            case Newline    : return "Newline";
            case Comment    : return "Comment";
            case Keyword    : return "Keyword";
            case Eof        : return "EOF";
            case Error      : return "Error";
            default         : return "Other";
            }
        }

        Token(): _type( Error ), _data( "Error" ), _line( -1 ) {}
        Token( Type t, QString d ) : _type( t ), _data( d ), _line( -1 ) {}
        Token( Type t, QString d, ulong l ) : _type( t ), _data( d ), _line( l ) {}

        inline bool operator==( const Token& other ) const
        { return ( this->type() == other.type() && this->data() == other.data() ); }

        inline bool operator!=( const Token& other ) const
        { return !( *this == other ); }

        inline Token& operator=( const Token& other )
        {
            this->_type = other.type();
            this->_data = other.data();
            this->_line = other.line();
            return *this;
        }

        inline QString to_string() const
        { return QString( "line: %1 { %2 } '%3'" ).arg( _line ).arg( type_toString( _type ) ).arg( _data ); }

        inline const Type& type() const { return _type; }
        inline const QString& data() const { return _data; }
        inline const ulong line() const { return _line; }

    private:
        Type _type;
        QString _data;
        ulong _line;
    };

};
