#include "basenode.h"
#include "../../models/section/section.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iomanip>
#include <openssl/evp.h> // Para la API de EVP
#include <openssl/sha.h> // Para SHA256_DIGEST_LENGTH
#include <sstream>

BaseNode::BaseNode()
    : nodeType("Unknown")
{}

BaseNode::BaseNode(const std::string &nodeType,
                   std::chrono::system_clock::time_point createdOn,
                   std::chrono::system_clock::time_point updatedOn)
    : nodeType(nodeType)
    , createdOn(createdOn)
    , updatedOn(updatedOn)
{}

BaseNode::BaseNode(const std::string &nodeType,
                   boost::uuids::uuid id,
                   std::chrono::system_clock::time_point createdOn,
                   std::chrono::system_clock::time_point updatedOn)
    : nodeType(nodeType)
    , id(id)
    , createdOn(createdOn)
    , updatedOn(updatedOn)
{}

const std::string &BaseNode::getNodeType() const
{
    return nodeType;
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

void BaseNode::generateUniqueId(std::string type,
                                bool allowItems,
                                std::map<std::string, std::string> props)
{
    std::stringstream dataStream;
    dataStream << std::chrono::system_clock::to_time_t(createdOn)
               << std::chrono::system_clock::to_time_t(updatedOn);

    if (!type.empty())
        dataStream << type << allowItems;

    if (!props.empty()) {
        for (const auto &[key, value] : props) {
            dataStream << key << value;
        }
    }

    std::string data = dataStream.str();
    std::string hash = generateSHA256(data);
    id = hashToUUID(hash);
}

void BaseNode::update()
{
    updatedOn = std::chrono::system_clock::now();
}

// Child

void BaseNode::addChild(const std::shared_ptr<BaseNode> &child)
{
    children.push_back(child);
    child->setParent(shared_from_this());
}

void BaseNode::insertChild(int index, const std::shared_ptr<BaseNode> &child)
{
    if (index < 0 || index > children.size()) {
        throw std::out_of_range("Index out of bounds");
    }
    children.insert(children.begin() + index, child);
    child->setParent(shared_from_this());
}

void BaseNode::removeChild(int index)
{
    if (index < 0 || index >= children.size()) {
        throw std::out_of_range("Index out of bounds");
    }
    children[index]->setParent(nullptr);
    children.erase(children.begin() + index);
}

void BaseNode::removeChild(std::vector<std::shared_ptr<BaseNode>>::iterator it)
{
    if (it == children.end()) {
        throw std::out_of_range("Iterator points to end of children vector");
    }

    (*it)->setParent(nullptr); // Desvincula el padre del hijo a eliminar
    children.erase(it);        // Borra el hijo del vector
}

void BaseNode::clearChildren()
{
    for (auto &child : children) {
        child->setParent(nullptr);
    }
    children.clear();
}

std::vector<std::shared_ptr<BaseNode>> &BaseNode::getChildren()
{
    return children;
}

const std::vector<std::shared_ptr<BaseNode>> &BaseNode::getChildren() const
{
    return children;
}

bool BaseNode::removeChildForById(std::string id)
{
    auto it = std::find_if(children.begin(),
                           children.end(),
                           [&id](const std::shared_ptr<BaseNode> &node) {
                               return node && boost::uuids::to_string(node->getId()) == id;
                           });

    if (it != children.end()) {
        children.erase(it);
        return true;
    }
    return false;
}

// Parent

void BaseNode::setParent(const std::shared_ptr<BaseNode> &parentNode)
{
    parent = parentNode;
}

std::shared_ptr<BaseNode> BaseNode::getParent() const
{
    return parent.lock();
}

// Get

std::chrono::system_clock::time_point BaseNode::getCreatedOn() const
{
    return createdOn;
}

std::chrono::system_clock::time_point BaseNode::getUpdatedOn() const
{
    return updatedOn;
}

boost::uuids::uuid BaseNode::getId() const
{
    return id;
}
