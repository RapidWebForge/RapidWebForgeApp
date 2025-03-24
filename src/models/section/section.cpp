#include "section.h"
#include "../component/component.h"
#include <boost/uuid/uuid_io.hpp>

Section::Section()
    : BaseNode("Section", std::chrono::system_clock::now(), std::chrono::system_clock::now())
    , name("")
{
    generateUniqueId();
}

Section::Section(const std::string &name)
    : BaseNode("Section", std::chrono::system_clock::now(), std::chrono::system_clock::now())
    , name(name)
{
    generateUniqueId();
}

Section::Section(const std::string &name, const std::string &path)
    : BaseNode("Section", std::chrono::system_clock::now(), std::chrono::system_clock::now())
    , name(name)
    , path(path)
{
    generateUniqueId();
}

Section::Section(const std::string &name,
                 boost::uuids::uuid id,
                 std::chrono::system_clock::time_point createdOn,
                 std::chrono::system_clock::time_point updatedOn)
    : BaseNode("Section", id, createdOn, updatedOn)
    , name(name)
{}

Section::Section(const std::string &name,
                 const std::string &path,
                 boost::uuids::uuid id,
                 std::chrono::system_clock::time_point createdOn,
                 std::chrono::system_clock::time_point updatedOn)
    : BaseNode("Section", id, createdOn, updatedOn)
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
    std::shared_ptr<Section> clonedSection;

    if (!this->path.empty()) {
        clonedSection = std::make_shared<Section>(this->name,
                                                  this->path,
                                                  this->id,
                                                  this->createdOn,
                                                  this->updatedOn);
    } else {
        clonedSection = std::make_shared<Section>(this->name,
                                                  this->id,
                                                  this->createdOn,
                                                  this->updatedOn);
    }

    clonedSection->getChildren().clear();

    // Clonar recursivamente cada hijo
    for (const auto &child : this->children) {
        clonedSection->addChild(child->clone());
    }

    return clonedSection;
}

bool Section::isDifferentFrom(const std::shared_ptr<BaseNode> &other) const
{
    auto otherSection = std::dynamic_pointer_cast<Section>(other);

    if (!otherSection)
        return true;

    // Comparar nombre
    if (this->getName() != otherSection->getName())
        return true;

    // Comparar path si existe
    if (!this->getPath().empty())
        if (this->getPath() != otherSection->getPath())
            return true;

    // Comparar cantidad de hijos
    if (this->getChildren().size() != otherSection->getChildren().size())
        return true;

    // Comparar cada hijo
    // for (size_t i = 0; i < this->getChildren().size(); ++i) {
    //     if (this->getChildren()[i]->isDifferentFrom(otherSection->getChildren()[i]))
    //         return true;
    // }

    return false;
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
    update();
}

void Section::setPath(const std::string &path)
{
    this->path = path;
    update();
}
