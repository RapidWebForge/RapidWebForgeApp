#ifndef FRONTENDASSISTANT_H
#define FRONTENDASSISTANT_H

#include <QDialog>
#include "../../models/project/project.h"

namespace Ui {
class FrontendAssistant;
}

class FrontendAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit FrontendAssistant(QWidget *parent = nullptr);
    std::string isValid(Project &project);
    ~FrontendAssistant();

private:
    Ui::FrontendAssistant *ui;
};

#endif // FRONTENDASSISTANT_H
