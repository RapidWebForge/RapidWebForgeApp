#ifndef SUMMARYASSISTANT_H
#define SUMMARYASSISTANT_H

#include <QWidget>
#include "../../models/project/project.h"

namespace Ui {
class SummaryAssistant;
}

class SummaryAssistant : public QWidget
{
    Q_OBJECT

public:
    explicit SummaryAssistant(QWidget *parent = nullptr);
    ~SummaryAssistant();

    void setProjectInformation(Project &project);

private:
    Ui::SummaryAssistant *ui;
    void applyStylesSummary();
};

#endif // SUMMARYASSISTANT_H
