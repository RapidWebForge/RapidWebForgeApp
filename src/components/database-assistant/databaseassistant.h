#ifndef DATABASEASSISTANT_H
#define DATABASEASSISTANT_H

#include <QDialog>
#include "../frontend-assistant/frontendassistant.h"  // Incluir la clase de la tercera pantalla

namespace Ui {
class DatabaseAssistant;
}

class DatabaseAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseAssistant(QWidget *parent = nullptr);
    ~DatabaseAssistant();

private slots:
    void on_nextButton_clicked();   // Slot para el botón Next

private:
    Ui::DatabaseAssistant *ui;
    FrontendAssistant *frontendAssistant;  // Puntero a la tercera pantalla
};

#endif // DATABASEASSISTANT_H
