#include "field.h"

// Constructor with parameters
Field::Field(std::string &name, std::string &type, bool isNull, bool isUnique)
    : name(name)
    , type(type)
    , isNull(isNull)
    , isUnique(isUnique)
    , primaryKey(false) // Inicializar como false por defecto
    , foreignKey(false) // Inicializar como false por defecto
{}
// Copy constructor
Field::Field(const Field &field)
    : name(field.name)
    , type(field.type)
    , isNull(field.isNull)
    , isUnique(field.isUnique)
    , primaryKey(field.primaryKey)
    , foreignKey(field.foreignKey)
{}

// Default constructor
Field::Field()
    : name("")
    , type("")
    , isNull(false)
    , isUnique(false)
    , primaryKey(false)
    , foreignKey(false)
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
