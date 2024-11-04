#ifndef FIELD_H
#define FIELD_H
#include <string>

class Field
{
public:
    Field(const Field &field);
    Field(std::string &name,
          std::string &type,
          bool isNull,
          bool isUnique,
          const std::string &foreignKeyTable);
    Field();

    // Getters
    std::string getName() const;
    std::string getType() const;
    bool getIsNull() const;
    bool getIsUnique() const;
    bool isPrimaryKey() const;
    bool isForeignKey() const; // Añadido para FK
    bool getHasCheck() const;  // Nuevo getter para CHECK
    bool getHasDefault() const; // Nuevo getter para DEFAULT
    std::string getForeignKeyTable() const;
    std::string getForeignKeyTableLower() const; // Getter para tabla FK en minúsculas

    // Setters
    void setName(const std::string &newName);
    void setType(const std::string &newType);
    void setIsNull(const bool &newIsNull);
    void setIsUnique(const bool &newIsUnique);
    void setIsPrimaryKey(bool value);
    void setIsForeignKey(bool value); // Añadido para FK
    void setHasCheck(bool value);     // Nuevo setter para CHECK
    void setHasDefault(bool value);   // Nuevo setter para DEFAULT
    void setForeignKeyTable(const std::string &tableName);
    void setForeignKeyTableLower(
        const std::string &tableNameLower); // Setter para tabla FK en minúsculas

private:
    std::string name;
    std::string type;
    bool isNull;
    bool isUnique;
    bool primaryKey;
    bool foreignKey; // Añadido para FK
    // Nuevos atributos para CHECK y DEFAULT
    bool hasCheck;
    bool hasDefault;
    std::string foreignKeyTable; // Nuevo campo para almacenar la tabla relacionada
    std::string foreignKeyTableLower; // Nuevo campo para almacenar la tabla relacionada en minúsculas
};

#endif // FIELD_H
