#include "frontenddashboard.h"
// #include <QFile>
// #include <QJsonArray>
// #include <QJsonDocument>
// #include <QJsonObject>
#include <QDropEvent>
#include <QMimeData>
#include "ui_frontenddashboard.h"

FrontendDashboard::FrontendDashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FrontendDashboard)
    , createViewDialog(nullptr)
{
    ui->setupUi(this);

    connect(ui->addSectionButton,
            &QPushButton::clicked,
            this,
            &FrontendDashboard::showCreateViewDialog);

    applyStylesFront();
    setUpTreeWidgets();
}

FrontendDashboard::~FrontendDashboard()
{
    delete ui;
}

void FrontendDashboard::applyStylesFront()
{
    this->setStyleSheet("QTreeWidget {"
                        "   border: 1px solid #eaeaea;"
                        "   border-radius: 8px;"
                        "   background-color: white;"
                        "   font-size: 14px;"
                        "   margin: 10px;"
                        "   padding: 5px;"
                        "}"
                        "QTreeWidget::item {"
                        "   border-radius: 5px;"
                        "   margin: 2px;"
                        "   padding: 5px;"
                        "}"
                        "QTreeWidget::item:selected {"
                        "   background-color: #0F66DE;"
                        "   color: white;"
                        "}"
                        "QGroupBox {"
                        "   border: 1px solid #dcdcdc;"
                        "   border-radius: 8px;"
                        "   margin-top: 15px;"
                        "   padding: 10px;"
                        "   font-size: 16px;"
                        "   color: #333;"
                        "}"
                        "QTreeWidget, QTableWidget, QListWidget {"
                        "   border: 1px solid #eaeaea;"
                        "   border-radius: 5px;"
                        "   font-size: 14px;"
                        "   background-color: #fafafa;"
                        "}"
                        "QPushButton {"
                        "   background-color: white;"
                        "   color: white;"
                        "   border: none;"
                        "   border-radius: 12px;"
                        "   padding: 5px 5px;"
                        "   font-size: 12px;"
                        "}"
                        "QPushButton:hover {"
                        "   background-color: #eaeaea;"
                        "   color: white;"
                        "}"
                        "QPushButton:pressed {"
                        "   background-color: #eaeaea;"
                        "}"
                        "QLabel#titleLabel {"
                        "   font-size: 36px;"
                        "   font-weight: bold;"
                        "   color: #333;"
                        "}"
                        "QGroupBox {"
                        "   border: 1px solid #dcdcdc;"
                        "   border-radius: 8px;"
                        "   padding: 10px;"
                        "   font-size: 16px;"
                        "   color: #333;"
                        "}");

    ui->titleLabel->setStyleSheet("font-size: 35px; color: #27292A; padding-top: 10px; "
                                  "padding-left: 40px; padding-bottom: 20px;");
}

void FrontendDashboard::setUpTreeWidgets()
{
    // Components Tree
    QTreeWidget *componentsTree = ui->componentsTree;

    componentsTree->setDragEnabled(true);
    componentsTree->setDropIndicatorShown(true);
    componentsTree->setDragDropMode(QAbstractItemView::InternalMove);

    setComponentsDraggable();

    // Current View Tree
    QTreeWidget *currentViewTree = ui->currentViewTree;

    currentViewTree->setAcceptDrops(true);
}

void FrontendDashboard::setComponentsDraggable()
{
    QTreeWidgetItem *inputsGroup = ui->componentsTree->topLevelItem(0); // Primer grupo, "Inputs"
    QTreeWidgetItem *labelsGroup = ui->componentsTree->topLevelItem(1); // Segundo grupo, "Labels"

    // Deshabilitar arrastre para el grupo "Inputs"
    if (inputsGroup) {
        inputsGroup->setFlags(inputsGroup->flags() & ~Qt::ItemIsDragEnabled);
    }

    // Deshabilitar arrastre para el grupo "Labels"
    if (labelsGroup) {
        labelsGroup->setFlags(labelsGroup->flags() & ~Qt::ItemIsDragEnabled);
    }

    // Habilitar arrastre de los hijos
    for (int i = 0; i < inputsGroup->childCount(); ++i) {
        QTreeWidgetItem *child = inputsGroup->child(i);
        child->setFlags(child->flags() | Qt::ItemIsDragEnabled);
    }

    for (int i = 0; i < labelsGroup->childCount(); ++i) {
        QTreeWidgetItem *child = labelsGroup->child(i);
        child->setFlags(child->flags() | Qt::ItemIsDragEnabled);
    }
}

void FrontendDashboard::populateCurrentViewTree()
{
    // Clear currentViewTree
    ui->currentViewTree->clear();

    // Check currentView components
    if (currentView.getComponents().empty()) {
        qDebug() << "Nothing found!" << "\n";
        return;
    }

    // Loop through the components of the currentView and add them to the QTreeWidget
    for (const auto &component : currentView.getComponents()) {
        QTreeWidgetItem *componentItem = new QTreeWidgetItem(ui->currentViewTree);
        componentItem->setText(0, QString::fromStdString(component.getType()));
    }

    ui->currentViewTree->expandAll();
}

void FrontendDashboard::convertTreeToViews()
{
    // Obtén el nombre de la vista actual
    std::string viewName = currentView.getName();

    // Encuentra la vista correspondiente en el vector de vistas
    auto it = std::find_if(views.begin(), views.end(), [&viewName](const View &v) {
        return v.getName() == viewName;
    });

    if (it != views.end()) {
        // Limpiar los componentes actuales de la vista
        std::vector<Component> newComponents;

        // Iterar sobre los componentes del QTreeWidget
        for (int i = 0; i < ui->currentViewTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem *componentItem = ui->currentViewTree->topLevelItem(i);

            // Crear un nuevo componente basado en el QTreeWidgetItem
            std::string componentType = componentItem->text(0).toStdString();
            // std::map<std::string, std::string> componentProps;

            // Crear un objeto Component con el tipo y las propiedades obtenidas
            Component newComponent(componentType);
            // Component newComponent(componentType, componentProps);

            // Si el componente tiene otros componentes anidados, manejarlos recursivamente
            if (componentItem->childCount() > 0) {
                populateNestedComponents(componentItem, newComponents);
            }

            // Agregar el componente al vector de componentes de la vista
            newComponents.push_back(newComponent);
        }

        // Actualizar los componentes de la vista
        it->setComponents(newComponents);
    }
}

// Función recursiva para manejar componentes anidados
void FrontendDashboard::populateNestedComponents(QTreeWidgetItem *parentItem,
                                                 std::vector<Component> &components)
{
    // Iterar sobre los hijos del QTreeWidgetItem actual
    for (int i = 0; i < parentItem->childCount(); ++i) {
        QTreeWidgetItem *childItem = parentItem->child(i);

        // Obtiene el tipo del componente hijo
        std::string childComponentType = childItem->text(0).toStdString();
        std::map<std::string, std::string> childComponentProps; // Props se mantienen vacíos

        // Crea un nuevo componente hijo
        Component childComponent(childComponentType, childComponentProps);

        // Si el componente hijo tiene otros componentes anidados, llamamos recursivamente
        if (childItem->childCount() > 0) {
            populateNestedComponents(childItem, components);
        }

        // Agregar el componente hijo al vector de componentes de la vista actual
        components.push_back(childComponent);
    }
}

void FrontendDashboard::showCreateViewDialog()
{
    if (!createViewDialog) {
        createViewDialog = new CreateView(this);

        connect(createViewDialog, &CreateView::routeSaved, this, &FrontendDashboard::onRouteSaved);
    }
    createViewDialog->exec();
}

void FrontendDashboard::onRouteSaved(const Route &route)
{
    routes.push_back(route);

    // View view(route.getComponent());
    // views.push_back(view);

    // setRoutes(routes);
}

// Getters
const std::vector<Route> &FrontendDashboard::getRoutes() const
{
    return routes;
}

std::vector<Route> &FrontendDashboard::getRoutes()
{
    return routes;
}

std::vector<View> &FrontendDashboard::getViews()
{
    return views;
}

const std::vector<View> &FrontendDashboard::getViews() const
{
    return views;
}

// Setters
void FrontendDashboard::setRoutes(const std::vector<Route> &routes)
{
    this->routes = routes;
}

void FrontendDashboard::setViews(const std::vector<View> &views)
{
    this->views = views;
}

void FrontendDashboard::setCurrentView(View &view)
{
    currentView = view;

    populateCurrentViewTree();
}
