#ifndef RENDERCALLBACK_H
#define RENDERCALLBACK_H

#include <inja/inja.hpp>
#include <string>

namespace RenderCallback {
std::string renderComponentCallback(inja::Environment &env, inja::Arguments &args);
std::string renderServiceImportsCallback(inja::Environment &env, inja::Arguments &args);
std::string renderStatesCallback(inja::Environment &env, inja::Arguments &args);
std::string renderRequestsCallback(inja::Environment &env, inja::Arguments &args);
} // namespace RenderCallback

#endif // RENDERCALLBACK_H
