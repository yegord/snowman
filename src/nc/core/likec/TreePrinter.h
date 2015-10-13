/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <QTextStream>

#include <nc/common/PrintCallback.h>

namespace nc {
namespace core {
namespace likec {

class BinaryOperator;
class Block;
class Break;
class CallOperator;
class CaseLabel;
class Commentable;
class CompilationUnit;
class Continue;
class Declaration;
class DefaultLabel;
class DoWhile;
class Expression;
class ExpressionStatement;
class FunctionDeclaration;
class FunctionDefinition;
class FunctionIdentifier;
class Goto;
class If;
class InlineAssembly;
class IntegerConstant;
class LabelIdentifier;
class LabelStatement;
class MemberAccessOperator;
class MemberDeclaration;
class Return;
class Statement;
class String;
class StructTypeDeclaration;
class Switch;
class TreeNode;
class Typecast;
class UnaryOperator;
class UndeclaredIdentifier;
class VariableDeclaration;
class VariableIdentifier;
class While;

/**
 * This class can print tree nodes into a stream.
 */
class TreePrinter {
    QTextStream &out_; ///< Output stream.
    PrintCallback<const TreeNode *> *callback_; ///< Print callback.
    int indentStep_; ///< Size of a single indentation step.
    int indent_; ///< Current indentation.

public:
    /**
     * \param out Output stream.
     * \param callback Pointer to the print callback. Can be NULL.
     */
    TreePrinter(QTextStream &out, PrintCallback<const TreeNode *> *callback);

    /**
     * Prints the given node to the stream passed to the constructor.
     *
     * \param node Valid pointer to a node.
     */
    void print(const TreeNode *node);

private:
    void doPrint(const TreeNode *node);

    void doPrint(const CompilationUnit *node);

    void doPrint(const Declaration *node);
    void doPrint(const FunctionDeclaration *node);
    void doPrint(const FunctionDefinition *node);
    void printSignature(const FunctionDeclaration *node);
    void doPrint(const MemberDeclaration *node);
    void doPrint(const StructTypeDeclaration *node);
    void doPrint(const VariableDeclaration *node);

    void doPrint(const Expression *node);
    void doPrint(const BinaryOperator *node);
    void doPrint(const CallOperator *node);
    void doPrint(const FunctionIdentifier *node);
    void doPrint(const IntegerConstant *node);
    void doPrint(const LabelIdentifier *node);
    void doPrint(const MemberAccessOperator *node);
    void doPrint(const String *node);
    void doPrint(const Typecast *node);
    void doPrint(const UnaryOperator *node);
    void doPrint(const VariableIdentifier *node);
    void doPrint(const UndeclaredIdentifier *node);

    void doPrint(const Statement *node);
    void doPrint(const Block *node);
    void doPrint(const Break *node);
    void doPrint(const Continue *node);
    void doPrint(const DoWhile *node);
    void doPrint(const ExpressionStatement *node);
    void doPrint(const Goto *node);
    void doPrint(const If *node);
    void doPrint(const LabelStatement *node);
    void doPrint(const Return *node);
    void doPrint(const While *node);
    void doPrint(const InlineAssembly *node);
    void doPrint(const Switch *node);
    void doPrint(const CaseLabel *node);
    void doPrint(const DefaultLabel *node);

    void printNestedStatement(const Statement *statement);
    void printComment(const Commentable *node);

    void indentMore();
    void indentLess();
    void printIndent();
};

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
