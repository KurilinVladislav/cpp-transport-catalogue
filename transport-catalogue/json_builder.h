#pragma once

#include "json.h"

namespace json {

class BuildContext;
class KeyContext;
class StartDictContext;
class StartArrayContext;
class ValueContext;
class ValueBuildContext;
class ValueAfterKeyContext;
class ValueAfterStartArrayContext;
class EndDictContext;
class EndArrayContext;
    
class Builder {
public:
    const std::vector<Node*>& GetNodesStack() const;
    KeyContext Key(const std::string& s);
    ValueBuildContext Value(Node::Value v);
    ValueAfterKeyContext Value(Node::Value v, int i);
    ValueAfterStartArrayContext Value(Node::Value v, bool i);
    StartDictContext StartDict();
    StartArrayContext StartArray();
    EndDictContext EndDict();
    EndArrayContext EndArray();
    Node Build();
    virtual ~Builder() {}
private:
    Node root_;
    std::vector<Node*> nodes_stack_ = {nullptr};
    bool complete = false;
    void CheckComplete();
};

class BuildContext : public Builder {
public:
    BuildContext(Builder& b);
    KeyContext Key(const std::string& s);
    BuildContext Value(Node::Value v);
    StartDictContext StartDict();
    StartArrayContext StartArray();
    EndDictContext EndDict();
    EndArrayContext EndArray();
    Node Build();
    Builder& GetB();
private:
    Builder& b_;
};

class KeyContext : public BuildContext {
public:
    KeyContext(Builder& b) : BuildContext(b) {}
    //using BuildContext::Value;
    //using BuildContext::StartDict;
    //using BuildContext::StartArray;
    
    ValueAfterKeyContext Value(Node::Value v);
    KeyContext Key(const std::string& s) = delete;
    EndDictContext EndDict() = delete;
    EndArrayContext EndArray() = delete;
    Node Build() = delete;
};
    
class StartDictContext : public BuildContext {
public:
    StartDictContext(Builder& b) : BuildContext(b) {}
    //using BuildContext::Key;
    //using BuildContext::EndDict;
    
    BuildContext Value(Node::Value v) = delete;
    StartDictContext StartDict() = delete;
    StartArrayContext StartArray() = delete;
    EndArrayContext EndArray() = delete;
    Node Build() = delete;
};
    
class StartArrayContext : public BuildContext {
public:
    StartArrayContext(Builder& b) : BuildContext(b) {}
    //using BuildContext::Value;
    //using BuildContext::StartDict;
    //using BuildContext::StartArray;
    //using BuildContext::EndArray;
    
    ValueAfterStartArrayContext Value(Node::Value v);
    KeyContext Key(const std::string& s) = delete;
    EndDictContext EndDict() = delete;
    Node Build() = delete;
};
    
class ValueContext : public BuildContext {
public:
    ValueContext(Builder& b) : BuildContext(b) {}
    /*KeyContext Key(const std::string& s) = delete;
    BuildContext Value(Node::Value v) = delete;
    StartDictContext StartDict() = delete;
    StartArrayContext StartArray() = delete;
    EndDictContext EndDict() = delete;
    EndArrayContext EndArray() = delete;
    Node Build() = delete;*/
};
    
class ValueBuildContext : public ValueContext {
public:
    ValueBuildContext(Builder& b) : ValueContext(b) {}
    //using BuildContext::Build;
    
    KeyContext Key(const std::string& s) = delete;
    BuildContext Value(Node::Value v) = delete;
    StartDictContext StartDict() = delete;
    StartArrayContext StartArray() = delete;
    EndDictContext EndDict() = delete;
    Builder& EndArray() = delete;
};
    
class ValueAfterKeyContext : public ValueContext {
public:
    ValueAfterKeyContext(Builder& b) : ValueContext(b) {}
    //using BuildContext::Key;
    //using BuildContext::EndDict;
    
    BuildContext Value(Node::Value v) = delete;
    StartDictContext StartDict() = delete;
    StartArrayContext StartArray() = delete;
    EndArrayContext EndArray() = delete;
    Node Build() = delete;
};
    
class ValueAfterStartArrayContext : public ValueContext {
public:
    ValueAfterStartArrayContext(Builder& b) : ValueContext(b) {}
    //using BuildContext::Value;
    //using BuildContext::StartDict;
    //using BuildContext::StartArray;
    //using BuildContext::EndArray;
    
    ValueAfterStartArrayContext Value(Node::Value v);
    KeyContext Key(const std::string& s) = delete;
    EndDictContext EndDict() = delete;
    Node Build() = delete;
};
    
class EndDictContext: public BuildContext {
public:
    EndDictContext(Builder& b) : BuildContext(b) {}
    ValueAfterStartArrayContext Value(Node::Value v);
};
    
class EndArrayContext: public BuildContext {
public:
    EndArrayContext(Builder& b) : BuildContext(b) {}
    ValueAfterStartArrayContext Value(Node::Value v);
};
    
} // end namespace json