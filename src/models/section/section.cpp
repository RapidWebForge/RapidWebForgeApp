#include "section.h"
#include "../component/component.h"

Section::Section()
    : BaseNode("Section")
    , name("")
{}

Section::Section(const std::string &name)
    : BaseNode("Section")
    , name(name)
{}

Section::Section(const std::string &name, const std::string &path)
    : BaseNode("Section")
    , name(name)
    , path(path)
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

std::shared_ptr<BaseNode> Section::clone() const
{
    // Crear una nueva instancia de Section con el mismo nombre
    auto clonedSection = std::make_shared<Section>(this->name);

    if (!this->path.empty())
        clonedSection->setPath(this->path);

    // Clonar recursivamente los hijos
    for (const auto &child : this->children) {
        clonedSection->addChild(child->clone());
    }

    return clonedSection;
}

// Getters

std::string Section::getName() const
{
    return name;
}

std::string Section::getPath() const
{
    return path;
}

// Setters

void Section::setName(const std::string &name)
{
    this->name = name;
}

void Section::setPath(const std::string &path)
{
    this->path = path;
}
