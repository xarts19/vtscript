#include "DebugTools.h"

#include <QTime>
#include <QThread>
#include <QApplication>

#include <assert.h>

QPlainTextEdit* DebugOutput::sink = NULL;
int DebugOutput::output_line_limit = 1000;

QList<DebugObject> DebugObject::list;

void DebugOutput::debugMessageDisplayFunc(QtMsgType type, const QMessageLogContext& /*ctx*/, const QString& msg)
{
    bool do_abort = false;
    const char* msgTypeStr = NULL;
    switch (type) {
    case QtDebugMsg:
        msgTypeStr = "Debug";
        break;
    case QtWarningMsg:
        msgTypeStr = "Warning";
        break;
    case QtCriticalMsg:
        msgTypeStr = "Critical";
        break;
    case QtFatalMsg:
        msgTypeStr = "Fatal";
        do_abort = true;
        break;
    default:
        assert(0);
        return;
    }
    QTime now = QTime::currentTime();
    QString formattedMessage = 
        QString::fromLatin1("%4 %5: %6")
            .arg(now.toString("hh:mm:ss:zzz"))
            .arg(msgTypeStr)
            .arg(msg);

    // print on console:
    fprintf( stderr, "%s\n", formattedMessage.toLocal8Bit().constData() );
    // print in debug log window
    {
        bool isMainThread = QThread::currentThread() == QApplication::instance()->thread();
        if(sink)
        {
            if( isMainThread )
                sink->appendPlainText( formattedMessage );
            else // additional code, so that qDebug calls in threads will work aswell
                QMetaObject::invokeMethod( sink, "appendPlainText", Qt::QueuedConnection, Q_ARG( QString, formattedMessage ) );
        }
    }
}
