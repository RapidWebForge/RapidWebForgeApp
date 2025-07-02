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
    bool isForeignKey() const;
    bool getHasCheck() const;
    bool getHasDefault() const;
    std::string getForeignKeyTable() const;
    std::string getForeignKeyTableLower() const;

    // Setters
    void setName(const std::string &newName);
    void setType(const std::string &newType);
    void setIsNull(const bool &newIsNull);
    void setIsUnique(const bool &newIsUnique);
    void setIsPrimaryKey(bool value);
    void setIsForeignKey(bool value);
    void setHasCheck(bool value);
    void setHasDefault(bool value);
    void setForeignKeyTable(const std::string &tableName);
    void setForeignKeyTableLower(const std::string &tableNameLower);

    const bool isDifferentFrom(const Field &other) const;

private:
    std::string name;
    std::string type;
    bool isNull;
    bool isUnique;
    bool primaryKey;
    bool foreignKey;
    bool hasCheck;
    bool hasDefault;
    std::string foreignKeyTable;
    std::string foreignKeyTableLower;
};

#endif // FIELD_H
