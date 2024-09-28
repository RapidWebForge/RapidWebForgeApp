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
    ~BackendAssistant();

private:
    Ui::BackendAssistant *ui;
};

#endif // BACKENDASSISTANT_H
