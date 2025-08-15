#include "codeeditor.h"
#include <QApplication>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  auto *editor = new CodeEditor;

  QStringList lines;
  lines << "#include <iostream>"
        << "using namespace std;"
        << ""
        << "int square(int x) { return x * x; }"
        << ""
        << "int main() {"
        << "    cout << \"Demo start\" << endl;";
  for (int i = 0; i < 150; ++i) {
    lines
        << QString(
               "    cout << \"line %1 => square(%1)=\" << square(%1) << endl;")
               .arg(i);
  }
  lines << "    cout << \"Demo end\" << endl;"
        << "    return 0;"
        << "}";
  editor->loadSample(lines.join('\n'));

  auto *startBtn = new QPushButton("Start Debugging");
  auto *stepBtn = new QPushButton("Step Over");
  auto *stopBtn = new QPushButton("Stop Debugging");

  QObject::connect(startBtn, &QPushButton::clicked, editor,
                   &CodeEditor::startDebug);
  QObject::connect(stepBtn, &QPushButton::clicked, editor, &CodeEditor::step);
  QObject::connect(stopBtn, &QPushButton::clicked, editor,
                   &CodeEditor::stopDebug);

  QWidget window;
  auto *layout = new QVBoxLayout(&window);
  layout->addWidget(editor, 1);

  auto *btns = new QHBoxLayout;
  btns->addStretch();
  btns->addWidget(startBtn);
  btns->addWidget(stepBtn);
  btns->addWidget(stopBtn);
  layout->addLayout(btns);

  window.resize(720, 480);
  window.setWindowTitle("PDB UI (beta)");
  window.show();

  return app.exec();
}
