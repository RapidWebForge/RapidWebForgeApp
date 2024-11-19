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
    Button,
    Form,
    HorizontalLayout,
    VerticalLayout,
    ModelLayout,
};

// Mapa de propiedades predeterminadas
const std::map<ComponentType, std::map<std::string, std::string>> componentPropertiesMap
    = {{ComponentType::HeaderH1, {{"class", ""}, {"text", "Default Header"}}},
       {ComponentType::HeaderH2, {{"class", ""}, {"text", "Default Header 2"}}},
       {ComponentType::HeaderH3, {{"class", ""}, {"text", "Default Header 3"}}},
       {ComponentType::Paragraph, {{"class", ""}, {"text", "Default Paragraph"}}},
       {ComponentType::Input,
        {{"class", ""}, {"placeholder", "Enter text"}, {"type", "text"}, {"value", ""}}},
       {ComponentType::TextArea, {{"class", ""}, {"placeholder", "Enter text"}}},
       {ComponentType::Button,
        {{"class", ""}, {"text", "Default Button"}, {"type", "button"}, {"click", ""}}},
       {ComponentType::Form, {{"class", ""}, {"method", ""}, {"model", ""}}},
       {ComponentType::HorizontalLayout, {{"class", ""}}},
       {ComponentType::VerticalLayout, {{"class", ""}}},
       {ComponentType::ModelLayout, {{"class", ""}, {"model", ""}}}};

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
    if (typeStr == "Button")
        return ComponentType::Button;
    if (typeStr == "Form")
        return ComponentType::Form;
    if (typeStr == "Horizontal Layout")
        return ComponentType::HorizontalLayout;
    if (typeStr == "Vertical Layout")
        return ComponentType::VerticalLayout;
    if (typeStr == "Model Layout")
        return ComponentType::ModelLayout;
    return ComponentType::Undefined;
}

#endif // COMPONENTTYPE_H
