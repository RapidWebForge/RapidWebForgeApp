#include "transaction.h"

// Copy Constructor
Transaction::Transaction(const Transaction &transaction)
    : id(transaction.getId())
    , name(transaction.getName())
    , fields(transaction.getFields())
{}

// Parameterized Constructor
Transaction::Transaction(int id, std::string &name, std::vector<Field> fields)
    : id(id)
    , name(name)
    , fields(fields)
{}

// Default Constructor
Transaction::Transaction()
    : id(0)
    , name("")
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

void Transaction::setFields(const std::vector<Field> &newFields)
{
    fields = newFields;
}
