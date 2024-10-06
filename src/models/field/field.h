#ifndef FIELD_H
#define FIELD_H
#include <string>

class Field
{
public:
    Field(const Field &field);
    Field(std::string &name, std::string &type, bool isNull, bool isUnique);
    Field();

    // Getters
    std::string getName() const;
    std::string getType() const;
    bool getIsNull() const;
    bool getIsUnique() const;

    // Setters
    void setName(const std::string &newName);
    void setType(const std::string &newType);
    void setIsNull(const bool &newIsNull);
    void setIsUnique(const bool &newIsUnique);

private:
    std::string name;
    std::string type;
    bool isNull;
    bool isUnique;
};

#endif // FIELD_H
