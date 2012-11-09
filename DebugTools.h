#pragma once

#include <QtGui>
#include <QString>
#include <QList>

#include <assert.h>

#define CTASTR2(pre,post) pre ## post
#define CTASTR(pre,post) CTASTR2(pre,post)
#define STATIC_ASSERT(cond,msg) \
    typedef struct { int CTASTR(static_assertion_failed__,msg) : !!(cond); } \
    CTASTR(static_assertion_failed_,__COUNTER__)

// use like this: STATIC_ASSERT(sizeof(long)==7, use_another_compiler_luke)

class DebugOutput
{
public:
    static int output_line_limit;
    static QPlainTextEdit* sink;
    static void debugMessageDisplayFunc(QtMsgType type, const char *msg);
};


/*
    Add object to watch view.

    Use like this, assuming that  [ QString my_convert_function(void*) ] converts void pointer to MyObject to QString,
        MyObject* obj = new MyObject();
        DebugObject::append( "my object", obj, my_convert_function );
        ...
        DebugObject::remove( "my object" );
        delete obj;

    * If object is destroyed while it's in the view, behavior becomes undefined.
    * So use remove method to delete your object from view beforehand.
    * When there's more than one item with the same name, first one will be removed.
*/
class DebugObject
{
public:
    typedef QString (*to_string)(void*);

    QString name() { return _name; }
    QString repr() { return _fnc_to_str(_data); }

    static void append(QString name, void* address, to_string fnc_conversion)
    {
        assert( address != NULL );
        list << DebugObject(name, address, fnc_conversion);
    }
    static void remove(QString name)
    {
        for (int i = 0; i < list.size(); ++i)
            if (list[i].name() == name) { list.removeAt(i); return; }
    }
    static void clear()
    {
        list.clear();
    }

    static QList<DebugObject> list;

private:
    DebugObject(QString name, void* data, to_string fnc_conversion) : _name(name), _data(data), _fnc_to_str(fnc_conversion) {}

    QString _name;
    void* _data;
    to_string _fnc_to_str;
};

inline QString int_to_qstring(void* data) { return QString("%1").arg( *(int*)data ); }
