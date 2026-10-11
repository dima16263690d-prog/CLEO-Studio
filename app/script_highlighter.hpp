#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
class ScriptHighlighter final : public QSyntaxHighlighter {
public:
    explicit ScriptHighlighter(QTextDocument* document);
protected:
    void highlightBlock(const QString& text) override;
private:
    QTextCharFormat keywordFormat_, directiveFormat_, stringFormat_, commentFormat_, labelFormat_, numberFormat_, opcodeFormat_;
};
