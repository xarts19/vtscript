#include "Highlighter.h"
 
#include <QtGui>


Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bwhile\\b" << "\\bif\\b" << "\\belse\\b"
                    << "\\breturn\\b" << "\\bbreak\\b" << "\\bcontinue\\b"
                    << "\\bdef\\b" << "\\band\\b" << "\\bor\\b"
                    << "\\bnot\\b" << "\\btrue\\b" << "\\bfalse\\b" << "\\bNone\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("//[^\\n\\r]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    
    doubleFormat.setForeground(Qt::darkCyan);
    rule.pattern = QRegExp("\\b\\d*\\.\\d+\\b");
    rule.format = doubleFormat;
    highlightingRules.append(rule);

    intFormat.setForeground(Qt::darkCyan);
    rule.pattern = QRegExp("\\b\\d+\\b");
    rule.format = intFormat;
    highlightingRules.append(rule);


    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("(" "\"[^\"\\r\\n]*\""
                           "|" "\'[^\'\\r\\n]*\'"
                           ")");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b(?:def )[_A-Za-z][_A-Za-z0-9]*\\(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}
