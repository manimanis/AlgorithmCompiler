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
    const Value* a = (type == ValueType::Ref && refPtr) ? refPtr : this;
    const Value* b = (other.type == ValueType::Ref && other.refPtr) ? other.refPtr : &other;

    if (a->type == ValueType::Pointer || b->type == ValueType::Pointer) {
        if (a->type == ValueType::Pointer && b->type == ValueType::Pointer) {
            return a->ptrVal == b->ptrVal;
        }
        if (a->type == ValueType::Pointer && b->type == ValueType::Null) {
            return a->ptrVal == nullptr;
        }
        if (a->type == ValueType::Null && b->type == ValueType::Pointer) {
            return b->ptrVal == nullptr;
        }
        return false;
    }

    if (a->isNumber() && b->isNumber()) {
        if (a->type == ValueType::Real || b->type == ValueType::Real) {
            return std::abs(a->asReal() - b->asReal()) < 1e-9;
        }
        return a->asInt() == b->asInt();
    }
    if (a->type == ValueType::String && b->type == ValueType::String) {
        return a->strVal == b->strVal;
    }
    if (a->type == ValueType::Bool && b->type == ValueType::Bool) {
        return a->boolVal == b->boolVal;
    }
    if (a->type == ValueType::Null && b->type == ValueType::Null) {
        return true;
    }
    if (a->type == ValueType::Record && b->type == ValueType::Record) {
        if (a->recVal.size() != b->recVal.size()) return false;
        for (const auto& kv : a->recVal) {
            auto it = b->recVal.find(kv.first);
            if (it == b->recVal.end()) return false;
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
