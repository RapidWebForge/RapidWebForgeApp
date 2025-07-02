#include "templatespanel.h"
#include <QMessageBox>
#include "ui_templatespanel.h"

TemplatesPanel::TemplatesPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TemplatesPanel)
{
    ui->setupUi(this);
    setupTemplates();
}

void TemplatesPanel::setupTemplates()
{
    QGridLayout *gridLayout = ui->gridLayout;
    gridLayout->setSpacing(10);

    // Limpiar el layout eliminando los widgets de manera segura
    QLayoutItem *child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater(); // Elimina los widgets de forma asíncrona
        }
        delete child; // Borra el item del layout
    }

    // Add projects to layout
    int row = 0;
    int column = 1;
    int maxColumns = 3;

    const std::vector<std::string> templates = {"crud"};

    for (const auto &temp : templates) {
        // Crear el contenedor para el botón y el texto
        QWidget *templateWidget = new QWidget(this);
        QVBoxLayout *templateLayout = new QVBoxLayout(templateWidget);

        // Ajustar márgenes y espaciado para que el texto esté más cerca del botón
        templateLayout->setAlignment(Qt::AlignTop);
        templateLayout->setSpacing(5); // Ajusta el espaciado entre el botón y el texto
        templateLayout->setContentsMargins(0, 0, 0, 0);     // Quita márgenes adicionales
        templateLayout->setContentsMargins(10, 10, 10, 10); // Márgenes del contenedor
        templateWidget->setFixedSize(210, 180);             // Tamaño fijo para el contenedor

        // Agregar el contenedor al layout principal
        gridLayout->addWidget(templateWidget, 0, 0);

        // Widget for each project
        QPushButton *templateButton = new QPushButton(this);
        templateButton->setText(QString::fromStdString(temp));
        templateButton->setFixedSize(202, 118);
        // Tamaño del botón, puedes ajustarlo según el diseño
        templateButton->setStyleSheet("QPushButton {"
                                      "   font-size: 48px;"
                                      "   color: #555;"
                                      "   background-color: #f0f0f0;"
                                      "   border-radius: 10px;"
                                      "}"
                                      "QPushButton:hover {"
                                      "   background-color: #e0e0e0;"
                                      "}");
        connect(templateButton, &QPushButton::clicked, this, [this, temp]() {
            onTemplateClicked(temp);
        });

        gridLayout->addWidget(templateWidget, row, column);

        column++;
        if (column >= maxColumns) {
            column = 0;
            row++;
        }
    }
    // Ajuste del tamaño mínimo para que el scroll funcione correctamente
    ui->scrollAreaWidgetContents->adjustSize(); // Ajusta el tamaño del contenedor según el contenido

    ui->scrollAreaWidgetContents->setMinimumHeight(gridLayout->sizeHint().height());
    // Optionally set spacing and margins for better visual appearance
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(10, 10, 10, 10);
    gridLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
}

void TemplatesPanel::onTemplateClicked(const std::string &templateName)
{
    bool pathStatus = confManager->getConfiguration().getStatus();

    if (!pathStatus) {
        QMessageBox::critical(this, "Warning", "You need to set the tech paths in 'Configuration'");
        return;
    }

    QWidget *projectsPanel = this;
    while (projectsPanel->parentWidget() != nullptr) {
        projectsPanel = projectsPanel->parentWidget();
    }
    projectsPanel->hide();

    // When the tempalte button is clicked, open the Stepper window
    Stepper *stepper = new Stepper(nullptr, templateName);
    stepper->show();

    // Show when create assistant is closed
    connect(stepper, &Stepper::destroyed, projectsPanel, &QWidget::show);

    connect(stepper, &Stepper::backToProjectsPanel, this, [this, stepper, projectsPanel]() {
        stepper->close();
        projectsPanel->show();
    });
}

TemplatesPanel::~TemplatesPanel()
{
    delete ui;
}
