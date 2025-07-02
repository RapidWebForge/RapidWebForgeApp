#ifndef DATABASEASSISTANT_H
#define DATABASEASSISTANT_H

#include <QWidget>
#include "../../models/project/project.h"

namespace Ui {
class DatabaseAssistant;
}

class DatabaseAssistant : public QWidget
{
    Q_OBJECT

public:
    explicit DatabaseAssistant(QWidget *parent = nullptr);
    std::string isValid(Project &project);
    ~DatabaseAssistant();

private slots:
    void on_testConnectionButton_clicked();

private:
    Ui::DatabaseAssistant *ui;
    void applyStylesDatabase();
    bool testCompleted;
};

#endif // DATABASEASSISTANT_H
