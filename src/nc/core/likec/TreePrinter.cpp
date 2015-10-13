/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "TreePrinter.h"

#include <QStringList>

#include <nc/common/Foreach.h>
#include <nc/common/Escaping.h>
#include <nc/common/Unreachable.h>

#include "ArgumentDeclaration.h"
#include "BinaryOperator.h"
#include "Block.h"
#include "Break.h"
#include "CallOperator.h"
#include "CaseLabel.h"
#include "CompilationUnit.h"
#include "Continue.h"
#include "DefaultLabel.h"
#include "DoWhile.h"
#include "Expression.h"
#include "ExpressionStatement.h"
#include "FunctionDefinition.h"
#include "Goto.h"
#include "If.h"
#include "InlineAssembly.h"
#include "IntegerConstant.h"
#include "LabelIdentifier.h"
#include "LabelStatement.h"
#include "MemberAccessOperator.h"
#include "Return.h"
#include "Statement.h"
#include "String.h"
#include "StructTypeDeclaration.h"
#include "Switch.h"
#include "Typecast.h"
#include "UnaryOperator.h"
#include "UndeclaredIdentifier.h"
#include "VariableDeclaration.h"
#include "While.h"

namespace nc {
namespace core {
namespace likec {

namespace {

/**
 * \param node Valid pointer to an expression.
 *
 * \return Precedence of the expression's operator:
 *  zero for non-operators,
 *  positive values for left-to-right associative operators,
 *  negative values for right-to-left associative operators.
 *
 * \see http://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Operator_precedence
 */
int getPrecedence(const Expression *node);
int getPrecedence(const BinaryOperator *node);
int getPrecedence(const MemberAccessOperator *node);
int getPrecedence(const Typecast *node);
int getPrecedence(const UnaryOperator *node);

int getPrecedence(const Expression *node) {
    switch (node->expressionKind()) {
        case Expression::BINARY_OPERATOR:
            return getPrecedence(node->as<BinaryOperator>());
        case Expression::CALL_OPERATOR:
            return 2;
        case Expression::FUNCTION_IDENTIFIER:
            return 0;
        case Expression::INTEGER_CONSTANT:
            return 0;
        case Expression::LABEL_IDENTIFIER:
            return 0;
        case Expression::MEMBER_ACCESS_OPERATOR:
            return getPrecedence(node->as<MemberAccessOperator>());
        case Expression::STRING:
            return 0;
        case Expression::TYPECAST:
            return getPrecedence(node->as<Typecast>());
        case Expression::UNARY_OPERATOR:
            return getPrecedence(node->as<UnaryOperator>());
        case Expression::VARIABLE_IDENTIFIER:
            return 0;
        case Expression::UNDECLARED_IDENTIFIER:
            return 0;
    }
    unreachable();
}

int getPrecedence(const BinaryOperator *node) {
    switch (node->operatorKind()) {
        case BinaryOperator::ARRAY_SUBSCRIPT:
            return 2;
        case BinaryOperator::MUL:
        case BinaryOperator::DIV:
        case BinaryOperator::REM:
            return 5;
        case BinaryOperator::ADD:
        case BinaryOperator::SUB:
            return 6;
        case BinaryOperator::SHL:
        case BinaryOperator::SHR:
            return 7;
        case BinaryOperator::LT:
        case BinaryOperator::LEQ:
        case BinaryOperator::GT:
        case BinaryOperator::GEQ:
            return 8;
        case BinaryOperator::EQ:
        case BinaryOperator::NEQ:
            return 9;
        case BinaryOperator::BITWISE_AND:
            return 10;
        case BinaryOperator::BITWISE_XOR:
            return 11;
        case BinaryOperator::BITWISE_OR:
            return 12;
        case BinaryOperator::LOGICAL_AND:
            return 13;
        case BinaryOperator::LOGICAL_OR:
            return 14;
        case BinaryOperator::ASSIGN:
            return -16;
        case BinaryOperator::COMMA:
            return 17;
    }
    unreachable();
}

int getPrecedence(const MemberAccessOperator *node) {
    switch (node->accessKind()) {
        case MemberAccessOperator::ARROW:
        case MemberAccessOperator::DOT:
            return 2;
    }
    unreachable();
}

int getPrecedence(const Typecast *node) {
    switch (node->castKind()) {
        case Typecast::C_STYLE_CAST:
            return -3;
        case Typecast::STATIC_CAST: /* FALLTHROUGH */
        case Typecast::REINTERPRET_CAST:
            return 2;
    }
    unreachable();
}

int getPrecedence(const UnaryOperator *node) {
    switch (node->operatorKind()) {
        case UnaryOperator::DEREFERENCE:
        case UnaryOperator::REFERENCE:
        case UnaryOperator::BITWISE_NOT:
        case UnaryOperator::LOGICAL_NOT:
        case UnaryOperator::NEGATION:
        case UnaryOperator::PREFIX_INCREMENT:
        case UnaryOperator::PREFIX_DECREMENT:
            return -3;
    }
    unreachable();
}

} // anonymous namespace

TreePrinter::TreePrinter(QTextStream &out, PrintCallback<const TreeNode *> *callback):
    out_(out), callback_(callback), indentStep_(4), indent_(0)
{}

void TreePrinter::print(const TreeNode *node) {
    assert(node);

    if (callback_) {
        callback_->onStartPrinting(node);
    }

    doPrint(node);

    if (callback_) {
        callback_->onEndPrinting(node);
    }
}

void TreePrinter::doPrint(const TreeNode *node) {
    switch (node->nodeKind()) {
        case TreeNode::COMPILATION_UNIT:
            doPrint(node->as<CompilationUnit>());
            break;
        case TreeNode::DECLARATION:
            doPrint(node->as<Declaration>());
            break;
        case TreeNode::EXPRESSION:
            doPrint(node->as<Expression>());
            break;
        case TreeNode::STATEMENT:
            doPrint(node->as<Statement>());
            break;
        default:
            unreachable();
    }
}

void TreePrinter::doPrint(const CompilationUnit *node) {
    foreach (const auto &declaration, node->declarations()) {
        out_ << endl;
        printIndent();
        print(declaration);
        out_ << endl;
    }
}

void TreePrinter::doPrint(const Declaration *node) {
    switch (node->declarationKind()) {
        case Declaration::FUNCTION_DECLARATION:
            doPrint(node->as<FunctionDeclaration>());
            break;
        case Declaration::FUNCTION_DEFINITION:
            doPrint(node->as<FunctionDefinition>());
            break;
        case Declaration::LABEL_DECLARATION:
            unreachable();
        case Declaration::MEMBER_DECLARATION:
            doPrint(node->as<MemberDeclaration>());
            break;
        case Declaration::STRUCT_TYPE_DECLARATION:
            doPrint(node->as<StructTypeDeclaration>());
            break;
        case Declaration::VARIABLE_DECLARATION:
            doPrint(node->as<VariableDeclaration>());
            break;
        default:
            unreachable();
    }
}

void TreePrinter::doPrint(const FunctionDeclaration *node) {
    printComment(node);
    printSignature(node);
    out_ << ';';
}

void TreePrinter::doPrint(const FunctionDefinition *node) {
    printComment(node);
    printSignature(node);
    out_ << ' ';
    print(node->block());
}

void TreePrinter::printSignature(const FunctionDeclaration *node) {
    out_ << *node->type()->returnType() << ' ';
    print(node->functionIdentifier());
    out_ << '(';

    bool comma = false;
    foreach (const auto &argument, node->arguments()) {
        if (comma) {
            out_ << ", ";
        } else {
            comma = true;
        }
        out_ << *argument->type() << ' ';
        print(argument->variableIdentifier());
    }

    if (node->type()->variadic()) {
        if (comma) {
            out_ << ", ";
        }
        out_ << "...";
    }

    out_ << ')';
}

void TreePrinter::doPrint(const MemberDeclaration *node) {
    out_ << *node->type() << ' ' << node->identifier() << ';';
}

void TreePrinter::doPrint(const StructTypeDeclaration *node) {
    out_ << "struct " << node->identifier() << " {" << endl;
    indentMore();
    foreach (const auto &member, node->type()->members()) {
        printIndent();
        print(member);
        out_ << endl;
    }
    indentLess();
    out_ << "};";
}

void TreePrinter::doPrint(const VariableDeclaration *node) {
    printComment(node);

    out_ << *node->type() << ' ';
    print(node->variableIdentifier());
    if (node->initialValue()) {
        out_ << " = ";
        print(node->initialValue());
    }
    out_ << ';';
}

void TreePrinter::doPrint(const Expression *node) {
    switch (node->expressionKind()) {
        case Expression::BINARY_OPERATOR:
            doPrint(node->as<BinaryOperator>());
            break;
        case Expression::CALL_OPERATOR:
            doPrint(node->as<CallOperator>());
            break;
        case Expression::FUNCTION_IDENTIFIER:
            doPrint(node->as<FunctionIdentifier>());
            break;
        case Expression::INTEGER_CONSTANT:
            doPrint(node->as<IntegerConstant>());
            break;
        case Expression::LABEL_IDENTIFIER:
            doPrint(node->as<LabelIdentifier>());
            break;
        case Expression::MEMBER_ACCESS_OPERATOR:
            doPrint(node->as<MemberAccessOperator>());
            break;
        case Expression::STRING:
            doPrint(node->as<String>());
            break;
        case Expression::TYPECAST:
            doPrint(node->as<Typecast>());
            break;
        case Expression::UNARY_OPERATOR:
            doPrint(node->as<UnaryOperator>());
            break;
        case Expression::VARIABLE_IDENTIFIER:
            doPrint(node->as<VariableIdentifier>());
            break;
        case Expression::UNDECLARED_IDENTIFIER:
            doPrint(node->as<UndeclaredIdentifier>());
            break;
        default:
            unreachable();
    }
}

void TreePrinter::doPrint(const BinaryOperator *node) {
    int precedence = getPrecedence(node);
    int absPrecedence = std::abs(precedence);

    int absLeftPrecedence = std::abs(getPrecedence(node->left()));
    bool leftInBraces =
        (absLeftPrecedence > absPrecedence) ||
        ((absLeftPrecedence == absPrecedence) && (precedence < 0));

    if (leftInBraces) {
        out_ << '(';
    }
    print(node->left());
    if (leftInBraces) {
        out_ << ')';
    }

    if (node->operatorKind() == BinaryOperator::ARRAY_SUBSCRIPT) {
        out_ << '[';
        print(node->right());
        out_ << ']';
        return;
    }

    switch (node->operatorKind()) {
        case BinaryOperator::ASSIGN:
            out_ << " = ";
            break;
        case BinaryOperator::ADD:
            out_ << " + ";
            break;
        case BinaryOperator::SUB:
            out_ << " - ";
            break;
        case BinaryOperator::MUL:
            out_ << " * ";
            break;
        case BinaryOperator::DIV:
            out_ << " / ";
            break;
        case BinaryOperator::REM:
            out_ << " % ";
            break;
        case BinaryOperator::BITWISE_AND:
            out_ << " & ";
            break;
        case BinaryOperator::LOGICAL_AND:
            out_ << " && ";
            break;
        case BinaryOperator::BITWISE_OR:
            out_ << " | ";
            break;
        case BinaryOperator::LOGICAL_OR:
            out_ << " || ";
            break;
        case BinaryOperator::BITWISE_XOR:
            out_ << " ^ ";
            break;
        case BinaryOperator::SHL:
            out_ << " << ";
            break;
        case BinaryOperator::SHR:
            out_ << " >> ";
            break;
        case BinaryOperator::EQ:
            out_ << " == ";
            break;
        case BinaryOperator::NEQ:
            out_ << " != ";
            break;
        case BinaryOperator::LT:
            out_ << " < ";
            break;
        case BinaryOperator::LEQ:
            out_ << " <= ";
            break;
        case BinaryOperator::GT:
            out_ << " > ";
            break;
        case BinaryOperator::GEQ:
            out_ << " >= ";
            break;
        case BinaryOperator::COMMA:
            out_ << ", ";
            break;
        default:
            unreachable();
    }

    int absRightPrecedence = std::abs(getPrecedence(node->right()));
    bool rightInBraces =
        (absRightPrecedence > absPrecedence) ||
        ((absRightPrecedence == absPrecedence) && (precedence > 0));

    if (rightInBraces) {
        out_ << '(';
    }
    print(node->right());
    if (rightInBraces) {
        out_ << ')';
    }
}

void TreePrinter::doPrint(const CallOperator *node) {
    print(node->callee());
    out_ << '(';
    bool comma = false;
    foreach (const auto &argument, node->arguments()) {
        if (comma) {
            out_ << ", ";
        } else {
            comma = true;
        }
        print(argument);
    }
    out_ << ')';
}

void TreePrinter::doPrint(const FunctionIdentifier *node) {
    out_ << node->declaration()->identifier();
}

void TreePrinter::doPrint(const IntegerConstant *node) {
    SignedConstantValue val = node->value().size() > 1 ? node->value().signedValue() : node->value().value();

    if ((0 <= val && val <= 100) || (-100 <= val && val < 0 && !node->type()->isUnsigned())) {
        out_ << val;
    } else {
        out_ << hex << "0x" << node->value().value() << dec;
    }
}

void TreePrinter::doPrint(const LabelIdentifier *node) {
    out_ << node->declaration()->identifier();
}

void TreePrinter::doPrint(const MemberAccessOperator *node) {
    bool braces = node->compound()->is<UnaryOperator>() ||
                  node->compound()->is<BinaryOperator>() ||
                  node->compound()->is<Typecast>();

    if (braces) {
        out_ << "(";
    }
    print(node->compound());
    if (braces) {
        out_ << ")";
    }

    switch (node->accessKind()) {
        case MemberAccessOperator::ARROW:
            out_ << "->";
            break;
        case MemberAccessOperator::DOT:
            out_ << '.';
            break;
        default:
            unreachable();
            break;
    }

    out_ << node->member()->identifier();
}

void TreePrinter::doPrint(const String *node) {
    out_ << '"' << escapeCString(node->characters()) << '"';
}

void TreePrinter::doPrint(const Typecast *node) {
    switch (node->castKind()) {
        case Typecast::C_STYLE_CAST: {
            int precedence = getPrecedence(node);
            int operandPrecedence = getPrecedence(node->operand());

            int absPrecedence = abs(precedence);
            int absOperandPrecedence = abs(operandPrecedence);

            bool operandInBraces = absOperandPrecedence > absPrecedence;

            out_ << '(' << *node->type() << ')';

            if (operandInBraces) {
                out_ << '(';
            }
            print(node->operand());
            if (operandInBraces) {
                out_ << ')';
            }
            break;
        }
        case Typecast::STATIC_CAST: {
            out_ << "static_cast<" << *node->type() << ">(";
            print(node->operand());
            out_ << ')';
            break;
        }
        case Typecast::REINTERPRET_CAST: {
            out_ << "reinterpret_cast<" << *node->type() << ">(";
            print(node->operand());
            out_ << ')';
            break;
        }
        default: {
            unreachable();
        }
    }
}

void TreePrinter::doPrint(const UnaryOperator *node) {
    switch (node->operatorKind()) {
        case UnaryOperator::DEREFERENCE:
            out_ << '*';
            break;
        case UnaryOperator::REFERENCE:
            out_ << '&';
            break;
        case UnaryOperator::BITWISE_NOT:
            out_ << '~';
            break;
        case UnaryOperator::LOGICAL_NOT:
            out_ << '!';
            break;
        case UnaryOperator::NEGATION:
            out_ << '-';
            break;
        case UnaryOperator::PREFIX_INCREMENT:
            out_ << "++";
            break;
        case UnaryOperator::PREFIX_DECREMENT:
            out_ << "--";
            break;
        default:
            unreachable();
            break;
    }

    int precedence = getPrecedence(node);
    int operandPrecedence = getPrecedence(node->operand());

    int absPrecedence = abs(precedence);
    int absOperandPrecedence = abs(operandPrecedence);

    bool operandInBraces = absOperandPrecedence > absPrecedence;

    /* Avoid too many minuses in a row. */
    if (node->operatorKind() == UnaryOperator::NEGATION || node->operatorKind() == UnaryOperator::PREFIX_DECREMENT) {
        if (auto unary = node->operand()->as<UnaryOperator>()) {
            if (unary->operatorKind() == UnaryOperator::NEGATION ||
                unary->operatorKind() == UnaryOperator::PREFIX_DECREMENT) {
                operandInBraces = true;
            }
        }
    }

    if (operandInBraces) {
        out_ << '(';
    }
    print(node->operand());
    if (operandInBraces) {
        out_ << ')';
    }
}

void TreePrinter::doPrint(const VariableIdentifier *node) {
    out_ << node->declaration()->identifier();
}

void TreePrinter::doPrint(const UndeclaredIdentifier *node) {
    out_ << node->name();
}

void TreePrinter::doPrint(const Statement *node) {
    switch (node->statementKind()) {
        case Statement::BLOCK:
            doPrint(node->as<Block>());
            break;
        case Statement::BREAK:
            doPrint(node->as<Break>());
            break;
        case Statement::CONTINUE:
            doPrint(node->as<Continue>());
            break;
        case Statement::DO_WHILE:
            doPrint(node->as<DoWhile>());
            break;
        case Statement::EXPRESSION_STATEMENT:
            doPrint(node->as<ExpressionStatement>());
            break;
        case Statement::GOTO:
            doPrint(node->as<Goto>());
            break;
        case Statement::IF:
            doPrint(node->as<If>());
            break;
        case Statement::LABEL_STATEMENT:
            doPrint(node->as<LabelStatement>());
            break;
        case Statement::RETURN:
            doPrint(node->as<Return>());
            break;
        case Statement::WHILE:
            doPrint(node->as<While>());
            break;
        case Statement::INLINE_ASSEMBLY:
            doPrint(node->as<InlineAssembly>());
            break;
        case Statement::SWITCH:
            doPrint(node->as<Switch>());
            break;
        case Statement::CASE_LABEL:
            doPrint(node->as<CaseLabel>());
            break;
        case Statement::DEFAULT_LABEL:
            doPrint(node->as<DefaultLabel>());
            break;
        default:
            unreachable();
    }
}

void TreePrinter::doPrint(const Block *node) {
    out_ << "{" << endl;
    indentMore();

    foreach (const auto &declaration, node->declarations()) {
        printIndent();
        print(declaration);
        out_ << endl;
    }

    if (!node->declarations().empty() && !node->statements().empty()) {
        out_ << endl;
    }

    foreach (const auto &statement, node->statements()) {
        bool isCaseLabel =
            statement->statementKind() == Statement::CASE_LABEL ||
            statement->statementKind() == Statement::DEFAULT_LABEL;

        if (isCaseLabel) {
            indentLess();
        }

        printIndent();
        print(statement);
        out_ << endl;

        if (isCaseLabel) {
            indentMore();
        }
    }

    indentLess();
    printIndent();
    out_ << "}";
}

void TreePrinter::doPrint(const Break *) {
    out_ << "break;";
}

void TreePrinter::doPrint(const Continue *) {
    out_ << "continue;";
}

void TreePrinter::doPrint(const DoWhile *node) {
    out_ << "do ";
    print(node->body());
    out_ << " while (";
    print(node->condition());
    out_ << ");";
}

void TreePrinter::doPrint(const ExpressionStatement *node) {
    print(node->expression());
    out_ << ';';
}

void TreePrinter::doPrint(const Goto *node) {
    out_ << "goto ";
    print(node->destination());
    out_ << ';';
}

void TreePrinter::doPrint(const If *node) {
    out_ << "if (";
    print(node->condition());
    out_ << ") ";
    printNestedStatement(node->thenStatement());
    if (node->elseStatement()) {
        out_ << " else ";
        printNestedStatement(node->elseStatement());
    }
}

void TreePrinter::doPrint(const LabelStatement *node) {
    print(node->identifier());
    out_ << ':';
}

void TreePrinter::doPrint(const Return *node) {
    if (node->returnValue()) {
        out_ << "return ";
        print(node->returnValue());
        out_ << ";";
    } else {
        out_ << "return;";
    }
}

void TreePrinter::doPrint(const While *node) {
    out_ << "while (";
    print(node->condition());
    out_ << ") ";
    printNestedStatement(node->body());
}

void TreePrinter::doPrint(const InlineAssembly *node) {
    out_ << "__asm__(\"" << node->code() << "\");";
}

void TreePrinter::doPrint(const Switch *node) {
    out_ << "switch (";
    print(node->expression());
    out_ << ") ";
    printNestedStatement(node->body());
}

void TreePrinter::doPrint(const CaseLabel *node) {
    out_ << "case ";
    print(node->expression());
    out_ << ":";
}

void TreePrinter::doPrint(const DefaultLabel *) {
    out_ << "default:";
}

void TreePrinter::printNestedStatement(const Statement *statement) {
    if (statement->is<Block>()) {
        print(statement);
    } else {
        out_ << endl;
        indentMore();
        printIndent();
        print(statement);
        indentLess();
    }
}

void TreePrinter::printComment(const Commentable *node) {
    if (node->comment().isEmpty()) {
        return;
    }

    QStringList lines = node->comment().split('\n');

    if (lines.size() == 1) {
        out_ << "/* " << lines.first() << " */" << endl;
    } else {
        out_ << "/*" << endl;
        foreach (const QString &line, lines) {
            printIndent();
            out_ << " * " << line << endl;
        }
        out_ << " */" << endl;
    }
}

void TreePrinter::indentMore() {
    indent_ += indentStep_;
}

void TreePrinter::indentLess() {
    indent_ -= indentStep_;
    assert(indent_ >= 0);
}

void TreePrinter::printIndent() {
    out_ << QString(indent_, ' ');
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
