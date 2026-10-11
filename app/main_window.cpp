#include "main_window.hpp"
#include "cleo_studio/analyzer.hpp"
#include "cleo_studio/opcode_catalog.hpp"
#include "script_editor.hpp"
#include <QAbstractItemView>
#include <QAction>
#include <QCloseEvent>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QStringConverter>
#include <QHeaderView>
#include <QLineEdit>
#include <QKeySequence>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QSettings>
#include <QDir>
#include <QSaveFile>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>
#include <QStringList>
#include <string_view>
#include <cstddef>
#include <QTextCursor>
#include <QTextDocument>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), tabs_(new QTabWidget(this)),
      outputDock_(new QDockWidget(tr("Результат проверки"), this)),
      output_(new QPlainTextEdit(outputDock_)),
      opcodeDock_(new QDockWidget(tr("Каталог опкодов"), this)),
      opcodeSearch_(new QLineEdit(opcodeDock_)),
      opcodeTable_(new QTableWidget(opcodeDock_)) {
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

    opcodeSearch_->setPlaceholderText(tr("Поиск по ID, описанию и числу параметров..."));
    opcodeTable_->setColumnCount(3);
    opcodeTable_->setHorizontalHeaderLabels({tr("Опкод"), tr("Параметры"), tr("Описание")});
    opcodeTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    opcodeTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    opcodeTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    opcodeTable_->setAlternatingRowColors(true);
    opcodeTable_->verticalHeader()->setVisible(false);
    opcodeTable_->horizontalHeader()->setStretchLastSection(true);
    auto* opcodePanel = new QWidget(opcodeDock_);
    auto* opcodeLayout = new QVBoxLayout(opcodePanel);
    opcodeLayout->setContentsMargins(6, 6, 6, 6);
    opcodeLayout->addWidget(opcodeSearch_);
    opcodeLayout->addWidget(opcodeTable_);
    opcodeDock_->setWidget(opcodePanel);
    addDockWidget(Qt::RightDockWidgetArea, opcodeDock_);
    opcodeDock_->hide();
    connect(opcodeSearch_, &QLineEdit::textChanged, this,
            [this](const QString& filter) { filterOpcodeTable(filter); });
    connect(opcodeTable_, &QTableWidget::cellDoubleClicked, this,
            [this](int row, int) {
                if (!currentEditor() || !opcodeTable_->item(row, 0)) return;
                const auto id = opcodeTable_->item(row, 0)->data(Qt::UserRole).toUInt();
                currentEditor()->insertPlainText(QStringLiteral("%1: ").arg(static_cast<qulonglong>(id), 4, 16, QChar('0')).toUpper());
                currentEditor()->setFocus();
            });

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
    auto* convertToUtf8Action = fileMenu->addAction(tr("Кодировка: преобразовать в UTF-8 без BOM"));
    connect(convertToUtf8Action, &QAction::triggered, this, [this] {
        if (auto* editor = currentEditor()) {
            editor->setEncodingName(QStringLiteral("UTF-8"));
            editor->setByteOrderMark(false);
            editor->document()->setModified(true);
            updateTabTitle(editor);
            updateCursorStatus();
            appendOutput(tr("Выбран UTF-8 без BOM. Сохраните файл, чтобы применить преобразование."));
        }
    });
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

    auto* buildMenu = menuBar()->addMenu(tr("Сборка"));
    auto* configureCompilerAction = buildMenu->addAction(tr("Указать путь к gta3sc.exe..."));
    connect(configureCompilerAction, &QAction::triggered, this, [this] { configureGta3sc(); });
    auto* compileAction = buildMenu->addAction(tr("Собрать CLEO через gta3sc (экспериментально)"));
    connect(compileAction, &QAction::triggered, this, [this] { compileWithGta3sc(); });

    auto* dataMenu = menuBar()->addMenu(tr("Данные"));
    auto* loadOpcodesAction = dataMenu->addAction(tr("Загрузить базу SCM.INI..."));
    connect(loadOpcodesAction, &QAction::triggered, this, [this] { loadOpcodeCatalog(); });
    auto* showOpcodesAction = dataMenu->addAction(tr("Показать каталог опкодов"));
    connect(showOpcodesAction, &QAction::triggered, this, [this] { opcodeDock_->show(); });

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
        tr("Исходники и скрипты (*.sc *.cs *.txt *.scm *.cleo);;Все файлы (*.*)"));
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Ошибка чтения"),
            tr("Не удалось открыть файл:\n%1").arg(file.errorString()));
        return;
    }

    QByteArray payload = file.readAll();
    QString encodingName;
    QString decodedText;
    bool hasBom = false;
    bool decoded = false;

    if (payload.startsWith(QByteArray::fromHex("efbbbf"))) {
        payload.remove(0, 3);
        encodingName = QStringLiteral("UTF-8");
        hasBom = true;
        QStringDecoder decoder(QStringConverter::Utf8);
        decodedText = decoder.decode(payload);
        decoded = !decoder.hasError();
    } else if (payload.startsWith(QByteArray::fromHex("fffe"))
               || payload.startsWith(QByteArray::fromHex("feff"))) {
        const bool littleEndian = payload.startsWith(QByteArray::fromHex("fffe"));
        payload.remove(0, 2);
        encodingName = littleEndian ? QStringLiteral("UTF-16LE") : QStringLiteral("UTF-16BE");
        hasBom = true;
        const auto codec = QStringConverter::encodingForName(encodingName.toLatin1());
        if (codec) {
            QStringDecoder decoder(*codec);
            decodedText = decoder.decode(payload);
            decoded = !decoder.hasError();
        }
    } else {
        QStringDecoder decoder(QStringConverter::Utf8);
        decodedText = decoder.decode(payload);
        decoded = !decoder.hasError();
        if (decoded) encodingName = QStringLiteral("UTF-8");
    }

    if (!decoded && !hasBom) {
        const QStringList candidates{
            QStringLiteral("Windows-1251"),
            QStringLiteral("Windows-1250"),
            QStringLiteral("Windows-1252"),
            QStringLiteral("System ANSI")
        };
        bool accepted = false;
        const QString chosen = QInputDialog::getItem(
            this, tr("Кодировка исходника"),
            tr("Файл не является корректным UTF-8. Выберите кодировку, чтобы не повредить текст:"),
            candidates, 0, false, &accepted);
        if (!accepted) return;

        encodingName = chosen;
        if (chosen == QStringLiteral("System ANSI")) {
            decodedText = QString::fromLocal8Bit(payload);
            decoded = true;
        } else {
            const auto codec = QStringConverter::encodingForName(chosen.toLatin1());
            if (codec) {
                QStringDecoder decoder(*codec);
                decodedText = decoder.decode(payload);
                decoded = !decoder.hasError();
            }
        }
    }

    if (!decoded) {
        QMessageBox::critical(this, tr("Ошибка декодирования"),
            tr("Не удалось прочитать файл в выбранной кодировке. Исходный файл не изменён."));
        return;
    }

    QString lineEnding = QStringLiteral("\r\n");
    if (decodedText.contains(QStringLiteral("\r\n"))) {
        lineEnding = QStringLiteral("\r\n");
    } else if (decodedText.contains(QLatin1Char('\n'))) {
        lineEnding = QStringLiteral("\n");
    } else if (decodedText.contains(QLatin1Char('\r'))) {
        lineEnding = QStringLiteral("\r");
    }

    auto* editor = new ScriptEditor(tabs_);
    editor->setFilePath(path);
    editor->setEncodingName(encodingName);
    editor->setByteOrderMark(hasBom);
    editor->setLineEnding(lineEnding);
    editor->setPlainText(decodedText);
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
        editor->filePath(), tr("Исходники GTA3script/Sanny (*.sc *.cs *.txt);;Все файлы (*.*)"));
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

    QString text = editor->toPlainText();
    const QString ending = editor->lineEnding();
    if (ending == QStringLiteral("\r\n")) {
        text.replace(QStringLiteral("\n"), QStringLiteral("\r\n"));
    } else if (ending == QStringLiteral("\r")) {
        text.replace(QStringLiteral("\n"), QStringLiteral("\r"));
    }

    QByteArray bytes;
    const QString encodingName = editor->encodingName();
    if (encodingName == QStringLiteral("System ANSI")) {
        bytes = text.toLocal8Bit();
        if (QString::fromLocal8Bit(bytes) != text) {
            QMessageBox::warning(this, tr("Потеря символов"),
                tr("Текст содержит символы, недоступные в системной ANSI-кодировке. "
                   "Выберите «Кодировка: преобразовать в UTF-8 без BOM» и повторите сохранение."));
            return false;
        }
    } else {
        const auto codec = QStringConverter::encodingForName(encodingName.toLatin1());
        if (!codec) {
            QMessageBox::critical(this, tr("Неизвестная кодировка"),
                tr("Не удалось сохранить файл: кодировка %1 не поддерживается.").arg(encodingName));
            return false;
        }

        QStringEncoder encoder(*codec);
        bytes = encoder.encode(text);
        if (encoder.hasError()) {
            QMessageBox::warning(this, tr("Потеря символов"),
                tr("В тексте есть символы, недоступные в кодировке %1. "
                   "Выберите «Кодировка: преобразовать в UTF-8 без BOM» и повторите сохранение.")
                    .arg(encodingName));
            return false;
        }
    }

    if (editor->hasByteOrderMark()) {
        if (encodingName == QStringLiteral("UTF-8")) {
            bytes.prepend(QByteArray::fromHex("efbbbf"));
        } else if (encodingName == QStringLiteral("UTF-16LE")) {
            bytes.prepend(QByteArray::fromHex("fffe"));
        } else if (encodingName == QStringLiteral("UTF-16BE")) {
            bytes.prepend(QByteArray::fromHex("feff"));
        }
    }

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
    statusBar()->showMessage(tr("Строка %1, столбец %2 | %3")
        .arg(cursor.blockNumber() + 1).arg(cursor.positionInBlock() + 1)
        .arg(editor->encodingName()));
}

void MainWindow::appendOutput(const QString& message) {
    output_->appendPlainText(message);
    outputDock_->show();
}

void MainWindow::loadOpcodeCatalog() {
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Загрузить базу опкодов"), QString(),
        tr("SCM opcode definitions (*.ini);;Все файлы (*.*)"));
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Ошибка чтения"),
            tr("Не удалось открыть базу опкодов:\n%1").arg(file.errorString()));
        return;
    }
    const QByteArray bytes = file.readAll();
    opcodeCatalog_ = cleo_studio::parseOpcodeCatalog(
        std::string_view(bytes.constData(), static_cast<std::size_t>(bytes.size())));

    opcodeTable_->setRowCount(static_cast<int>(opcodeCatalog_.opcodes.size()));
    int row = 0;
    for (const auto& [id, definition] : opcodeCatalog_.opcodes) {
        const QString idText = QString::number(id, 16).toUpper().rightJustified(4, QLatin1Char('0'));
        auto* idItem = new QTableWidgetItem(idText);
        idItem->setData(Qt::UserRole, static_cast<unsigned int>(id));
        opcodeTable_->setItem(row, 0, idItem);
        opcodeTable_->setItem(row, 1,
            new QTableWidgetItem(QString::number(static_cast<qulonglong>(definition.parameterCount))));
        opcodeTable_->setItem(row, 2,
            new QTableWidgetItem(QString::fromUtf8(definition.description.data(),
                static_cast<qsizetype>(definition.description.size()))));
        ++row;
    }
    opcodeSearch_->clear();
    filterOpcodeTable(QString());
    opcodeDock_->show();

    appendOutput(tr("База %1: загружено %2 уникальных ID опкодов.")
        .arg(QFileInfo(path).fileName()).arg(opcodeCatalog_.opcodes.size()));
    if (!opcodeCatalog_.version.empty()) appendOutput(tr("Версия данных: %1").arg(QString::fromStdString(opcodeCatalog_.version)));
    if (!opcodeCatalog_.publisher.empty()) appendOutput(tr("Источник/издатель: %1").arg(QString::fromStdString(opcodeCatalog_.publisher)));
    if (!opcodeCatalog_.date.empty()) appendOutput(tr("Дата данных: %1").arg(QString::fromStdString(opcodeCatalog_.date)));
    for (const auto& diagnostic : opcodeCatalog_.diagnostics) {
        const QString severity = diagnostic.severity == cleo_studio::Severity::error
            ? tr("ошибка") : tr("предупреждение");
        appendOutput(QString("%1:%2: %3 [%4]: %5")
            .arg(QFileInfo(path).fileName()).arg(diagnostic.line).arg(severity)
            .arg(QString::fromStdString(diagnostic.code))
            .arg(QString::fromStdString(diagnostic.message)));
    }
    statusBar()->showMessage(opcodeCatalog_.hasErrors()
        ? tr("Каталог загружен частично: есть ошибки в определениях.")
        : tr("Каталог опкодов загружен."), 6000);
}

void MainWindow::configureGta3sc() {
    QSettings settings;
    const QString currentPath = settings.value(QStringLiteral("compiler/gta3scPath")).toString();
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Выбрать компилятор gta3sc"), currentPath,
        tr("gta3sc executable (gta3sc.exe);;Программы (*.exe);;Все файлы (*.*)"));
    if (path.isEmpty()) return;
    settings.setValue(QStringLiteral("compiler/gta3scPath"), path);
    appendOutput(tr("Внешний компилятор сохранён: %1").arg(path));
    statusBar()->showMessage(tr("Путь к gta3sc сохранён."), 4000);
}

void MainWindow::compileWithGta3sc() {
    if (compilerProcess_ && compilerProcess_->state() != QProcess::NotRunning) {
        QMessageBox::information(this, tr("Сборка выполняется"),
            tr("Дождитесь завершения текущей сборки."));
        return;
    }

    auto* editor = currentEditor();
    if (!editor) return;
    if (editor->filePath().isEmpty() || editor->document()->isModified()) {
        if (!saveFile()) return;
    }

    const QString sourcePath = editor->filePath();
    if (QFileInfo(sourcePath).suffix().compare(QStringLiteral("sc"), Qt::CaseInsensitive) != 0) {
        QMessageBox::warning(this, tr("Неподдерживаемый формат исходника"),
            tr("Экспериментальный backend gta3sc здесь принимает исходники GTA3script с расширением .sc. "
               "Это не означает полной совместимости со всеми возможностями языка Sanny Builder."));
        return;
    }

    QSettings settings;
    QString compilerPath = settings.value(QStringLiteral("compiler/gta3scPath")).toString();
    if (compilerPath.isEmpty() || !QFileInfo::exists(compilerPath)) {
        configureGta3sc();
        compilerPath = settings.value(QStringLiteral("compiler/gta3scPath")).toString();
    }
    if (compilerPath.isEmpty() || !QFileInfo::exists(compilerPath)) {
        appendOutput(tr("Сборка отменена: путь к gta3sc.exe не указан."));
        return;
    }

    const QFileInfo sourceInfo(sourcePath);
    const QString defaultOutput = QDir(sourceInfo.absolutePath()).filePath(
        sourceInfo.completeBaseName() + QStringLiteral("_compiled.cs"));
    const QString outputPath = QFileDialog::getSaveFileName(
        this, tr("Куда сохранить CLEO-скрипт"), defaultOutput,
        tr("CLEO scripts (*.cs);;Все файлы (*.*)"));
    if (outputPath.isEmpty()) return;
    if (QFileInfo(outputPath).absoluteFilePath() == QFileInfo(sourcePath).absoluteFilePath()) {
        QMessageBox::warning(this, tr("Неверный путь"),
            tr("Выходной файл не должен совпадать с исходником."));
        return;
    }

    auto* process = new QProcess(this);
    compilerProcess_ = process;
    process->setProgram(compilerPath);
    process->setWorkingDirectory(QFileInfo(compilerPath).absolutePath());
    process->setArguments({
        QStringLiteral("compile"),
        QStringLiteral("--config=gtasa"),
        QStringLiteral("--guesser"),
        QStringLiteral("-fbreak-continue"),
        QStringLiteral("-fcleo"),
        QStringLiteral("--cs"),
        QStringLiteral("-o"),
        outputPath,
        sourcePath
    });

    appendOutput(tr("Запуск gta3sc для %1").arg(QFileInfo(sourcePath).fileName()));
    appendOutput(tr("Профиль: GTA San Andreas; вывод: %1").arg(outputPath));
    outputDock_->show();
    statusBar()->showMessage(tr("Компиляция через внешний gta3sc..."));

    connect(process, &QProcess::readyReadStandardOutput, this, [this, process] {
        const QString text = QString::fromLocal8Bit(process->readAllStandardOutput()).trimmed();
        if (!text.isEmpty()) appendOutput(text);
    });
    connect(process, &QProcess::readyReadStandardError, this, [this, process] {
        const QString text = QString::fromLocal8Bit(process->readAllStandardError()).trimmed();
        if (!text.isEmpty()) appendOutput(text);
    });
    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart) return;
        appendOutput(tr("Не удалось запустить gta3sc: %1").arg(process->errorString()));
        statusBar()->showMessage(tr("Не удалось запустить внешний компилятор."), 6000);
        if (compilerProcess_ == process) compilerProcess_ = nullptr;
        process->deleteLater();
    });
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
        [this, process, outputPath](int exitCode, QProcess::ExitStatus exitStatus) {
            const QString standardOut = QString::fromLocal8Bit(process->readAllStandardOutput()).trimmed();
            const QString standardError = QString::fromLocal8Bit(process->readAllStandardError()).trimmed();
            if (!standardOut.isEmpty()) appendOutput(standardOut);
            if (!standardError.isEmpty()) appendOutput(standardError);

            const bool succeeded = exitStatus == QProcess::NormalExit
                && exitCode == 0 && QFileInfo::exists(outputPath);
            if (succeeded) {
                appendOutput(tr("Сборка завершена: %1").arg(outputPath));
                statusBar()->showMessage(tr("CLEO-скрипт создан внешним компилятором."), 7000);
            } else {
                appendOutput(tr("Сборка не удалась (код выхода %1). Проверьте журнал и совместимость исходника.")
                    .arg(exitCode));
                statusBar()->showMessage(tr("Внешний компилятор завершился с ошибкой."), 7000);
            }
            if (compilerProcess_ == process) compilerProcess_ = nullptr;
            process->deleteLater();
        });
    process->start();
}

void MainWindow::filterOpcodeTable(const QString& filter) {
    const QString query = filter.trimmed();
    for (int row = 0; row < opcodeTable_->rowCount(); ++row) {
        bool match = query.isEmpty();
        for (int column = 0; !match && column < opcodeTable_->columnCount(); ++column) {
            const auto* item = opcodeTable_->item(row, column);
            match = item && item->text().contains(query, Qt::CaseInsensitive);
        }
        opcodeTable_->setRowHidden(row, !match);
    }
}

void MainWindow::validateCurrent() {
    auto* editor = currentEditor();
    if (!editor) return;
    output_->clear();
    const auto result = cleo_studio::analyze(editor->toPlainText().toUtf8().toStdString(),
        opcodeCatalog_.opcodes.empty() ? nullptr : &opcodeCatalog_);
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
    if (compilerProcess_ && compilerProcess_->state() != QProcess::NotRunning) {
        QMessageBox::information(this, tr("Сборка выполняется"),
            tr("Дождитесь завершения внешнего компилятора перед выходом."));
        event->ignore();
        return;
    }
    for (int i = 0; i < tabs_->count(); ++i) {
        auto* editor = static_cast<ScriptEditor*>(tabs_->widget(i));
        if (!maybeSave(editor)) { event->ignore(); return; }
    }
    event->accept();
}
