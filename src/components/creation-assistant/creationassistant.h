#ifndef CREATIONASSISTANT_H
#define CREATIONASSISTANT_H

#include <QDialog>

namespace Ui {
class CreationAssistant;
}

class CreationAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit CreationAssistant(QWidget *parent = nullptr);
    ~CreationAssistant();

private slots:
    void handleNextButton();
    void on_browseButton_clicked();

private:
    Ui::CreationAssistant *ui;
};

#endif // CREATIONASSISTANT_H
