#ifndef TRANSACTION_H
#define TRANSACTION_H
#include "../field/field.h"
#include <string>
#include <vector>

class Transaction
{
public:
    Transaction(const Transaction &transaction);
    Transaction(int id, std::string &name, std::vector<Field> fields);
    Transaction();

    // Getters
    int getId() const;
    std::string getName() const;
    std::string getNameConst() const;
    const std::vector<Field> &getFields() const;
    std::vector<Field> &getFields();

    // Setters
    void setName(const std::string &newName);
    void setNameConst(const std::string &newNameConst);
    void setFields(const std::vector<Field> &newFields);

    bool isDifferentFrom(const Transaction &other) const;

    // Fields Operations
    Field &getFieldByName(const std::string &fieldName);
    void removeFieldByName(const std::string &fieldName);
    void addField(const Field field);

private:
    int id;
    std::string name;
    std::string nameConst;
    std::vector<Field> fields;
};

#endif // TRANSACTION_H
