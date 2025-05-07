#ifndef TEMPLATESPANEL_H
#define TEMPLATESPANEL_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include "../../core/configuration-manager/configurationmanager.h"
#include "../stepper/stepper.h"
#include <vector>

namespace Ui {
class TemplatesPanel;
}

class TemplatesPanel : public QWidget
{
    Q_OBJECT

public:
    explicit TemplatesPanel(QWidget *parent = nullptr);
    ~TemplatesPanel();

private:
    Ui::TemplatesPanel *ui;
    QGridLayout *gridLayout;
    ConfigurationManager *confManager = nullptr;

    void setupTemplates();

private slots:
    void onTemplateClicked(const std::string &templateName);
};

#endif // TEMPLATESPANEL_H
