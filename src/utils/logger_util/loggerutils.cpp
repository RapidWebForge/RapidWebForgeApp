#include "loggerutils.h"

namespace LoggerUtils {
void LoggerUtils::logPropertyChange(ActionLoggerJson &logger,
                                    const QString &property,
                                    const QString &newValue,
                                    const std::string &componentType)
{
    std::string prop = property.toStdString();
    std::string val = newValue.toStdString();
    std::string type = componentType;

    // Registrar log de la acción
    std::string logMessage = "Property '" + prop + "' updated to '" + val + "'";
    logger.logAction("edit-tag-attributes", logMessage);

    if (prop == "placeholder") {
        logMessage = "Placeholder updated to " + val;
        logger.logAction("edit-placeholder", logMessage);
    }
    if (prop == "maxlength") {
        logMessage = "MaxLength updated to " + val;
        logger.logAction("add-maxlength", logMessage);
    }
    if (prop == "required") {
        logMessage = "Required updated to " + val;
        logger.logAction("add-required", logMessage);
    }
    if (prop == "href") {
        logMessage = "Navigating to view " + val;
        logger.logAction("navigate-between-views", logMessage);

        logMessage = "Configuring hyperlink properties with: " + val;
        logger.logAction("configure-link-properties", logMessage);
    }
    if (prop == "target") {
        logMessage = "Configuring hyperlink properties with: " + val;
        logger.logAction("configure-link-properties", logMessage);
    }
    if (prop == "src" || prop == "alt") {
        logMessage = "Configuring image properties with: " + val;
        logger.logAction("configure-image-properties", logMessage);
    }
    if ((prop == "method" || prop == "model") && type == "Form") {
        logMessage = "Configuring form properties with: " + val;
        logger.logAction("configure-form-properties", logMessage);
    }

    if (prop == "class") {
        if (type == "Button")
            logger.logAction("style-button-tailwind",
                             "User applied styles in class property of a button.");

        // Lista de logs y patrones de Tailwind a detectar
        std::vector<std::pair<std::string, std::string>> tailwindLogs
            = {{"apply-text-styling", "text-"},     {"responsive-text", "text-base"},
               {"responsive-text", "text-xl"},      {"use-flexbox-grid", "flex"},
               {"use-flexbox-grid", "grid"},        {"responsive-columns", "grid"},
               {"use-flexbox-grid", "grid-cols-"},  {"responsive-columns", "grid-cols-"},
               {"responsive-design", "sm:"},        {"responsive-design", "md:"},
               {"responsive-design", "lg:"},        {"responsive-design", "xl:"},
               {"responsive-design", "2xl:"},       {"apply-bg-styling", "bg-"},
               {"add-spacing-tailwind", "m-"},      {"add-spacing-tailwind", "mt-"},
               {"add-spacing-tailwind", "mb-"},     {"add-spacing-tailwind", "mr-"},
               {"add-spacing-tailwind", "ml-"},     {"add-spacing-tailwind", "mx-"},
               {"add-spacing-tailwind", "my-"},     {"add-spacing-tailwind", "p-"},
               {"add-spacing-tailwind", "pt-"},     {"add-spacing-tailwind", "pb-"},
               {"add-spacing-tailwind", "pr-"},     {"add-spacing-tailwind", "pl-"},
               {"add-spacing-tailwind", "px-"},     {"add-spacing-tailwind", "py-"},
               {"responsive-visibility", "hidden"}, {"responsive-visibility", "block"}};

        for (const auto &[logType, pattern] : tailwindLogs) {
            if (val.find(pattern) != std::string::npos) {
                logger.logAction(logType, "User applied '" + pattern + "' in class property.");
                break; // Para evitar múltiples registros del mismo cambio
            }
        }
    }
}
} // namespace LoggerUtils
