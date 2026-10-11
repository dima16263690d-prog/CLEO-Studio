#include "main_window.hpp"
#include "cleo_studio/analyzer.hpp"
#include "script_editor.hpp"
#include <QAction>
#include <QCloseEvent>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSaveFile>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextCursor>
#include <QTextDocument>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), tabs_(new QTabWidget(this)),
      outputDock_(new QDockWidget(tr("Результат проверки"), this)),
      output_(new QPlainTextEdit(outputDock_)) {
    setWindowTitle(tr("CLEO Studio"));
    resize(1180, 780);
    setMinimumSize(760, 520);
    tabs_->setTabsClosable(true);
    tabs_->setMovable(true);
    tabs_->setDocumentMode(true);
    setCentralWidget(tabs_);

    output_->setReadOnly(true);
    output_->setMaximumBlockCount(5000);
    outputDock_->setWidget(output_);
    addDockWidget(Qt::BottomDockWidgetArea, outputDock_);
    outputDock_->hide();

    auto* fileMenu = menuBar()->addMenu(tr("Файл"));
    auto* newAction = fileMenu->addAction(tr("Новый файл"));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, [this] { newFile(); });
    auto* openAction = fileMenu->addAction(tr("Открыть..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, [this] { openFile(); });
    fileMenu->addSeparator();
    auto* saveAction = fileMenu->addAction(tr("Сохранить"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, [this] { saveFile(); });
    auto* saveAsAction = fileMenu->addAction(tr("Сохранить как..."));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, [this] { saveFileAs(); });
    fileMenu->addSeparator();
    auto* closeAction = fileMenu->addAction(tr("Закрыть вкладку"));
    closeAction->setShortcut(QKeySequence::Close);
    connect(closeAction, &QAction::triggered, this, [this] { closeDocument(tabs_->currentIndex()); });
    fileMenu->addSeparator();
    auto* exitAction = fileMenu->addAction(tr("Выход"));
    connect(exitAction, &QAction::triggered, this, [this] { close(); });

    auto* editMenu = menuBar()->addMenu(tr("Правка"));
    auto* undoAction = editMenu->addAction(tr("Отменить"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, [this] { if (auto* e = currentEditor()) e->undo(); });
    auto* redoAction = editMenu->addAction(tr("Повторить"));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, this, [this] { if (auto* e = currentEditor()) e->redo(); });
    editMenu->addSeparator();
    auto* cutAction = editMenu->addAction(tr("Вырезать"));
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, this, [this] { if (auto* e = currentEditor()) e->cut(); });
    auto* copyAction = editMenu->addAction(tr("Копировать"));
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, [this] { if (auto* e = currentEditor()) e->copy(); });
    auto* pasteAction = editMenu->addAction(tr("Вставить"));
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, this, [this] { if (auto* e = currentEditor()) e->paste(); });

    auto* validationMenu = menuBar()->addMenu(tr("Проверка"));
    auto* validateAction = validationMenu->addAction(tr("Проверить структуру исходника"));
    validateAction->setShortcut(QKeySequence(Qt::Key_F7));
    connect(validateAction, &QAction::triggered, this, [this] { validateCurrent(); });
    auto* viewMenu = menuBar()->addMenu(tr("Вид"));
    auto* outputAction = viewMenu->addAction(tr("Показать результат проверки"));
    connect(outputAction, &QAction::triggered, this, [this] { outputDock_->show(); });

    connect(tabs_, &QTabWidget::tabCloseRequested, this, [this](int i) { closeDocument(i); });
    connect(tabs_, &QTabWidget::currentChanged, this, [this](int) { updateCursorStatus(); });

    setStyleSheet(QStringLiteral(
        "QMainWindow, QMenuBar, QMenu, QTabWidget::pane { background: #252731; color: #e7e9f0; }"
        "QMenuBar::item:selected, QMenu::item:selected { background: #3d4356; }"
        "QTabBar::tab { background: #2e303b; color: #bfc4d4; padding: 8px 14px; }"
        "QTabBar::tab:selected { background: #3a3e4c; color: #ffffff; }"
        "QDockWidget { color: #e7e9f0; }"
        "QStatusBar { background: #20222a; color: #d7dae3; }"
        "QPlainTextEdit { background: #1e2028; color: #e7e9f0; selection-background-color: #424e68; border: 0; }"
        "QMenu { border: 1px solid #414552; }"));
    newFile();
    statusBar()->showMessage(tr("Готово. F7 — лексическая проверка, компиляция SCM пока не реализована."));
}

ScriptEditor* MainWindow::currentEditor() const {
    return static_cast<ScriptEditor*>(tabs_->currentWidget());
}

void MainWindow::newFile() {
    auto* editor = new ScriptEditor(tabs_);
    const int index = tabs_->addTab(editor, tr("Без имени"));
    tabs_->setCurrentIndex(index);
    connect(editor->document(), &QTextDocument::modificationChanged, this,
            [this, editor](bool) { updateTabTitle(editor); });
    connect(editor, &QPlainTextEdit::cursorPositionChanged, this, [this, editor] {
        if (currentEditor() == editor) updateCursorStatus();
    });
    editor->document()->setModified(false);
    updateTabTitle(editor);
    editor->setFocus();
    updateCursorStatus();
}

void MainWindow::openFile() {
    const QString path = QFileDialog::getOpenFileName(this, tr("Открыть скрипт"), QString(),
        tr("Скрипты GTA/CLEO (*.cs *.txt *.scm *.cleo);;Все файлы (*.*)"));
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Ошибка чтения"),
            tr("Не удалось открыть файл:\n%1").arg(file.errorString()));
        return;
    }
    const QByteArray bytes = file.readAll();
    auto* editor = new ScriptEditor(tabs_);
    editor->setFilePath(path);
    editor->setPlainText(QString::fromUtf8(bytes));
    editor->document()->setModified(false);
    const int index = tabs_->addTab(editor, QString());
    tabs_->setCurrentIndex(index);
    connect(editor->document(), &QTextDocument::modificationChanged, this,
            [this, editor](bool) { updateTabTitle(editor); });
    connect(editor, &QPlainTextEdit::cursorPositionChanged, this, [this, editor] {
        if (currentEditor() == editor) updateCursorStatus();
    });
    updateTabTitle(editor);
    editor->setFocus();
    statusBar()->showMessage(tr("Открыт файл: %1").arg(QFileInfo(path).fileName()), 4000);
}

bool MainWindow::saveFile() {
    auto* editor = currentEditor();
    if (!editor) return false;
    if (editor->filePath().isEmpty()) return saveFileAs();
    return writeFile(editor, editor->filePath());
}

bool MainWindow::saveFileAs() {
    auto* editor = currentEditor();
    if (!editor) return false;
    const QString path = QFileDialog::getSaveFileName(this, tr("Сохранить скрипт"),
        editor->filePath(), tr("Скрипты GTA/CLEO (*.cs *.txt);;Все файлы (*.*)"));
    return path.isEmpty() ? false : writeFile(editor, path);
}

bool MainWindow::writeFile(ScriptEditor* editor, const QString& path) {
    if (!editor) return false;
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Ошибка записи"),
            tr("Не удалось записать файл:\n%1").arg(file.errorString()));
        return false;
    }
    const QByteArray bytes = editor->toPlainText().toUtf8();
    if (file.write(bytes) != bytes.size() || !file.commit()) {
        QMessageBox::critical(this, tr("Ошибка записи"),
            tr("Не удалось завершить сохранение файла:\n%1").arg(file.errorString()));
        return false;
    }
    editor->setFilePath(path);
    editor->document()->setModified(false);
    updateTabTitle(editor);
    statusBar()->showMessage(tr("Сохранено: %1").arg(QFileInfo(path).fileName()), 4000);
    return true;
}

bool MainWindow::maybeSave(ScriptEditor* editor) {
    if (!editor || !editor->document()->isModified()) return true;
    const QString name = editor->filePath().isEmpty() ? tr("Без имени") : QFileInfo(editor->filePath()).fileName();
    const auto answer = QMessageBox::warning(this, tr("Несохранённые изменения"),
        tr("Сохранить изменения в файле «%1»?").arg(name),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (answer == QMessageBox::Save) {
        if (currentEditor() != editor) tabs_->setCurrentWidget(editor);
        return saveFile();
    }
    return answer == QMessageBox::Discard;
}

bool MainWindow::closeDocument(int index) {
    if (index < 0 || index >= tabs_->count()) return true;
    auto* editor = static_cast<ScriptEditor*>(tabs_->widget(index));
    if (!maybeSave(editor)) return false;
    tabs_->removeTab(index);
    delete editor;
    if (tabs_->count() == 0) newFile();
    return true;
}

void MainWindow::updateTabTitle(ScriptEditor* editor) {
    if (!editor) return;
    const int index = tabs_->indexOf(editor);
    if (index < 0) return;
    QString title = editor->filePath().isEmpty() ? tr("Без имени") : QFileInfo(editor->filePath()).fileName();
    if (editor->document()->isModified()) title += QLatin1Char('*');
    tabs_->setTabText(index, title);
    editor->setWindowTitle(title);
}

void MainWindow::updateCursorStatus() {
    auto* editor = currentEditor();
    if (!editor) { statusBar()->showMessage(tr("Нет открытого документа")); return; }
    const QTextCursor cursor = editor->textCursor();
    statusBar()->showMessage(tr("Строка %1, столбец %2")
        .arg(cursor.blockNumber() + 1).arg(cursor.positionInBlock() + 1));
}

void MainWindow::appendOutput(const QString& message) {
    output_->appendPlainText(message);
    outputDock_->show();
}

void MainWindow::validateCurrent() {
    auto* editor = currentEditor();
    if (!editor) return;
    output_->clear();
    const auto result = cleo_studio::analyze(editor->toPlainText().toUtf8().toStdString());
    const QString fileName = editor->filePath().isEmpty() ? tr("Без имени") : QFileInfo(editor->filePath()).fileName();
    for (const auto& d : result.diagnostics) {
        const QString severity = d.severity == cleo_studio::Severity::error ? tr("ошибка") : tr("предупреждение");
        appendOutput(QString("%1:%2:%3: %4 [%5]: %6")
            .arg(fileName).arg(d.line).arg(d.column).arg(severity)
            .arg(QString::fromStdString(d.code)).arg(QString::fromStdString(d.message)));
    }
    if (result.diagnostics.empty())
        appendOutput(tr("Анализ исходника пройден. Токенов: %1.").arg(result.tokens.size()));
    else
        appendOutput(tr("Диагностик: %1.").arg(result.diagnostics.size()));
    appendOutput(tr("Внимание: байткод SCM/CLEO не создавался; синтаксис и типы параметров проверяются не полностью."));
    statusBar()->showMessage(result.hasErrors() ? tr("Проверка завершена: обнаружены ошибки.")
                                                : tr("Анализ исходника завершён."), 6000);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    for (int i = 0; i < tabs_->count(); ++i) {
        auto* editor = static_cast<ScriptEditor*>(tabs_->widget(i));
        if (!maybeSave(editor)) { event->ignore(); return; }
    }
    event->accept();
}
