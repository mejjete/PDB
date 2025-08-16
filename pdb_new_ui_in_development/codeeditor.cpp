#include "codeeditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QFontDatabase>
#include <QMouseEvent>

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor), codeEditor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    codeEditor->lineNumberAreaPaintEvent(event);
}

void LineNumberArea::mousePressEvent(QMouseEvent* event) {
    // Toggle breakpoint: define line by vertical coordinate Y and ignore X.
    codeEditor->toggleBreakpointAtGutterY(static_cast<int>(event->position().y()));
    update(); // Redraw gutter.
}

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(true);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    lineNumberArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
}

int CodeEditor::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    const int space = 12 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space + 18; // Place for breakpoint and arrow.
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(30, 30, 30));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    const int w = lineNumberArea->width();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const QRect rowRect(0, top, w - 18, bottom - top);
            const int midY = (top + bottom) / 2;

            // Line number - center by block height.
            painter.setPen(Qt::darkGray);
            painter.drawText(rowRect, Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(blockNumber + 1));

            // Breakpoint, center at midY.
            if (breakpoints.contains(blockNumber)) {
                painter.setRenderHint(QPainter::Antialiasing);
                painter.setBrush(QColor(220, 50, 47));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPoint(9, midY), 6, 6);
            }

            // Arrow of the current line - also by midY
            if (blockNumber == execLine) {
                const QPoint pts[3] = {
                    QPoint(2, midY - 6),
                    QPoint(2, midY + 6),
                    QPoint(16,  midY)
                };
                painter.setBrush(QColor(38, 139, 210));
                painter.setPen(Qt::NoPen);
                painter.drawPolygon(pts, 3);
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::updateLineNumberAreaWidth()
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    const QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::loadSample(const QString &text)
{
    setPlainText(text);
    moveCursor(QTextCursor::Start);
    execLine = -1;
    breakpoints.clear();
    lineNumberArea->update();
}

void CodeEditor::startDebug()
{
    // Do not start "debugging" if there are no breakpoints at all.
    if (breakpoints.isEmpty())
        return;

    if (execLine < 0) {
        // First start: go to the highest breakpoint.
        int first = -1;
        for (int ln : std::as_const(breakpoints)) {
            if (first < 0 || ln < first)
                first = ln;
        }
        if (first < 0)
            return; // Just in case.
        execLine = qBound(0, first, blockCount() - 1);
    } else {
        // Continue: Move to the next breakpoint below the current line.
        int nextStop = -1;
        for (int ln : std::as_const(breakpoints)) {
            if (ln > execLine && (nextStop < 0 || ln < nextStop))
                nextStop = ln;
        }
        if (nextStop < 0)
            return; // there are no further breakpoints - we stay in place.
        execLine = nextStop;
    }

    ensureLineVisible(execLine);
    lineNumberArea->update();
    update();
}

void CodeEditor::step()
{
    if (execLine < 0)
        return; // Do nothing if "Start Debugging" was not pressed.
    else
        execLine = qMin(execLine + 1, blockCount() - 1);

    ensureLineVisible(execLine);
    lineNumberArea->update();
    update(); // Ensure that the arrow is redrawn.
}

void CodeEditor::stopDebug()
{
    if (execLine != -1) {
        execLine = -1;                 // Arrow is hidden.
        lineNumberArea->update();      // Redraw gutter.
        update();                      // Redraw viewport.
    }
}

void CodeEditor::ensureLineVisible(int line)
{
    const QTextBlock block = document()->findBlockByNumber(line);
    if (!block.isValid())
        return;
    QTextCursor c(block);
    setTextCursor(c);
    centerCursor();
}

void CodeEditor::toggleBreakpointAtGutterY(int localY)
{
    // Calculate the block (row) under the Y-coordinate in the gutter.
    QTextBlock block = firstVisibleBlock();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && block.isVisible()) {
        if (localY >= top && localY < bottom) {
            const int ln = block.blockNumber();
            if (breakpoints.contains(ln))
                breakpoints.remove(ln);
            else
                breakpoints.insert(ln);
            break;
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
    }
}
