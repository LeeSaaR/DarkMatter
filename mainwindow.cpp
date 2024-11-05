#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QKeySequence>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->action_new->setShortcut(QKeySequence("Ctrl+N"));
    ui->action_open->setShortcut(QKeySequence("Ctrl+O"));
    ui->action_save->setShortcut(QKeySequence("Ctrl+S"));
    ui->action_save_as->setShortcut(QKeySequence("Shift+Ctrl+S"));
    ui->action_close->setShortcut(QKeySequence("Ctrl+Q"));

    connect(ui->action_new, &QAction::triggered, this, &MainWindow::onNew);
    connect(ui->action_open, &QAction::triggered, this, &MainWindow::onOpen);
    connect(ui->action_save, &QAction::triggered, this, &MainWindow::onSave);
    connect(ui->action_save_as, &QAction::triggered, this, &MainWindow::onSaveAs);
    connect(ui->action_close, &QAction::triggered, this, &MainWindow::onClose);
    connect(ui->tab_files, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabClose);
    connect(ui->tab_files, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    enableActionsSave();
}

MainWindow::~MainWindow()
{
    delete ui;
}

const QList<QString> MainWindow::load()
{
    const QString filePath = QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "C:/NoxTut/Qt/TextEditor",
                tr("All Files (*)"));

    if (filePath.isEmpty()) return QList<QString> {};


    QFile selectedFile(filePath);
    if (!selectedFile.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::information(this, tr("Info"), tr("Could not open file!"), QMessageBox::Ok);
        return QList<QString> {};
    }

    QTextStream in(&selectedFile);


    const QString _fileName    = QFileInfo(selectedFile.fileName()).fileName();
    const QString _filePath    = filePath;
    const QString _fileContent = in.readAll();

    selectedFile.close();

    return QList<QString> {_fileName, _filePath, _fileContent};
}

void MainWindow::saveDialog(TextEditorUi *_editor)
{
    const QString filePath = QFileDialog::getSaveFileName(
                this,
                tr("Save File"),
                "C:/NoxTut/Qt/TextEditor",
                tr("All Files (*)"));

    if (filePath.isEmpty()) return;

    QFile selectedFile(filePath);
    if (!selectedFile.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::information(this, tr("Info"), tr("Could not save file!"), QMessageBox::Ok);
        return;
    }

    QTextStream out(&selectedFile);

    out << _editor->plainText();

    const QString _fileName    = QFileInfo(selectedFile.fileName()).fileName();
    _editor->setFileName(_fileName);
    _editor->setFilePath(filePath);
    _editor->setIsSaved(true);
    ui->tab_files->setTabText(ui->tab_files->currentIndex(), _fileName);
    setCurrentFilePath();

    selectedFile.flush();
    selectedFile.close();
    return;
}

void MainWindow::save(TextEditorUi *_editor)
{
    QFile selectedFile(_editor->filePath());
    if (!selectedFile.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::information(this, tr("Info"), tr("Could not save file!"), QMessageBox::Ok);
        return;
    }

    QTextStream out(&selectedFile);

    out << _editor->plainText();
    _editor->setIsSaved(true);
    setCurrentFilePathColor();

    selectedFile.flush();
    selectedFile.close();
    return;
}

void MainWindow::newTab()
{
    int counter = 0;
    for (int i = 0; i < ui->tab_files->count(); i++) {
        QString tabName = QString("new %1").arg(counter);
        if (ui->tab_files->tabText(i) == tabName) {
            counter++;
        }
    }

    const QString tabName = QString("new %1").arg(counter);
    int _index = ui->tab_files->addTab(new TextEditorUi(this), tabName);
    ui->tab_files->setCurrentIndex(_index);
}

void MainWindow::newTab(const QList<QString> &data)
{
    int _index = ui->tab_files->addTab(new TextEditorUi(this), data[0]);
    ui->tab_files->setCurrentIndex(_index);
    TextEditorUi *_editor = qobject_cast<TextEditorUi *>(ui->tab_files->currentWidget());
    _editor->setFileName(data[0]);
    _editor->setFilePath(data[1]);
    _editor->setPlainText(data[2]);
}

void MainWindow::setCurrentFilePath()
{
    if (ui->tab_files->currentWidget() == nullptr) {
        ui->lbl_current_file->setText("");
        return;
    }
    TextEditorUi *_editor = qobject_cast<TextEditorUi *>(ui->tab_files->currentWidget());
    ui->lbl_current_file->setText(_editor->filePath());
    setCurrentFilePathColor();
}

void MainWindow::setCurrentFilePathColor()
{
    TextEditorUi *_editor = qobject_cast<TextEditorUi *>(ui->tab_files->currentWidget());
    if (_editor->isSaved()) {
        ui->lbl_current_file->setStyleSheet(styleSaved);
    }
    else {
        ui->lbl_current_file->setStyleSheet(styleUnsaved);
    }
}

void MainWindow::enableActionsSave()
{
    if (ui->tab_files->count() == 0) {
        ui->action_save->setDisabled(true);
        ui->action_save_as->setDisabled(true);
    }
    else {
        ui->action_save->setEnabled(true);
        ui->action_save_as->setEnabled(true);
    }
}

void MainWindow::onNew()
{
    qDebug() << Q_FUNC_INFO;
    newTab();
    TextEditorUi *_editor = qobject_cast<TextEditorUi *>(ui->tab_files->currentWidget());
    connect(_editor, &TextEditorUi::isSavedChanged, this, &MainWindow::onIsSavedChanged);
    _editor->setIsSaved(false);
    setCurrentFilePath();
    enableActionsSave();
}

void MainWindow::onOpen()
{
    qDebug() << Q_FUNC_INFO;
    const QList<QString> _data = load();
    if (_data.isEmpty()) return;
    newTab(_data);
    TextEditorUi *_editor = qobject_cast<TextEditorUi *>(ui->tab_files->currentWidget());
    connect(_editor, &TextEditorUi::isSavedChanged, this, &MainWindow::onIsSavedChanged);
    _editor->setIsSaved(true);
    setCurrentFilePath();
    enableActionsSave();
}

void MainWindow::onSave()
{
    qDebug() << Q_FUNC_INFO;
    if (ui->tab_files->currentWidget() == nullptr) return;
    TextEditorUi *_editor = qobject_cast<TextEditorUi *>(ui->tab_files->currentWidget());
    if (_editor->filePath().isEmpty()) {
        onSaveAs();
        return;
    }
    save(_editor);
}

void MainWindow::onSaveAs()
{
    qDebug() << Q_FUNC_INFO;
    if (ui->tab_files->currentWidget() == nullptr) return;
    TextEditorUi *_editor = qobject_cast<TextEditorUi *>(ui->tab_files->currentWidget());
    saveDialog(_editor);
}

void MainWindow::onClose()
{
    qDebug() << Q_FUNC_INFO;
    // 1. Alle gespeicherten Tabs schließen.
    for (int i = ui->tab_files->count(); i != 0; i--) {
        TextEditorUi *_editor = qobject_cast<TextEditorUi *>(ui->tab_files->widget(i-1));
        if (_editor->isSaved()) ui->tab_files->removeTab(i-1);
    }

    // 2. Wenn übrige Tabs gibt.
    if (ui->tab_files->count() != 0) {
        // Was soll mit diesen passieren?
        QMessageBox msgBox;
        msgBox.setText("Some files are not saved.");
        msgBox.setInformativeText("What will you do?");
        msgBox.addButton(QMessageBox::SaveAll);
        msgBox.addButton(QMessageBox::No);
        msgBox.button(QMessageBox::No)->setText("Discard All");
        msgBox.addButton(QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::SaveAll);
        int ret = msgBox.exec();
        switch (ret) {
        case (QMessageBox::SaveAll):
            // alle speichern.
            for (int i = ui->tab_files->count(); i != 0; i--) onTabClose(i-1);
            break;
        case (QMessageBox::No):
            // alle verwerfen.
            exit(0);
        case (QMessageBox::Cancel):
            // abbrechen.
            return;
        }
    }
    // Keine Tabs? Programm beenden
    if (ui->tab_files->count() == 0) exit(0);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << Q_FUNC_INFO;
    event->ignore();
    onClose();
}

void MainWindow::onTabClose(int _index)
{
    qDebug() << Q_FUNC_INFO;
    TextEditorUi *_editor = qobject_cast<TextEditorUi *>(ui->tab_files->widget(_index));
    if (_editor->isSaved()) {
        ui->tab_files->removeTab(_index);
    }
    else {
        QMessageBox msgBox;
        msgBox.setText(QString("\"%1\" is not saved.").arg(ui->tab_files->tabText(_index)));
        msgBox.setInformativeText("Do you want to save changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        switch (ret) {
        case (QMessageBox::Save):
            if (_editor->filePath().isEmpty()) {
                saveDialog(_editor);
            }
            else {
                save(_editor);
            }
            if (_editor->isSaved()) {
                ui->tab_files->removeTab(_index);
            }
            else {
                break;
            }
        case (QMessageBox::Discard):
            ui->tab_files->removeTab(_index);
            break;
        case (QMessageBox::Cancel):
            return;
        }
    }
    enableActionsSave();
}

void MainWindow::onTabChanged()
{
    qDebug() << Q_FUNC_INFO;
    setCurrentFilePath();
}

void MainWindow::onIsSavedChanged()
{
    TextEditorUi *_editor = qobject_cast<TextEditorUi *>(ui->tab_files->currentWidget());
    _editor->setIsSaved(false);
    setCurrentFilePathColor();
}


