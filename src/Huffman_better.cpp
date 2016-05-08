/*
  Okay, lets start compressing;
  Load -- need file, named "config" for decoding, pls be sure that this file
  exists in the same path. DO NOT CHANGE this file either you'll fail decoding.

  encode and decode --- works as well

  Save -- saves the file, called "config"

  everything else should be intuitively understandable
*/

// SELF-MADE LIBRATIES
#include "Huffman_better.h"
#include "Node_better.h"
#include "Suffix_tree.h"
#include "Trie.h"


// NEEDED LIBRARIES
#include <inttypes.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <utility>
#include <queue>

// DEBUG LIBRARY (REMOVE TODO)
#include <iostream>

namespace Codecs {

  class comp {
   public:
    bool operator() (std::pair<Node*, int64_t> l, std::pair<Node*, int64_t> r) {
      if (l.first->getFrequency() == r.first->getFrequency()) {
        return r.second > l.second;
      }
      return l.first->getFrequency() > r.first->getFrequency();
    }
  };

  HuffmanCodec::HuffmanCodec() {
    root_for_decode = new Node("", 0);
    root_for_encode = new Node("", 0);
  }

  void HuffmanCodec::encode(string& encoded, const string_view& raw) const {
    encoded.reserve(2 * raw.size());
    unsigned char buf = 0;
    int8_t count = CHAR_SIZE - LOG_CHAR_SIZE;
    size_t j = 0;
    size_t i = 0;
    size_t uz = 0;
    size_t len_uz = 0;
    size_t last_uz = 0;

    string cur;
    cur.reserve(20);
    while (i < raw.size()) {
      j = 0;
      uz = 0;
      cur.clear();
      cur.push_back(raw[i]);

      uz = trie.next(uz, static_cast<unsigned char>(raw[i]));
      last_uz = uz;
      len_uz = 1;
      while (i + 1 < raw.size() && trie.is_next(uz, raw[i + 1])) {
        uz = trie.next(uz, raw[i + 1]);
        ++i;
        if (trie.nodes[uz].is_terminal) {
          len_uz = cur.size() + 1;
          last_uz = uz;
        }
        cur.push_back(raw[i]);
      }
      while (__builtin_expect(cur.size() > len_uz, false)) {
        cur.pop_back();
        --i;
      }

      // fuck this shit
      size_t size_of_path = trie.nodes[last_uz].code.size();
      while (j < size_of_path) {
        while (j < size_of_path && count) {
          buf <<= 1;
          buf += trie.nodes[last_uz].code[j];
          ++j;
          --count;
        }
        if (!count) {
          count = CHAR_SIZE;
          encoded.push_back(buf);
          buf = 0;
        }
      }
      ++i;
    }
    if (count != CHAR_SIZE) {
      encoded.push_back(buf << count);
    }
    unsigned char c = encoded[0];
    c |= (static_cast<unsigned char>(CHAR_SIZE - count)) <<
     (CHAR_SIZE - LOG_CHAR_SIZE);
    encoded[0] = c;
  }


  void HuffmanCodec::decode(string& raw, const string_view& encoded) const {
    if (__builtin_expect(encoded.size() == 1, false)) {
      return;
    }
    raw.reserve(2 * encoded.size());
    int8_t count = LOG_CHAR_SIZE;
    Node* cur = root_for_decode;
    unsigned char byte = encoded[0];
    int64_t encoded_size = static_cast<int64_t>(encoded.size()) - 1;

    while (true) {
      if (byte & (1 << (CHAR_SIZE - count - 1))) {
        cur = cur->right;
      } else {
        cur = cur->left;
      }
      if (!cur->left) {
        raw += cur->getData();
        cur = root_for_decode;
      }
      ++count;
      if (count == CHAR_SIZE) {
        count = 0;
        byte = encoded[1];
        break;
      }
    }

    // till almost the last element
    for (int64_t i = 1; i < encoded_size; ++i) {
      raw += cur->to_go[byte].first;
      cur = cur->to_go[byte].second;
      byte = encoded[i + 1];
    }

    // last element
    while (true) {
      if (byte & (1 << (CHAR_SIZE - count - 1))) {
        cur = cur->right;
      } else {
        cur = cur->left;
      }
      if (!cur->left) {
        raw += cur->getData();
        cur = root_for_decode;
      }
      ++count;
      if (count == CHAR_SIZE ||
        (static_cast<unsigned char>(encoded[0]) >> (CHAR_SIZE - LOG_CHAR_SIZE)) == count) {
        break;
      }
    }
  }

  string HuffmanCodec::save() const {
    std::ofstream output("config", std::ios_base::binary);
    for (const auto& c : ans) {
      output << c.first << " " << c.second << std::endl;
    }
    output.close();
    return string();
  }

  void HuffmanCodec::Build_table(Node* root_for_table,
    std::vector<int32_t>& code) {
    if (root_for_table->left != nullptr) {
      code.push_back(false);
      Build_table(root_for_table->left, code);
    }

    if (root_for_table->right != nullptr) {
      code.push_back(true);
      Build_table(root_for_table->right, code);
    }

    if (root_for_table->left == root_for_table->right) {
      string s = root_for_table->getData();
      trie.insert(s, code);
    }
    code.pop_back();
  }

  void HuffmanCodec::load(const string_view& config) {
    trie.reset();
    ans.clear();
    ans.shrink_to_fit();

    for (int32_t c = 0; c < (1 << CHAR_SIZE); ++c) {
      std::string s(1, c);
      ans.push_back({s, 0});
    }

    std::ifstream input(config.to_string(), std::ios_base::binary);
    std::string now;
    while (input.good()) {
      getline(input, now);
      int64_t count = 0;
      int64_t i = now.size() - 1;
      while (now[i] <= '9' && now[i] >= '0') {
        --i;
      }
      for (size_t j = i + 1; j < now.size(); ++j) {
        count = 10 * count + (now[j] - '0');
      }
      if (i == 1) {
        ans[static_cast<unsigned char>(now[0])].second = count;
      } else {
        ans.push_back({now.substr(0, i), count});
      }
    }
    input.close();


    std::priority_queue<std::pair<Node*, int64_t>,
     std::vector<std::pair<Node*, int64_t> >, comp> table_cur;
    int64_t i = 0;
    for (const auto& c : ans) {
      Node* p = new Node(c.first, c.second);
      table_cur.push({p, i});
      ++i;
    }

    while (table_cur.size() != 1) {
      Node* left_son = table_cur.top().first;
      table_cur.pop();
      Node* right_son = table_cur.top().first;
      table_cur.pop();
      Node* parent = new Node(left_son, right_son);
      table_cur.push({parent, i});
      ++i;
    }
    root_for_decode = table_cur.top().first;
    Node* root_for_table = root_for_decode;
    build_jumps(root_for_table);
  }

  size_t HuffmanCodec::sample_size(size_t records_total) const {
    return std::min(static_cast<size_t>(10000), records_total);
  }

  void HuffmanCodec::learn(const StringViewVector& sample) {
    trie = Trie();

    for (int32_t c = 0; c < (1 << CHAR_SIZE); ++c) {
      std::string s(1, c);
      ans.push_back({s, 0});
      trie.insert(s);
    }
    std::string concat;
    concat.reserve(5e5);
    int64_t cnt_letters = 0;
    for (size_t i = 0; i < sample.size(); ++i) {
      concat += sample[i].to_string() + '\n';
      cnt_letters += sample[i].size() + 1;
      if (cnt_letters >= static_cast<int64_t>(5e5)) {
        concat.resize(5e5);
      // constant TODO(danlark1)
        break;
      }
    }
    concat += '\n';
    tree = new suff_tree(concat);
    std::vector<std::pair<std::string, int64_t> > ans1 = tree->find_substr();
    for (const auto& t : ans1) {
      if (t.first.size() == 1) {
        ans[static_cast<unsigned char>(t.first[0])].second = t.second;
      } else {
        ans.push_back({t.first, t.second});
      }
    }
    delete tree;
    concat.clear();
    concat.shrink_to_fit();
    ans1.clear();
    ans1.shrink_to_fit();

    // need to use pair because of the non-deterministic Heap;
    std::priority_queue<std::pair<Node*, int64_t>,
     std::vector<std::pair<Node*, int64_t> >, comp> table_cur;
    int64_t i = 0;
    for (const auto& c : ans) {
      Node* p = new Node(c.first, c.second);
      table_cur.push({p, i});
      ++i;
    }

    while (table_cur.size() != 1) {
      Node* left_son = table_cur.top().first;
      table_cur.pop();
      Node* right_son = table_cur.top().first;
      table_cur.pop();
      Node* parent = new Node(left_son, right_son);
      table_cur.push({parent, i});
      ++i;
    }
    root_for_encode = table_cur.top().first;
    Node* root_for_table = root_for_encode;

    std::vector<int32_t> code;
    Build_table(root_for_table, code);
    code.clear();
    code.shrink_to_fit();
  }

  void HuffmanCodec::build_jumps(Node* node) {
    Node* cur;
    for (size_t byte = 0; byte < (1 << CHAR_SIZE); ++byte) {
      cur = node;
      if (!cur->right || !cur->left) {
        continue;
      }
      for (size_t j = 0; j < CHAR_SIZE; ++j) {
        if (byte & (1 << (CHAR_SIZE - j - 1))) {
          cur = cur->right;
        } else {
          cur = cur->left;
        }
        if (!cur->left) {  // is equal to cur->left == nullptr == cur->right
          node->to_go[byte].first += cur->getData();
          cur = root_for_decode;
        }
      }
      node->to_go[byte].second = cur;
    }
    if (node->left != nullptr) {
      build_jumps(node->left);
      build_jumps(node->right);
    }
  }

  void HuffmanCodec::reset() {
    trie.reset();
    ans.clear();
    ans.shrink_to_fit();

    delete root_for_decode;
    delete root_for_encode;
  }
}  // namespace Codecs
