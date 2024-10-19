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
    bool isPrimaryKey() const;
    bool isForeignKey() const; // Añadido para FK

    // Setters
    void setName(const std::string &newName);
    void setType(const std::string &newType);
    void setIsNull(const bool &newIsNull);
    void setIsUnique(const bool &newIsUnique);
    void setIsPrimaryKey(bool value);
    void setIsForeignKey(bool value); // Añadido para FK

private:
    std::string name;
    std::string type;
    bool isNull;
    bool isUnique;
    bool primaryKey;
    bool foreignKey; // Añadido para FK
};

#endif // FIELD_H
