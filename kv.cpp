#include "kv.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cctype>

namespace ml {

class KVBuilder {
public:
    static std::unique_ptr<KV> Make(const std::string& key) {
        return std::unique_ptr<KV>(new KV(key));
    }
    static void SetValue(KV& n, const std::string& v) { n.value_ = v; }
    static void AddChild(KV& n, std::unique_ptr<KV> c) { n.children_.push_back(std::move(c)); }
};

namespace {

struct Tokenizer {
    const std::string& s;
    size_t pos = 0;
    std::string source;

    Tokenizer(const std::string& str, std::string src)
        : s(str), source(std::move(src)) {}

    void SkipWs() {
        // Срезаем UTF-8 BOM
        if (pos == 0 && s.size() >= 3 &&
            (unsigned char)s[0] == 0xEF &&
            (unsigned char)s[1] == 0xBB &&
            (unsigned char)s[2] == 0xBF) {
            pos = 3;
        }
        while (pos < s.size()) {
            char c = s[pos];
            if (std::isspace((unsigned char)c)) { pos++; continue; }
            if (c == '/' && pos + 1 < s.size() && s[pos + 1] == '/') {
                while (pos < s.size() && s[pos] != '\n') pos++;
                continue;
            }
            if (c == '/' && pos + 1 < s.size() && s[pos + 1] == '*') {
                pos += 2;
                while (pos + 1 < s.size() && !(s[pos] == '*' && s[pos + 1] == '/')) pos++;
                if (pos + 1 < s.size()) pos += 2;
                continue;
            }
            break;
        }
    }

    bool Eof() { SkipWs(); return pos >= s.size(); }

    std::string Next() {
        SkipWs();
        if (pos >= s.size()) return "";

        if (s[pos] == '"') {
            pos++;
            std::string out;
            while (pos < s.size() && s[pos] != '"') {
                if (s[pos] == '\\' && pos + 1 < s.size()) {
                    pos++;
                    char c = s[pos++];
                    switch (c) {
                        case 'n': out += '\n'; break;
                        case 't': out += '\t'; break;
                        case '"': out += '"';  break;
                        case '\\': out += '\\'; break;
                        default: out += c; break;
                    }
                } else {
                    out += s[pos++];
                }
            }
            if (pos < s.size()) pos++;
            return out;
        }

        std::string out;
        while (pos < s.size() && !std::isspace((unsigned char)s[pos]) &&
               s[pos] != '{' && s[pos] != '}' && s[pos] != '"') {
            out += s[pos++];
        }
        return out;
    }

    char Peek() {
        SkipWs();
        return pos < s.size() ? s[pos] : '\0';
    }

    void Eat() { if (pos < s.size()) pos++; }
};

std::unique_ptr<KV> ParseBlock(Tokenizer& t) {
    std::string key = t.Next();
    if (key.empty()) return nullptr;

    auto node = KVBuilder::Make(key);

    char c = t.Peek();
    if (c == '{') {
        t.Eat();
        while (true) {
            char p = t.Peek();
            if (p == '\0') {
                throw std::runtime_error("KV: unexpected EOF, missing '}' in " + t.source);
            }
            if (p == '}') { t.Eat(); break; }
            auto child = ParseBlock(t);
            if (child) KVBuilder::AddChild(*node, std::move(child));
        }
    } else {
        std::string val = t.Next();
        KVBuilder::SetValue(*node, val);
    }

    return node;
}

} // namespace

const KV* KV::Find(const std::string& key) const {
    for (auto& c : children_) {
        if (c->key_ == key) return c.get();
    }
    return nullptr;
}

bool KV::GetBool(const std::string& key, bool def) const {
    const KV* n = Find(key);
    if (!n) return def;
    const std::string& v = n->value_;
    if (v == "1" || v == "true"  || v == "yes" || v == "on")  return true;
    if (v == "0" || v == "false" || v == "no"  || v == "off") return false;
    return def;
}

int KV::GetInt(const std::string& key, int def) const {
    const KV* n = Find(key);
    if (!n) return def;
    try { return std::stoi(n->value_); } catch (...) { return def; }
}

float KV::GetFloat(const std::string& key, float def) const {
    const KV* n = Find(key);
    if (!n) return def;
    try { return std::stof(n->value_); } catch (...) { return def; }
}

std::string KV::GetString(const std::string& key, const std::string& def) const {
    const KV* n = Find(key);
    return n ? n->value_ : def;
}

void KV::Dump(int indent) const {
    std::string pad(indent * 2, ' ');
    if (children_.empty()) {
        std::cout << pad << key_ << " = \"" << value_ << "\"\n";
    } else {
        std::cout << pad << key_ << " {\n";
        for (auto& c : children_) c->Dump(indent + 1);
        std::cout << pad << "}\n";
    }
}

std::unique_ptr<KV> KV::ParseString(const std::string& text, const std::string& source) {
    Tokenizer t(text, source);
    return ParseBlock(t);
}

std::unique_ptr<KV> KV::ParseFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        std::cerr << "KV: cannot open " << path << "\n";
        return nullptr;
    }
    std::stringstream ss;
    ss << f.rdbuf();
    try {
        return ParseString(ss.str(), path);
    } catch (const std::exception& e) {
        std::cerr << "KV parse error: " << e.what() << "\n";
        return nullptr;
    }
}

} // namespace ml
