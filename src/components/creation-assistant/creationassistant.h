#ifndef CREATIONASSISTANT_H
#define CREATIONASSISTANT_H

#include <QDialog>
#include "../database-assistant/databaseassistant.h"  // Incluir la clase de la segunda pantalla

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
    DatabaseAssistant *dbAssistant;  // Puntero a la segunda pantalla
};

#endif // CREATIONASSISTANT_H
