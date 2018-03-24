// Copyright 2017, Danila Kutenin

#include "Node.h"

#include <algorithm>
#include <string>
#include <vector>

Node::Node()
    : left(nullptr)
    , right(nullptr)
    , frequency(0)
    , data("") {
}

Node::Node(const std::string& Data, const int64_t freq, const Node_type t) {  // make a leaf
    type = t;
    data = std::move(Data);
    frequency = freq;
    left = nullptr;
    right = nullptr;
    to_go.resize(1 << CHAR_SIZE);
    std::fill(to_go.begin(), to_go.end(), std::make_pair("", nullptr));
}

Node::Node(Node* l, Node* r, Node_type t) {  // make a bind
    type = t;
    data.clear();
    data.shrink_to_fit();
    frequency = l->GetFrequency() + r->GetFrequency();
    left = l;
    right = r;
    to_go.resize(1 << CHAR_SIZE);
    std::fill(to_go.begin(), to_go.end(), std::make_pair("", nullptr));
}

Node::~Node() {
    if (left) {
        delete left;
    }
    if (right) {
        delete right;
    }
}

int64_t Node::GetFrequency() const {  // frequency of a node
    return frequency;
}

std::string Node::GetData() const {
    return data;
}
