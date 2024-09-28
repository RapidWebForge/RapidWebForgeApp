#ifndef DATABASEASSISTANT_H
#define DATABASEASSISTANT_H

#include <QDialog>

namespace Ui {
class DatabaseAssistant;
}

class DatabaseAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseAssistant(QWidget *parent = nullptr);
    ~DatabaseAssistant();

private:
    Ui::DatabaseAssistant *ui;
};

#endif // DATABASEASSISTANT_H
