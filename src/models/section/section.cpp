#include "section.h"
#include "../component/component.h"

Section::Section()
    : name("")
    , components()
{}

Section::Section(const std::string &name)
    : name(name)
    , components()
{}

Section::Section(const std::string &name, const std::vector<std::shared_ptr<BaseNode>> &components)
    : name(name)
    , components(components)
{}

void Section::generateCode(inja::Environment &env) const
{
    // nlohmann::json data = {{"type", "Component"},
    //                        {"componentType", componentTypeToString(type)},
    //                        {"props", props}};
    // std::string result = env.render("Componente: {{ type }} {{ componentType }}", data);
    // Usar el resultado como necesites
}

void Section::updateFromJson(const nlohmann::json &json)
{
    if (json.contains("name")) {
        setName(json["name"]);
    }
}

// Getters

std::string Section::getName() const
{
    return name;
}

const std::vector<std::shared_ptr<BaseNode>> &Section::getComponents() const
{
    return components;
}

// Setters

void Section::setName(const std::string &name)
{
    this->name = name;
}

void Section::setComponents(const std::vector<std::shared_ptr<BaseNode>> &components)
{
    this->components = components;
}

void Section::addComponent(const std::shared_ptr<BaseNode> &component)
{
    components.push_back(component);
}

void Section::insertComponent(int index, const std::shared_ptr<BaseNode> &component)
{
    components.insert(components.begin() + index, component);
}

bool Section::removeComponentByName(const std::string &name)
{
    auto it = std::find_if(components.begin(),
                           components.end(),
                           [&name](const std::shared_ptr<BaseNode> &node) {
                               auto component = std::dynamic_pointer_cast<Component>(node);
                               return component
                                      && componentTypeToString(component->getType()) == name;
                           });

    if (it != components.end()) {
        components.erase(it);
        return true;
    }
    return false;
}

void Section::clearComponents()
{
    components.clear();
}
