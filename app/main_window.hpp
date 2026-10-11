#pragma once
#include "cleo_studio/opcode_catalog.hpp"
#include <QMainWindow>
class QCloseEvent;
class QDockWidget;
class QLineEdit;
class QPlainTextEdit;
class QProcess;
class QTabWidget;
class QTableWidget;
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
    QDockWidget* opcodeDock_{nullptr};
    QLineEdit* opcodeSearch_{nullptr};
    QTableWidget* opcodeTable_{nullptr};
    cleo_studio::OpcodeCatalog opcodeCatalog_;
    QProcess* compilerProcess_{nullptr};
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
    void loadOpcodeCatalog();
    void filterOpcodeTable(const QString& filter);
    void configureGta3sc();
    void compileWithGta3sc();
};
