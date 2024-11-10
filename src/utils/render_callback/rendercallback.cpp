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
std::string renderComponentCallback(inja::Environment &env, inja::Arguments &args)
{
    if (args.empty() || !args[0]->is_object()) {
        fmt::print(stderr, "Invalid argument passed to render_component.\n");
        return "<!-- Invalid argument -->";
    }

    const nlohmann::json &componentJson = *args[0];
    std::string type;
    std::string parentType = args.at(1)->get<std::string>();

    if (componentJson.contains("type") && componentJson["type"].is_string()) {
        type = componentJson["type"];
    } else {
        fmt::print(stderr,
                   "Unsupported component type format or missing: {}\n",
                   componentJson.dump());
        return "<!-- Unsupported component type -->";
    }

    const auto &props = componentJson["props"];
    if (!props.is_object()) {
        fmt::print(stderr, "Invalid props format: must be an object.\n");
        return "<!-- Invalid props format -->";
    }

    std::string output;

    if (type == "Header H1") {
        std::string className = props.value("class", "");
        std::string value = props.value("text", "Default Header");

        output = "<h1 className=\"" + className + "\">" + value + "</h1>";
    } else if (type == "Header H2") {
        std::string className = props.value("class", "");
        std::string value = props.value("text", "Default Header 2");

        output = "<h2 className=\"" + className + "\">" + value + "</h2>";
    } else if (type == "Header H3") {
        std::string className = props.value("class", "");
        std::string value = props.value("text", "Default Header 3");

        output = "<h3 className=\"" + className + "\">" + value + "</h3>";
    } else if (type == "Paragraph") {
        std::string className = props.value("class", "");
        std::string value = props.value("text", "Default Header");

        output = "<p className=\"" + className + "\">" + value + "</p>";
    } else if (type == "Input") {
        std::string className = props.value("class", "");
        std::string placeholder = props.value("placeholder", "");
        std::string type = props.value("type", "text");
        std::string value = props.value("value", "");

        output = "<input className=\"" + className + "\" type=\"" + type + "\" placeholder=\""
                 + placeholder + "\" value=\"" + value + "\""
                 + (parentType == "Form" ? "onChange={handleChange}" : "") + " /> ";

    } else if (type == "Text Area") {
        std::string className = props.value("class", "");
        std::string placeholder = props.value("placeholder", "");

        output = "<textarea className=\"" + className + "\" placeholder=\"" + placeholder
                 + "\"></textarea>";
    } else if (type == "Button") {
        std::string className = props.value("class", "");
        std::string value = props.value("text", "Default Button");
        std::string type = props.value("type", "button");

        output = "<button className=\"" + className + "\" type=\"" + type + "\" >" + value
                 + "</button>";
    } else if (type == "Horizontal Layout" || type == "Vertical Layout" || type == "Model Layout") {
        std::string layoutClass;

        if (type == "Model Layout") {
            layoutClass = props.value("class", "");
        } else {
            layoutClass = (type == "Horizontal Layout") ? "flex flex-row" : "flex flex-col";

            if (props.contains("class") && !props["class"].get<std::string>().empty()) {
                layoutClass += " " + props["class"].get<std::string>();
            }
        }
        output = "<div className=\"" + layoutClass + "\">";

        if (type == "Model Layout") {
            // Add map to iterate only if 'model' is valid
            std::string model = props.value("model", "Model");
            if (!model.empty() && model != "Model") {
                std::string lowerModel = toLower(model);
                output += "{" + lowerModel + ".map((obj, index) => (";
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
        } else {
            fmt::print(stderr, "Invalid or missing nestedComponents array.\n");
        }

        if (type == "Model Layout") {
            std::string model = props.value("model", "Model");
            if (!model.empty() && model != "Model") {
                output += "))}";
            }
        }

        output += "</div>";
    } else if (type == "Form") {
        std::string className = props.value("class", "");
        std::string onSubmit = "";

        std::string method = props.value("method", "Method");
        if (!method.empty() && method != "Method") {
            if (method == "POST" || method == "PUT")
                onSubmit += "handleSubmit";
        }

        output = "<form className=\"" + className + "\" onSubmit={" + onSubmit + "} >";

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
        } else {
            fmt::print(stderr, "Invalid or missing nestedComponents array.\n");
        }

        output += "</form>";
    } else {
        fmt::print(stderr, "Unsupported component type: {}\n", type);
        output = "<!-- Unsupported component type: " + type + " -->";
    }

    return output;
}

std::string renderServiceImportsCallback(inja::Environment &env, inja::Arguments &args)
{
    // Validar que el argumento sea un array de componentes
    if (args.empty() || !args[0]->is_array()) {
        fmt::print(stderr, "Invalid argument passed to renderServiceImportsCallback.\n");
        return "<!-- Invalid argument -->";
    }

    const nlohmann::json &components = *args[0];
    std::string output;

    // Recorrer cada componente en el array
    for (const auto &componentJson : components) {
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

        std::string componentType = componentJson["type"];

        if (componentJson.contains("type")
            && (componentType == "Model Layout" || componentType == "Form")) {
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
    handleChange = "const handleChange = (e) => {\n";
    handleChange += "  const { name, value } = e.target;\n";

    // Generar el handleSubmit
    handleSubmit = "const handleSubmit = async (e) => {\n";
    handleSubmit += "  e.preventDefault();\n\n";
    handleSubmit += "  try {\n";

    for (const auto &componentJson : components) {
        if (componentJson.contains("type") && componentJson["type"] == "Form") {
            const auto &props = componentJson["props"];
            if (props.contains("model") && props["model"].is_string()
                && !props["model"].get<std::string>().empty() && props["model"] != "Model") {
                std::string modelName = props["model"];
                std::string method = props["method"];
                std::string lowerModel = toLower(modelName);
                std::string lowerMethod = toLower(method);

                // Agregar código para handleChange
                handleChange += "  set" + lowerMethod + modelName + "((prevData) => ({\n";
                handleChange += "    ...prevData,\n";
                handleChange += "    [name]: value,\n";
                handleChange += "  }));\n";

                // Agregar código para handleSubmit
                handleSubmit += "    const response = await " + modelName + "Service.create"
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
