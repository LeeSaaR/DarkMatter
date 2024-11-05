#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QDebug>
#include "texteditorui.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // FILE MENU METHODS
    const QList<QString> load();
    void saveDialog(TextEditorUi *_editor);
    void save(TextEditorUi *_editor);
    void newTab();
    void newTab(const QList<QString> &data);
    void setCurrentFilePath();
    void setCurrentFilePathColor();
    void enableActionsSave();

private slots:
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onClose();
    void onTabClose(int _index);
    void onTabChanged();
    void onIsSavedChanged();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    const QString styleSaved   = "QLabel{color: #a0a0a0;padding-left: 5px;}";
    const QString styleUnsaved = "QLabel{color: #9c855d;padding-left: 5px;}";

};
#endif // MAINWINDOW_H
