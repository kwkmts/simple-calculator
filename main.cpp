#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
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

//
// lexer
//

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

void lex() {
    auto begin = input.begin();
    auto it = begin;
    char ch = *it;

    while (ch) {
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
        } else if (strchr("*/+-", ch)) {
            tokenList.push_back(Token(ch, it - begin));
            ch = *(++it);
        } else {
            logErrorAt("unknown token", it - begin);
        }
    }
}

//
// parser
//

class Node {
  public:
    virtual ~Node() = default;
    virtual double calc() = 0;
};

class NumNode : public Node {
    double val;

  public:
    explicit NumNode(double val) : val(val) {}

    double calc() override { return val; }
};

class BinNode : public Node {
    char op;
    std::unique_ptr<Node> LHS, RHS;

  public:
    BinNode(char op, std::unique_ptr<Node> LHS, std::unique_ptr<Node> RHS)
        : op(std::move(op)), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    double calc() override {
        switch (op) {
        case '*':
            return LHS->calc() * RHS->calc();
        case '/':
            return LHS->calc() / RHS->calc();
        case '+':
            return LHS->calc() + RHS->calc();
        case '-':
            return LHS->calc() - RHS->calc();
        default:
            logError("unknown operator");
        }
    }
};

std::unique_ptr<Node> AST;

bool isNum(std::vector<Token>::iterator tok) { return tok->getKind() == NUM; }

bool isReserved(std::vector<Token>::iterator tok, char ch) {
    return tok->getKind() == RESERVED && tok->getReserved() == ch;
}

std::unique_ptr<Node> primary();
std::unique_ptr<Node> mul();
std::unique_ptr<Node> add();

// primary := num
std::unique_ptr<Node> primary() {
    if (!isNum(curTok)) {
        logErrorAt("not a number", curTok->getLoc());
    }
    auto node = std::make_unique<NumNode>(curTok->getNum());
    curTok++;
    return std::move(node);
}

// mul := primary ("*" primary | "/" primary)*
std::unique_ptr<Node> mul() {
    auto node = primary();

    for (;;) {
        if (isReserved(curTok, '*') || isReserved(curTok, '/')) {
            char op = curTok->getReserved();
            curTok++;
            node = std::make_unique<BinNode>(op, std::move(node), primary());
        } else {
            return std::move(node);
        }
    }
}

// add := mul ("+" mul | "-" mul)*
std::unique_ptr<Node> add() {
    auto node = mul();

    for (;;) {
        if (isReserved(curTok, '+') || isReserved(curTok, '-')) {
            char op = curTok->getReserved();
            curTok++;
            node = std::make_unique<BinNode>(op, std::move(node), mul());
        } else {
            return std::move(node);
        }
    }
}

void parse() { AST = add(); }

int main() {
    std::cout << "> ";
    std::getline(std::cin, input);

    lex();

    curTok = tokenList.begin();

    parse();

    double ans = AST->calc();

    std::cout << "ans = " << ans << std::endl;
}
