#pragma once
#include <QPlainTextEdit>
class QPaintEvent;
class QRect;
class QWidget;
class ScriptEditor final : public QPlainTextEdit {
public:
    explicit ScriptEditor(QWidget* parent = nullptr);
    [[nodiscard]] int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    [[nodiscard]] QString filePath() const;
    void setFilePath(const QString& path);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    QWidget* lineNumberArea_{nullptr};
    QString filePath_;
    void updateLineNumberAreaWidth(int);
    void updateLineNumberArea(const QRect& rect, int dy);
    void highlightCurrentLine();
};
