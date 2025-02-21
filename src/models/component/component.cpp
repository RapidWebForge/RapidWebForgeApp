#include "component.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iomanip>
#include <openssl/evp.h> // Para la API de EVP
#include <openssl/sha.h> // Para SHA256_DIGEST_LENGTH
#include <sstream>

Component::Component()
    : BaseNode("Component")
    , type(ComponentType::Undefined)
    , createdOn(std::chrono::system_clock::now())
    , updatedOn(createdOn)
{
    initializeDefaultProps();
}

Component::Component(boost::uuids::uuid id,
                     std::chrono::system_clock::time_point createdOn,
                     std::chrono::system_clock::time_point updatedOn)
    : BaseNode("Component")
    , type(ComponentType::Undefined)
    , createdOn(createdOn)
    , updatedOn(updatedOn)
    , id(id)
{
    initializeDefaultProps();
}

Component::Component(ComponentType type)
    : BaseNode("Component")
    , type(type)
    , createdOn(std::chrono::system_clock::now())
    , updatedOn(createdOn)
{
    initializeDefaultProps();
    generateUniqueId();
}

Component::Component(ComponentType type,
                     boost::uuids::uuid id,
                     std::chrono::system_clock::time_point createdOn,
                     std::chrono::system_clock::time_point updatedOn)
    : BaseNode("Component")
    , type(type)
    , createdOn(createdOn)
    , updatedOn(updatedOn)
    , id(id)
{
    initializeDefaultProps();
}

Component::Component(ComponentType type,
                     const std::map<std::string, std::string> &props,
                     bool allowItems)
    : BaseNode("Component")
    , type(type)
    , props(props)
    , allowItems(allowItems)
    , createdOn(std::chrono::system_clock::now())
    , updatedOn(createdOn)
{
    initializeDefaultProps();
    generateUniqueId();
}

// Methods from BaseNode

void Component::generateCode(inja::Environment &env) const
{
    // nlohmann::json data = {{"type", "Component"},
    //                        {"componentType", componentTypeToString(type)},
    //                        {"props", props}};
    // std::string result = env.render("Componente: {{ type }} {{ componentType }}", data);
    // Usar el resultado como necesites
}

void Component::updateFromJson(const nlohmann::json &json)
{
    if (json.contains("type")) {
        setType(stringToComponentType(json["type"]));
    }
    if (json.contains("props")) {
        setProps(json["props"].get<std::map<std::string, std::string>>());
    }
}

std::shared_ptr<BaseNode> Component::clone() const
{
    auto clonedComponent = std::make_shared<Component>(this->type,
                                                       this->id,
                                                       this->createdOn,
                                                       this->updatedOn);
    clonedComponent->props = this->props;
    return clonedComponent;
}

// Función para generar un hash SHA-256
std::string generateSHA256(const std::string &input)
{
    EVP_MD_CTX *context = EVP_MD_CTX_new(); // Crear un contexto para el hash
    if (!context) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    // Inicializar el contexto para usar SHA-256
    if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to initialize SHA-256");
    }

    // Procesar los datos de entrada
    if (EVP_DigestUpdate(context, input.c_str(), input.size()) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to update SHA-256");
    }

    // Obtener el hash resultante
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned int lengthOfHash = 0;
    if (EVP_DigestFinal_ex(context, hash, &lengthOfHash) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to finalize SHA-256");
    }

    // Liberar el contexto
    EVP_MD_CTX_free(context);

    // Convertir el hash a una cadena hexadecimal
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int) hash[i];
    }
    return ss.str();
}

// Función para convertir un hash en un UUID
boost::uuids::uuid hashToUUID(const std::string &hash)
{
    boost::uuids::uuid uuid;
    std::memcpy(&uuid, hash.data(), 16); // Copia los primeros 16 bytes del hash
    return uuid;
}

void Component::generateUniqueId()
{
    std::stringstream dataStream;
    dataStream << componentTypeToString(type) << allowItems
               << std::chrono::system_clock::to_time_t(createdOn)
               << std::chrono::system_clock::to_time_t(updatedOn);

    for (const auto &[key, value] : props) {
        dataStream << key << value;
    }

    std::string data = dataStream.str();
    std::string hash = generateSHA256(data);
    id = hashToUUID(hash);
}

void Component::initializeDefaultProps()
{
    auto it = componentPropertiesMap.find(type);
    if (it != componentPropertiesMap.end()) {
        props = it->second;
    }

    switch (type) {
    case ComponentType::Form:
    case ComponentType::HorizontalLayout:
    case ComponentType::VerticalLayout:
    case ComponentType::ModelLayout:
        this->allowItems = true;
        break;
    default:
        this->allowItems = false;
        break;
    }
}

void Component::update()
{
    updatedOn = std::chrono::system_clock::now();
}

// Getters

ComponentType Component::getType() const
{
    return type;
}

const std::map<std::string, std::string> &Component::getProps() const
{
    return props;
}

bool Component::isAllowingItems() const
{
    return this->allowItems;
}

std::chrono::system_clock::time_point Component::getCreatedOn() const
{
    return createdOn;
}

std::chrono::system_clock::time_point Component::getUpdatedOn() const
{
    return updatedOn;
}

boost::uuids::uuid Component::getId() const
{
    return id;
}

// Setters

void Component::setType(ComponentType type)
{
    this->type = type;
    update();

    initializeDefaultProps();
    if (id.is_nil())
        generateUniqueId();
}

void Component::setProps(const std::map<std::string, std::string> &props)
{
    this->props = props;
    update();
}

void Component::setAllowItems(bool allow)
{
    this->allowItems = allow;
    update();
}

// TODO: update on add components
