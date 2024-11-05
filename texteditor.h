#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QPlainTextEdit>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE

class LineNumberArea;

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEditor(QWidget *parent = nullptr);

    // LINE NUMBER AREA
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    // SORT
    void sort(const QString &sortMode);
    void sortAll(const QString &sortMode);
    void sortSelection(const QString &sortMode);
    const QList<QString> allBlocks();
    const QList<QString> sortNormal(QList<QString> blockList);
    const QList<QString> sortReverse(QList<QString> blockList);
    const QList<QString> sortInvert(QList<QString> blockList);

    // FIND
    void findMatches(QString _pattern, bool regexp, bool caseSensitive);
    void findNext();
    void findPrev();
    int findNextMatchIndex();
    int findPrevMatchIndex();

    // REPLACE
    void replaceMatch(QString replacement);
    void replaceAll(QString replacement);

    // HELPER
    const QString preparePattern(QString _pattern, bool regexp);
    const QString convertPattern(QString _pattern);
    void clearMatches();
    void cursorToStart();
    void jumpToMatch(int i);
    void setHasMatches();
    bool selectionIsMatch();

signals:
    void enableButtons(bool);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *lineNumberArea;
    QTextCharFormat formatMatch;
    QTextCharFormat formatReset;

    struct Match
    {
        int start  = -1;
        int end    = -1;
        int length = -1;

        Match() {}
        Match(int s, int e, int l) : start(s), end(e), length(l) {}
        void offset(int o) {start+=o; end+=o;}
    };

    QList<Match> matches;
    int currentMatchIndex = -1;
    int nextPossibleMatchIndex = -1;
    int prevPossibleMatchIndex = -1;
    bool hasMatches = false;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEditor *editor) : QWidget(editor), textEditor(editor)
    {}

    QSize sizeHint() const override
    {
        return QSize(textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        textEditor->lineNumberAreaPaintEvent(event);
    }

private:
    TextEditor *textEditor;
};

#endif // TEXTEDITOR_H
