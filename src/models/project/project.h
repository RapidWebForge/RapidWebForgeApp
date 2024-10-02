#ifndef PROJECT_H
#define PROJECT_H

#include <chrono>
#include <string>

class Project
{
private:
    int id;
    std::string name;
    std::string description;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point updatedAt;

public:
    // Constructors
    Project(int id, const std::string &name, const std::string &description);
    Project();

    // Getters
    int getId() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getCreatedAt() const;
    std::string getUpdatedAt() const;

    // Setters
    void setName(const std::string &newName);
    void setDescription(const std::string &newDescription);
    void setUpdatedAt();
};

#endif // PROJECT_H
