#ifndef BACKENDASSISTANT_H
#define BACKENDASSISTANT_H

#include <QDialog>

namespace Ui {
class BackendAssistant;
}

class BackendAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit BackendAssistant(QWidget *parent = nullptr);
    std::string isValid();
    ~BackendAssistant();

private:
    Ui::BackendAssistant *ui;
    void applyStylesBack();
};

#endif // BACKENDASSISTANT_H
