#include "rendercallback.h"

#include <QDebug>
#include <algorithm>
#include <cctype>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

std::string toLower(const std::string &str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

namespace RenderCallback {

std::unordered_set<std::string> customComponentsCache;

std::string renderCustomComponent(const nlohmann::json componentJson)
{
    std::string output;

    if (!componentJson.contains("name"))
        return output;

    std::string id = componentJson["id"];

    std::string componentName = componentJson["name"];
    if (customComponentsCache.find(componentName) != customComponentsCache.end()) {
        output = "<" + componentName + " data-id=\"" + id + "\" />";
    } else {
        fmt::print(stderr, "Unsupported custom component: {}\n", componentName);
        output = "<!-- Unsupported custom component: " + componentName + " -->";
    }
    return output;
}

std::string renderComponent(inja::Environment &env,
                            const nlohmann::json componentJson,
                            std::string type,
                            std::string parentType)
{
    std::string output = "";

    if (!parentType.empty()) {
        // output = "\n";
    }

    const auto &props = componentJson["props"];
    if (!props.is_object()) {
        fmt::print(stderr, "Invalid props format: must be an object.\n");
        return "<!-- Invalid props format -->";
    }

    std::string id, className, value;
    id = componentJson["id"];
    className = props.value("class", "");

    if (type.find("Header") != std::string::npos) {
        char numberChar = type[type.length() - 1];

        if (std::isdigit(numberChar)) {
            std::string number(1, numberChar);

            value = props.value("text", "Default Header H" + number);

            output += "<h" + number;

            if (!className.empty())
                output += " className=\"" + className + "\"";

            output += " data-id=\"" + id + "\">" + value + "</h" + number + ">";

        } else {
            fmt::print(stderr, "Unsupported component type: {}\n", type);
            output = "<!-- Unsupported component type: " + type + " -->";
        }
    } else if (type == "Paragraph") {
        value = props.value("text", "Default Header");

        output += "<p";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        output += " data-id=\"" + id + "\">" + value + "</p>";

    } else if (type == "Input") {
        std::string placeholder = props.value("placeholder", "");
        std::string type = props.value("type", "text");
        std::string inputValue = "value=";

        if (value[0] == '{') {
            inputValue += value;
        } else {
            inputValue += "\"" + value + "\" ";
        }

        output += "<input";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        if (!placeholder.empty())
            placeholder += " placeholder=\"" + placeholder + "\"";

        if (!type.empty())
            output += " type=\"" + type + "\"";

        output += inputValue + (parentType == "Form" ? "onChange={handleChange}" : "")
                  + " data-id=\"" + id + "\"/>";

    } else if (type == "Text Area") {
        std::string placeholder = props.value("placeholder", "");

        output += "<textarea";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        if (!placeholder.empty())
            placeholder += " placeholder=\"" + placeholder + "\"";

        output += " data-id=\"" + id + "\" />";
    } else if (type == "Button") {
        value = props.value("text", "Default Button");
        std::string type = props.value("type", "button");

        output += "<button";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        if (!type.empty())
            type += " type=\"" + type + "\"";

        output += " data-id=\"" + id + "\">" + value + "</button>";
    } else if (type == "Horizontal Layout" || type == "Vertical Layout" || type == "Model Layout") {
        std::string layoutClass, model;

        if (type == "Model Layout") {
            layoutClass = props.value("class", "");
            model = props.value("model", "Model");
        } else {
            layoutClass = (type == "Horizontal Layout") ? "flex flex-row" : "flex flex-col";

            if (props.contains("class") && !props["class"].get<std::string>().empty()) {
                layoutClass += " " + props["class"].get<std::string>();
            }
        }
        output += "<div data-id=\"" + id + "\"";

        bool modelIsValid = !model.empty() && model != "Model";

        if (!layoutClass.empty())
            output += " className=\"" + layoutClass + "\"";

        if (modelIsValid)
            output += " data-rwf-model=\"" + model + "\"";

        output += ">";

        if (type == "Model Layout") {
            // Add map to iterate only if 'model' is valid
            if (modelIsValid) {
                std::string lowerModel = toLower(model);
                output += "{" + lowerModel + ".map((obj, index) => (";
                output += "<div index={index}>";
            }
        }

        if (componentJson.contains("nestedComponents")
            && componentJson["nestedComponents"].is_array()) {
            for (const auto &nestedComponent : componentJson["nestedComponents"]) {
                try {
                    nlohmann::json contextWithNested;
                    contextWithNested["nestedComponent"] = nestedComponent;

                    output += env.render("{{ render_component(nestedComponent, \"" + type + "\") }}",
                                         contextWithNested);
                } catch (const std::exception &e) {
                    fmt::print(stderr, "Error rendering nested component: {}\n", e.what());
                    output += "<!-- Error rendering nested component -->";
                }
            }
        }

        if (type == "Model Layout") {
            if (modelIsValid) {
                output += "</div>";
                output += "))}";
            }
        }

        output += "</div>";
    } else if (type == "Form") {
        className = props.value("class", "");
        std::string onSubmit = "";
        std::string model = props.value("model", "Model");
        std::string method = props.value("method", "Method");

        bool modelIsValid = !model.empty() && model != "Model";

        if (!method.empty() && method != "Method") {
            if (method == "POST" || method == "PUT")
                onSubmit += "handleSubmit";
        }

        output += "<form data-id=\"" + id + "\"";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        if (!onSubmit.empty()) {
            output += " onSubmit={" + onSubmit + "}";
            output += " data-rwf-method=\"" + method + "\"";
        }

        if (modelIsValid)
            output += " data-rwf-model=\"" + model + "\"";

        output += ">";

        if (componentJson.contains("nestedComponents")
            && componentJson["nestedComponents"].is_array()) {
            for (const auto &nestedComponent : componentJson["nestedComponents"]) {
                try {
                    nlohmann::json contextWithNested;
                    contextWithNested["nestedComponent"] = nestedComponent;
                    output += env.render("{{ render_component(nestedComponent, \"" + type + "\") }}",
                                         contextWithNested);
                } catch (const std::exception &e) {
                    fmt::print(stderr, "Error rendering nested component: {}", e.what());
                    output += "<!-- Error rendering nested component -->";
                }
            }
        } else {
            fmt::print(stderr, "Invalid or missing nestedComponents array.");
        }

        output += "</form>";
    } else {
        fmt::print(stderr, "Unsupported component type: {}", type);
        output = "<!-- Unsupported component type: " + type + " -->";
    }

    return output;
}

std::string renderComponentCallback(inja::Environment &env, inja::Arguments &args)
{
    if (args.empty() || !args[0]->is_object()) {
        fmt::print(stderr, "Invalid argument passed to renderComponentCallback.\n");
        return "<!-- Invalid argument -->";
    }

    const nlohmann::json &componentJson = *args[0];

    if (componentJson.contains("type") && componentJson["type"].is_string()) {
        std::string type = componentJson["type"];
        std::string parentType = args.at(1)->get<std::string>();

        return renderComponent(env, componentJson, type, parentType);
    } else {
        return renderCustomComponent(componentJson);
    }
}

std::string renderServiceImportsCallback(const nlohmann::json componentJson)
{
    std::string output;

    // Verificar si el componente es de tipo "Model Layout"
    if (componentJson.contains("type")
        && (componentJson["type"] == "Model Layout" || componentJson["type"] == "Form")) {
        // Obtener las propiedades del componente
        const auto &props = componentJson["props"];

        // Verificar si el modelo no es "Model" y no es un string vacío
        if (props.contains("model") && props["model"].is_string()
            && !props["model"].get<std::string>().empty() && props["model"] != "Model") {
            std::string modelName = props["model"];

            // Generar el código de importación
            output += "import " + modelName + "Service from \"../services/" + modelName
                      + "Service\";\n";
            output += "import " + modelName + " from \"../models/" + modelName + "\";\n";
        }
    }

    return output;
}

std::string renderCustomComponentsImportsCallback(const nlohmann::json componentJson)
{
    std::string output;

    if (!componentJson.contains("name"))
        return output;

    std::string customComponentName = componentJson["name"];

    output += "import " + customComponentName + " from \"../components/" + customComponentName
              + "\";\n";

    return output;
}

std::string renderImportsCallback(inja::Environment &env, inja::Arguments &args)
{
    // Validar que el argumento sea un array de componentes
    if (args.empty() || !args[0]->is_array()) {
        fmt::print(stderr, "Invalid argument passed to renderImportsCallback.\n");
        return "<!-- Invalid argument -->";
    }

    const nlohmann::json &components = *args[0];
    std::string output;

    for (const auto &componentJson : components) {
        if (componentJson.contains("type"))
            output += renderServiceImportsCallback(componentJson);
        else
            output += renderCustomComponentsImportsCallback(componentJson);
    }

    return output;
}

std::string renderStatesCallback(inja::Environment &env, inja::Arguments &args)
{
    // Validar que el argumento sea un array de componentes
    if (args.empty() || !args[0]->is_array()) {
        fmt::print(stderr, "Invalid argument passed to renderStatesCallback.\n");
        return "<!-- Invalid argument -->";
    }

    const nlohmann::json &components = *args[0];
    std::string output;

    // Recorrer cada componente en el array
    for (const auto &componentJson : components) {
        // Verificar si el componente es de tipo "Model Layout" o "Form"

        if (componentJson.contains("type")
            && (componentJson["type"] == "Model Layout" || componentJson["type"] == "Form")) {
            std::string componentType = componentJson["type"];

            // Obtener las propiedades del componente
            const auto &props = componentJson["props"];

            // Verificar si el modelo no es "Model" y no es un string vacío
            if (props.contains("model") && props["model"].is_string()
                && !props["model"].get<std::string>().empty() && props["model"] != "Model") {
                std::string modelName = props["model"];
                std::string lowerModelName = toLower(modelName);

                if (componentType == "Model Layout") {
                    // Generar la declaración del estado usando useState
                    output += "const [" + lowerModelName + ", set" + modelName + "] = useState<"
                              + modelName + "[]>([]);\n";
                } else if (componentType == "Form") {
                    if (props.contains("method") && props["method"].is_string()
                        && !props["method"].get<std::string>().empty()
                        && props["method"] != "Method") {
                        std::string modelName = props["model"];
                        std::string method = props["method"];

                        std::string lowerMethod = toLower(method);

                        output += "const [" + lowerMethod + modelName + ", set" + lowerMethod
                                  + modelName + "] = useState<" + modelName + ">();\n";
                    }
                }
            }
        }
    }

    return output;
}

std::string renderHandleFoosCallback(inja::Environment &env, inja::Arguments &args)
{
    // Validar que el argumento sea un array de componentes
    if (args.empty() || !args[0]->is_array()) {
        fmt::print(stderr, "Invalid argument passed to renderHandleFoosCallback.\n");
        return "<!-- Invalid argument -->";
    }

    const nlohmann::json &components = *args[0];
    std::string handleChange, handleSubmit;

    // Verificar si existe al menos un "Form" con un "prop.model" válido
    bool hasValidModel = false;
    for (const auto &componentJson : components) {
        if (componentJson.contains("type") && componentJson["type"] == "Form") {
            const auto &props = componentJson["props"];
            if (props.contains("model") && props["model"].is_string()
                && !props["model"].get<std::string>().empty() && props["model"] != "Model") {
                hasValidModel = true;
                break;
            }
        }
    }

    // Si no hay un "Form" con un "prop.model" válido, retornar vacío
    if (!hasValidModel) {
        return ""; // No se genera los handle
    }

    // Generar el handleChange
    handleChange = "const handleChange = (e: any) => {\n";
    handleChange += "  const { name, value } = e.target;\n";

    // Generar el handleSubmit
    handleSubmit = "const handleSubmit = async (e: React.FormEvent) => {\n";
    handleSubmit += "  e.preventDefault();\n\n";

    for (const auto &componentJson : components) {
        if (componentJson.contains("type") && componentJson["type"] == "Form") {
            const auto &props = componentJson["props"];
            if (props.contains("model") && props["model"].is_string()
                && !props["model"].get<std::string>().empty() && props["model"] != "Model") {
                std::string modelName = props["model"];
                std::string method = props["method"];
                std::string lowerModel = toLower(modelName);
                std::string lowerMethod = toLower(method);

                std::string methodService;

                if (modelName == "PUT")
                    methodService = "update";
                else if (modelName == "POST")
                    methodService = "create";

                // Agregar código para handleChange
                handleChange += "  set" + lowerMethod + modelName + "((prevData) => ({\n";
                handleChange += "    ...prevData,\n";
                handleChange += "    [name]: value,\n";
                handleChange += "  }));\n";

                // Agregar código para handleSubmit
                handleSubmit += "  if (!" + lowerMethod + modelName + ") {\n";
                handleSubmit += "  console.error(\"Data is undefined\");\n";
                handleSubmit += "  return;\n";
                handleSubmit += "  }\n\n";
                handleSubmit += "  try {\n";
                handleSubmit += "    const response = await " + methodService + "Service.create"
                                + modelName + "(" + lowerMethod + modelName + ");\n";
                handleSubmit += "    console.log(\"Form submitted successfully:\", response);\n";
            }
        }
    }

    handleChange += "};\n\n";
    handleSubmit += "  } catch (error) {\n";
    handleSubmit += "    console.error(\"Error submitting form:\", error);\n";
    handleSubmit += "  }\n";
    handleSubmit += "};\n";

    return handleChange + handleSubmit;
}

std::string renderRequestsCallback(inja::Environment &env, inja::Arguments &args)
{
    // Validar que el argumento sea un array de componentes
    if (args.empty() || !args[0]->is_array()) {
        fmt::print(stderr, "Invalid argument passed to renderRequestsCallback.\n");
        return "<!-- Invalid argument -->";
    }

    const nlohmann::json &components = *args[0];
    std::string output;

    // Verificar si existe al menos un "Model Layout" con un "prop.model" válido
    bool hasValidModel = false;
    for (const auto &componentJson : components) {
        if (componentJson.contains("type") && componentJson["type"] == "Model Layout") {
            const auto &props = componentJson["props"];
            if (props.contains("model") && props["model"].is_string()
                && !props["model"].get<std::string>().empty() && props["model"] != "Model") {
                hasValidModel = true;
                break;
            }
        }
    }

    // Si no hay un "Model Layout" con un "prop.model" válido, retornar vacío
    if (!hasValidModel) {
        return output; // No se genera el useEffect
    }

    // Generar el useEffect si existe un "Model Layout" válido
    output = "useEffect(() => {\n";

    for (const auto &componentJson : components) {
        if (componentJson.contains("type") && componentJson["type"] == "Model Layout") {
            const auto &props = componentJson["props"];
            if (props.contains("model") && props["model"].is_string()
                && !props["model"].get<std::string>().empty() && props["model"] != "Model") {
                std::string modelName = props["model"];
                output += "  " + modelName + "Service.getAll" + modelName + "()\n";
                output += "    .then((response) => {\n";
                output += "      set" + modelName + "(response);\n";
                output += "    })\n";
                output += "    .catch((error) => {\n";
                output += "      console.error(\"Error fetching " + modelName
                          + " data:\", error);\n";
                output += "    });\n";
            }
        }
    }

    output += "}, []); // Empty dependency array to run once\n";
    return output;
}

std::string renderTypeFrontendModel(inja::Environment &env, inja::Arguments &args)
{
    if (args.empty() || !args[0]->is_string()) {
        fmt::print(stderr, "Invalid argument passed to renderTypeFrontendModel.\n");
        return "<!-- Invalid argument -->";
    }

    std::string output;

    const nlohmann::json &type = *args[0];

    if (type == "STRING")
        output = "string";
    else if (type == "INTEGER")
        output = "number";
    else if (type == "DATE")
        output = "Date";

    return output;
}
} // namespace RenderCallback
