#include "summaryassistant.h"
#include "src/components/summary-assistant/ui_summaryassistant.h"

SummaryAssistant::SummaryAssistant(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SummaryAssistant)
{
    ui->setupUi(this);
}

SummaryAssistant::~SummaryAssistant()
{
    delete ui;
}
