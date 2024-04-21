#include <memory>
#include <iostream>
#include <vector>
#include <variant>
#include <string>
#include <cctype>
#include <unordered_map>

struct OpeningBracket {};
struct ClosingBracket {};
struct Number {
    int value;
};
struct UnknownToken {
    std::string value;
};
struct MinToken {};
struct AbsToken {};
struct Plus {};
struct Minus {};
struct Multiply {};
struct Modulo {};
struct Divide {};
struct Sqr {};

using Token = std::variant<OpeningBracket, ClosingBracket, Number, UnknownToken, MinToken, AbsToken, Plus, Minus, Multiply, Modulo, Divide, Sqr>;

class WrongExpressionError : public std::runtime_error {
public:
    explicit WrongExpressionError(const std::string& message) : std::runtime_error(message) {}
};

class IExpression {
public:
    virtual int Calculate() const = 0;
    virtual ~IExpression() = default;
};

class Constant final : public IExpression {
    int value_;
public:
    explicit Constant(int value) : value_(value) {}
    int Calculate() const final { return value_; }
};

class IBinaryOperation : public IExpression {
protected:
    std::unique_ptr<IExpression> left_;
    std::unique_ptr<IExpression> right_;
    virtual int Operation(int lhs, int rhs) const = 0;
public:
    IBinaryOperation(std::unique_ptr<IExpression> left, std::unique_ptr<IExpression> right)
        : left_(std::move(left)), right_(std::move(right)) {}
    int Calculate() const final {
        const auto left_res = left_->Calculate();
        const auto right_res = right_->Calculate();
        return Operation(left_res, right_res);
    }
};

class Sum final : public IBinaryOperation {
    int Operation(int lhs, int rhs) const final { return lhs + rhs; }
public:
    using IBinaryOperation::IBinaryOperation;
};

class Subtract final : public IBinaryOperation {
    int Operation(int lhs, int rhs) const final { return lhs - rhs; }
public:
    using IBinaryOperation::IBinaryOperation;
};

class MultiplyOp final : public IBinaryOperation {
    int Operation(int lhs, int rhs) const final { return lhs * rhs; }
public:
    using IBinaryOperation::IBinaryOperation;
};

class DivideOp final : public IBinaryOperation {
    int Operation(int lhs, int rhs) const final { return lhs / rhs; }
public:
    using IBinaryOperation::IBinaryOperation;
};

class ModuloOp final : public IBinaryOperation {
    int Operation(int lhs, int rhs) const final { return lhs % rhs; }
public:
    using IBinaryOperation::IBinaryOperation;
};

class SqrOp final : public IExpression {
    std::unique_ptr<IExpression> expr_;
public:
    explicit SqrOp(std::unique_ptr<IExpression> expr) : expr_(std::move(expr)) {}
    int Calculate() const final {
        const auto expr_res = expr_->Calculate();
        return expr_res * expr_res;
    }
};

std::unordered_map<std::string_view, Token> TokenMap = {
    {"+", Plus{}},
    {"-", Minus{}},
    {"*", Multiply{}},
    {"/", Divide{}},
    {"%", Modulo{}},
    {"(", OpeningBracket{}},
    {")", ClosingBracket{}},
    {"s", Sqr{}}
};

std::vector<Token> Tokenize(const std::string_view& input) {
    std::vector<Token> tokens;
    size_t pos = 0;
    while (pos < input.size()) {
        if (std::isspace(input[pos])) {
            ++pos;
            continue;
        }
        else if (std::isdigit(input[pos])) {
            size_t start = pos;
            while (pos < input.size() && std::isdigit(input[pos])) {
                ++pos;
            }
            tokens.emplace_back(Number{ std::stoi(input.substr(start, pos - start).data()) });
        }
        else {
            // Check if the current character is a token
            std::string_view token_str(&input[pos], 1);
            if (TokenMap.find(token_str) != TokenMap.end()) {
                tokens.push_back(TokenMap[token_str]);
                ++pos;
            }
            else {
                ++pos;
            }
        }
    }
    return tokens;
}

std::unique_ptr<IExpression> ParseTerm(const std::vector<Token>& tokens, size_t& pos);
std::unique_ptr<IExpression> ParseFactor(const std::vector<Token>& tokens, size_t& pos);

std::unique_ptr<IExpression> ParseExpression(const std::vector<Token>& tokens, size_t& pos) {
    auto expression = ParseTerm(tokens, pos);

    while (pos < tokens.size()) {
        const auto& token = tokens[pos];
        if (std::holds_alternative<Plus>(token)) {
            ++pos;
            expression = std::make_unique<Sum>(std::move(expression), ParseTerm(tokens, pos));
        }
        else if (std::holds_alternative<Minus>(token)) {
            ++pos;
            expression = std::make_unique<Subtract>(std::move(expression), ParseTerm(tokens, pos));
        }
        else {
            break;
        }
    }

    return expression;
}

std::unique_ptr<IExpression> ParseTerm(const std::vector<Token>& tokens, size_t& pos) {
    auto expression = ParseFactor(tokens, pos);

    while (pos < tokens.size()) {
        const auto& token = tokens[pos];
        if (std::holds_alternative<Divide>(token)) {
            expression = std::make_unique<DivideOp>(std::move(expression), ParseFactor(tokens, ++pos));
        }
        else if (std::holds_alternative<Modulo>(token)) {
            expression = std::make_unique<ModuloOp>(std::move(expression), ParseFactor(tokens, ++pos));
        }
        else if (std::holds_alternative<Multiply>(token)) {
            expression = std::make_unique<MultiplyOp>(std::move(expression), ParseFactor(tokens, ++pos));
        }
        else {
            break; // Прерываем цикл, если не встретили операцию деления, остатка или умножения
        }
    }

    return expression;
}

std::unique_ptr<IExpression> ParseFactor(const std::vector<Token>& tokens, size_t& pos) {
    if (pos >= tokens.size()) {
        throw WrongExpressionError("Unexpected end of expression");
    }

    const auto& token = tokens[pos];
    if (std::holds_alternative<Number>(token)) {
        auto value = std::get<Number>(token).value;
        ++pos;
        return std::make_unique<Constant>(value);
    }
    else if (std::holds_alternative<OpeningBracket>(token)) {
        ++pos;
        auto expression = ParseExpression(tokens, pos);
        if (pos >= tokens.size() || !std::holds_alternative<ClosingBracket>(tokens[pos])) {
            throw WrongExpressionError("Expected ')'");
        }
        ++pos;
        return expression;
    }
    else if (std::holds_alternative<Plus>(token)) {
        ++pos;
        return ParseFactor(tokens, pos);
    }
    else if (std::holds_alternative<Minus>(token)) {
        if (pos + 1 < tokens.size() && std::holds_alternative<Number>(tokens[pos + 1])) {
            ++pos;
            auto number = std::get<Number>(tokens[pos]).value;
            auto factor = std::make_unique<Constant>(number);
            return std::make_unique<Subtract>(std::make_unique<Constant>(0), std::move(factor));
        }
        else {
            ++pos;
            return std::make_unique<Subtract>(ParseFactor(tokens, pos), ParseTerm(tokens, pos));
        }
    }
    else if (std::holds_alternative<Divide>(token) || std::holds_alternative<Modulo>(token) || std::holds_alternative<Multiply>(token)) {
        throw WrongExpressionError("Unexpected token");
    }
    else if (std::holds_alternative<Sqr>(token)) {
        ++pos;
        if (pos >= tokens.size() || !std::holds_alternative<OpeningBracket>(tokens[pos])) {
            throw WrongExpressionError("Expected '(' after 'sqr'");
        }
        ++pos;
        auto expression = ParseExpression(tokens, pos);
        if (pos >= tokens.size() || !std::holds_alternative<ClosingBracket>(tokens[pos])) {
            throw WrongExpressionError("Expected ')' after expression inside 'sqr'");
        }
        ++pos;
        return std::make_unique<SqrOp>(std::move(expression));
    }
    else {
        throw WrongExpressionError("Unexpected token");
    }
}

int Calculate(const std::string_view input) {
    const auto tokens = Tokenize(input);
    size_t pos = 0;
    const auto expression = ParseExpression(tokens, pos);
    if (pos != tokens.size()) {
        throw WrongExpressionError("Unexpected token at the end of expression");
    }
    return expression->Calculate();
}

int main() {
    try {
        std::cout << Calculate("1 + 3 * (4 - 2) / (2 - 1) + sqr(4)") << std::endl;
    }
    catch (const WrongExpressionError& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}