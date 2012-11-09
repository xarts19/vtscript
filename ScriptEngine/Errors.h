#pragma once

#include "Enums.h"

#include <exception>

namespace VTScript
{
    class LexError : public std::runtime_error
    {
    public:
        LexError( QString msg ) : std::runtime_error( msg.toStdString() ) {}
    };

    class ParseError : public std::runtime_error
    {
    public:
        ParseError( QString msg ) : std::runtime_error( msg.toStdString() ) {}
    };

    class CheckerError : public std::runtime_error
    {
    public:
        CheckerError( QString msg ) : std::runtime_error( msg.toStdString() ) {}
    };

    class InterpretError : public std::runtime_error
    {
    public:
        explicit InterpretError(const char* msg) : std::runtime_error(msg) {}
        explicit InterpretError(QString msg) : std::runtime_error(msg.toStdString()) {}
    };

    class InterruptError : public std::runtime_error
    {
    public:
        InterruptError() : std::runtime_error("") {}
    };

    class WrongArgumentError : public InterpretError
    {
    public:
        explicit WrongArgumentError(WSTypes::WSType t1, WSTypes::WSType t2) : 
        InterpretError(QString("Wrong argument type '%1'; expected '%2'").arg(WSTypes::to_string(t1)).arg(WSTypes::to_string(t2))) {}
    };

    class WrongNumberOfArgumentsError : public InterpretError
    {
    public:
        explicit WrongNumberOfArgumentsError(int num) : 
        InterpretError(QString("Wrong number of arguments (%1) for a function call").arg(num)) {}
    };

}