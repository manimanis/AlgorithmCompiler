#pragma once

#include "common.hpp"

namespace algo {

enum class ValueType {
    Null,
    Int,
    Real,
    Bool,
    String,
    Array,
    Record,
    FileHandle,
    Ref
};

class Value {
public:
    ValueType type;
    int64_t intVal;
    double realVal;
    bool boolVal;
    std::string strVal;
    std::vector<Value> arrVal;
    std::map<std::string, Value> recVal;
    std::string fileLogicalName;
    Value* refPtr = nullptr;

    Value() : type(ValueType::Null), intVal(0), realVal(0.0), boolVal(false), refPtr(nullptr) {}
    Value(int64_t v) : type(ValueType::Int), intVal(v), realVal((double)v), boolVal(false), refPtr(nullptr) {}
    Value(int v) : type(ValueType::Int), intVal(v), realVal((double)v), boolVal(false), refPtr(nullptr) {}
    Value(double v) : type(ValueType::Real), intVal((int64_t)v), realVal(v), boolVal(false), refPtr(nullptr) {}
    Value(bool v) : type(ValueType::Bool), intVal(v ? 1 : 0), realVal(0.0), boolVal(v), refPtr(nullptr) {}
    Value(std::string v) : type(ValueType::String), intVal(0), realVal(0.0), boolVal(false), strVal(std::move(v)), refPtr(nullptr) {}
    Value(const char* v) : type(ValueType::String), intVal(0), realVal(0.0), boolVal(false), strVal(v), refPtr(nullptr) {}
    Value(std::vector<Value> v) : type(ValueType::Array), intVal(0), realVal(0.0), boolVal(false), arrVal(std::move(v)), refPtr(nullptr) {}
    Value(std::map<std::string, Value> v) : type(ValueType::Record), intVal(0), realVal(0.0), boolVal(false), recVal(std::move(v)), refPtr(nullptr) {}

    static Value makeRef(Value* ptr) {
        Value v;
        v.type = ValueType::Ref;
        v.refPtr = ptr;
        return v;
    }

    bool isNull() const { return (type == ValueType::Ref && refPtr) ? refPtr->isNull() : type == ValueType::Null; }
    bool isInt() const { return (type == ValueType::Ref && refPtr) ? refPtr->isInt() : type == ValueType::Int; }
    bool isReal() const { return (type == ValueType::Ref && refPtr) ? refPtr->isReal() : type == ValueType::Real; }
    bool isNumber() const { return (type == ValueType::Ref && refPtr) ? refPtr->isNumber() : (type == ValueType::Int || type == ValueType::Real); }
    bool isBool() const { return (type == ValueType::Ref && refPtr) ? refPtr->isBool() : type == ValueType::Bool; }
    bool isString() const { return (type == ValueType::Ref && refPtr) ? refPtr->isString() : type == ValueType::String; }
    bool isArray() const { return (type == ValueType::Ref && refPtr) ? refPtr->isArray() : type == ValueType::Array; }
    bool isRecord() const { return (type == ValueType::Ref && refPtr) ? refPtr->isRecord() : type == ValueType::Record; }
    bool isFile() const { return (type == ValueType::Ref && refPtr) ? refPtr->isFile() : type == ValueType::FileHandle; }
    bool isRef() const { return type == ValueType::Ref; }

    int64_t asInt() const {
        if (type == ValueType::Ref && refPtr) return refPtr->asInt();
        if (type == ValueType::Int) return intVal;
        if (type == ValueType::Real) return (int64_t)realVal;
        if (type == ValueType::Bool) return boolVal ? 1 : 0;
        return 0;
    }

    double asReal() const {
        if (type == ValueType::Ref && refPtr) return refPtr->asReal();
        if (type == ValueType::Real) return realVal;
        if (type == ValueType::Int) return (double)intVal;
        return 0.0;
    }

    double toNumber() const {
        if (type == ValueType::Ref && refPtr) return refPtr->toNumber();
        if (type == ValueType::Int) return (double)intVal;
        if (type == ValueType::Real) return realVal;
        return 0.0;
    }

    bool asBool() const {
        if (type == ValueType::Ref && refPtr) return refPtr->asBool();
        if (type == ValueType::Bool) return boolVal;
        if (type == ValueType::Int) return intVal != 0;
        if (type == ValueType::Real) return realVal != 0.0;
        if (type == ValueType::String) return !strVal.empty();
        return false;
    }

    std::string asString() const {
        if (type == ValueType::Ref && refPtr) return refPtr->asString();
        if (type == ValueType::String) return strVal;
        if (type == ValueType::Bool) return boolVal ? "Vrai" : "Faux";
        if (type == ValueType::Int) return std::to_string(intVal);
        if (type == ValueType::Real) {
            std::ostringstream ss;
            ss.precision(10);
            ss << realVal;
            std::string s = ss.str();
            return s;
        }
        if (type == ValueType::Null) return "";
        if (type == ValueType::Record) {
            std::string s = "{";
            bool first = true;
            for (const auto& kv : recVal) {
                if (!first) s += ", ";
                first = false;
                s += kv.first + ": " + kv.second.asString();
            }
            s += "}";
            return s;
        }
        if (type == ValueType::FileHandle) return fileLogicalName;
        return "";
    }

    // Binary operations
    Value add(const Value& other) const;
    Value sub(const Value& other) const;
    Value mul(const Value& other) const;
    Value divReal(const Value& other) const;
    Value divInt(const Value& other) const;
    Value mod(const Value& other) const;

    // Comparisons
    bool equals(const Value& other) const;
    bool notEquals(const Value& other) const { return !equals(other); }
    bool lessThan(const Value& other) const;
    bool lessOrEqual(const Value& other) const;
    bool greaterThan(const Value& other) const;
    bool greaterOrEqual(const Value& other) const;
    bool contains(const Value& elem) const;
};

} // namespace algo
