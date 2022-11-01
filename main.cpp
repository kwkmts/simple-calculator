#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

std::string input;

void logError(const char *str) {
    std::cerr << str << std::endl;
    exit(1);
}

void logErrorAt(const char *str, int loc) {
    std::cerr << input << '\n';
    std::cerr << std::setw(loc) << "";
    std::cerr << "^ " << str << std::endl;
    exit(1);
}

typedef enum { NUM, RESERVED } TokenKind;

class Token {
    TokenKind kind;
    double num;
    char reserved;
    int loc;

  public:
    Token(double num, int loc)
        : kind(NUM), num(num), reserved('\0'), loc(loc) {}
    Token(char reserved, int loc)
        : kind(RESERVED), num(0), reserved(reserved), loc(loc) {}

    TokenKind getKind() const { return kind; }
    double getNum() const { return num; }
    char getReserved() const { return reserved; }
    int getLoc() const { return loc; }
};

std::vector<Token> tokenList;
std::vector<Token>::iterator curTok;

class Node {
  public:
    virtual ~Node() = default;
    virtual double calc() = 0;
};

class NumNode : public Node {
    double val;

  public:
    NumNode(double val) : val(val) {}
    double getVal() const { return val; }

    double calc() override { return val; }
};

class BinNode : public Node {
    char op;
    Node *LHS, *RHS;

  public:
    BinNode(char op, Node *LHS, Node *RHS) : op(op), LHS(LHS), RHS(RHS) {}
    Node *getLHS() const { return LHS; }
    Node *getRHS() const { return RHS; }

    double calc() override {
        switch (op) {
        case '*':
            return LHS->calc() * RHS->calc();
        case '/':
            return LHS->calc() / RHS->calc();
        default:
            logError("unknown operator");
        }
    }
};

Node *AST;

void lex() {
    auto begin = input.begin();
    auto it = begin;
    char ch = *it;

    while (ch) {
        // std::cout << it - begin << std::endl;
        if (isspace(ch)) {
            do {
                ch = *(++it);
            } while (isspace(ch));

        } else if (isdigit(ch)) {
            std::string numStr;
            do {
                numStr += ch;
                ch = *(++it);
            } while (isdigit(ch));
            tokenList.push_back(
                Token(strtod(numStr.c_str(), nullptr), it - begin));
        } else if (strchr("*/", ch)) {
            tokenList.push_back(Token(ch, it - begin));
            ch = *(++it);
        } else {
            logErrorAt("unknown token", it - begin);
        }
    }
}

bool isNum(std::vector<Token>::iterator tok) { return tok->getKind() == NUM; }

bool isReserved(std::vector<Token>::iterator tok, char ch) {
    return tok->getKind() == RESERVED && tok->getReserved() == ch;
}

Node *primary();
Node *mul();

// primary := num
Node *primary() {
    if (!isNum(curTok)) {
        logErrorAt("not a number", curTok->getLoc());
    }
    Node *node = new NumNode(curTok->getNum());
    curTok++;
    return node;
}

// mul := primary ("*" primary | "/" primary)*
Node *mul() {
    Node *node = primary();

    for (;;) {
        if (isReserved(curTok, '*') || isReserved(curTok, '/')) {
            char op = curTok->getReserved();
            curTok++;
            node = new BinNode(op, node, primary());
        } else {
            return node;
        }
    }
}

void parse() { AST = mul(); }

int main() {
    std::cout << "> ";
    std::getline(std::cin, input);

    lex();

    curTok = tokenList.begin();

    parse();

    double ans = AST->calc();

    std::cout << "ans = " << ans << std::endl;
}
