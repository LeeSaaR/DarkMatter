#include "texteditorui.h"
#include "ui_texteditorui.h"

TextEditorUi::TextEditorUi(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TextEditorUi)
{
    ui->setupUi(this);
    connect(ui->btn_sort, &QPushButton::clicked, this, &TextEditorUi::onSort);
    connect(ui->btn_find, &QPushButton::clicked, this, &TextEditorUi::onFind);
    connect(ui->btn_previous, &QPushButton::clicked, this, &TextEditorUi::onPrev);
    connect(ui->btn_next, &QPushButton::clicked, this, &TextEditorUi::onNext);
    connect(ui->btn_replace, &QPushButton::clicked, this, &TextEditorUi::onReplace);
    connect(ui->btn_replace_all, &QPushButton::clicked, this, &TextEditorUi::onReplaceAll);
    connect(ui->le_find, &QLineEdit::textChanged, this, &TextEditorUi::onFind);
    connect(ui->btnGroup_sort, &QButtonGroup::buttonClicked, this, &TextEditorUi::onSortModeChanged);
    connect(ui->editor, &TextEditor::textChanged, this, &TextEditorUi::isSavedChanged);

    connect(ui->editor, &TextEditor::enableButtons, this, &TextEditorUi::onEnableButtons);
    connect(ui->editor, &TextEditor::cursorPositionChanged, this, &TextEditorUi::onCursorPositionChanged);

    onEnableButtons(false);
}

TextEditorUi::~TextEditorUi()
{
    delete ui;
}

bool TextEditorUi::isSaved() const
{
    return m_isSaved;
}

void TextEditorUi::setIsSaved(bool newIsSaved)
{
    m_isSaved = newIsSaved;
}

const QString &TextEditorUi::filePath() const
{
    return m_filePath;
}

void TextEditorUi::setFilePath(const QString &newFilePath)
{
    m_filePath = newFilePath;
}

void TextEditorUi::setPlainText(const QString &fileContent)
{
    ui->editor->blockSignals(true);
    ui->editor->setPlainText(fileContent);
    ui->editor->blockSignals(false);
}

const QString TextEditorUi::plainText()
{
    return ui->editor->toPlainText();
}

const QString &TextEditorUi::fileName() const
{
    return m_fileName;
}

void TextEditorUi::setFileName(const QString &newFileName)
{
    m_fileName = newFileName;
}

void TextEditorUi::onSort()
{
    qDebug() << Q_FUNC_INFO;
    ui->editor->sort(sortMode);
}

void TextEditorUi::onSortModeChanged(QAbstractButton *btn)
{
    qDebug() << Q_FUNC_INFO;
    sortMode = btn->text();
}

void TextEditorUi::onEnableButtons(bool enable)
{
    qDebug() << Q_FUNC_INFO;
    ui->btn_previous->setEnabled(enable);
    ui->btn_next->setEnabled(enable);
    ui->btn_replace->setEnabled(enable);
    ui->btn_replace_all->setEnabled(enable);
}

void TextEditorUi::onCursorPositionChanged()
{
    qDebug() << Q_FUNC_INFO;
    ui->btn_replace->setEnabled(ui->editor->selectionIsMatch());
}

void TextEditorUi::onFind()
{
    qDebug() << Q_FUNC_INFO;
    if (!ui->le_find->text().isEmpty()) {
        ui->editor->findMatches(
                    ui->le_find->text(),
                    ui->btn_regexp->isChecked(),
                    ui->btn_case->isChecked());
    }
    else {
        ui->editor->clearMatches();
    }

}

void TextEditorUi::onNext()
{
    qDebug() << Q_FUNC_INFO;
    ui->editor->findNext();
}

void TextEditorUi::onPrev()
{
    qDebug() << Q_FUNC_INFO;
    ui->editor->findPrev();
}

void TextEditorUi::onReplace()
{
    qDebug() << Q_FUNC_INFO;
    ui->editor->replaceMatch(ui->le_replace->text());
}

void TextEditorUi::onReplaceAll()
{
    qDebug() << Q_FUNC_INFO;
    ui->editor->replaceAll(ui->le_replace->text());
}
















