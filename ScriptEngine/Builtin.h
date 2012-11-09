#pragma once

#include "Objects.h"

namespace VTScript
{
    namespace Builtin
    {
        struct print : public WS::Function
        {
            WS::SP_Object operator()(WS::ObjectList& args);
            QString __repr__() const { return "print(a, b, ...) : Print objects; built-in"; }
        };

        struct help : public WS::Function
        {
            WS::SP_Object operator()(WS::ObjectList& args);
            QString __repr__() const { return "help(a) : Print info about an object"; }
        };

        struct _int : public WS::Function
        {
            _int() { _num_args = 1; }
            WS::SP_Object operator()(WS::ObjectList& args);
            QString __repr__() const { return "int(a) : Convert double to int; built-in"; }
        };

        struct _double : public WS::Function
        {
            _double() { _num_args = 1; }
            WS::SP_Object operator()(WS::ObjectList& args);
            QString __repr__() const { return "double(a) : Convert int to double; built-in"; }
        };

        struct _bool : public WS::Function
        {
            _bool() { _num_args = 1; }
            WS::SP_Object operator()(WS::ObjectList& args);
            QString __repr__() const { return "bool(a) : Convert any type to ; built-in"; }
        };

        struct exec : public WS::Function
        {
            exec() { _num_args = 1; }
            WS::SP_Object operator()(WS::ObjectList& args);
            QString __repr__() const { return "exec(script_path) : executes another script; no return value ; built-in"; }
        };

    };
};