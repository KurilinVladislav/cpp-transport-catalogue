#pragma once

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stdexcept>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
   /* Реализуйте Node, используя std::variant */
    using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

    template<typename T>
    Node(T val);
    
    Node(std::string_view s);

    Node()
        : value_(nullptr) {
    }

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    const Value& GetValue() const;
    
    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;

private:
    Value value_;
    
};
    
template<typename T>
Node::Node(T val)
    : value_(std::move(val)) {
}


class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
