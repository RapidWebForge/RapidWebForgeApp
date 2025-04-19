#include "transaction.h"
#include <boost/algorithm/string.hpp>

// Copy Constructor
Transaction::Transaction(const Transaction &transaction)
    : id(transaction.getId())
    , name(transaction.getName())
    , nameConst(transaction.getNameConst())
    , fields(transaction.getFields())
{}

// Parameterized Constructor
Transaction::Transaction(int id, std::string &name, std::vector<Field> fields)
    : id(id)
    , name(name)
    , nameConst(boost::to_lower_copy(name))
    , fields(fields)
{}

// Default Constructor
Transaction::Transaction()
    : id(0)
    , name("")
    , nameConst("")
    , fields()
{}

// Getters
int Transaction::getId() const
{
    return id;
}

std::string Transaction::getName() const
{
    return name;
}

std::string Transaction::getNameConst() const
{
    return nameConst;
}

const std::vector<Field> &Transaction::getFields() const
{
    return fields;
}

std::vector<Field> &Transaction::getFields()
{
    return fields;
}

// Setters
void Transaction::setName(const std::string &newName)
{
    name = newName;
}

void Transaction::setNameConst(const std::string &newNameConst)
{
    nameConst = newNameConst;
}

void Transaction::setFields(const std::vector<Field> &newFields)
{
    fields = newFields;
}

bool Transaction::isDifferentFrom(const Transaction &other) const
{
    // if (this->getName() != other.getName())
    //     return true;

    if (this->getFields().size() != other.getFields().size())
        return true;

    for (const auto &fieldA : this->getFields()) {
        for (const auto &fieldB : other.getFields()) {
            if (fieldA.getName() == fieldB.getName()) {
                if (fieldA.isDifferentFrom(fieldB)) {
                    return true;
                }
            }
        }
    }

    return false;
}
