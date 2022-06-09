#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);
    
void SkipWhiteSpaces(istream& input) {
    //std::cout << "Skip" << std::endl;
    for(char c; input >> c; ) {
        if (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\v') {
            // do nothing
        } else {
            input.putback(c);
            return;
        }
    }
}

Node LoadArray(istream& input) {
    //std::cout << "LoadArray" << std::endl;
    SkipWhiteSpaces(input);
    Array result;

    bool correct = false;
    for (char c; input;) {
        c = static_cast<char>(input.get());
        if (c != ',') {
            input.putback(c);
        }
        if (c == ']') {
            input.get(); // ']'
            correct = true;
            break;
        }
        result.push_back(LoadNode(input));
        SkipWhiteSpaces(input);
    }
    if (correct == false) {
        throw ParsingError("Unexpected end of array");
    }

    return Node(move(result));
}

//using Number = std::variant<int, double>;

Node LoadNumber(std::istream& input) {
    //std::cout << "LoadNumber" << std::endl;
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected: \""s + static_cast<char>(input.peek()) + "\""s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    //std::cout << "LoadString" << std::endl;
    using namespace std::literals;
    
    bool correct = false;
    std::string s;
    char c;
    while (input) {
        c = static_cast<char>(input.get());
        if (c == '"') {
            // Встретили закрывающую кавычку
            correct = true;
            break;
        } else if (c == '\\') {
            // Встретили начало escape-последовательности
            input >> c;
            if (!c) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error"s);
            }
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (c) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + c);
            }
        } else if (c == '\n' || c == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(c);
        }
    }
    if (correct == false) {
        throw ParsingError("Unexpected end of string"s);
    }

    return Node(move(s));
}

Node LoadDict(istream& input) {
    //std::cout << "LoadDict:" << std::endl;
    SkipWhiteSpaces(input);
    Dict result = {};

    bool correct = false;
    for (char c; input;) {
        c = static_cast<char>(input.get());
        if (c != ',') {
            input.putback(c);
        }
        if (c == '}') {
            input.get(); // '}'
            correct = true;
            break;
        }

        string key = LoadNode(input).AsString();
        SkipWhiteSpaces(input);
        input >> c; // skip ':'
        result.insert({move(key), LoadNode(input)});
        SkipWhiteSpaces(input);
    }
    if (correct == false) {
        throw ParsingError("Unexpected end of Dict");
    }
    return Node(move(result));
}
    
Node LoadNullOrBool(istream& input) {
    //std::cout << "LoadNullOrBool" << std::endl;
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    
    std::string s;
    while (true) {
        if (it == end) {
            break;
        }
        const char ch = *it;
        if (!std::isalpha(input.peek())) {
            //++it;
            break;
        } else {
            s.push_back(ch);
        }
        ++it;
    }
    
    if (s == "null") {
        return Node(nullptr);
    } else if (s == "true") {
        return Node(true);
    } else if (s == "false") {
        return Node(false);
    } else {
        throw ParsingError("Expected null or bool, not \"" + s + "\"");
    } 
}

Node LoadNode(istream& input) {
    //std::cout << "LoadNode" << std::endl;
    SkipWhiteSpaces(input);
    char c;
    input >> c;
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else {
        input.putback(c);
        if (c == 'n' || c == 't' || c == 'f') {
            return LoadNullOrBool(input);
        } else {
            return LoadNumber(input);
        }
    }
}

}  // namespace
    
////////////////////   Node   //////////////////////

Node::Node(std::string_view s)
    : value_(std::move(static_cast<std::string>(s))){
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}
    
bool Node::IsDouble() const {
    return std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_);
}
    
bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}
    
bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}
    
bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}
    
bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}
    
bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}
    
bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_);
}
    
int Node::AsInt() const {
    if(!IsInt()) {
        throw std::logic_error{"Node is not int type"};
    }
    return std::get<int>(value_);
}
    
bool Node::AsBool() const {
    if(!IsBool()) {
        throw std::logic_error{"Node is not bool type"};
    }
    return std::get<bool>(value_);
}
    
double Node::AsDouble() const {
    if(!IsDouble()) {
        throw std::logic_error{"Node is not int or double type"};
    }
    if (IsInt()) {
        return std::get<int>(value_);
    }
    return std::get<double>(value_);
}
    
const std::string& Node::AsString() const {
    if(!IsString()) {
        throw std::logic_error{"Node is not string type"};
    }
    return std::get<std::string>(value_);
}
    
const Array& Node::AsArray() const {
    if(!IsArray()) {
        throw std::logic_error{"Node is not array type"};
    }
    return std::get<Array>(value_);
}
    
const Dict& Node::AsMap() const {
    if(!IsMap()) {
        throw std::logic_error{"Node is not map type"};
    }
    return std::get<Dict>(value_);
}
    
using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;
const Value& Node::GetValue() const {
    return value_;
}
    
bool Node::operator==(const Node& other) const {
    return value_ == other.value_;
}
    
bool Node::operator!=(const Node& other) const {
    return value_ != other.value_;
}
    
///////////////////   Document   ///////////////////////////

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}
    
bool Document::operator==(const Document& other) const {
    return root_ == other.root_;
}
    
bool Document::operator!=(const Document& other) const {
    return root_ != other.root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

////////////////////  Printing  ///////////////////////////

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }
    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

void PrintNode(const Node& node, const PrintContext& ctx);

void PrintValue(const std::string& s, const PrintContext& ctx) {
    ctx.out << "\""sv;
    for(char c: s) {
        if (c == '\n') {
            ctx.out << "\\n"sv;
        } else if (c == '\r') {
            ctx.out << "\\r"sv;
        } else if (c == '\"') {
            ctx.out << "\\"sv << "\""sv;
        } else if (c == '\\') {
            ctx.out << "\\\\"sv;
        } else {
            ctx.out << c;
        }
    }
    ctx.out << "\""sv;
}

// Шаблон, подходящий для вывода double и int
template <typename Number>
void PrintValue(const Number& value, const PrintContext& ctx) {
    ctx.out << value;
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

void PrintValue(bool val, const PrintContext& ctx) {
    if(val == true) {
        ctx.out << "true"sv;
    } else {
        ctx.out << "false"sv;
    }
}

void PrintValue(const Array& array, const PrintContext& ctx) {
    ctx.out << "["sv << std::endl;
    bool first = true;
    for (auto& val: array) {
        if (first == false) {
            ctx.Indented().out << ","sv << std::endl;
        } else {
            first = false;
        }
        ctx.Indented().PrintIndent();
        PrintNode(val, ctx.Indented());
    }
    ctx.out << std::endl;
    ctx.PrintIndent();
    ctx.out << "]"sv;
}

void PrintValue(const Dict& dict, const PrintContext& ctx) {
    ctx.out << "{"sv << std::endl;
    bool first = true;
    for (auto& [key,val]: dict) {
        if (first == false) {
            ctx.Indented().out << ","sv << std::endl;
        } else {
            first = false;
        }
        ctx.Indented().PrintIndent();
        PrintValue(key, ctx.Indented());
        ctx.out << ": "sv;
        PrintNode(val, ctx.Indented());
    }
    ctx.out << std::endl;
    ctx.PrintIndent();
    ctx.out << "}"sv;
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value){ PrintValue(value, ctx); },
        node.GetValue());
}
    
void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{output});
}

}  // namespace json
