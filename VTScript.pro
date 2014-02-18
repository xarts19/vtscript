DESTDIR = $$shadowed($$PWD)
ROOTDIR = $$PWD

TEMPLATE = app
TARGET = VTScript

CONFIG += depend_includepath
CONFIG += c++11
CONFIG += qt
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release) : DEFINES *= _DEBUG
CONFIG(release, debug|release) : DEFINES *= NDEBUG
DEFINES += _CRT_SECURE_NO_WARNINGS

unix {
    CONFIG += object_parallel_to_source   # prevent object name collisions
    QMAKE_CXXFLAGS_WARN_ON = ""
    QMAKE_CXXFLAGS += -Wall -Wextra
    QMAKE_CXXFLAGS += -Wno-unknown-pragmas -Wno-missing-braces -Wno-missing-field-initializers
    #QMAKE_CXXFLAGS += -pedantic   # controversial
    
    CONFIG(release, debug|release) {
        QMAKE_CXXFLAGS += -flto
    }
}

win32-msvc* {
    # don't create separate debug and release folders (QtCreater does this instead)
    CONFIG -= debug_and_release debug_and_release_target

    QMAKE_CFLAGS_WARN_ON -= -W3
    QMAKE_CFLAGS_WARN_ON += -W4
}

HEADERS += $$files(*.h) $$files(ScriptEngine/*.h)
SOURCES += $$files(*.cpp) $$files(ScriptEngine/*.cpp)
FORMS += $$files(*.ui)
RESOURCES += $$files(*.qrc)
