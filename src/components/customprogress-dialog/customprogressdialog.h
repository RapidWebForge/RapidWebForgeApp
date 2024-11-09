#ifndef CUSTOMPROGRESSDIALOG_H
#define CUSTOMPROGRESSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

class CustomProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CustomProgressDialog(QWidget *parent = nullptr);

private:
    QLabel *label;
    QProgressBar *progressBar;
};

#endif // CUSTOMPROGRESSDIALOG_H
