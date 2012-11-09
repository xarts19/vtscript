#pragma once

#include <QString>
#include <exception>

namespace VTScript
{
    namespace OperatorTypes
    {

        enum OperatorType
        {
            Error = 0,
            Assign,    // "="
            Subscript, // "["
            Dot,       // "." Element selection
            Not,       // "not"
            Plus,      // "+"
            UnaryPlus, // "+"
            Minus,     // "-"
            UnaryMinus,// "-"
            Mult,      // "*"
            Div,       // "/"
            Mod,       // "%"
            Less,      // "<"
            Greater,   // ">"
            LessEq,    // "<="
            GreaterEq, // ">="
            Equal,     // "=="
            NotEqual,  // "!="
            And,       // "and"
            Or,        // "or"
        };

        inline QString to_string(OperatorType type)
        {
            switch (type)
            {
            case Assign     : return "=";
            case Subscript  : return "__item__";
            case Dot        : return ".";
            case Not        : return "__not__";
            case Plus       : return "__add__";
            case UnaryPlus  : return "__uplus__";
            case Minus      : return "__sub__";
            case UnaryMinus : return "__uminus__";
            case Mult       : return "__mul__";
            case Div        : return "__div__";
            case Mod        : return "__mod__";
            case Less       : return "__lt__";
            case Greater    : return "__gt__";
            case LessEq     : return "__le__";
            case GreaterEq  : return "__ge__";
            case Equal      : return "__eq__";
            case NotEqual   : return "__ne__";
            case And        : return "__and__";
            case Or         : return "__or__";
            default         : return "`error`";
            }
        }
    };

    typedef OperatorTypes::OperatorType OperatorType;

    namespace WSTypes
    {
        enum WSType
        {
            Base,
            None,
            Integral,
            Rational,
            String,
            Bool,
            Function,
            List,
            WowPlayer
        };

        inline QString to_string(WSType t)
        {
            switch (t)
            {
            case Base     : return "Base class";
            case None     : return "None";
            case Integral : return "Integral";
            case Rational : return "Rational";
            case String   : return "String";
            case Bool     : return "Bool";
            case Function : return "Function";
            case List     : return "List";
            case WowPlayer: return "WowPlayer";
            default       : return "Error";
            }
        }

        QString to_string(WSType t);
    }

    typedef WSTypes::WSType WSType;

}
