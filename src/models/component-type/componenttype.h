#ifndef COMPONENTTYPE_H
#define COMPONENTTYPE_H

#include <map>
#include <string>
#include <unordered_map>

enum class ComponentType {
    Undefined,
    HeaderH1,
    HeaderH2,
    HeaderH3,
    Paragraph,
    Input,
    TextArea,
    Button,
    Hyperlink,
    Form,
    HorizontalLayout,
    VerticalLayout,
    ModelLayout,
};

// Mapa de propiedades predeterminadas
const std::map<ComponentType, std::map<std::string, std::string>> componentPropertiesMap = {
    {ComponentType::HeaderH1, {{"class", ""}, {"text", "Default Header"}}},
    {ComponentType::HeaderH2, {{"class", ""}, {"text", "Default Header 2"}}},
    {ComponentType::HeaderH3, {{"class", ""}, {"text", "Default Header 3"}}},
    {ComponentType::Paragraph, {{"class", ""}, {"text", "Default Paragraph"}}},
    {ComponentType::Input,
     {{"class", ""}, {"placeholder", "Enter text"}, {"type", "text"}, {"value", ""}, {"name", ""}}},
    {ComponentType::TextArea, {{"class", ""}, {"placeholder", "Enter text"}}},
    {ComponentType::Button,
     {{"class", ""}, {"text", "Default Button"}, {"type", "button"}, {"click", ""}}},
    {ComponentType::Hyperlink,
     {{"class", ""}, {"text", "Default Hyperlink"}, {"href", ""}, {"target", ""}, {"rel", ""}}},
    {ComponentType::Form, {{"class", ""}, {"method", ""}, {"model", ""}}},
    {ComponentType::HorizontalLayout, {{"class", ""}}},
    {ComponentType::VerticalLayout, {{"class", ""}}},
    {ComponentType::ModelLayout, {{"class", ""}, {"model", ""}}},
    {ComponentType::ModelLayout, {{"name", ""}}}};

// Función para convertir ComponentType a std::string
inline std::string componentTypeToString(ComponentType type)
{
    switch (type) {
    case ComponentType::HeaderH1:
        return "Header H1";
    case ComponentType::HeaderH2:
        return "Header H2";
    case ComponentType::HeaderH3:
        return "Header H3";
    case ComponentType::Paragraph:
        return "Paragraph";
    case ComponentType::Input:
        return "Input";
    case ComponentType::TextArea:
        return "Text Area";
    case ComponentType::Button:
        return "Button";
    case ComponentType::Hyperlink:
        return "Hyperlink";
    case ComponentType::Form:
        return "Form";
    case ComponentType::HorizontalLayout:
        return "Horizontal Layout";
    case ComponentType::VerticalLayout:
        return "Vertical Layout";
    case ComponentType::ModelLayout:
        return "Model Layout";
    default:
        return "Undefined";
    }
}

// Función para convertir std::string a ComponentType
inline ComponentType stringToComponentType(const std::string &typeStr)
{
    static const std::unordered_map<std::string, ComponentType> typeMap = {
        {"Header H1", ComponentType::HeaderH1},
        {"Header H2", ComponentType::HeaderH2},
        {"Header H3", ComponentType::HeaderH3},
        {"Paragraph", ComponentType::Paragraph},
        {"Input", ComponentType::Input},
        {"Text Area", ComponentType::TextArea},
        {"Button", ComponentType::Button},
        {"Hyperlink", ComponentType::Hyperlink},
        {"Form", ComponentType::Form},
        {"Horizontal Layout", ComponentType::HorizontalLayout},
        {"Vertical Layout", ComponentType::VerticalLayout},
        {"Model Layout", ComponentType::ModelLayout},
    };

    auto it = typeMap.find(typeStr);
    return (it != typeMap.end()) ? it->second : ComponentType::Undefined;
}

#endif // COMPONENTTYPE_H
