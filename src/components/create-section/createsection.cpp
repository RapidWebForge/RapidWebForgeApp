#include "createsection.h"
#include <QFile>
#include <QMessageBox>
#include "../../core/logging/actionloggerjson.h"
#include "../../models/component-type/componenttype.h"
#include "ui_createsection.h"
#include <boost/algorithm/string.hpp>

CreateSection::CreateSection(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateSection)
{
    ui->setupUi(this);
    applyStyles();
}

CreateSection::~CreateSection()
{
    delete ui;
}

void CreateSection::applyStyles()
{
    QFile primaryButtonStyleFile(":/styles/primarybutton");
    if (primaryButtonStyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(primaryButtonStyleFile.readAll());
        ui->createButton->setStyleSheet(styleSheet);
    }

    ui->cancelButton->setStyleSheet("QPushButton {"
                                    "   color: black;"
                                    "   border-radius: 5px;"
                                    "   padding: 4px 30px;"
                                    "   font-size: 14px;"
                                    "}"
                                    "QPushButton:hover {"
                                    "   background-color: #eaeaea;"
                                    "}"
                                    "QPushButton:pressed {"
                                    "   background-color: #004494;"
                                    "}");
}

void CreateSection::on_createButton_clicked()
{
    std::string sectionName = ui->sectionNameLineEdit->text().toStdString();

    bool isView = ui->isViewCheckBox->isChecked();

    if (isView) {
        std::string path = ui->viewRouteLineEdit->text().toStdString();

        if (!sectionName.empty() && !path.empty() && isView) {
            const std::shared_ptr<Section> view = std::make_shared<Section>(sectionName,
                                                                            boost::to_lower_copy(
                                                                                path));

            emit onSectionSaved(view);

            // Log para crear una nueva vista React
            loggerJson.logAction("create-react-view",
                                 "sectionName=" + sectionName + ", path=" + path);

            resetFields();
            accept();
        } else
            QMessageBox::warning(this, "Warning", "Fill all the fields to create");
    } else {
        if (!sectionName.empty() && !isView) {
            const std::shared_ptr<Section> customComponent = std::make_shared<Section>(sectionName);

            emit onSectionSaved(customComponent);

            // Log para crear un nuevo componente React
            loggerJson.logAction("create-react-component", "sectionName=" + sectionName);

            resetFields();
            accept();
        } else
            QMessageBox::warning(this, "Warning", "Fill all the fields to create");
    }
}

void CreateSection::on_cancelButton_clicked()
{
    accept();
}

void CreateSection::on_isViewCheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    bool isView = ui->isViewCheckBox->isChecked();

    ui->viewRouteLabel->setEnabled(isView);
    ui->viewRouteLineEdit->setEnabled(isView);
}

void CreateSection::resetFields()
{
    ui->isViewCheckBox->setChecked(false);
    ui->viewRouteLineEdit->setText("");
    ui->sectionNameLineEdit->setText("");
}
