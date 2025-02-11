#ifndef FRONTENDASSISTANT_H
#define FRONTENDASSISTANT_H

#include <QWidget>
#include "../../models/project/project.h"

namespace Ui {
class FrontendAssistant;
}

class FrontendAssistant : public QWidget
{
    Q_OBJECT

public:
    explicit FrontendAssistant(QWidget *parent = nullptr);
    std::string isValid(Project &project);
    ~FrontendAssistant();

private:
    Ui::FrontendAssistant *ui;
    void applyStylesFront();
};

#endif // FRONTENDASSISTANT_H
