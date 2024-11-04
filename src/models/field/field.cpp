#include "field.h"
#include <algorithm> // Incluye esta línea para usar std::transform

// Constructor with parameters
Field::Field(std::string &name,
             std::string &type,
             bool isNull,
             bool isUnique,
             const std::string &foreignKeyTable)
    : name(name)
    , type(type)
    , isNull(isNull)
    , isUnique(isUnique)
    , primaryKey(false)                     // Inicializar como false por defecto
    , foreignKey(false)                     // Inicializar como false por defecto
    , hasCheck(false)                       // Inicializar como false por defecto
    , hasDefault(false)                     // Inicializar como false por defecto
    , foreignKeyTable(foreignKeyTable)      // Inicializa con el nombre de la tabla de Foreign Key
    , foreignKeyTableLower(foreignKeyTable) // Inicializa también la tabla en minúsculas

{
    // Convertir foreignKeyTableLower a minúsculas
    std::transform(foreignKeyTableLower.begin(),
                   foreignKeyTableLower.end(),
                   foreignKeyTableLower.begin(),
                   ::tolower);
}
// Copy constructor
Field::Field(const Field &field)
    : name(field.name)
    , type(field.type)
    , isNull(field.isNull)
    , isUnique(field.isUnique)
    , primaryKey(field.primaryKey)
    , foreignKey(field.foreignKey)
    , hasCheck(field.hasCheck)
    , hasDefault(field.hasDefault)
    , foreignKeyTable(field.foreignKeyTable) // Asegurarse de copiar la tabla de Foreign Key
    , foreignKeyTableLower(
          field.foreignKeyTableLower) // Asegurarse de copiar la tabla en minúsculas también

{}

// Default constructor
Field::Field()
    : name("")
    , type("")
    , isNull(false)
    , isUnique(false)
    , primaryKey(false)
    , foreignKey(false)
    , hasCheck(false)
    , hasDefault(false)
    , foreignKeyTable("")
    , foreignKeyTableLower("")

{}
// Getters
std::string Field::getName() const
{
    return name;
}

std::string Field::getType() const
{
    return type;
}

bool Field::getIsNull() const
{
    return isNull;
}

bool Field::getIsUnique() const
{
    return isUnique;
}

bool Field::isPrimaryKey() const
{
    return primaryKey;
}

bool Field::isForeignKey() const
{ // Getter para FK
    return foreignKey;
}
bool Field::getHasCheck() const
{
    return hasCheck;
}

bool Field::getHasDefault() const
{
    return hasDefault;
}

std::string Field::getForeignKeyTable() const
{
    return foreignKeyTable;
}
std::string Field::getForeignKeyTableLower() const
{
    return foreignKeyTableLower; // Nuevo getter para el nombre en minúsculas
}

// Setters
void Field::setName(const std::string &newName)
{
    name = newName;
}

void Field::setType(const std::string &newType)
{
    type = newType;
}

void Field::setIsNull(const bool &newIsNull)
{
    isNull = newIsNull;
}

void Field::setIsUnique(const bool &newIsUnique)
{
    isUnique = newIsUnique;
}

void Field::setIsPrimaryKey(bool value)
{
    primaryKey = value;
}

void Field::setIsForeignKey(bool value)
{ // Setter para FK
    foreignKey = value;
}
void Field::setHasCheck(bool value)
{
    hasCheck = value;
}

void Field::setHasDefault(bool value)
{
    hasDefault = value;
}
void Field::setForeignKeyTable(const std::string &tableName)
{
    foreignKeyTable = tableName;
    foreignKey = !tableName.empty(); // Si la tabla relacionada no está vacía, es una FK
}
void Field::setForeignKeyTableLower(const std::string &tableNameLower)
{
    foreignKeyTableLower = tableNameLower;
}
