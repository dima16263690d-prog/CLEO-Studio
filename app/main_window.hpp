#pragma once
#include <QMainWindow>
class QCloseEvent;
class QDockWidget;
class QPlainTextEdit;
class QTabWidget;
class QString;
class ScriptEditor;
class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    QTabWidget* tabs_{nullptr};
    QDockWidget* outputDock_{nullptr};
    QPlainTextEdit* output_{nullptr};
    [[nodiscard]] ScriptEditor* currentEditor() const;
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    bool writeFile(ScriptEditor* editor, const QString& path);
    bool maybeSave(ScriptEditor* editor);
    bool closeDocument(int index);
    void updateTabTitle(ScriptEditor* editor);
    void updateCursorStatus();
    void validateCurrent();
    void appendOutput(const QString& message);
};
