#include "frontend/parser/Parser.hpp"
#include "nodes/Program.hpp"
#include "utils/Error.hpp"
#include "visitors/Visitor.hpp"

std::unique_ptr<Statement> Parser::ParseExprStmt() {
    std::unique_ptr<Expression> expr = ParseExpr();

    if (!expectAndConsume(TokenType::SEMICOLON, "Missing ';' after expression"))
        return nullptr;

    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<BlockStmt> Parser::ParseBlockStmt() {
    getNextToken();
    auto Result = std::make_unique<BlockStmt>();

    while (peekCurr().tokentype != TokenType::RIGHT_CURLY) {
        auto stmt = ParseStmt();

        if (stmt)
            Result->addStmt(std::move(stmt));

        if (peekCurr().tokentype == TokenType::END_OF_FILE) {
            expect(peekCurr(), "Expected '}'");
            return nullptr;
        }
    }

    getNextToken();
    return Result;
}

std::unique_ptr<Statement> Parser::ParseIfStmt() {
    getNextToken();

    if (!expectAndConsume(TokenType::LEFT_ROUND, "Expected '(' after if"))
        return nullptr;

    auto condn = ParseExpr();

    if (!expectAndConsume(TokenType::RIGHT_ROUND, "Expected ')'"))
        return nullptr;

    auto ifbody = ParseStmt();

    auto elsestmt = ParseElseStmt();
    return std::make_unique<IfStmt>(std::move(condn), std::move(ifbody),
                                    std::move(elsestmt));
}

std::unique_ptr<Statement> Parser::ParseElseStmt() {
    if (peekCurr().tokentype != TokenType::ELSE)
        return nullptr;

    getNextToken();

    auto elsebody = ParseStmt();
    return std::make_unique<ElseStmt>(std::move(elsebody));
}

std::unique_ptr<Statement> Parser::ParseWhileStmt() {
    getNextToken();

    if (!expectAndConsume(TokenType::LEFT_ROUND, "Expected '(' after while"))
        return nullptr;

    auto condn = ParseExpr();

    if (!expectAndConsume(TokenType::RIGHT_ROUND, "Expected ')'"))
        return nullptr;

    auto whilebody = ParseStmt();
    return std::make_unique<WhileStmt>(std::move(condn), std::move(whilebody));
}

std::unique_ptr<Statement> Parser::ParseReturnStmt() {
    getNextToken();

    std::unique_ptr<Statement> Result;

    if (peekCurr().tokentype == TokenType::SEMICOLON) {
        auto retexpr = std::make_unique<EmptyExpr>();
        retexpr->line = peekCurr().line;
        retexpr->column = peekCurr().column;
        Result = std::make_unique<ReturnStmt>(std::move(retexpr));
    } else {
        auto retexpr = ParseExpr();
        Result = std::make_unique<ReturnStmt>(std::move(retexpr));
    }

    getNextToken();
    return Result;
}

std::unique_ptr<Statement> Parser::ParseDeclStmt() {
    auto [typek, varname, tline, tcol] = getTypeNamePair();

    int lastTokenLine, lastTokenCol;
    std::unique_ptr<Expression> expr;

    if (peekCurr().tokentype == TokenType::EQUALS) {
        getNextToken();
        expr = ParseExpr();

        lastTokenLine = expr->line;
        lastTokenCol = expr->column;
    } else {
        lastTokenLine = peekCurr().line;
        lastTokenCol = peekCurr().column;

        expr = nullptr;
    }

    if (peekCurr().tokentype != TokenType::SEMICOLON) {
        Error error(lastTokenLine, lastTokenCol,
                    "Missing ';' after declaration");
        numOfErrors += 1;
        advToSyncPoint();
        return nullptr;
    }

    getNextToken();
    return std::make_unique<DeclStmt>(typek, varname, std::move(expr), tline,
                                      tcol);
}

std::unique_ptr<Statement> Parser::ParseStmt() {
    switch (peekCurr().tokentype) {
        case TokenType::RIGHT_CURLY: {
            expect(peekCurr(), "Unexpected '}'");
            return nullptr;
        } break;

        case TokenType::RIGHT_ROUND: {
            expect(peekCurr(), "Unexpected ')'");
            return nullptr;
        } break;

        case TokenType::LEFT_CURLY: {
            return ParseBlockStmt();
        } break;

        case TokenType::INT:
        case TokenType::CHAR:
        case TokenType::UINT8: {
            return ParseDeclStmt();
        } break;

        case TokenType::IF: {
            return ParseIfStmt();
        } break;

        case TokenType::WHILE: {
            return ParseWhileStmt();
        } break;

        case TokenType::BREAK:
        case TokenType::CONTINUE: {
            std::unique_ptr<Statement> Result;

            if (peekCurr().tokentype == TokenType::BREAK) {
                Result = std::make_unique<BreakStmt>(peekCurr().line, peekCurr().column);
            } else {
                Result = std::make_unique<ContinueStmt>(peekCurr().line, peekCurr().column);
            }

            // consume break/Consume
            getNextToken();

            if (!expectAndConsume(TokenType::SEMICOLON, "Expected ';'"))
                return nullptr;

            return Result;
        } break;

        case TokenType::RETURN: {
            return ParseReturnStmt();
        } break;

        default: {
            return ParseExprStmt();
        }
    }
}
