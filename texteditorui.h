#ifndef TEXTEDITORUI_H
#define TEXTEDITORUI_H

#include <QWidget>
#include <QDebug>
#include <QAbstractButton>

namespace Ui {
class TextEditorUi;
}

class TextEditorUi : public QWidget
{
    Q_OBJECT

public:
    explicit TextEditorUi(QWidget *parent = nullptr);
    ~TextEditorUi();

    // GETTER
    const QString &fileName() const;
    const QString &filePath() const;
    const QString plainText();
    bool isSaved() const;

    // SETTER
    void setFileName(const QString &newFileName);
    void setFilePath(const QString &newFilePath);
    void setPlainText(const QString &fileContent);
    void setIsSaved(bool newIsSaved);

signals:
    void isSavedChanged();

private slots:
    void onSort();
    void onFind();
    void onNext();
    void onPrev();
    void onReplace();
    void onReplaceAll();
    void onSortModeChanged(QAbstractButton *btn);
    void onEnableButtons(bool enable);
    void onCursorPositionChanged();

private:
    Ui::TextEditorUi *ui;
    QString m_fileName;
    QString m_filePath;
    bool m_isSaved;

    QString sortMode = "normal";
};

#endif // TEXTEDITORUI_H
