#include "field.h"

// Constructor with parameters
Field::Field(std::string &name, std::string &type, bool isNull, bool isUnique)
    : name(name)
    , type(type)
    , isNull(isNull)
    , isUnique(isUnique)
{}

// Copy constructor
Field::Field(const Field &field)
    : name(field.name)
    , type(field.type)
    , isNull(field.isNull)
    , isUnique(field.isUnique)
{}

// Default constructor
Field::Field()
    : name("")
    , type("")
    , isNull(false)
    , isUnique(false)
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
