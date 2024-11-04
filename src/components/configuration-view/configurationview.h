#ifndef CONFIGURATIONVIEW_H
#define CONFIGURATIONVIEW_H

#include <QDialog>
#include "../../core/configuration-manager/configurationmanager.h"

namespace Ui {
class ConfigurationView;
}

class ConfigurationView : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigurationView(QWidget *parent = nullptr);
    ~ConfigurationView();

private slots:
    void on_ngInxPathButton_clicked();

    void on_saveButton_clicked();

private:
    Ui::ConfigurationView *ui;
    ConfigurationManager configManager;
};

#endif // CONFIGURATIONVIEW_H