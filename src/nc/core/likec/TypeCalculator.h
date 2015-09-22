/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <boost/unordered_map.hpp>

namespace nc {
namespace core {
namespace likec {

class BinaryOperator;
class CallOperator;
class Expression;
class FunctionIdentifier;
class IntegerConstant;
class LabelIdentifier;
class MemberAccessOperator;
class String;
class Tree;
class Type;
class Typecast;
class UnaryOperator;
class UndeclaredIdentifier;
class VariableIdentifier;

class TypeCalculator {
    Tree &tree_;

public:
    explicit TypeCalculator(Tree &tree): tree_(tree) {}

    const Type *getType(const Expression *node);
    const Type *getType(const BinaryOperator *node);
    const Type *getType(const CallOperator *node);
    const Type *getType(const FunctionIdentifier *node);
    const Type *getType(const IntegerConstant *node);
    const Type *getType(const LabelIdentifier *node);
    const Type *getType(const MemberAccessOperator *node);
    const Type *getType(const String *node);
    const Type *getType(const Typecast *node);
    const Type *getType(const UnaryOperator *node);
    const Type *getType(const VariableIdentifier *node);
    const Type *getType(const UndeclaredIdentifier *node);
    const Type *getBinaryOperatorType(int operatorKind, const Expression *left, const Expression *right);
};

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
