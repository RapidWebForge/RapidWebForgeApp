#ifndef RENDERCALLBACK_H
#define RENDERCALLBACK_H

#include <inja/inja.hpp>
#include <string>
#include <unordered_map>

namespace RenderCallback {

extern std::unordered_map<std::string, nlohmann::json> customComponentsCache;

std::string renderCustomComponent(const nlohmann::json componentJson);
std::string renderComponent(inja::Environment &env,
                            const nlohmann::json componentJson,
                            std::string type,
                            std::string parentType);
std::string renderComponentCallback(inja::Environment &env, inja::Arguments &args);
std::string renderServiceImportsCallback(const nlohmann::json componentJson);
std::string renderCustomComponentsImportsCallback(const nlohmann::json componentJson);
std::string renderImportsCallback(inja::Environment &env, inja::Arguments &args);
std::string renderStatesCallback(inja::Environment &env, inja::Arguments &args);
std::string renderHandleFoosCallback(inja::Environment &env, inja::Arguments &args);
std::string renderRequestsCallback(inja::Environment &env, inja::Arguments &args);

std::string renderTypeFrontendModel(inja::Environment &env, inja::Arguments &args);
} // namespace RenderCallback

#endif // RENDERCALLBACK_H
