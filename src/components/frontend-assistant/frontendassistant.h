#ifndef FRONTENDASSISTANT_H
#define FRONTENDASSISTANT_H

#include <QDialog>

namespace Ui {
class FrontendAssistant;
}

class FrontendAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit FrontendAssistant(QWidget *parent = nullptr);
    ~FrontendAssistant();

private:
    Ui::FrontendAssistant *ui;
};

#endif // FRONTENDASSISTANT_H
