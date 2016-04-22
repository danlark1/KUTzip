#pragma once

#include <map>
#include <vector>
#include <string>
#include <experimental/string_view>
#include <climits>


enum Node_type {LEAF, BIND};
const size_t CHAR_SIZE = (sizeof(char) * CHAR_BIT);

class Node {  
public:
  Node_type type;
  int64_t frequency;
  std::string data;
  Node* left;
  Node* right;
  std::vector<std::pair<std::string, Node*> > to_go;
  Node(std::string d, int64_t s, Node_type = LEAF); // Construct a list
  Node(Node* l, Node* r, Node_type = BIND); // Construct a bind node
  ~Node();
  int64_t getFrequency();
  std::string getData();
};