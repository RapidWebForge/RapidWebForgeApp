#include "CodeViewer.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>

CodeViewer::CodeViewer(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Último Cambio en el Código");
    editor = new QPlainTextEdit(this);
    editor->setReadOnly(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(editor);
    setLayout(layout);
    resize(600, 400);
}

void CodeViewer::loadFile(const QString &filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        editor->setPlainText(in.readAll());
    }
}
