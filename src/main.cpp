#include <cctype>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <vector>
#include <filesystem>
#include <iostream>
#include <unordered_map>


std::unordered_map<std::string, int> INT_VARS;
std::unordered_map<std::string, float> FLOAT_VARS;
std::unordered_map<std::string, std::string> STRING_VARS;

enum class TokenType { T_INT=0, V_INT=1, T_FLOAT=2, V_FLOAT=4, T_STRING=5, V_STRING=6, IDEN=7, EQUAL=8, PRINT=9, SEMICOLON=10, QUOTATION=11, O_BRACKET=12, C_BRACKET=13, COMMA=14 };
std::unordered_map<std::string, TokenType> TypeLookup = {
  {"int", TokenType::T_INT},
  {"float", TokenType::T_FLOAT},
  {"string", TokenType::T_STRING},
  {"=", TokenType::EQUAL},
  {";", TokenType::SEMICOLON},
  {"\"", TokenType::QUOTATION},
  {"(", TokenType::O_BRACKET},
  {")", TokenType::C_BRACKET},
  {",", TokenType::COMMA},
  {"print", TokenType::PRINT}
};

std::unordered_map<TokenType, std::string> err_msg_map = {
  {TokenType::IDEN, "Excpected a variable name."},
  {TokenType::EQUAL, "Excpected ;."},
  {TokenType::V_INT, "Excpected value for the variable"},
};

struct Token {
  TokenType type;
  std::string content;
  int line, col;
};

bool isInt(std::string in) {
  if (in.empty()) return false;
  size_t i = 0;
  while (i<in.size())
    if (!std::isdigit(in[i++])) return false;

  return true;
}

bool isFloat(std::string in) {
  if (in.empty()) return false;
  int decimalPoints = 0;
  size_t i = 0;
  while (i<in.size()) {
    if (in[i] == '.') decimalPoints++;
    else if (!std::isdigit(in[i])) return false;
    i++;
  }
  return decimalPoints == 1;
}

std::vector<Token> lex(std::string src) {
  std::vector<Token> out;

  int line = 1;
  int col = 1;
  size_t i = 0;
  
  while (i<src.size()) {
    if (src[i] == '\n') { i++; line++; col=1; continue; }
    else if (src[i] == '"') {
      size_t j = i+1;
      std::string word;
      while (j<src.size() && src[j] != '"') {
        word += src[j++];
      }
      if (j >= src.size()) throw std::runtime_error("Error");
      i = j+1;
      out.push_back({TokenType::V_STRING, word, line, col});
      col += word.size();
    }
    else if (std::isalnum(src[i])) {
      size_t j = i;
      std::string word;
      while (j<src.size()) {
        if (std::isalnum(src[j]) || src[j] == '.') {
          word += src[j++];
        }
        else break;
      }
      i = j;
      bool typeExists = TypeLookup.find(word) != TypeLookup.end();
      TokenType type;
      if (typeExists) type = TypeLookup[word];
      else {
        if (isInt(word)) type = TokenType::V_INT;
        else if (isFloat(word)) type = TokenType::V_FLOAT;
        else type = TokenType::IDEN;
      }

      out.push_back({type, word, line, col});
      col += word.size();
    } 
    else if (std::ispunct(src[i])) {
      out.push_back({TypeLookup[std::string(1, src[i])], std::string(1, src[i]), line, col});
      col++;
      i++;
    }
    else {
      i++; col++;
    }
  } 
  return out;
}

void parse(std::vector<Token> tokens) {
  for (size_t i=0; i<tokens.size(); i++) {
    Token t = tokens[i];
    switch (t.type) {
      // case TokenType::T_INT:
      //   std::cout<<tokens[i+3].content<<std::endl;
      //   INT_VARS[tokens[++i].content] = std::stoi(tokens[i+2].content);
      //   i += 3;
      //
      //   break;
      case TokenType::T_STRING:
        STRING_VARS[tokens[++i].content] = tokens[i+4].content;
        i += 5;
      default:
        break;
    }
  }
}

std::string getSrc(std::string fileName) {
  if (!std::filesystem::exists(fileName)) {
    throw "File not found: "+fileName;
    return nullptr;
  }
  std::string line;
  std::string content = "";

  std::ifstream file(fileName);

  while (std::getline(file, line)) 
    content += line + "\n";

  file.close();

  return content;
}

bool check(const std::vector<Token> &ts, TokenType t, const size_t &i) {
  return i<ts.size() && ts[i].type == t;
}

bool match(const std::vector<Token> &ts, TokenType t, size_t &i) {
  if (check(ts, t, i)) { i++; return true; }
  return false;
}

Token expect(const std::vector<Token> &ts, TokenType t, size_t &i) {
  if (!check(ts, t, i)) throw std::runtime_error(err_msg_map[t]);
  return ts[i++];
}

void interpret(std::string fileName) {
  std::vector<Token> ts = lex(getSrc(fileName));

  size_t i=0;

  while (i<ts.size()) {
    switch (ts[i].type) {
      case TokenType::T_INT: {
        i++;
        Token iden = expect(ts, TokenType::IDEN, i);
        int value;
        if (match(ts, TokenType::EQUAL, i)) {
          Token v = expect(ts, TokenType::V_INT, i);
          value = std::stoi(v.content);
        }
        expect(ts, TokenType::SEMICOLON, i);
        INT_VARS[iden.content] = value;
        break;
      }
      case TokenType::T_FLOAT: {
        i++;
        Token iden = expect(ts, TokenType::IDEN, i);
        float value;
        if (match(ts, TokenType::EQUAL, i)) {
          Token v = expect(ts, TokenType::V_FLOAT, i);
          value = std::stof(v.content);
        }
        expect(ts, TokenType::SEMICOLON, i);
        FLOAT_VARS[iden.content] = value;
        break;
      }
      case TokenType::T_STRING: {
        i++;
        Token iden = expect(ts, TokenType::IDEN, i);
        std::string value;
        if (match(ts, TokenType::EQUAL, i)) {
          Token v = expect(ts, TokenType::V_STRING, i);
          value = v.content;
        }
        expect(ts, TokenType::SEMICOLON, i);
        STRING_VARS[iden.content] = value;
        break;
      }
      case TokenType::PRINT: {
        i++;
        expect(ts, TokenType::O_BRACKET, i);
        bool running = true;
        std::string word;
        while (running) {
          if (match(ts, TokenType::COMMA, i)) word+= " ";
          if (check(ts, TokenType::V_STRING, i) || check(ts, TokenType::V_INT, i) || check(ts, TokenType::V_FLOAT, i)) word += ts[i++].content;
          else if (check(ts, TokenType::IDEN, i)) {
            // std::cout<<ts[i].content<<std::endl;
            // std::cout<<ts[i].content<<std::endl;
            bool isIntVar = INT_VARS.find(ts[i].content) != INT_VARS.end();
            bool isFloatVar = FLOAT_VARS.find(ts[i].content) != FLOAT_VARS.end();
            bool isStringVar = STRING_VARS.find(ts[i].content) != STRING_VARS.end();
            // std::cout<<(isIntVar ? "Int" : "Not Int")<<std::endl;
            // std::cout<<(isFloatVar ? "Float" : "Not Float")<<std::endl;
            // std::cout<<(isStringVar ? "String" : "Not String")<<std::endl;
            if (isIntVar) word += std::to_string(INT_VARS[ts[i].content]);
            else if (isFloatVar) word += std::to_string(FLOAT_VARS[ts[i].content]);
            else if (isStringVar) word += STRING_VARS[ts[i].content];
            else throw std::runtime_error("Variable not found.");
            i++;
          }
          if (match(ts, TokenType::C_BRACKET, i)) running = false;
        }
        expect(ts, TokenType::SEMICOLON, i);
        std::cout<<word<<std::endl;
      }
    }
  }
}
int main(int argc, char* argv[]) {
  std::vector<Token> tokens = lex(getSrc(argv[argc-1]));
  // parse(tokens);
  interpret(argv[argc-1]);
  // for (const auto& [type, content, line, col] : tokens) {
  //   std::cout<<static_cast<int>(type)<<", "<<content<<", "<<line<<":"<<col<<std::endl;
  // }

  // std::cout<<"Ints"<<std::endl;
  // for (const auto& [key, val] : INT_VARS) {
  //   std::cout<<key<<":"<<val<<std::endl;
  // }
  // std::cout<<"Strings"<<std::endl;
  // for (const auto& [key, val] : STRING_VARS) {
  //   std::cout<<key<<":"<<val<<std::endl;
  // }
  return 0;
}
