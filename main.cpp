#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

std::string input;

void logError(const char *str) {
    std::cerr << str << std::endl;
    std::exit(1);
}

void logErrorAt(const char *str, int loc) {
    std::cerr << input << '\n';
    std::cerr << std::setw(loc) << "";
    std::cerr << "^ " << str << std::endl;
    std::exit(1);
}

//
// lexer
//

using TokenKind = enum { TK_NUM, TK_RESERVED, TK_EOF };

class Token {
    TokenKind kind;
    double num;
    char reserved;
    int loc;

  public:
    Token(double num, int loc)
        : kind(TK_NUM), num(num), reserved('\0'), loc(loc) {}

    Token(char reserved, int loc)
        : kind(TK_RESERVED), num(0), reserved(reserved), loc(loc) {}

    Token(int loc) : kind(TK_EOF), num(0), reserved('\0'), loc(loc) {}

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
        if (std::isspace(ch)) {
            do {
                ch = *(++it);
            } while (std::isspace(ch));

        } else if (std::isdigit(ch)) {
            std::string numStr;
            do {
                numStr += ch;
                ch = *(++it);
            } while (std::isdigit(ch));
            tokenList.push_back(
                Token(std::strtod(numStr.c_str(), nullptr), it - begin));
        } else if (std::strchr("*/+-()^", ch)) {
            tokenList.push_back(Token(ch, it - begin));
            ch = *(++it);
        } else {
            logErrorAt("unknown token", it - begin);
        }
    }

    tokenList.push_back(Token(input.length()));
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
        case '^':
            return std::pow(LHS->calc(), RHS->calc());
        default:
            logError("unknown operator");
        }
    }
};

std::unique_ptr<Node> AST;

bool isNum() { return curTok->getKind() == TK_NUM; }

bool isReserved(char ch) {
    return curTok->getKind() == TK_RESERVED && curTok->getReserved() == ch;
}

std::unique_ptr<Node> primary();
std::unique_ptr<Node> pow();
std::unique_ptr<Node> mul();
std::unique_ptr<Node> add();

// primary := num | "(" add ")"
std::unique_ptr<Node> primary() {
    std::unique_ptr<Node> node;

    if (isReserved('(')) {
        curTok++;
        node = add();
        if (!isReserved(')')) {
            logErrorAt("')' expected", curTok->getLoc());
        }
        curTok++;

    } else if (isNum()) {
        node = std::make_unique<NumNode>(curTok->getNum());
        curTok++;
    } else {
        logErrorAt("not a number", curTok->getLoc());
    }

    return node;
}

// pow := primary ("^" primary)*
std::unique_ptr<Node> pow() {
    auto node = primary();

    for (;;) {
        if (isReserved('^')) {
            curTok++;
            node = std::make_unique<BinNode>('^', std::move(node), primary());
        } else {
            return node;
        }
    }
}

// mul := pow ("*" pow | "/" pow)*
std::unique_ptr<Node> mul() {
    auto node = pow();

    for (;;) {
        if (isReserved('*') || isReserved('/')) {
            char op = curTok->getReserved();
            curTok++;
            node = std::make_unique<BinNode>(op, std::move(node), pow());
        } else {
            return node;
        }
    }
}

// add := mul ("+" mul | "-" mul)*
std::unique_ptr<Node> add() {
    auto node = mul();

    for (;;) {
        if (isReserved('+') || isReserved('-')) {
            char op = curTok->getReserved();
            curTok++;
            node = std::make_unique<BinNode>(op, std::move(node), mul());
        } else {
            return node;
        }
    }
}

void parse() { AST = add(); }

int main(int argc, char **argv) {
    if (argc == 1) {
        std::cout << "> ";
        std::getline(std::cin, input);
    } else if (argc == 2) {
        input = argv[1];
    } else {
        logError("bad arguments");
    }

    lex();

    curTok = tokenList.begin();

    parse();

    double ans = AST->calc();

    std::cout << "ans = " << ans << std::endl;
}
