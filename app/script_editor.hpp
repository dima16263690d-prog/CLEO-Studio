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

    [[nodiscard]] QString encodingName() const;
    void setEncodingName(const QString& name);
    [[nodiscard]] bool hasByteOrderMark() const;
    void setByteOrderMark(bool enabled);
    [[nodiscard]] QString lineEnding() const;
    void setLineEnding(const QString& lineEnding);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    QWidget* lineNumberArea_{nullptr};
    QString filePath_;
    QString encodingName_{QStringLiteral("UTF-8")};
    QString lineEnding_{QStringLiteral("\r\n")};
    bool hasByteOrderMark_{false};
    void updateLineNumberAreaWidth(int);
    void updateLineNumberArea(const QRect& rect, int dy);
    void highlightCurrentLine();
};
