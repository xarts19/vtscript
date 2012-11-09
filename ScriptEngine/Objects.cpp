#include "Objects.h"
#include "Interpreter.h"

using namespace VTScript;
using namespace VTScript::WS;

None::VTableT None::build_vtable()
{
    VTableT table;
    table["__bool__"] = &None::__bool__;
    table["__eq__"] = &None::__eq__;
    table["__ne__"] = &None::__ne__;
    return table;
}

SP_Object None::__bool__(ObjectList& args)
{
    check_num(args, 0);
    return SP_Object(new Bool(false));
}

SP_Object None::__eq__(ObjectList& args)
{
    check_num(args, 1);
    return SP_Object(new Bool( args.at(0)->__type__() == __stype__ ));
}

SP_Object None::__ne__(ObjectList& args)
{
    check_num(args, 1);
    return SP_Object(new Bool( args.at(0)->__type__() != __stype__ ));
}


ObjectList::VTableT ObjectList::build_vtable()
{
    VTableT table;
    return table;
}


Integral::VTableT Integral::build_vtable()
{
    VTableT table;
    table["__double__"] = &__double__;
    table["__mod__"] = &__mod__;
    table["__lt__"] = &__lt__;
    table["__gt__"] = &__gt__;
    table["__le__"] = &__le__;
    table["__ge__"] = &__ge__;
    table["__eq__"] = &IComparable::__eq__;
    table["__ne__"] = &IComparable::__ne__;
    table["__add__"] = &__add__;
    table["__sub__"] = &__sub__;
    table["__mul__"] = &__mul__;
    table["__div__"] = &__div__;
    table["__uminus__"] = &__uminus__;
    table["__bool__"] = &Number::__bool__;
    return table;
}

SP_Object Integral::__mod__(ObjectList& args)
{
    check<Integral>(args);
    return SP_Object(new Integral( value() % args.at<Integral>(0)->value() ));
}

SP_Object Integral::__double__(ObjectList& args)
{
    check_num(args, 0);
    return SP_Object(new Rational( value() ));
}


Rational::VTableT Rational::build_vtable()
{
    VTableT table;
    table["__int__"] = &__int__;
    table["__lt__"] = &__lt__;
    table["__gt__"] = &__gt__;
    table["__le__"] = &__le__;
    table["__ge__"] = &__ge__;
    table["__eq__"] = &IComparable::__eq__;
    table["__ne__"] = &IComparable::__ne__;
    table["__add__"] = &__add__;
    table["__sub__"] = &__sub__;
    table["__mul__"] = &__mul__;
    table["__div__"] = &__div__;
    table["__uminus__"] = &__uminus__;
    table["__bool__"] = &Number::__bool__;
    return table;
}

SP_Object Rational::__int__(ObjectList& args)
{
    check_num(args, 0);
    return SP_Object(new Integral( value() ));
}


String::VTableT String::build_vtable()
{
    VTableT table;
    table["__bool__"] = &__bool__;
    table["__add__"] = &__add__;
    table["__lt__"] = &__lt__;
    table["__gt__"] = &__gt__;
    table["__le__"] = &__le__;
    table["__ge__"] = &__ge__;
    table["__eq__"] = &IComparable::__eq__;
    table["__ne__"] = &IComparable::__ne__;
    return table;
}

SP_Object String::__add__(ObjectList& args)
{
    check<String>(args);
    return SP_Object(new String( _value + args.at<String>(0)->value() ));
}

SP_Object String::__bool__(ObjectList& args)
{
    check_num(args, 0);
    return SP_Object(new Bool( _value.size() > 0 ));
}


Bool::VTableT Bool::build_vtable()
{
    VTableT table;
    table["__bool__"] = &__bool__;
    table["__and__"] = &__and__;
    table["__or__"] = &__or__;
    table["__not__"] = &__not__;
    table["__eq__"] = &IComparable::__eq__;
    table["__ne__"] = &IComparable::__ne__;
    return table;
}

SP_Object Bool::__bool__(ObjectList& args)
{
    check_num(args, 0);
    return SP_Object(new Bool(_value));
}

SP_Object Bool::__and__(ObjectList& args)
{
    check<Bool>(args);
    return SP_Object( new Bool(_value && args.at<Bool>(0)->value()) );
}

SP_Object Bool::__or__(ObjectList& args)
{
    check<Bool>(args);
    return SP_Object( new Bool(_value || args.at<Bool>(0)->value()) );
}

SP_Object Bool::__not__(ObjectList& args)
{
    check_num(args, 0);
    return SP_Object( new Bool(!_value) );
}


Function::VTableT Function::build_vtable()
{
    VTableT table;
    return table;
}


SP_Object UserFunction::operator()(ObjectList& args)
{
    return _ctx->exec_user_fnc(this, args);
}
