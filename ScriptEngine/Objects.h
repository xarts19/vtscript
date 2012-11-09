#pragma once

#include "Errors.h"
#include "Enums.h"

#include <QString>
#include <QStringList>
#include <QHash>
#include <QSharedPointer>

#include <assert.h>

/*
    Something to keep in mind:

        method signature: SP_Object <method_name>(ObjectList arguments);

        __bool__  member should ALWAYS return Bool shared pointer, otherwise bad things will happen

        ObjectList::at<X>  method should always supply correct X argument as the function uses static cast,
                           so make sure you use WSObjectList::check<...> before
*/

#define DEFINE_V_TABLE_T(_typename_) \
    typedef SP_Object (_typename_::*PREPR_MethodT)(ObjectList& args); \
    typedef QHash<QString, PREPR_MethodT> VTableT; 

namespace VTScript
{
    // forward declarations
    class Interpreter;
    namespace AST
    {
        class Block;
    }
    
    namespace WS
    {
        // forward declarations
        class Object;
        class ObjectList;
        
        typedef QSharedPointer<Object> SP_Object;
        
        /*
            Base class for objects.
            But you should inherit from VTableObject, not from Object.
        */
        class Object
        {
        public:
            virtual ~Object() {}

            static const WSTypes::WSType __stype__ = WSTypes::Base;
            virtual WSTypes::WSType __type__() const { return __stype__; }
            
            virtual QString __repr__() const { return QString("%1(%2)").arg( WSTypes::to_string(__type__()) ).arg(__str__()); }
            virtual QString __str__() const { return "Not implemented"; }

            virtual SP_Object invoke(QString method_name, ObjectList arguments) = 0;
        };


        /*
            Utility class, don't try to directly use it,
            instead, inherit from it with your class as template argument.
            Your class should provide build_vtable() funciton 
            that returns QHash from QString to your class' method type

            Example:
                class YourClass
                {
                public:
                    DEFINE_V_TABLE_T(YourClass);
                    static VTableT build_vtable();
                ...
                
        */
        template <typename T>
        class VTableObject : public Object
        {
        public:
            SP_Object invoke(QString method_name, ObjectList arguments)
            {
                T_Method method = vtable[method_name];

                if (method != NULL)
                    return (static_cast<T*>(this)->*method)(arguments);

                throw InterpretError(QString("%1 has no method %2").arg(__repr__()).arg(method_name));
            }

        private:
            typedef SP_Object (T::*T_Method)(ObjectList& args);
            typedef QHash<QString, T_Method> VTableT;
            static VTableT vtable;
            static VTableT build_vtable();
        };
        template <typename T>
        typename VTableObject<T>::VTableT VTableObject<T>::vtable = T::build_vtable();

        
        class None : public VTableObject<None>
        {
        public:
            DEFINE_V_TABLE_T(None);
            static VTableT build_vtable();

        public:
            static const WSTypes::WSType __stype__ = WSTypes::None;
            virtual WSTypes::WSType __type__() const { return __stype__; }

            QString __str__() const { return "None"; }

            /* METHODS */
            SP_Object __bool__(ObjectList& args);
            SP_Object __eq__(ObjectList& args);
            SP_Object __ne__(ObjectList& args);
        };


        class ObjectList : public VTableObject<ObjectList>
        {
        public:
            DEFINE_V_TABLE_T(ObjectList);
            static VTableT build_vtable();

        public:
            static const WSTypes::WSType __stype__ = WSTypes::List;
            WSTypes::WSType __type__() const { return __stype__; }

            QString __str__() const
            {
                QStringList str;
                foreach(SP_Object obj, _list)
                    str << obj->__str__();
                return "[" + str.join(", ") + "]";
            }
            
            inline int size() const { return _list.size(); }
            inline SP_Object at(int index) const { return _list[index]; }
            inline SP_Object takeFirst() { return _list.takeFirst(); }

            inline void append(Object* obj) { _list << SP_Object(obj); }
            inline void append(SP_Object obj) { _list << obj; }
            
            inline const QList< SP_Object >& values() const { return _list; }

            template <typename T>
            QSharedPointer<T> at(int pos)
            {
                assert(pos >= 0 && pos < _list.size());
                assert(T::__stype__ == _list[pos]->__type__());
                return _list[pos].staticCast<T>();
            }

            /* METHODS */
            // Not implemented

        private:
            QList< SP_Object > _list;
        };


        template <typename T_SpecificObject>
        class IComparable
        {
        public:
            /* METHODS */
            SP_Object __lt__(ObjectList& args)
            {
                check<T_SpecificObject>(args);
                return SP_Object(new Bool( static_cast<T_SpecificObject*>(this)->value()
                                                            < args.at<T_SpecificObject>(0)->value() ));
            }
            SP_Object __gt__(ObjectList& args)
            {
                check<T_SpecificObject>(args);
                return SP_Object(new Bool( static_cast<T_SpecificObject*>(this)->value()
                                                            > args.at<T_SpecificObject>(0)->value() ));
            }
            SP_Object __le__(ObjectList& args)
            {
                check<T_SpecificObject>(args);
                return SP_Object(new Bool( static_cast<T_SpecificObject*>(this)->value()
                                                            <= args.at<T_SpecificObject>(0)->value() ));
            }
            SP_Object __ge__(ObjectList& args)
            {
                check<T_SpecificObject>(args);
                return SP_Object(new Bool( static_cast<T_SpecificObject*>(this)->value()
                                                            >= args.at<T_SpecificObject>(0)->value() ));
            }
            SP_Object __eq__(ObjectList& args)
            {
                check_num(args, 1);
                if (args.at(0)->__type__() == None::__stype__)
                    return SP_Object(new Bool(false));

                check<T_SpecificObject>(args);
                return SP_Object(new Bool( static_cast<T_SpecificObject*>(this)->value()
                                                            == args.at<T_SpecificObject>(0)->value() ));
            }
            SP_Object __ne__(ObjectList& args)
            {
                check_num(args, 1);
                if (args.at(0)->__type__() == None::__stype__)
                    return SP_Object(new Bool(true));

                check<T_SpecificObject>(args);
                return SP_Object(new Bool( static_cast<T_SpecificObject*>(this)->value()
                                                            != args.at<T_SpecificObject>(0)->value() ));
            }

        };

        /*
            Base class for numerical types
        */
        template <typename T_SpecificNumber, typename ValueType>
        class Number : public VTableObject<T_SpecificNumber>
        {
        public:
            Number(ValueType value) : _value(value) {}
            virtual ~Number() {}

            QString __str__() const { return QString::number(_value); }

            inline ValueType value() const { return _value; }

            /* METHODS */
            SP_Object __add__(ObjectList& args)
            {
                check<T_SpecificNumber>(args);
                return SP_Object(new T_SpecificNumber( _value + args.at<T_SpecificNumber>(0)->value() ));
            }
            SP_Object __sub__(ObjectList& args)
            {
                check<T_SpecificNumber>(args);
                return SP_Object(new T_SpecificNumber( _value - args.at<T_SpecificNumber>(0)->value() ));
            }
            SP_Object __mul__(ObjectList& args)
            {
                check<T_SpecificNumber>(args);
                return SP_Object(new T_SpecificNumber( _value * args.at<T_SpecificNumber>(0)->value() ));
            }
            SP_Object __div__(ObjectList& args)
            {
                check<T_SpecificNumber>(args);
                return SP_Object(new T_SpecificNumber( _value / args.at<T_SpecificNumber>(0)->value() ));
            }
            SP_Object __uminus__(ObjectList& args)
            {
                check_num(args, 0);
                return SP_Object(new T_SpecificNumber( - _value ));
            }
            SP_Object __bool__(ObjectList& args)
            {
                check_num(args, 0);
                return SP_Object(new Bool( _value != 0 ));
            }

        private:
            ValueType _value;
        };


        class Integral : public Number<Integral, long long>, public IComparable<Integral>
        {
        public:
            DEFINE_V_TABLE_T(Integral);
            static VTableT build_vtable();

        public:
            Integral(long long value) : Number<Integral, long long>(value) {}

            static const WSTypes::WSType __stype__ = WSTypes::Integral;
            WSTypes::WSType __type__() const { return __stype__; }
            
            /* METHODS */
            SP_Object __mod__(ObjectList& args);
            SP_Object __double__(ObjectList& args);
        };


        class Rational : public Number<Rational, double>, public IComparable<Rational>
        {
        public:
            DEFINE_V_TABLE_T(Rational);
            static VTableT build_vtable();

        public:
            Rational(double value) : Number<Rational, double>(value) {}

            static const WSTypes::WSType __stype__ = WSTypes::Rational;
            WSTypes::WSType __type__() const { return __stype__; }
            
            /* METHODS */
            SP_Object __int__(ObjectList& args);
        };


        class String : public VTableObject<String>, public IComparable<String>
        {
        public:
            DEFINE_V_TABLE_T(String);
            static VTableT build_vtable();

        public:
            String(QString value) : _value(value) {}

            static const WSTypes::WSType __stype__ = WSTypes::String;
            WSTypes::WSType __type__() const { return __stype__; }

            QString __str__() const { return _value; }
            
            inline QString value() const { return _value; }

            /* METHODS */
            SP_Object __add__(ObjectList& args);
            SP_Object __bool__(ObjectList& args);

        private:
            QString _value;
        };

        class Bool : public VTableObject<Bool>, public IComparable<Bool>
        {
        public:
            DEFINE_V_TABLE_T(Bool);
            static VTableT build_vtable();

        public:
            Bool(bool value): _value(value) {}

            static const WSTypes::WSType __stype__ = WSTypes::Bool;
            WSTypes::WSType __type__() const { return __stype__; }

            QString __repr__() const { return QString("%1(%2)").arg( WSTypes::to_string(__stype__) ).arg(_value ? "true" : "false"); }
            QString __str__() const { return (_value ? "true" : "false"); }
            
            inline bool value() const { return _value; }

            /* METHODS */
            SP_Object __bool__(ObjectList& args);
            SP_Object __and__(ObjectList& args);
            SP_Object __or__(ObjectList& args);
            SP_Object __not__(ObjectList& args);


        private:
            bool _value;
        };

        /*
            Base class for all functions
        */
        class Function : public VTableObject<Function>
        {
        public:
            DEFINE_V_TABLE_T(Function);
            static VTableT build_vtable();

        public:
            Function() : _num_args(-1) {}
            virtual ~Function() {}

            static const WSTypes::WSType __stype__ = WSTypes::Function;
            virtual WSTypes::WSType __type__() const { return __stype__; }

            virtual QString __repr__() const = 0;
            virtual QString __str__() const { return "{Functions don't have string representations}"; }
            
            virtual SP_Object operator()(ObjectList& args) = 0;

            virtual int number_of_arguments() const { return _num_args; }
            virtual bool check_num_arguments(int num) { return (_num_args == num || _num_args == -1); }

        protected:
            int _num_args;
        };

        class UserFunction : public Function
        {
        public:
            UserFunction(QString name, QStringList parameters, AST::Block* body) :
                                    _ctx(NULL), _name(name), _parameters(parameters), _body(body)
            {
                _num_args = parameters.size();
            }

            SP_Object operator()(ObjectList& args);
            virtual QString __repr__() const { return QString("%1 (%2) : User-defined function").arg(_name).arg(_parameters.join(",")); };

            inline const QString& name() const { return _name; }
            inline const QStringList& parameters() const { return _parameters; }
            inline AST::Block* body() const { return _body; }

            inline const void set_interpreter(Interpreter* ctx) { _ctx = ctx; }

        private:
            Interpreter* _ctx;
            QString _name;
            QStringList _parameters;
            AST::Block* _body;
        };

        inline void check_num(const ObjectList& list, int num)
        {
            if (list.size() != num) throw WrongNumberOfArgumentsError(list.size());
        }

        template <typename T1>
        void check(const ObjectList& list)
        {
            if (list.size() != 1) throw WrongNumberOfArgumentsError(list.size());
            if (list.at(0)->__type__() != T1::__stype__) throw WrongArgumentError(list.at(0)->__type__(), T1::__stype__);
        }

        template <typename T1, typename T2>
        void check(const ObjectList& list)
        {
            if (list.size() != 2) throw WrongNumberOfArgumentsError(list.size());
            if (list.at(0)->__type__() != T1::__stype__) throw WrongArgumentError(list.at(0)->__type__(), T1::__stype__);
            if (list.at(1)->__type__() != T2::__stype__) throw WrongArgumentError(list.at(1)->__type__(), T2::__stype__);
        }

        template <typename T1, typename T2, typename T3>
        void check(const ObjectList& list)
        {
            if (list.size() != 3) throw WrongNumberOfArgumentsError(list.size());
            if (list.at(0)->__type__() != T1::__stype__) throw WrongArgumentError(list.at(0)->__type__(), T1::__stype__);
            if (list.at(1)->__type__() != T2::__stype__) throw WrongArgumentError(list.at(1)->__type__(), T2::__stype__);
            if (list.at(2)->__type__() != T3::__stype__) throw WrongArgumentError(list.at(2)->__type__(), T3::__stype__);
        }

    };      // WS

};      // VTScript