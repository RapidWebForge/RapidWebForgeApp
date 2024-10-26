#ifndef COMPONENTTYPE_H
#define COMPONENTTYPE_H

#include <map>
#include <string>

enum class ComponentType {
    Undefined,
    HeaderH1,
    HeaderH2,
    HeaderH3,
    Paragraph,
    Input,
    TextArea,
    HorizontalLayout,
    VerticalLayout,
    ModelLayout,
};

// Mapa de propiedades predeterminadas
const std::map<ComponentType, std::map<std::string, std::string>> componentPropertiesMap
    = {{ComponentType::HeaderH1, {{"text", "Default Header"}}},
       {ComponentType::HeaderH2, {{"text", "Default Subheader"}}},
       {ComponentType::HeaderH3, {{"text", "Default Header 3"}}},
       {ComponentType::Paragraph, {{"text", "Default Paragraph"}}},
       {ComponentType::Input, {{"placeholder", "Enter text"}, {"type", "text"}}},
       {ComponentType::TextArea, {{"placeholder", "Enter text"}}},
       {ComponentType::HorizontalLayout, {}},
       {ComponentType::VerticalLayout, {}},
       {ComponentType::ModelLayout, {{"model", "Default Model"}}}};

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
    if (typeStr == "Header H1")
        return ComponentType::HeaderH1;
    if (typeStr == "Header H2")
        return ComponentType::HeaderH2;
    if (typeStr == "Header H3")
        return ComponentType::HeaderH3;
    if (typeStr == "Paragraph")
        return ComponentType::Paragraph;
    if (typeStr == "Input")
        return ComponentType::Input;
    if (typeStr == "Text Area")
        return ComponentType::TextArea;
    if (typeStr == "Horizontal Layout")
        return ComponentType::HorizontalLayout;
    if (typeStr == "Vertical Layout")
        return ComponentType::VerticalLayout;
    if (typeStr == "Model Layout")
        return ComponentType::ModelLayout;
    return ComponentType::Undefined;
}

#endif // COMPONENTTYPE_H
