#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace ml {

class KVBuilder;

class KV {
public:
    KV() = default;
    KV(std::string key) : key_(std::move(key)) {}

    static std::unique_ptr<KV> ParseFile(const std::string& path);
    static std::unique_ptr<KV> ParseString(const std::string& text, const std::string& source = "<string>");

    const std::string& Key()   const { return key_; }
    const std::string& Value() const { return value_; }

    const std::vector<std::unique_ptr<KV>>& Children() const { return children_; }
    const KV* Find(const std::string& key) const;

    bool  GetBool (const std::string& key, bool  def = false) const;
    int   GetInt  (const std::string& key, int   def = 0)     const;
    float GetFloat(const std::string& key, float def = 0.0f)  const;
    std::string GetString(const std::string& key, const std::string& def = "") const;

    void Dump(int indent = 0) const;

private:
    friend class KVBuilder;
    std::string key_;
    std::string value_;
    std::vector<std::unique_ptr<KV>> children_;
};

} // namespace ml
