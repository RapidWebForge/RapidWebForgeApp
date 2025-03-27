#ifndef SECTION_H
#define SECTION_H

#include "../base-node/basenode.h"
#include <memory>
#include <string>
#include <vector>

class Section : public BaseNode
{
private:
    std::string name;
    std::string path;

public:
    Section();
    explicit Section(const std::string &name);
    explicit Section(const std::string &name, const std::string &path);
    explicit Section(const std::string &name,
                     boost::uuids::uuid id,
                     std::chrono::system_clock::time_point createdOn,
                     std::chrono::system_clock::time_point updatedOn);
    explicit Section(const std::string &name,
                     const std::string &path,
                     boost::uuids::uuid id,
                     std::chrono::system_clock::time_point createdOn,
                     std::chrono::system_clock::time_point updatedOn);

    // From BaseNode
    void generateCode(inja::Environment &env) const override;
    void updateFromJson(const nlohmann::json &json) override;
    std::shared_ptr<BaseNode> clone() const override;
    bool isDifferentFrom(const std::shared_ptr<BaseNode> &other) const override;
    // Getters
    std::string getName() const;
    std::string getPath() const;
    // Setters
    void setName(const std::string &name);
    void setPath(const std::string &path);
};

#endif // SECTION_H
