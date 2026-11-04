#include "../include/value.hpp"

namespace algo {

Value Value::add(const Value& other) const {
    if (type == ValueType::String || other.type == ValueType::String) {
        return Value(asString() + other.asString());
    }
    if (type == ValueType::Real || other.type == ValueType::Real) {
        return Value(asReal() + other.asReal());
    }
    return Value(asInt() + other.asInt());
}

Value Value::sub(const Value& other) const {
    if (type == ValueType::Real || other.type == ValueType::Real) {
        return Value(asReal() - other.asReal());
    }
    return Value(asInt() - other.asInt());
}

Value Value::mul(const Value& other) const {
    if (type == ValueType::Real || other.type == ValueType::Real) {
        return Value(asReal() * other.asReal());
    }
    return Value(asInt() * other.asInt());
}

Value Value::divReal(const Value& other) const {
    double r = other.asReal();
    if (r == 0.0) {
        throw AlgoError("Division par zéro");
    }
    return Value(asReal() / r);
}

Value Value::divInt(const Value& other) const {
    int64_t r = other.asInt();
    if (r == 0) {
        throw AlgoError("Division entière par zéro");
    }
    // Floor division
    double res = std::floor((double)asInt() / (double)r);
    return Value((int64_t)res);
}

Value Value::mod(const Value& other) const {
    int64_t r = other.asInt();
    if (r == 0) {
        throw AlgoError("Modulo par zéro");
    }
    return Value(asInt() % r);
}

bool Value::equals(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        if (type == ValueType::Real || other.type == ValueType::Real) {
            return std::abs(asReal() - other.asReal()) < 1e-9;
        }
        return asInt() == other.asInt();
    }
    if (type == ValueType::String && other.type == ValueType::String) {
        return strVal == other.strVal;
    }
    if (type == ValueType::Bool && other.type == ValueType::Bool) {
        return boolVal == other.boolVal;
    }
    if (type == ValueType::Null && other.type == ValueType::Null) {
        return true;
    }
    if (type == ValueType::Record && other.type == ValueType::Record) {
        if (recVal.size() != other.recVal.size()) return false;
        for (const auto& kv : recVal) {
            auto it = other.recVal.find(kv.first);
            if (it == other.recVal.end()) return false;
            if (!kv.second.equals(it->second)) return false;
        }
        return true;
    }
    return false;
}

bool Value::lessThan(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        return asReal() < other.asReal();
    }
    if (type == ValueType::String && other.type == ValueType::String) {
        return strVal < other.strVal;
    }
    return false;
}

bool Value::lessOrEqual(const Value& other) const {
    return lessThan(other) || equals(other);
}

bool Value::greaterThan(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        return asReal() > other.asReal();
    }
    if (type == ValueType::String && other.type == ValueType::String) {
        return strVal > other.strVal;
    }
    return false;
}

bool Value::greaterOrEqual(const Value& other) const {
    return greaterThan(other) || equals(other);
}

bool Value::contains(const Value& elem) const {
    if (type == ValueType::Array) {
        for (const auto& item : arrVal) {
            if (item.equals(elem)) return true;
        }
        return false;
    }
    if (type == ValueType::String) {
        std::string needle = elem.asString();
        return strVal.find(needle) != std::string::npos;
    }
    throw AlgoError("Opérande droit de '∈' doit être un ensemble ou un tableau");
}

} // namespace algo
