/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>
#include <vector>

#include "TypeCalculator.h"

namespace nc {
namespace core {
namespace likec {

class BinaryOperator;
class Block;
class CallOperator;
class CompilationUnit;
class Declaration;
class DoWhile;
class Expression;
class ExpressionStatement;
class FunctionDefinition;
class Goto;
class If;
class LabelDeclaration;
class LabelStatement;
class MemberAccessOperator;
class Return;
class Statement;
class Switch;
class Tree;
class TreeNode;
class Typecast;
class UnaryOperator;
class VariableDeclaration;
class While;

class Simplifier {
    Tree &tree_;
    TypeCalculator typeCalculator_;

public:
    explicit Simplifier(Tree &tree);

    std::unique_ptr<TreeNode> simplify(std::unique_ptr<TreeNode> node);
    std::unique_ptr<CompilationUnit> simplify(std::unique_ptr<CompilationUnit> node);

private:
    std::unique_ptr<Declaration> simplify(std::unique_ptr<Declaration> node);
    std::unique_ptr<FunctionDefinition> simplify(std::unique_ptr<FunctionDefinition> node);
    std::unique_ptr<LabelDeclaration> simplify(std::unique_ptr<LabelDeclaration> node);
    std::unique_ptr<VariableDeclaration> simplify(std::unique_ptr<VariableDeclaration> node);

    std::unique_ptr<Expression> simplify(std::unique_ptr<Expression> node);
    std::unique_ptr<Expression> simplify(std::unique_ptr<BinaryOperator> node);
    std::unique_ptr<CallOperator> simplify(std::unique_ptr<CallOperator> node);
    std::unique_ptr<MemberAccessOperator> simplify(std::unique_ptr<MemberAccessOperator> node);
    std::unique_ptr<Expression> simplify(std::unique_ptr<Typecast> node);
    std::unique_ptr<Expression> simplify(std::unique_ptr<UnaryOperator> node);

    std::unique_ptr<Statement> simplify(std::unique_ptr<Statement> node);
    std::unique_ptr<Block> simplify(std::unique_ptr<Block> node);
    std::unique_ptr<DoWhile> simplify(std::unique_ptr<DoWhile> node);
    std::unique_ptr<ExpressionStatement> simplify(std::unique_ptr<ExpressionStatement> node);
    std::unique_ptr<Goto> simplify(std::unique_ptr<Goto> node);
    std::unique_ptr<If> simplify(std::unique_ptr<If> node);
    std::unique_ptr<LabelStatement> simplify(std::unique_ptr<LabelStatement> node);
    std::unique_ptr<Return> simplify(std::unique_ptr<Return> node);
    std::unique_ptr<While> simplify(std::unique_ptr<While> node);
    std::unique_ptr<Switch> simplify(std::unique_ptr<Switch> node);
    std::unique_ptr<Expression> simplifyBooleanExpression(std::unique_ptr<Expression> node);

    template<class T>
    std::vector<T> simplify(std::vector<T> range);
};

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
