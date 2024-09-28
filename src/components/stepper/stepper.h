#ifndef STEPPER_H
#define STEPPER_H

#include <QWidget>
#include "../creation-assistant/creationassistant.h"
#include "../database-assistant/databaseassistant.h"
#include "../frontend-assistant/frontendassistant.h"
#include "../summary-assistant/summaryassistant.h"

namespace Ui {
class Stepper;
}

class Stepper : public QWidget
{
    Q_OBJECT

public:
    explicit Stepper(QWidget *parent = nullptr);
    ~Stepper();

private slots:
    void on_nextButton_clicked();

    void on_backButton_clicked();

private:
    Ui::Stepper *ui;
    CreationAssistant * creationAssistant;
    DatabaseAssistant * databaseAssistant;
    FrontendAssistant * frontendAssistant;
    SummaryAssistant * summaryAssistant;
};

#endif // STEPPER_H
