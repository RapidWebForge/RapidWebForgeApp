#ifndef SUMMARYASSISTANT_H
#define SUMMARYASSISTANT_H

#include <QDialog>
#include "../../models/project/project.h"

namespace Ui {
class SummaryAssistant;
}

class SummaryAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit SummaryAssistant(QWidget *parent = nullptr);
    ~SummaryAssistant();

    void setProjectInformation(Project &project);

private:
    Ui::SummaryAssistant *ui;
};

#endif // SUMMARYASSISTANT_H
