#ifndef DATABASEASSISTANT_H
#define DATABASEASSISTANT_H

#include <QDialog>
#include "../../models/project/project.h"

namespace Ui {
class DatabaseAssistant;
}

class DatabaseAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseAssistant(QWidget *parent = nullptr);
    std::string isValid(Project &project);
    ~DatabaseAssistant();

private:
    Ui::DatabaseAssistant *ui;
};

#endif // DATABASEASSISTANT_H
