#ifndef CREATIONASSISTANT_H
#define CREATIONASSISTANT_H

#include <QDialog>
#include "../../models/project/project.h"

namespace Ui {
class CreationAssistant;
}

class CreationAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit CreationAssistant(QWidget *parent = nullptr);
    std::string isValid(Project &project);
    ~CreationAssistant();

private slots:
    void on_browseButton_clicked();

private:
    Ui::CreationAssistant *ui;
    void applyStylesCA(); // Declaración de la función applyStyles
    bool shouldCreateGitRepo(); // Método para verificar si se debe crear un repositorio Git
};

#endif // CREATIONASSISTANT_H
