#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QSet>

class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent* event);

    QWidget* lineNumberArea;
    QSet<int> breakpoints; // 0-based
    int execLine = -1;     // -1 = not running

signals:
    void debugStateChanged(bool running); // True at the start of a "session", false when stopped
    void breakpointsChanged(int count);   // Whenever a set of breakpoints changes

public slots:
    void loadSample(const QString &text);
    void startDebug();
    void step();
    void stopDebug();

protected:
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void updateLineNumberAreaWidth();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    void ensureLineVisible(int line);

    // Auxiliary: toggle breakpoint by local Y-coordinate in hutter
    void toggleBreakpointAtGutterY(int localY);

    friend class LineNumberArea;
};

class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override; // Click on the line number.

private:
    CodeEditor* codeEditor;
};

#endif // CODEEDITOR_H
