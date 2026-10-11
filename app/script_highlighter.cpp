#include "script_highlighter.hpp"
#include <QColor>
#include <QFont>
#include <QRegularExpression>

ScriptHighlighter::ScriptHighlighter(QTextDocument* document) : QSyntaxHighlighter(document) {
    keywordFormat_.setForeground(QColor("#c792ea"));
    keywordFormat_.setFontWeight(QFont::DemiBold);
    directiveFormat_.setForeground(QColor("#ffcb6b"));
    directiveFormat_.setFontWeight(QFont::DemiBold);
    stringFormat_.setForeground(QColor("#c3e88d"));
    commentFormat_.setForeground(QColor("#7f879a"));
    commentFormat_.setFontItalic(true);
    labelFormat_.setForeground(QColor("#f78c6c"));
    labelFormat_.setFontWeight(QFont::DemiBold);
    numberFormat_.setForeground(QColor("#f78c6c"));
    opcodeFormat_.setForeground(QColor("#82aaff"));
    opcodeFormat_.setFontWeight(QFont::DemiBold);
}
void ScriptHighlighter::highlightBlock(const QString& text) {
    static const QRegularExpression keywords(QStringLiteral(R"(\b(wait|if|then|else|end|while|repeat|until|for|to|step|next|break|continue|jump|goto|gosub|return|call|thread|terminate|script_name)\b)"), QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression directives(QStringLiteral(R"(\{\$[^}]*\})"));
    static const QRegularExpression strings(QStringLiteral(R"("(?:\\.|[^"\\])*")"));
    static const QRegularExpression comments(QStringLiteral(R"(//.*$)"));
    static const QRegularExpression labels(QStringLiteral(R"(:[A-Za-z_\x80-\xff][A-Za-z0-9_\x80-\xff]*)"));
    static const QRegularExpression numbers(QStringLiteral(R"(\b(?:0[xX][0-9A-Fa-f]+|\d+)\b)"));
    static const QRegularExpression opcodes(QStringLiteral(R"(^\s*[0-9A-Fa-f]{4}:)"));
    const auto apply = [this, &text](const QRegularExpression& pattern, const QTextCharFormat& format) {
        auto matches = pattern.globalMatch(text);
        while (matches.hasNext()) {
            const auto match = matches.next();
            setFormat(match.capturedStart(), match.capturedLength(), format);
        }
    };
    apply(comments, commentFormat_);
    apply(strings, stringFormat_);
    apply(directives, directiveFormat_);
    apply(labels, labelFormat_);
    apply(numbers, numberFormat_);
    apply(keywords, keywordFormat_);
    apply(opcodes, opcodeFormat_);
}
