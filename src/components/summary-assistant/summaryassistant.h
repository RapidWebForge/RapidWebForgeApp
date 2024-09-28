#ifndef SUMMARYASSISTANT_H
#define SUMMARYASSISTANT_H


#include <QDialog>

namespace Ui {
class SummaryAssistant;
}

class SummaryAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit SummaryAssistant(QWidget *parent = nullptr);
    ~SummaryAssistant();

    //void setProjectSummary(const QString &name, const QString &frontend, const QString &backend, const QString &database);

private:
    Ui::SummaryAssistant *ui;
};

#endif // SUMMARYASSISTANT_H
