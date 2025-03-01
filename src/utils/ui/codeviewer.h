#ifndef CODEVIEWER_H
#define CODEVIEWER_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QString>

class CodeViewer : public QDialog {
    Q_OBJECT

public:
    explicit CodeViewer(QWidget *parent = nullptr);
    void loadFile(const QString &filePath);

private:
    QPlainTextEdit *editor;
};

#endif // CODEVIEWER_H
