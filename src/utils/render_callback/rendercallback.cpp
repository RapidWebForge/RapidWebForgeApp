#include "rendercallback.h"
#include <QDebug>
#include <algorithm>
#include <cctype>
#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <unordered_map>

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
        output = {};
    }
    return output;
}

std::string renderComponent(inja::Environment &env,
                            const nlohmann::json componentJson,
                            std::string type,
                            const nlohmann::json parentProps)
{
    std::string output = "";

    const auto &props = componentJson["props"];
    if (!props.is_object()) {
        fmt::print(stderr, "Invalid props format: must be an object.\n");
        return {};
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
            output = {};
        }
    } else if (type == "Paragraph") {
        value = props.value("text", "Default Header");

        output += "<p";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        output += " data-id=\"" + id + "\">" + value + "</p>";

    } else if (type == "Input") {
        std::string placeholder = props.value("placeholder", "");
        std::string inputType = props.value("type", "text");
        std::string value = props.value("value", "");
        std::string inputId = props.value("inputid", "");
        std::string name = props.value("name", "");
        std::string minlength = props.value("minlength", "");
        std::string maxlength = props.value("maxlength", "");
        std::string required = props.value("required", "");
        std::string inputValue = "";
        std::string onChange = "";

        if (value[0] == '{') {
            inputValue = value;
        } else if (!value.empty()) {
            inputValue = "\"" + value + "\" ";
        }

        output += "<input";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        if (!placeholder.empty())
            output += " placeholder=\"" + placeholder + "\"";

        if (!inputId.empty())
            output += " id=\"" + inputId + "\"";

        if (!name.empty())
            output += " name=\"" + name + "\"";

        if (!minlength.empty())
            output += " minlength=\"" + minlength + "\"";

        if (!maxlength.empty())
            output += " maxlength=\"" + maxlength + "\"";

        if (!required.empty() && required == "true")
            output += " required";

        if (!inputType.empty())
            output += " type=\"" + inputType + "\"";

        if (!inputValue.empty())
            output += " value=" + inputValue;

        if (parentProps.is_object() && !parentProps.empty()) {
            std::string model = parentProps.value("model", "");
            std::string method = parentProps.value("method", "");

            if (model != "" && method != "") {
                std::string methodCapitalize;

                if (method == "PUT")
                    methodCapitalize = "Put";
                else if (method == "POST")
                    methodCapitalize = "Post";

                onChange = "onChange={handleChange" + methodCapitalize + model + "}";
            }
        }

        output += onChange + " data-id=\"" + id + "\"/>";

    } else if (type == "Label") {
        value = props.value("text", "Default Label");
        std::string htmlFor = props.value("for", "");

        output += "<label";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        if (!htmlFor.empty())
            output += " htmlFor=\"" + htmlFor + "\"";

        output += " data-id=\"" + id + "\">" + value + "</label>";

    } else if (type == "Text Area") {
        std::string placeholder = props.value("placeholder", "");
        std::string minlength = props.value("minlength", "");
        std::string maxlength = props.value("maxlength", "");

        output += "<textarea";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        if (!placeholder.empty())
            output += " placeholder=\"" + placeholder + "\"";

        if (!minlength.empty())
            output += " minlength=\"" + minlength + "\"";

        if (!maxlength.empty())
            output += " maxlength=\"" + maxlength + "\"";

        output += " data-id=\"" + id + "\" />";
    } else if (type == "Button") {
        value = props.value("text", "Default Button");
        std::string type = props.value("type", "button");
        std::string click = props.value("click", "");

        output += "<button";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        if (!type.empty())
            output += " type=\"" + type + "\"";

        if (!click.empty())
            output += " onClick={ () => {" + click + "} }";

        output += " data-id=\"" + id + "\">" + value + "</button>";
    } else if (type == "Hyperlink") {
        value = props.value("text", "Default Hyperlink");
        std::string href = props.value("href", "");
        std::string target = props.value("target", "");
        std::string rel = props.value("rel", "");
        std::string hyperlinkRef = "";

        if (href[0] == '{') {
            hyperlinkRef = href;
        } else if (!href.empty()) {
            hyperlinkRef = "\"" + href + "\" ";
        }

        output += "<a";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        if (!hyperlinkRef.empty())
            output += " href=" + hyperlinkRef;

        if (!target.empty())
            output += " target=\"" + target + "\"";

        if (!rel.empty())
            output += " rel=\"" + rel + "\"";

        output += " data-id=\"" + id + "\">" + value + "</a>";
    } else if (type == "Image") {
        std::string src = props.value("src", "");
        std::string alt = props.value("alt", "");
        std::string width = props.value("width", "");
        std::string height = props.value("height", "");

        output += "<img";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        if (!src.empty())
            output += " src=\"" + src + "\"";

        if (!alt.empty())
            output += " alt=\"" + alt + "\"";

        if (!width.empty())
            output += " width=\"" + width + "\"";

        if (!height.empty())
            output += " height=\"" + height + "\"";

        output += " data-id=\"" + id + "\" />";
    } else if (type == "Iframe") {
        std::string src = props.value("src", "");
        std::string title = props.value("title", "");

        output += "<iframe";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        if (!src.empty())
            output += " src=\"" + src + "\"";

        if (!title.empty())
            output += " title=\"" + title + "\"";

        output += " data-id=\"" + id + "\" />";
    } else if (type == "Horizontal Layout" || type == "Vertical Layout" || type == "Model Layout"
               || type == "Layout") {
        std::string layoutClass, model;

        if (type == "Model Layout") {
            layoutClass = props.value("class", "");
            model = props.value("model", "");
        } else {
            layoutClass = type == "Layout"                ? ""
                          : (type == "Horizontal Layout") ? "flex flex-row"
                                                          : "flex flex-col";

            if (props.contains("class") && !props["class"].get<std::string>().empty()) {
                layoutClass += " " + props["class"].get<std::string>();
            }
        }

        bool modelIsValid = !model.empty();
        bool getAll = false, getId = false;
        std::string get;
        std::string lowerModel;
        if (props.contains("get") && !props["get"].get<std::string>().empty())
            get = props["get"];

        // Add map to iterate only if 'model' is valid
        if (get == "ALL" && modelIsValid) {
            lowerModel = toLower(model);
            getAll = true;
            // output += "{" + lowerModel + ".length > 0 && (";
        }
        if (get == "ID" && modelIsValid) {
            lowerModel = toLower(model);
            getId = true;
            // output += "{" + lowerModel + " && (";
        }

        output += "<div data-id=\"" + id + "\"";

        if (!layoutClass.empty())
            output += " className=\"" + layoutClass + "\"";

        if (modelIsValid)
            output += " data-rwf-model=\"" + model + "\"";

        if (getId || getAll)
            output += " data-rwf-get=\"" + get + "\"";

        output += ">";

        // Model Layout
        if (modelIsValid && getAll) {
            output += "{" + lowerModel + ".map((obj, index) => (";
            output += "<div key={index}>";
        }

        if (componentJson.contains("nestedComponents")
            && componentJson["nestedComponents"].is_array()) {
            for (const auto &nestedComponent : componentJson["nestedComponents"]) {
                try {
                    nlohmann::json contextWithNested;
                    contextWithNested["nestedComponent"] = nestedComponent;
                    contextWithNested["parentProps"] = parentProps;

                    output += env.render(R"({{ render_component(nestedComponent, parentProps) }})",
                                         contextWithNested);
                } catch (const std::exception &e) {
                    fmt::print(stderr, "Error rendering nested component: {}\n", e.what());
                    output += {};
                }
            }
        }

        // Model Layout
        if (modelIsValid && getAll) {
            output += "</div>";
            output += "))}";
        }

        output += "</div>";

        // Model Layout
        // if ((get == "ALL" || get == "ID") && modelIsValid)
        //     output += ")}";

    } else if (type == "Form") {
        bool sendProps = true;
        className = props.value("class", "");
        std::string onSubmit = "";
        std::string model = props.value("model", "");
        std::string method = props.value("method", "");

        if (!method.empty() && !model.empty()) {
            if (method == "POST" || method == "PUT") {
                std::string methodCapitalize;

                if (method == "PUT")
                    methodCapitalize = "Put";
                else if (method == "POST")
                    methodCapitalize = "Post";

                onSubmit += "handleSubmit" + methodCapitalize + model;
            } else {
                sendProps = false;
            }
        }

        output += "<form data-id=\"" + id + "\"";

        if (!className.empty())
            output += " className=\"" + className + "\"";

        if (!onSubmit.empty()) {
            output += " onSubmit={" + onSubmit + "}";
            output += " data-rwf-method=\"" + method + "\"";
        }

        if (!model.empty())
            output += " data-rwf-model=\"" + model + "\"";
        else {
            sendProps = false;
        }

        output += ">";

        if (componentJson.contains("nestedComponents")
            && componentJson["nestedComponents"].is_array()) {
            for (const auto &nestedComponent : componentJson["nestedComponents"]) {
                try {
                    nlohmann::json contextWithNested;
                    contextWithNested["nestedComponent"] = nestedComponent;

                    if (sendProps) {
                        contextWithNested["parentProps"] = props;
                    } else {
                        contextWithNested["parentProps"] = nlohmann::json::object();
                    }

                    output += env.render(R"({{ render_component(nestedComponent, parentProps) }})",
                                         contextWithNested);
                } catch (const std::exception &e) {
                    fmt::print(stderr, "Error rendering nested component: {}", e.what());
                    output += {};
                }
            }
        } else {
            fmt::print(stderr, "Invalid or missing nestedComponents array.");
        }

        output += "</form>";
    } else {
        fmt::print(stderr, "Unsupported component type: {}", type);
        output = {};
    }

    return output;
}

std::string renderComponentCallback(inja::Environment &env, inja::Arguments &args)
{
    if (args.empty() || !args[0]->is_object() || !args[1]->is_object()) {
        fmt::print(stderr, "Invalid argument passed to renderComponentCallback.\n");
        return {};
    }

    const nlohmann::json &componentJson = *args[0];

    if (componentJson.contains("type") && componentJson["type"].is_string()) {
        std::string type = componentJson["type"];
        const nlohmann::json &parentProps = *args[1];

        return renderComponent(env, componentJson, type, parentProps);
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
            && !props["model"].get<std::string>().empty()) {
            std::string modelName = props["model"];

            // Generar el código de importación
            output += "import " + modelName + "Service from \"../services/" + modelName
                      + "Service\";\n";
            output += "import " + modelName + ", { " + modelName + "Defaults } from \"../models/"
                      + modelName + "\";\n";
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

std::string renderImportParamsCallback(inja::Arguments &args)
{
    // Validar que el argumento sea un array de componentes
    if (args.empty() || !args[0]->is_string()) {
        fmt::print(stderr, "Invalid argument passed to renderImportsCallback.\n");
        return {};
    }

    const std::string &path = *args[0];
    std::string output;

    // Buscar un segmento que comience por ':'
    // (ej: "/product/:id/details/:tab")
    if (path.find('/:') != std::string::npos) {
        output += "import { useParams } from \"react-router-dom\";\n";
    }

    return output;
}

std::string renderImportsCallback(inja::Environment &env, inja::Arguments &args)
{
    // Validar que el argumento sea un array de componentes
    if (args.empty() || !args[0]->is_array()) {
        fmt::print(stderr, "Invalid argument passed to renderImportsCallback.\n");
        return {};
    }

    const nlohmann::json &components = *args[0];
    std::string output;

    for (const auto &componentJson : components) {
        if (componentJson.contains("type"))
            output += renderServiceImportsCallback(componentJson);
        else
            output += renderCustomComponentsImportsCallback(componentJson);

        if (componentJson.contains("nestedComponents")
            && componentJson["nestedComponents"].is_array()) {
            nlohmann::json contextWithNested;
            contextWithNested["components"] = componentJson["nestedComponents"];

            output += env.render(R"({{ render_imports(components) }})", contextWithNested);
        }
    }

    return output;
}

std::string renderParamsCallback(inja::Arguments &args)
{
    if (args.empty() || !args[0]->is_string()) {
        fmt::print(stderr, "Invalid argument passed to renderParamsCallback.\n");
        return {};
    }

    const std::string &path = *args[0];
    std::vector<std::string> params;

    std::istringstream iss(path);
    std::string segment;
    while (std::getline(iss, segment, '/')) {
        if (!segment.empty() && segment[0] == ':') {
            params.push_back(segment.substr(1)); // remove ':'
        }
    }

    if (params.empty())
        return {};

    std::string output = "const { ";
    for (size_t i = 0; i < params.size(); ++i) {
        output += params[i];
        if (i != params.size() - 1)
            output += ", ";
    }
    output += " } = useParams();";

    return output;
}

std::string renderStatesCallback(inja::Environment &env, inja::Arguments &args)
{
    // Validar que el argumento sea un array de componentes
    if (args.empty() || !args[0]->is_array()) {
        fmt::print(stderr, "Invalid argument passed to renderStatesCallback.\n");
        return {};
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
                && !props["model"].get<std::string>().empty()) {
                std::string modelName = props["model"];
                std::string lowerModelName = toLower(modelName);

                if (componentType == "Model Layout") {
                    if (props.contains("get") && props["get"].is_string()
                        && !props["get"].get<std::string>().empty()) {
                        std::string get = props["get"];
                        if (get == "ALL" || get == "ID") {
                            // Generar la declaración del estado usando useState
                            output += "const [" + lowerModelName + ", set" + modelName
                                      + "] = useState<" + modelName;
                            if (get == "ALL")
                                output += "[]>([]);\n";
                            if (get == "ID")
                                output += ">();\n";
                        }
                    }

                } else if (componentType == "Form") {
                    if (props.contains("method") && props["method"].is_string()
                        && !props["method"].get<std::string>().empty()
                        && props["method"] != "Method") {
                        std::string modelName = props["model"];
                        std::string method = props["method"];

                        std::string methodCapitalize;

                        if (method == "PUT")
                            methodCapitalize = "Put";
                        else if (method == "POST")
                            methodCapitalize = "Post";

                        std::string lowerMethod = toLower(method);

                        output += "const [" + lowerMethod + modelName + ", set" + methodCapitalize
                                  + modelName + "] = useState<" + modelName + ">(" + modelName
                                  + "Defaults.default" + methodCapitalize + modelName + ");\n";
                    }
                }
            }
        }

        if (componentJson.contains("nestedComponents")
            && componentJson["nestedComponents"].is_array()) {
            nlohmann::json contextWithNested;
            contextWithNested["components"] = componentJson["nestedComponents"];

            output += env.render(R"({{ render_states(components) }})", contextWithNested);
        }
    }

    return output;
}

std::string extractObjectName(const std::string &input)
{
    std::string prefix = "delete";
    std::string suffix = "ById";

    size_t start = input.find(prefix);
    if (start == std::string::npos)
        return "";

    start += prefix.length();
    size_t end = input.find(suffix, start);
    if (end == std::string::npos)
        return "";

    return input.substr(start, end - start);
}

std::string renderHandleFoosCallback(inja::Environment &env, inja::Arguments &args)
{
    // Validar que el argumento sea un array de componentes
    if (args.empty() || !args[0]->is_array()) {
        fmt::print(stderr, "Invalid argument passed to renderHandleFoosCallback.\n");
        return {};
    }

    const nlohmann::json &components = *args[0];
    std::string output;

    for (const auto &componentJson : components) {
        if (componentJson.contains("type") && componentJson["type"] == "Button") {
            const auto &props = componentJson["props"];
            if (props.contains("click") && props["click"].is_string()
                && !props["click"].get<std::string>().empty()) {
                std::string click = props["click"];
                if (click.empty())
                    continue;
                std::string model = extractObjectName(click);
                if (model.empty())
                    continue;

                std::string deleteModel;

                // Generar el deleteModelById()
                deleteModel = "const delete" + model + "ById";
                deleteModel += " = async (id: number) => {\n";
                deleteModel += "  try {\n";
                deleteModel += "    const response = await " + model + "Service.delete" + model
                               + "ById(id);\n";
                deleteModel += "    console.log(\"Element deleted successfully:\", response);\n";
                deleteModel += "  } catch (error) {\n";
                deleteModel += "    console.error(\"Error deleting element:\", error);\n";
                deleteModel += "  }\n";
                deleteModel += "};\n";

                output += deleteModel;
            }
        }
        if (componentJson.contains("type") && componentJson["type"] == "Form") {
            const auto &props = componentJson["props"];
            if (props.contains("model") && props["model"].is_string()
                && !props["model"].get<std::string>().empty()) {
                std::string method = props["method"];
                std::string modelName = props["model"];
                std::string methodCapitalize;

                if (method == "PUT")
                    methodCapitalize = "Put";
                else if (method == "POST")
                    methodCapitalize = "Post";

                std::string handleChange, handleSubmit;

                // Generar el handleChange
                handleChange = "const handleChange" + methodCapitalize + modelName;
                handleChange += " = (e: any) => {\n";
                handleChange += "  const { name, value } = e.target;\n";

                // Generar el handleSubmit
                handleSubmit = "const handleSubmit" + methodCapitalize + modelName;
                handleSubmit += " = async (e: React.FormEvent) => {\n";
                handleSubmit += "  e.preventDefault();\n\n";

                std::string lowerMethod = toLower(method);
                std::string methodService;

                if (method == "PUT")
                    methodService = "update";
                else if (method == "POST")
                    methodService = "create";

                // Agregar código para handleChange
                handleChange += "  set" + methodCapitalize + modelName + "((prevData) => ({\n";
                handleChange += "    ...prevData,\n";
                handleChange += "    [name]: value,\n";
                handleChange += "  }));\n";

                // Agregar código para handleSubmit
                handleSubmit += "  if (!" + lowerMethod + modelName + ") {\n";
                handleSubmit += "  console.error(\"Data is undefined\");\n";
                handleSubmit += "  return;\n";
                handleSubmit += "  }\n\n";
                handleSubmit += "  try {\n";
                handleSubmit += "    const response = await " + modelName + "Service."
                                + methodService + modelName;
                if (method == "PUT")
                    handleSubmit += "ById(" + lowerMethod + modelName + ".id, " + lowerMethod
                                    + modelName + ");\n";
                if (method == "POST")
                    handleSubmit += "(" + lowerMethod + modelName + ");\n";
                handleSubmit += "    console.log(\"Form submitted successfully:\", response);\n";
                handleSubmit += "alert(\"Element" + methodService + "d successfully\");\n";
                if (method == "POST")
                    handleSubmit += "set" + methodCapitalize + modelName + "(" + modelName
                                    + "Defaults.default" + methodCapitalize + modelName + " );\n";
                handleChange += "};\n\n";
                handleSubmit += "  } catch (error) {\n";
                handleSubmit += "    console.error(\"Error submitting form:\", error);\n";
                handleSubmit += "  }\n";
                handleSubmit += "};\n";

                output += handleChange + handleSubmit;
            }
        }

        if (componentJson.contains("nestedComponents")
            && componentJson["nestedComponents"].is_array()) {
            nlohmann::json contextWithNested;
            contextWithNested["components"] = componentJson["nestedComponents"];

            output += env.render(R"({{ render_handles(components) }})", contextWithNested);
        }
    }

    return output;
}

std::string renderRequestsCallback(inja::Environment &env, inja::Arguments &args)
{
    // Validar que el argumento sea un array de componentes
    if (args.empty() || !args[0]->is_array()) {
        fmt::print(stderr, "Invalid argument passed to renderRequestsCallback.\n");
        return {};
    }

    const nlohmann::json &components = *args[0];
    std::string path;
    if (args.size() > 1 && args[1]->is_string()) {
        path = *args[1];
    } else {
        path = "";
    }

    std::string output;

    std::vector<std::string> params;
    if (!path.empty()) {
        std::istringstream iss(path);
        std::string segment;
        while (std::getline(iss, segment, '/')) {
            if (!segment.empty() && segment[0] == ':') {
                params.push_back(segment.substr(1)); // remove ':'
            }
        }
    }

    for (const auto &componentJson : components) {
        if (componentJson.contains("type")) {
            if (componentJson["type"] == "Form") {
                bool hasValidModel = false, hasValidMethod = false;
                std::string modelName;
                std::string lowerModel;
                std::string lowerModelParam;

                const auto &props = componentJson["props"];
                if (props.contains("model") && props["model"].is_string()
                    && !props["model"].get<std::string>().empty()) {
                    modelName = props["model"];
                    lowerModel = toLower(modelName);
                    lowerModelParam = lowerModel + "Id";

                    if (std::find(params.begin(), params.end(), lowerModelParam) != params.end())
                        hasValidModel = true;
                }
                if (props.contains("method") && props["method"].is_string()
                    && !props["method"].get<std::string>().empty()) {
                    std::string method = props["method"];
                    if (method == "PUT")
                        hasValidMethod = true;
                }

                // Si el "Form" no tiene un "prop.model" válido ni un "prop.method" válido, seguir
                if (!hasValidModel || !hasValidMethod) {
                    continue;
                }

                // Solo puede ser este
                std::string methodCapitalize = "Put";

                output += "useEffect(() => {\n";
                output += "  " + modelName + "Service.get" + modelName + "ById(" + lowerModelParam
                          + ")\n";
                output += "    .then((response) => {\n";
                output += "      set" + methodCapitalize + modelName + "(response);\n";
                output += "    })\n";
                output += "    .catch((error) => {\n";
                output += "      console.error(\"Error fetching " + modelName
                          + " data by id:\", error);\n";
                output += "    });\n";
                output += "}, [" + lowerModelParam + "]);\n";
            }
            if (componentJson["type"] == "Model Layout") {
                bool hasValidModel = false, hasValidGet = false;
                std::string get;
                std::string modelName;
                std::string lowerModel;
                std::string lowerModelParam;

                const auto &props = componentJson["props"];
                if (props.contains("model") && props["model"].is_string()
                    && !props["model"].get<std::string>().empty()) {
                    modelName = props["model"];

                    hasValidModel = true;
                }

                if (props.contains("get") && props["get"].is_string()
                    && !props["get"].get<std::string>().empty()) {
                    get = props["get"];
                    if (get == "ALL" || get == "ID") {
                        hasValidGet = true;

                        if (get == "ID") {
                            lowerModel = toLower(modelName);
                            lowerModelParam = lowerModel + "Id";

                            if (std::find(params.begin(), params.end(), lowerModelParam)
                                == params.end())
                                hasValidModel = false;
                        }
                    }
                }

                // Si el "Model Layout" no tiene un "prop.model" válido, seguir
                if (!hasValidModel || !hasValidGet) {
                    continue;
                }

                output += "useEffect(() => {\n";
                if (get == "ALL")
                    output += "  " + modelName + "Service.getAll" + modelName + "()\n";
                else // ID
                    output += "  " + modelName + "Service.get" + modelName + "ById("
                              + lowerModelParam + ")\n";
                output += "    .then((response) => {\n";
                output += "      set" + modelName + "(response);\n";
                output += "    })\n";
                output += "    .catch((error) => {\n";

                output += "      console.error(\"Error fetching " + modelName;
                if (get == "ALL")
                    output += " data:\", error);\n";
                if (get == "ID")
                    output += " data by id:\", error);\n";
                output += "    });\n";
                if (get == "ALL")
                    output += "}, []);";
                if (get == "ID")
                    output += "}, [" + lowerModelParam + "]);\n";
            }
        }

        if (componentJson.contains("nestedComponents")
            && componentJson["nestedComponents"].is_array()) {
            nlohmann::json contextWithNested;
            contextWithNested["components"] = componentJson["nestedComponents"];
            contextWithNested["path"] = path;

            output += env.render(R"({{ render_requests(components, path) }})", contextWithNested);
        }
    }

    return output;
}

std::string renderTypeFrontendModel(inja::Environment &env, inja::Arguments &args)
{
    if (args.empty() || !args[0]->is_string()) {
        fmt::print(stderr, "Invalid argument passed to renderTypeFrontendModel.\n");
        return {};
    }

    const nlohmann::json &type = *args[0];

    static const std::unordered_map<std::string, std::string> typeMap = {
        {"STRING", "string"},
        {"TEXT", "string"},
        {"CHAR", "string"},
        {"DATE", "string"},
        {"DATEONLY", "string"},
        {"TIME", "string"},
        {"BOOLEAN", "boolean"},
        {"INTEGER", "number"},
        {"BIGINT", "number"},
        {"FLOAT", "number"},
        {"DOUBLE", "number"},
        {"DECIMAL", "number"},
    };

    auto it = typeMap.find(type);
    if (it != typeMap.end()) {
        return it->second;
    }

    // Default type
    return "any";
}

std::string renderDefaultTypeFrontendModel(inja::Environment &env, inja::Arguments &args)
{
    if (args.empty() || !args[0]->is_string()) {
        fmt::print(stderr, "Invalid argument passed to renderDefaultTypeFrontendModel.\n");
        return {};
    }

    const nlohmann::json &type = *args[0];

    static const std::unordered_map<std::string, std::string> typeMap = {
        {"STRING", "\"\""},
        {"TEXT", "\"\""},
        {"CHAR", "\"\""},
        {"DATE", "\"\""},
        {"DATEONLY", "\"\""},
        {"TIME", "\"\""},
        {"BOOLEAN", "false"},
        {"INTEGER", "0"},
        {"BIGINT", "0"},
        {"FLOAT", "0"},
        {"DOUBLE", "0"},
        {"DECIMAL", "0"},
    };

    auto it = typeMap.find(type);
    if (it != typeMap.end()) {
        return it->second;
    }

    // Default type
    return "\"\"";
}
} // namespace RenderCallback
