#include "component.h"

Component::Component()
    : BaseNode("Component", std::chrono::system_clock::now(), std::chrono::system_clock::now())
    , type(ComponentType::Undefined)
{
    initializeDefaultProps();
}

Component::Component(boost::uuids::uuid id,
                     std::chrono::system_clock::time_point createdOn,
                     std::chrono::system_clock::time_point updatedOn)
    : BaseNode("Component", id, createdOn, updatedOn)
    , type(ComponentType::Undefined)
{
    initializeDefaultProps();
}

Component::Component(ComponentType type)
    : BaseNode("Component", std::chrono::system_clock::now(), std::chrono::system_clock::now())
    , type(type)
{
    initializeDefaultProps();
    generateUniqueId(componentTypeToString(type), allowItems, props);
}

Component::Component(ComponentType type,
                     boost::uuids::uuid id,
                     std::chrono::system_clock::time_point createdOn,
                     std::chrono::system_clock::time_point updatedOn)
    : BaseNode("Component", id, createdOn, updatedOn)
    , type(type)
{
    initializeDefaultProps();
}

Component::Component(ComponentType type,
                     const std::map<std::string, std::string> &props,
                     bool allowItems,
                     boost::uuids::uuid id,
                     std::chrono::system_clock::time_point createdOn,
                     std::chrono::system_clock::time_point updatedOn)
    : BaseNode("Component", id, createdOn, updatedOn)
    , type(type)
    , props(props)
    , allowItems(allowItems)
{}

Component::Component(ComponentType type,
                     const std::map<std::string, std::string> &props,
                     bool allowItems)
    : BaseNode("Component", std::chrono::system_clock::now(), std::chrono::system_clock::now())
    , type(type)
    , props(props)
    , allowItems(allowItems)
{
    // initializeDefaultProps();
    generateUniqueId(componentTypeToString(type), allowItems, props);
}

// Methods from BaseNode

void Component::generateCode(inja::Environment &env) const
{
    // nlohmann::json data = {{"type", "Component"},
    //                        {"componentType", componentTypeToString(type)},
    //                        {"props", props}};
    // std::string result = env.render("Componente: {{ type }} {{ componentType }}", data);
    // Usar el resultado como necesites
}

void Component::updateFromJson(const nlohmann::json &json)
{
    if (json.contains("type")) {
        setType(stringToComponentType(json["type"]));
    }
    if (json.contains("props")) {
        setProps(json["props"].get<std::map<std::string, std::string>>());
    }
}

std::shared_ptr<BaseNode> Component::clone() const
{
    auto clonedComponent = std::make_shared<Component>(this->type,
                                                       this->props,
                                                       this->allowItems,
                                                       this->id,
                                                       this->createdOn,
                                                       this->updatedOn);

    clonedComponent->getChildren().clear();

    for (const auto &child : this->children) {
        clonedComponent->addChild(child->clone());
    }

    return clonedComponent;
}

bool Component::isDifferentFrom(const std::shared_ptr<BaseNode> &other) const
{
    auto otherComponent = std::dynamic_pointer_cast<Component>(other);

    if (!otherComponent)
        return true;

    // Comparar el tipo del componente
    if (this->getType() != otherComponent->getType())
        return true;

    // Comparar propiedades
    if (this->getProps() != otherComponent->getProps())
        return true;

    // Comparar cantidad de hijos
    // if (this->getChildren().size() != otherComponent->getChildren().size())
    //     return true;

    // Comparar cada hijo
    // for (size_t i = 0; i < this->getChildren().size(); ++i) {
    //     if (this->getChildren()[i]->isDifferentFrom(otherComponent->getChildren()[i]))
    //         return true;
    // }

    return false;
}

void Component::initializeDefaultProps()
{
    auto it = componentPropertiesMap.find(type);
    if (it != componentPropertiesMap.end()) {
        props = it->second;
    }

    switch (type) {
    case ComponentType::Form:
    case ComponentType::HorizontalLayout:
    case ComponentType::VerticalLayout:
    case ComponentType::ModelLayout:
    case ComponentType::Layout:
        this->allowItems = true;
        break;
    default:
        this->allowItems = false;
        break;
    }
}

// Getters

ComponentType Component::getType() const
{
    return type;
}

const std::map<std::string, std::string> &Component::getProps() const
{
    return props;
}

std::map<std::string, std::string> &Component::getProps()
{
    return props;
}

bool Component::isAllowingItems() const
{
    return this->allowItems;
}

// Setters

void Component::setType(ComponentType type)
{
    this->type = type;
    update();

    initializeDefaultProps();
    if (id.is_nil())
        generateUniqueId(componentTypeToString(type), allowItems, props);
}

void Component::setProps(const std::map<std::string, std::string> &props)
{
    this->props = props;
    update();
}

void Component::setAllowItems(bool allow)
{
    this->allowItems = allow;
    update();
}
