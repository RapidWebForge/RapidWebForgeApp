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
    std::string isValid();
    ~CreationAssistant();

private slots:
    void on_browseButton_clicked();

private:
    Ui::CreationAssistant *ui;
};

#endif // CREATIONASSISTANT_H
