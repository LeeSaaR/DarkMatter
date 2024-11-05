#include "texteditor.h"

#include <QPainter>
#include <QTextBlock>
#include <QRegularExpression>

TextEditor::TextEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &TextEditor::blockCountChanged, this, &TextEditor::updateLineNumberAreaWidth);
    connect(this, &TextEditor::updateRequest, this, &TextEditor::updateLineNumberArea);

    updateLineNumberAreaWidth(0);

    formatMatch.setBackground(QColor(115, 51, 42, 255));
    formatReset.setBackground(QColor(0,0,0,0));
}

int TextEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void TextEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    int s_width = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * 2;
    setViewportMargins(lineNumberAreaWidth() + s_width, 0, 0, 0);
}

void TextEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void TextEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    int s_width = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * 2;
    lineNumberArea->setGeometry(QRect(
                                    cr.left(),
                                    cr.top(),
                                    lineNumberAreaWidth() + s_width,
                                    cr.height()));
}

void TextEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.setFont(QFont("Liberation Mono", 12));
    painter.fillRect(event->rect(), QColor(38, 38, 38, 255));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    int s_width = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * 1;
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QPen(QColor(128, 128, 128, 255)));
            painter.drawText(
                        0,
                        top,
                        lineNumberArea->width() - s_width,
                        fontMetrics().height(),
                        Qt::AlignRight,
                        number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void TextEditor::sort(const QString &sortMode)
{
    qDebug() << Q_FUNC_INFO;
    blockSignals(true);
    if (!textCursor().hasSelection()) {
        sortAll(sortMode);
    }
    else {
        sortSelection(sortMode);
    }
    blockSignals(false);
}

void TextEditor::sortAll(const QString &sortMode)
{
    qDebug() << Q_FUNC_INFO;
    QList<QString> blockList = allBlocks();
    if (blockList.isEmpty()) return;

    if (sortMode == "normal") blockList = sortNormal(blockList);
    if (sortMode == "reverse")  blockList = sortReverse(blockList);
    if (sortMode == "invert") blockList = sortInvert(blockList);

    clear();
    insertPlainText(blockList.join("\n"));
}

void TextEditor::sortSelection(const QString &sortMode)
{
    qDebug() << Q_FUNC_INFO;
    QTextCursor cursor = textCursor();
    int start = cursor.selectionStart();
    int end   = cursor.selectionEnd();

    cursor.setPosition(start);
    int blockStartIndex = cursor.blockNumber();
    cursor.setPosition(end);
    int blockEndIndex   = cursor.blockNumber();
    cursor.setPosition(start);

    QList<QString> blockList;
    int selectedBlockCount = blockEndIndex - blockStartIndex + 1;
    for (int i = 0; i < selectedBlockCount; i++) {
        if (!cursor.block().text().isEmpty()) {
            blockList.append(cursor.block().text());
        }
        cursor.movePosition(QTextCursor::NextBlock);
    }
    if (blockList.isEmpty()) return;

    if (sortMode == "normal") blockList = sortNormal(blockList);
    if (sortMode == "reverse")  blockList = sortReverse(blockList);
    if (sortMode == "invert") blockList = sortInvert(blockList);

    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(blockList.join("\n"));
    setTextCursor(cursor);
}

const QList<QString> TextEditor::allBlocks()
{
    qDebug() << Q_FUNC_INFO;
    QList<QString> blockList;
    for (QTextBlock _block = document()->begin(); _block != document()->end(); _block = _block.next()) {
        if (!_block.text().isEmpty()) {
            blockList.append(_block.text());
        }
    }
    return blockList;
}

const QList<QString> TextEditor::sortNormal(QList<QString> blockList)
{
    qDebug() << Q_FUNC_INFO;
    std::sort(blockList.begin(), blockList.end());
    return blockList;
}

const QList<QString> TextEditor::sortReverse(QList<QString> blockList)
{
    qDebug() << Q_FUNC_INFO;
    std::sort(blockList.rbegin(), blockList.rend());
    return blockList;
}

const QList<QString> TextEditor::sortInvert(QList<QString> blockList)
{
    qDebug() << Q_FUNC_INFO;
    std::reverse(blockList.begin(), blockList.end());
    return blockList;
}

void TextEditor::findMatches(QString _pattern, bool regexp, bool caseSensitive)
{
    qDebug() << Q_FUNC_INFO;
    clearMatches();
    const QString preparedPattern = preparePattern(_pattern, regexp);
    if (preparedPattern.isEmpty()) return;
    QRegularExpression pattern(preparedPattern);

    blockSignals(true);
    if (caseSensitive) {
        while (find(pattern, QTextDocument::FindCaseSensitively)) {
            if (textCursor().selectedText().isEmpty()) break;
            textCursor().setCharFormat(formatMatch);
            matches.append(Match(
                               textCursor().selectionStart(),
                               textCursor().selectionEnd(),
                               textCursor().selectedText().length()));
        }
    }
    else {
        while (find(pattern)) {
            if (textCursor().selectedText().isEmpty()) break;
            textCursor().setCharFormat(formatMatch);
            matches.append(Match(
                               textCursor().selectionStart(),
                               textCursor().selectionEnd(),
                               textCursor().selectedText().length()));
        }
    }
    blockSignals(false);
    jumpToMatch(0);
    setHasMatches();
}

void TextEditor::jumpToMatch(int i)
{
    qDebug() << Q_FUNC_INFO;
    if (matches.isEmpty()) return;
    if (i == matches.size()) i = 0;
    if (i == -1) i = matches.size()-1;
    QTextCursor cursor = textCursor();
    cursor.setPosition(matches[i].start);
    cursor.setPosition(matches[i].end, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    currentMatchIndex = i;
}

void TextEditor::findNext()
{
    qDebug() << Q_FUNC_INFO;
    if (currentMatchIndex != -1) {
        jumpToMatch(currentMatchIndex+1);
    }
    else {
        jumpToMatch(nextPossibleMatchIndex);
    }

}

void TextEditor::findPrev()
{
    qDebug() << Q_FUNC_INFO;
    if (currentMatchIndex != -1) {
        jumpToMatch(currentMatchIndex-1);
    }
    else {
        jumpToMatch(prevPossibleMatchIndex);
    }
}

int TextEditor::findNextMatchIndex()
{
    qDebug() << Q_FUNC_INFO;
    if (textCursor().position() > matches.last().start) {
        return 0;
    }
    for (int i = 0; i < matches.size(); i++) {
        if (textCursor().position() <= matches[i].start) {
            return i;
        }
    }
    return -1;
}

int TextEditor::findPrevMatchIndex()
{
    qDebug() << Q_FUNC_INFO;
    if (textCursor().position() < matches.first().end) {
        return matches.size()-1;
    }
    for (int i = matches.size(); i != 0; i--) {
        if (textCursor().position() >= matches[i-1].end) {
            return i-1;
        }
    }
    return -1;
}

void TextEditor::replaceMatch(QString replacement)
{
    qDebug() << Q_FUNC_INFO;
    if (replacement.contains("[M]")) replacement.replace("[M]", textCursor().selectedText());
    replacement.replace("\\n", "\n");
    replacement.replace("\\t", "\t");

    int offset = replacement.size() - matches[currentMatchIndex].length;
    for (int i = currentMatchIndex+1; i < matches.size(); i++) matches[i].offset(offset);

    textCursor().setCharFormat(formatReset);
    blockSignals(true);
    textCursor().insertText(replacement);
    blockSignals(false);
    matches.removeAt(currentMatchIndex);
    jumpToMatch(currentMatchIndex);
    setHasMatches();
}

void TextEditor::replaceAll(QString replacement)
{
    qDebug() << Q_FUNC_INFO;
    while (hasMatches) {
        replaceMatch(replacement);
    }
}

const QString TextEditor::preparePattern(QString _pattern, bool regexp)
{
    qDebug() << Q_FUNC_INFO;
    if (regexp) {
        QRegularExpression pattern(_pattern);
        if (pattern.isValid()) return _pattern;
        return QString();
    }
    else {
        return convertPattern(_pattern);
    }
    return QString();
}

const QString TextEditor::convertPattern(QString _pattern)
{
    qDebug() << Q_FUNC_INFO;
    _pattern.replace("(", "\\(");
    _pattern.replace(")", "\\)");
    _pattern.replace("}", "\\{");
    _pattern.replace("}", "\\}");
    _pattern.replace("[", "\\[");
    _pattern.replace("]", "\\]");
    _pattern.replace(".", "\\.");
    _pattern.replace("-", "\\-");
    _pattern.replace("\\", "\\\\");
    _pattern.replace("+", "\\+");
    _pattern.replace("|", "\\|");
    _pattern.replace(":", "\\:");
    _pattern.replace("?", "\\?");
    _pattern.replace("*", "\\*");
    _pattern.replace("^", "\\^");
    _pattern.replace("$", "\\$");
    _pattern.replace("&", "\\&");
    _pattern.replace("=", "\\=");
    _pattern.replace("\\\"", "\\\\\"");
    return _pattern;
}

void TextEditor::clearMatches()
{
    qDebug() << Q_FUNC_INFO;
    blockSignals(true);
    matches.clear();
    selectAll();
    textCursor().setCharFormat(formatReset);
    cursorToStart();
    setHasMatches();
    blockSignals(false);
}

void TextEditor::cursorToStart()
{
    qDebug() << Q_FUNC_INFO;
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    setTextCursor(cursor);
}

void TextEditor::setHasMatches()
{
    qDebug() << Q_FUNC_INFO;
    hasMatches = (matches.isEmpty()) ? false : true;
    if (!hasMatches) {
        currentMatchIndex      = -1;
        nextPossibleMatchIndex = -1;
        prevPossibleMatchIndex = -1;
    }
    emit enableButtons(hasMatches);
}

bool TextEditor::selectionIsMatch()
{
    qDebug() << Q_FUNC_INFO;
    if (matches.isEmpty()) return false;
    if (textCursor().hasSelection()) {
        int start = textCursor().selectionStart();
        int end   = textCursor().selectionEnd();
        for (int i = 0; i < matches.size(); i++) {
            if ((start == matches[i].start) && (end == matches[i].end)) {
                currentMatchIndex = i;
                nextPossibleMatchIndex = -1;
                prevPossibleMatchIndex = -1;
                return true;
            }
        }
    }
    currentMatchIndex = -1;
    nextPossibleMatchIndex = findNextMatchIndex();
    prevPossibleMatchIndex = findPrevMatchIndex();
    return false;
}












