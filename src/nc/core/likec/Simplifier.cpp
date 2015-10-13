#include "Simplifier.h"

#include <nc/common/Foreach.h>
#include <nc/common/Unreachable.h>
#include <nc/common/make_unique.h>

#include "BinaryOperator.h"
#include "CallOperator.h"
#include "CompilationUnit.h"
#include "Declaration.h"
#include "DoWhile.h"
#include "Expression.h"
#include "ExpressionStatement.h"
#include "FunctionDefinition.h"
#include "Goto.h"
#include "If.h"
#include "IntegerConstant.h"
#include "LabelStatement.h"
#include "MemberAccessOperator.h"
#include "Return.h"
#include "Statement.h"
#include "StructType.h"
#include "Switch.h"
#include "Tree.h"
#include "Typecast.h"
#include "Types.h"
#include "UnaryOperator.h"
#include "Utils.h"
#include "While.h"

namespace nc {
namespace core {
namespace likec {

namespace {

struct IsNull {
    template<class T>
    bool operator()(const T& item) const {
        return !item;
    }
};

template<class T, class U>
std::unique_ptr<T> as(std::unique_ptr<U> ptr) {
    assert(ptr);
    auto result = std::unique_ptr<T>(ptr.release()->template as<T>());
    assert(result);
    return result;
}

} // anonymous namespace

Simplifier::Simplifier(Tree &tree) : typeCalculator_(tree) {
}

std::unique_ptr<CompilationUnit> Simplifier::simplify(std::unique_ptr<CompilationUnit> node) {
    node->declarations() = simplify(std::move(node->declarations()));
    return node;
}

std::unique_ptr<Declaration> Simplifier::simplify(std::unique_ptr<Declaration> node) {
    switch (node->declarationKind()) {
        case Declaration::FUNCTION_DECLARATION:
            return node;
        case Declaration::FUNCTION_DEFINITION:
            return simplify(as<FunctionDefinition>(std::move(node)));
        case Declaration::LABEL_DECLARATION:
            return simplify(as<LabelDeclaration>(std::move(node)));
        case Declaration::MEMBER_DECLARATION:
            return node;
        case Declaration::STRUCT_TYPE_DECLARATION:
            return node;
        case Declaration::VARIABLE_DECLARATION:
            return simplify(as<VariableDeclaration>(std::move(node)));
    }
    unreachable();
}

std::unique_ptr<FunctionDefinition> Simplifier::simplify(std::unique_ptr<FunctionDefinition> node) {
    node->block() = simplify(std::move(node->block()));
    node->labels() = simplify(std::move(node->labels()));
    return node;
}

std::unique_ptr<LabelDeclaration> Simplifier::simplify(std::unique_ptr<LabelDeclaration> node) {
    return node;
}

std::unique_ptr<VariableDeclaration> Simplifier::simplify(std::unique_ptr<VariableDeclaration> node) {
    if (node->initialValue()) {
        node->initialValue() = std::move(node->initialValue());
    }
    return node;
}

std::unique_ptr<Expression> Simplifier::simplify(std::unique_ptr<Expression> node) {
    switch (node->expressionKind()) {
        case Expression::BINARY_OPERATOR:
            return simplify(as<BinaryOperator>(std::move(node)));
        case Expression::CALL_OPERATOR:
            return simplify(as<CallOperator>(std::move(node)));
        case Expression::FUNCTION_IDENTIFIER:
            return node;
        case Expression::INTEGER_CONSTANT:
            return node;
        case Expression::LABEL_IDENTIFIER:
            return node;
        case Expression::MEMBER_ACCESS_OPERATOR:
            return simplify(as<MemberAccessOperator>(std::move(node)));
        case Expression::STRING:
            return node;
        case Expression::TYPECAST:
            return simplify(as<Typecast>(std::move(node)));
        case Expression::UNARY_OPERATOR:
            return simplify(as<UnaryOperator>(std::move(node)));
        case Expression::VARIABLE_IDENTIFIER:
            return node;
        case Expression::UNDECLARED_IDENTIFIER:
            return node;
    }
    unreachable();
}

std::unique_ptr<Expression> Simplifier::simplify(std::unique_ptr<BinaryOperator> node) {
    node->left() = simplify(std::move(node->left()));
    node->right() = simplify(std::move(node->right()));

    /* Remove typecasts of operands if this won't change anything. */
    switch (node->operatorKind()) {
        case BinaryOperator::ADD:
        case BinaryOperator::SUB:
        case BinaryOperator::MUL:
        case BinaryOperator::DIV:
        case BinaryOperator::REM: {
            auto rewrite = [&](std::unique_ptr<Expression> &left, std::unique_ptr<Expression> &right) {
                if (auto typecast = left->as<Typecast>()) {
                    if (typecast->type()->size() >= typeCalculator_.getType(typecast->operand().get())->size()) {
                        if (typeCalculator_.getType(node.get()) ==
                            typeCalculator_.getBinaryOperatorType(node->operatorKind(), typecast->operand().get(),
                                                                  right.get())) {
                            left = std::move(typecast->operand());
                        }
                    }
                }
            };
            rewrite(node->left(), node->right());
            rewrite(node->right(), node->left());
            break;
        }
        default:
            break;
    }

    /* Rewrite computing of member address. */
    if (node->operatorKind() == BinaryOperator::ADD) {
        auto rewrite = [&](Expression *left, Expression *right) -> std::unique_ptr<Expression> {
            if (auto typecast = left->as<Typecast>()) {
                if (typecast->type()->isInteger() &&
                    typecast->type()->size() == typeCalculator_.getType(typecast->operand().get())->size()) {
                    if (auto pointerType = typeCalculator_.getType(typecast->operand().get())->as<PointerType>()) {
                        if (auto structType = pointerType->pointeeType()->as<StructType>()) {
                            if (auto constant = right->as<IntegerConstant>()) {
                                if (const MemberDeclaration *member =
                                        structType->getMember(constant->value().value() * CHAR_BIT)) {
                                    return simplify(std::make_unique<UnaryOperator>(
                                        UnaryOperator::REFERENCE,
                                        std::make_unique<MemberAccessOperator>(
                                            MemberAccessOperator::ARROW, std::move(typecast->operand()), member)));
                                }
                            }
                        }
                    }
                }
            }
            return nullptr;
        };
        if (auto result = rewrite(node->left().get(), node->right().get())) {
            return result;
        }
        if (auto result = rewrite(node->right().get(), node->left().get())) {
            return result;
        }
    }

    /*
     * Handling pointer arithmetic:
     *
     * rdi2 = (int32_t*)((int64_t)rdi2 + 4); -> rdi2 = (int32_t*)(int64_t)(rdi2 + 1);
     */

    auto rewritePointerArithmetic = [&](Expression *left, Expression *right) -> std::unique_ptr<BinaryOperator> {
        if (auto typecast = left->as<Typecast>()) {
            if (typecast->type()->isInteger() &&
                typecast->type()->size() == typeCalculator_.getType(typecast->operand().get())->size()) {
                if (auto pointerType = typeCalculator_.getType(typecast->operand().get())->as<PointerType>()) {
                    if (pointerType->pointeeType()->size() != 0 && pointerType->pointeeType()->size() % CHAR_BIT == 0) {
                        if (auto quotient = divide(right, pointerType->pointeeType()->size() / CHAR_BIT)) {
                            return std::make_unique<BinaryOperator>(
                                node->operatorKind(), std::move(typecast->operand()), std::move(quotient));
                        }
                    }
                }
            }
        }
        return nullptr;
    };

    switch (node->operatorKind()) {
        case BinaryOperator::ADD:
            if (auto result = rewritePointerArithmetic(node->left().get(), node->right().get())) {
                return simplify(std::move(result));
            }
            if (auto result = rewritePointerArithmetic(node->right().get(), node->left().get())) {
                return simplify(std::move(result));
            }
            break;
        case BinaryOperator::SUB:
            if (auto result = rewritePointerArithmetic(node->left().get(), node->right().get())) {
                return simplify(std::move(result));
            }
            break;
        default:
            break;
    }

    /*
     * Handle mathematical identities.
     */
    switch (node->operatorKind()) {
        case BinaryOperator::ADD: {
            if (isZero(node->left().get())) {
                auto type = typeCalculator_.getType(node.get());
                return simplify(std::make_unique<Typecast>(Typecast::STATIC_CAST, type, std::move(node->right())));
            }
            if (isZero(node->right().get())) {
                auto type = typeCalculator_.getType(node.get());
                return simplify(std::make_unique<Typecast>(Typecast::STATIC_CAST, type, std::move(node->left())));
            }
            break;
        }
        case BinaryOperator::SUB: {
            if (isZero(node->right().get())) {
                auto type = typeCalculator_.getType(node.get());
                return simplify(std::make_unique<Typecast>(Typecast::STATIC_CAST, type, std::move(node->left())));
            }
            if (isZero(node->left().get())) {
                auto type = typeCalculator_.getType(node.get());
                return simplify(std::make_unique<UnaryOperator>(
                    UnaryOperator::NEGATION,
                    std::make_unique<Typecast>(Typecast::STATIC_CAST, type, std::move(node->right()))));
            }
            break;
        }
        case BinaryOperator::MUL: {
            if (isOne(node->left().get())) {
                auto type = typeCalculator_.getType(node.get());
                return simplify(std::make_unique<Typecast>(Typecast::STATIC_CAST, type, std::move(node->right())));
            }
            if (isOne(node->right().get())) {
                auto type = typeCalculator_.getType(node.get());
                return simplify(std::make_unique<Typecast>(Typecast::STATIC_CAST, type, std::move(node->left())));
            }
            break;
        }
        case BinaryOperator::SHL:
        case BinaryOperator::SHR: {
            if (isZero(node->right().get())) {
                auto type = typeCalculator_.getType(node.get());
                return simplify(std::make_unique<Typecast>(Typecast::STATIC_CAST, type, std::move(node->left())));
            }
            break;
        }
        case BinaryOperator::BITWISE_OR:
        case BinaryOperator::BITWISE_XOR:
        case BinaryOperator::LOGICAL_OR: {
            if (isZero(node->left().get())) {
                auto type = typeCalculator_.getType(node.get());
                return simplify(std::make_unique<Typecast>(Typecast::STATIC_CAST, type, std::move(node->right())));
            }
            if (isZero(node->right().get())) {
                auto type = typeCalculator_.getType(node.get());
                return simplify(std::make_unique<Typecast>(Typecast::STATIC_CAST, type, std::move(node->left())));
            }
            break;
        }
        case BinaryOperator::LOGICAL_AND: {
            if (isOne(node->right().get())) {
                auto type = typeCalculator_.getType(node.get());
                return simplify(std::make_unique<Typecast>(Typecast::STATIC_CAST, type, std::move(node->left())));
            }
            if (isOne(node->left().get())) {
                auto type = typeCalculator_.getType(node.get());
                return simplify(std::make_unique<Typecast>(Typecast::STATIC_CAST, type, std::move(node->right())));
            }
            break;
        }
    }

    /* Simplifying boolean subexpressions. */
    switch (node->operatorKind()) {
        case BinaryOperator::LOGICAL_OR:
        case BinaryOperator::LOGICAL_AND: {
            node->left() = simplifyBooleanExpression(std::move(node->left()));
            node->right() = simplifyBooleanExpression(std::move(node->right()));
            break;
        }
    }

    /*
     * a + -1 -> a - 1
     * a - -1 -> a + 1
     */
    switch (node->operatorKind()) {
        case BinaryOperator::ADD: {
            if (auto constant = node->left()->as<IntegerConstant>()) {
                if (constant->type()->isSigned() && constant->value().size() > 1 && constant->value().signedValue() < 0) {
                    node->setOperatorKind(BinaryOperator::SUB);
                    constant->setValue(SizedValue(constant->value().size(), constant->value().absoluteValue()));
                }
            }
            if (auto constant = node->right()->as<IntegerConstant>()) {
                if (constant->type()->isSigned() && constant->value().size() > 1 && constant->value().signedValue() < 0) {
                    node->setOperatorKind(BinaryOperator::SUB);
                    constant->setValue(SizedValue(constant->value().size(), constant->value().absoluteValue()));
                }
            }
            break;
        }
        case BinaryOperator::SUB: {
            if (auto constant = node->right()->as<IntegerConstant>()) {
                if (constant->type()->isSigned() && constant->value().size() > 1 && constant->value().signedValue() < 0) {
                    node->setOperatorKind(BinaryOperator::ADD);
                    constant->setValue(SizedValue(constant->value().size(), constant->value().absoluteValue()));
                }
            }
            break;
        }
    }

    /*
     * Handling increments and decrements.
     *
     * x = x + 1; -> ++x;
     * x = x - 1; -> --x;
     */
    if (node->operatorKind() == BinaryOperator::ASSIGN) {
        if (VariableIdentifier *leftIdent = node->left()->as<VariableIdentifier>()) {
            if (BinaryOperator *binary = node->right()->as<BinaryOperator>()) {

                auto rewrite = [&](const Expression *left, const Expression *right) -> std::unique_ptr<Expression> {
                    if (auto rightIdent = left->as<VariableIdentifier>()) {
                        if (leftIdent->declaration() == rightIdent->declaration()) {
                            if (auto constant = right->as<IntegerConstant>()) {
                                if (constant->value().value() == 1) {
                                    return std::make_unique<UnaryOperator>(binary->operatorKind() == BinaryOperator::ADD
                                                                               ? UnaryOperator::PREFIX_INCREMENT
                                                                               : UnaryOperator::PREFIX_DECREMENT,
                                                                           std::move(node->left()));
                                } else if (constant->value().signedValue() == -1) {
                                    return std::make_unique<UnaryOperator>(binary->operatorKind() == BinaryOperator::ADD
                                                                               ? UnaryOperator::PREFIX_DECREMENT
                                                                               : UnaryOperator::PREFIX_INCREMENT,
                                                                           std::move(node->left()));
                                }
                            }
                        }
                    }
                    return nullptr;
                };
                if (binary->operatorKind() == BinaryOperator::ADD) {
                    if (auto result = rewrite(binary->left().get(), binary->right().get())) {
                        return result;
                    }
                    if (auto result = rewrite(binary->right().get(), binary->left().get())) {
                        return result;
                    }
                } else if (binary->operatorKind() == BinaryOperator::SUB) {
                    if (auto result = rewrite(binary->left().get(), binary->right().get())) {
                        return result;
                    }
                }
            }
        }
    }

    return std::move(node);
}

std::unique_ptr<CallOperator> Simplifier::simplify(std::unique_ptr<CallOperator> node) {
    node->callee() = simplify(std::move(node->callee()));
    node->arguments() = simplify(std::move(node->arguments()));
    return node;
}

std::unique_ptr<MemberAccessOperator> Simplifier::simplify(std::unique_ptr<MemberAccessOperator> node) {
    node->compound() = std::move(node->compound());
    return node;
}

std::unique_ptr<Expression> Simplifier::simplify(std::unique_ptr<Typecast> node) {
    node->operand() = simplify(std::move(node->operand()));

    /* Convert cast of pointer to a structure to a cast of pointer to its first field. */
    if (node->type()->isPointer() && !node->type()->isStructurePointer()) {
        if (auto pointerType = typeCalculator_.getType(node->operand().get())->as<PointerType>()) {
            if (const StructType *structType = pointerType->pointeeType()->as<StructType>()) {
                if (const MemberDeclaration *member = structType->getMember(0)) {
                    node->operand() = std::make_unique<UnaryOperator>(
                        UnaryOperator::REFERENCE,
                        std::make_unique<MemberAccessOperator>(MemberAccessOperator::ARROW,
                                                               std::move(node->operand()), member));
                }
            }
        }
    }

    /*
     * (int32_t*)(int64_t)expr -> (int32_t*)expr
     */
    if (node->type()->isScalar()) {
        if (auto typecast = node->operand()->as<Typecast>()) {
            auto operandType = typeCalculator_.getType(typecast->operand().get());

            if (typecast->type()->isScalar() &&
                operandType->isScalar() &&
                node->type()->size() == typecast->type()->size() &&
                typecast->type()->size() == operandType->size())
            {
                node->operand() = std::move(typecast->operand());
            }
        }
    }

    /* This really must be the last rule. */
    if (node->type() == typeCalculator_.getType(node->operand().get())) {
        return std::move(node->operand());
    }

    return std::move(node);
}

std::unique_ptr<Expression> Simplifier::simplify(std::unique_ptr<UnaryOperator> node) {
    node->operand() = simplify(std::move(node->operand()));

    if (node->operatorKind() == UnaryOperator::BITWISE_NOT &&
        typeCalculator_.getType(node->operand().get())->size() == 1) {
        node->setOperatorKind(UnaryOperator::LOGICAL_NOT);
    }

    switch (node->operatorKind()) {
        case UnaryOperator::DEREFERENCE: {
            if (auto unary = node->operand()->as<UnaryOperator>()) {
                if (unary->operatorKind() == UnaryOperator::REFERENCE) {
                    return std::move(unary->operand());
                }
            }
            if (auto binary = node->operand()->as<BinaryOperator>()) {
                if (binary->operatorKind() == BinaryOperator::ADD) {
                    if (typeCalculator_.getType(binary->left().get())->isPointer()) {
                        return std::make_unique<BinaryOperator>(BinaryOperator::ARRAY_SUBSCRIPT,
                                                                std::move(binary->left()), std::move(binary->right()));
                    } else if (typeCalculator_.getType(binary->right().get())->isPointer()) {
                        return std::make_unique<BinaryOperator>(BinaryOperator::ARRAY_SUBSCRIPT,
                                                                std::move(binary->right()), std::move(binary->left()));
                    }
                }
            }
            break;
        }
        case UnaryOperator::LOGICAL_NOT: {
            node->operand() = simplifyBooleanExpression(std::move(node->operand()));

            if (auto binary = node->operand()->as<BinaryOperator>()) {
                switch (binary->operatorKind()) {
                    case BinaryOperator::EQ:
                        binary->setOperatorKind(BinaryOperator::NEQ);
                        return std::move(node->operand());
                    case BinaryOperator::NEQ:
                        binary->setOperatorKind(BinaryOperator::EQ);
                        return std::move(node->operand());
                    case BinaryOperator::LT:
                        binary->setOperatorKind(BinaryOperator::GEQ);
                        return std::move(node->operand());
                    case BinaryOperator::LEQ:
                        binary->setOperatorKind(BinaryOperator::GT);
                        return std::move(node->operand());
                    case BinaryOperator::GT:
                        binary->setOperatorKind(BinaryOperator::LEQ);
                        return std::move(node->operand());
                    case BinaryOperator::GEQ:
                        binary->setOperatorKind(BinaryOperator::LT);
                        return std::move(node->operand());
                    default:
                        break;
                }
            }
            if (auto unary = node->operand()->as<UnaryOperator>()) {
                if (unary->operatorKind() == UnaryOperator::LOGICAL_NOT) {
                    if (typeCalculator_.getType(unary->operand().get())->size() == 1) {
                        return std::move(unary->operand());
                    }
                }
            }
            break;
        }
        default:
            break;
    }

    return std::move(node);
}

std::unique_ptr<Expression> Simplifier::simplifyBooleanExpression(std::unique_ptr<Expression> node) {
    while (auto typecast = node->as<Typecast>()) {
        auto operandType = typeCalculator_.getType(typecast->operand().get());
        if (typecast->type()->isScalar() && operandType->isScalar() &&
            typecast->type()->size() >= operandType->size()) {
            node = std::move(typecast->operand());
        } else {
            break;
        }
    }

    if (auto unary1 = node->as<UnaryOperator>()) {
        if (unary1->operatorKind() == UnaryOperator::LOGICAL_NOT) {
            if (auto unary2 = unary1->operand()->as<UnaryOperator>()) {
                if (unary2->operatorKind() == UnaryOperator::LOGICAL_NOT) {
                    return simplifyBooleanExpression(std::move(unary2->operand()));
                }
            }
        }
    } else if (auto binary = node->as<BinaryOperator>()) {
        if (binary->operatorKind() == BinaryOperator::NEQ) {
            if (isZero(binary->right().get())) {
                return simplifyBooleanExpression(std::move(binary->left()));
            }
            if (isZero(binary->left().get())) {
                return simplifyBooleanExpression(std::move(binary->right()));
            }
        } else if (binary->operatorKind() == BinaryOperator::EQ) {
            if (isZero(binary->right().get())) {
                return simplify(std::make_unique<UnaryOperator>(UnaryOperator::LOGICAL_NOT, std::move(binary->left())));
            }
            if (isZero(binary->left().get())) {
                return simplify(
                    std::make_unique<UnaryOperator>(UnaryOperator::LOGICAL_NOT, std::move(binary->right())));
            }
        }
    }

    return node;
}

std::unique_ptr<Statement> Simplifier::simplify(std::unique_ptr<Statement> node) {
    switch (node->statementKind()) {
        case Statement::BLOCK:
            return simplify(as<Block>(std::move(node)));
        case Statement::BREAK:
            return node;
        case Statement::CONTINUE:
            return node;
        case Statement::DO_WHILE:
            return simplify(as<DoWhile>(std::move(node)));
        case Statement::EXPRESSION_STATEMENT:
            return simplify(as<ExpressionStatement>(std::move(node)));
        case Statement::GOTO:
            return simplify(as<Goto>(std::move(node)));
        case Statement::IF:
            return simplify(as<If>(std::move(node)));
        case Statement::LABEL_STATEMENT:
            return simplify(as<LabelStatement>(std::move(node)));
        case Statement::RETURN:
            return simplify(as<Return>(std::move(node)));
        case Statement::WHILE:
            return simplify(as<While>(std::move(node)));
        case Statement::INLINE_ASSEMBLY:
            return node;
        case Statement::SWITCH:
            return simplify(as<Switch>(std::move(node)));
        case Statement::CASE_LABEL:
            return node;
        case Statement::DEFAULT_LABEL:
            return node;
    }
    unreachable();
}

std::unique_ptr<Block> Simplifier::simplify(std::unique_ptr<Block> node) {
    node->declarations() = simplify(std::move(node->declarations()));
    node->statements() = simplify(std::move(node->statements()));
    return node;
}

std::unique_ptr<DoWhile> Simplifier::simplify(std::unique_ptr<DoWhile> node) {
    node->condition() = simplifyBooleanExpression(simplify(std::move(node->condition())));
    node->body() = simplify(std::move(node->body()));
    return node;
}

std::unique_ptr<ExpressionStatement> Simplifier::simplify(std::unique_ptr<ExpressionStatement> node) {
    node->expression() = simplify(std::move(node->expression()));
    return node;
}

std::unique_ptr<Goto> Simplifier::simplify(std::unique_ptr<Goto> node) {
    node->destination() = simplify(std::move(node->destination()));
    return node;
}

std::unique_ptr<If> Simplifier::simplify(std::unique_ptr<If> node) {
    node->thenStatement() = simplify(std::move(node->thenStatement()));

    if (node->elseStatement()) {
        node->elseStatement() = simplify(std::move(node->elseStatement()));

        if (auto block = node->elseStatement()->as<Block>()) {
            if (block->statements().empty()) {
                node->elseStatement() = nullptr;
            }
        }

        if (node->elseStatement()) {
            if (auto block = node->thenStatement()->as<Block>()) {
                if (block->statements().empty()) {
                    node->thenStatement() = std::move(node->elseStatement());
                    node->condition() = simplify(
                        std::make_unique<UnaryOperator>(UnaryOperator::LOGICAL_NOT, std::move(node->condition())));
                }
            }
        }
    }

    node->condition() = simplifyBooleanExpression(simplify(std::move(node->condition())));

    return node;
}

std::unique_ptr<LabelStatement> Simplifier::simplify(std::unique_ptr<LabelStatement> node) {
    if (node->identifier()->declaration()->referenceCount() == 0) {
        return nullptr;
    }
    return node;
}

std::unique_ptr<Return> Simplifier::simplify(std::unique_ptr<Return> node) {
    if (node->returnValue()) {
        node->returnValue() = simplify(std::move(node->returnValue()));
    }
    return node;
}

std::unique_ptr<While> Simplifier::simplify(std::unique_ptr<While> node) {
    node->condition() = simplifyBooleanExpression(simplify(std::move(node->condition())));
    node->body() = simplify(std::move(node->body()));
    return node;
}

std::unique_ptr<Switch> Simplifier::simplify(std::unique_ptr<Switch> node) {
    node->expression() = simplifyBooleanExpression(simplify(std::move(node->expression())));
    node->body() = simplify(std::move(node->body()));
    return node;
}

template<class T>
std::vector<T> Simplifier::simplify(std::vector<T> range) {
    foreach (auto &item, range) {
        item = simplify(std::move(item));
    }
    range.erase(std::remove_if(range.begin(), range.end(), IsNull()), range.end());
    return range;
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
