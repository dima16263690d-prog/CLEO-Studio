#include "script_editor.hpp"
#include "script_highlighter.hpp"
#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextFormat>
#include <algorithm>

namespace {
class LineNumberArea final : public QWidget {
public:
    explicit LineNumberArea(ScriptEditor* editor) : QWidget(editor), editor_(editor) {}
    QSize sizeHint() const override { return QSize(editor_->lineNumberAreaWidth(), 0); }
protected:
    void paintEvent(QPaintEvent* event) override { editor_->lineNumberAreaPaintEvent(event); }
private:
    ScriptEditor* editor_;
};
}
ScriptEditor::ScriptEditor(QWidget* parent)
    : QPlainTextEdit(parent), lineNumberArea_(new LineNumberArea(this)) {
    setLineWrapMode(QPlainTextEdit::NoWrap);
    QFont editorFont("Consolas", 11);
    editorFont.setStyleHint(QFont::Monospace);
    setFont(editorFont);
    setTabStopDistance(QFontMetricsF(editorFont).horizontalAdvance(QLatin1Char(' ')) * 4.0);
    setStyleSheet(QStringLiteral("QPlainTextEdit { background: #1e2028; color: #e7e9f0; selection-background-color: #424e68; border: 0; }"));
    new ScriptHighlighter(document());
    connect(this, &QPlainTextEdit::blockCountChanged, this, [this](int) { updateLineNumberAreaWidth(0); });
    connect(this, &QPlainTextEdit::updateRequest, this, [this](const QRect& r, int dy) { updateLineNumberArea(r, dy); });
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, [this] { highlightCurrentLine(); });
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}
int ScriptEditor::lineNumberAreaWidth() const {
    int digits = 1, maximum = std::max(1, blockCount());
    while (maximum >= 10) { maximum /= 10; ++digits; }
    return 18 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}
void ScriptEditor::updateLineNumberAreaWidth(int) { setViewportMargins(lineNumberAreaWidth(), 0, 0, 0); }
void ScriptEditor::updateLineNumberArea(const QRect& rect, int dy) {
    if (dy != 0) lineNumberArea_->scroll(0, dy);
    else lineNumberArea_->update(0, rect.y(), lineNumberArea_->width(), rect.height());
    if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}
void ScriptEditor::resizeEvent(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);
    const QRect r = contentsRect();
    lineNumberArea_->setGeometry(QRect(r.left(), r.top(), lineNumberAreaWidth(), r.height()));
}
void ScriptEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(lineNumberArea_);
    painter.fillRect(event->rect(), QColor("#252731"));
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    qreal top = blockBoundingGeometry(block).translated(contentOffset()).top();
    qreal bottom = top + blockBoundingRect(block).height();
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.setPen(QColor("#81889c"));
            painter.drawText(0, static_cast<int>(top), lineNumberArea_->width() - 7,
                             fontMetrics().height(), Qt::AlignRight, QString::number(blockNumber + 1));
        }
        block = block.next();
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
        ++blockNumber;
    }
}
void ScriptEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> selections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor("#292c37"));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }
    setExtraSelections(selections);
}
QString ScriptEditor::filePath() const { return filePath_; }
void ScriptEditor::setFilePath(const QString& path) { filePath_ = path; }
QString ScriptEditor::encodingName() const { return encodingName_; }
void ScriptEditor::setEncodingName(const QString& name) { encodingName_ = name; }
bool ScriptEditor::hasByteOrderMark() const { return hasByteOrderMark_; }
void ScriptEditor::setByteOrderMark(bool enabled) { hasByteOrderMark_ = enabled; }
QString ScriptEditor::lineEnding() const { return lineEnding_; }
void ScriptEditor::setLineEnding(const QString& ending) { lineEnding_ = ending; }
