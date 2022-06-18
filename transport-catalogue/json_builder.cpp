#include "json_builder.h"


namespace json {
    
void Builder::CheckComplete() {
    if (complete == true) {
        throw std::logic_error("JSON is already complete");
    }
}
    
const std::vector<Node*>& Builder::GetNodesStack() const {
    return nodes_stack_;
}
    
KeyContext Builder::Key(const std::string& s) {
    CheckComplete();
    if (nodes_stack_.back() == nullptr || !nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Key must be within a map and go first");
    }
    nodes_stack_.emplace_back(&(const_cast<Dict&>(nodes_stack_.back()->AsDict())[std::move(s)]));
    return {*this};
}

    /*
BuildContext& Builder::Value(Node::Value v) {
    CheckComplete();
    if (nodes_stack_.back() == nullptr) {
        const_cast<Node::Value&>(root_.GetValue()) = std::move(v);
        nodes_stack_.pop_back();
    } else if (nodes_stack_.back()->IsDict() && !std::holds_alternative<Dict>(v)) {
        throw std::logic_error("Value must be after ctor, key or array element");
    } else if (nodes_stack_.back()->IsArray()) {
        Node tmp;
        const_cast<Node::Value&>(tmp.GetValue()) = std::move(v);
        const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(std::move(tmp));
    } else {
        const_cast<Node::Value&>((*nodes_stack_.back()).GetValue()) = std::move(v);
        nodes_stack_.pop_back();
    }
    if (nodes_stack_.size() == 0) {
        complete = true;
    }
    if (nodes_stack_.size() > 0) {  // Как тут возвращать разные контексты?
        if (nodes_stack_.back()->IsDict()) {
            return dynamic_cast<ValueAfterKeyContext&>(*this);
        } else if (nodes_stack_.back()->IsArray()) {
            return dynamic_cast<ValueAfterStartArrayContext&>(*this);
        }
    }
    return dynamic_cast<ValueBuildContext&>(*this);
}*/
    
ValueBuildContext Builder::Value(Node::Value v) {
    CheckComplete();
    const_cast<Node::Value&>(root_.GetValue()) = std::move(v);
    nodes_stack_.pop_back();
    if (nodes_stack_.size() == 0) {
        complete = true;
    }
    return {*this};
}
    
ValueAfterKeyContext Builder::Value(Node::Value v, int i) {
    i=0; if (i!=0) {}
    CheckComplete();
    const_cast<Node::Value&>((*nodes_stack_.back()).GetValue()) = std::move(v);
    nodes_stack_.pop_back();
    if (nodes_stack_.size() == 0) {
        complete = true;
    }
    return {*this};
}
    
ValueAfterStartArrayContext Builder::Value(Node::Value v, bool i) {
    i=false; if (i!=false) {}
    CheckComplete();
    Node tmp;
    const_cast<Node::Value&>(tmp.GetValue()) = std::move(v);
    const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(std::move(tmp));
    if (nodes_stack_.size() == 0) {
        complete = true;
    }
    return {*this};
}

StartDictContext Builder::StartDict() {
    CheckComplete();
    if (nodes_stack_.back() == nullptr) {
        root_ = Node{Dict{}};
        nodes_stack_.back() = &root_;
    } else if (nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Bad map start");
    } else if (nodes_stack_.back()->IsArray()) {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(Node{Dict{}});
        nodes_stack_.emplace_back(const_cast<Node*>(&(nodes_stack_.back()->AsArray().back())));
    } else {
        *nodes_stack_.back() = Node{Dict{}};
    }
    return {*this};
}

StartArrayContext Builder::StartArray() {
    CheckComplete();
    if (nodes_stack_.back() == nullptr) {
        root_ = Node{Array{}};
        nodes_stack_.back() = &root_;
    } else if (nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Bad array start");
    } else if (nodes_stack_.back()->IsArray()) {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(Node{Array{}});
        nodes_stack_.emplace_back(const_cast<Node*>(&(nodes_stack_.back()->AsArray().back())));
    } else {
        *nodes_stack_.back() = Node{Array{}};
    }
    return {*this};
}

EndDictContext Builder::EndDict() {
    CheckComplete();
    if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Bad map end");
    }
    nodes_stack_.pop_back();
    if (nodes_stack_.size() == 0) {
        complete = true;
    }
    return {*this};
}

EndArrayContext Builder::EndArray() {
    CheckComplete();
    if (!nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Bad array end");
    }
    nodes_stack_.pop_back();
    if (nodes_stack_.size() == 0) {
        complete = true;
    }
    return {*this};
}

Node Builder::Build() {
    if (complete == false) {
        throw std::logic_error("JSON is not complete");
    }
    return root_;
}
    
////////////////    BuildContext    //////////////////
    
BuildContext::BuildContext(Builder& b) : b_(b) {}
    
KeyContext BuildContext::Key(const std::string& s) {
    return b_.Key(s);
}

BuildContext BuildContext::Value(Node::Value v) {
    return Builder::Value(v, false);
}

StartDictContext BuildContext::StartDict() {
    return b_.StartDict();
}
StartArrayContext BuildContext::StartArray() {
    return b_.StartArray();
}
EndDictContext BuildContext::EndDict() {
    return b_.EndDict();
}
EndArrayContext BuildContext::EndArray() {
    return b_.EndArray();
}
Node BuildContext::Build() {
    return b_.Build();
}
Builder& BuildContext::GetB() {
    return b_;
}

//////////////////    Other contexts    ///////////////////////    

ValueAfterKeyContext KeyContext::Value(Node::Value v) {
    return GetB().Value(v, 0);
}
    
ValueAfterStartArrayContext StartArrayContext::Value(Node::Value v) {
    return GetB().Value(v, false);
}

ValueAfterStartArrayContext ValueAfterStartArrayContext::Value(Node::Value v) {
    return GetB().Value(v, false);
}
    
ValueAfterStartArrayContext EndDictContext::Value(Node::Value v) {
    return GetB().Value(v, false);
}
    
ValueAfterStartArrayContext EndArrayContext::Value(Node::Value v) {
    return GetB().Value(v, false);
}

} // end namespace json