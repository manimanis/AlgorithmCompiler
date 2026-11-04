#pragma once

#include "value.hpp"

namespace algo {

struct VariableBinding {
    std::function<Value()> get;
    std::function<void(const Value&)> set;
};

class Environment : public std::enable_shared_from_this<Environment> {
public:
    explicit Environment(std::shared_ptr<Environment> parent = nullptr)
        : parent_(std::move(parent)) {}

    bool has(const std::string& name) const {
        std::string n = normalize(name);
        if (bindings_.find(n) != bindings_.end()) return true;
        if (parent_) return parent_->has(name);
        return false;
    }

    bool hasOwn(const std::string& name) const {
        return bindings_.find(normalize(name)) != bindings_.end();
    }

    void define(const std::string& name, Value initialVal = Value()) {
        std::string n = normalize(name);
        auto valPtr = std::make_shared<Value>(std::move(initialVal));
        bindings_[n] = {
            [valPtr]() -> Value { return *valPtr; },
            [valPtr](const Value& v) { *valPtr = v; }
        };
    }

    void defineRef(const std::string& name, std::function<Value()> getter, std::function<void(const Value&)> setter) {
        std::string n = normalize(name);
        bindings_[n] = {std::move(getter), std::move(setter)};
    }

    Value get(const std::string& name, int line = -1) const {
        std::string n = normalize(name);
        auto it = bindings_.find(n);
        if (it != bindings_.end()) {
            return it->second.get();
        }
        if (parent_) {
            return parent_->get(name, line);
        }
        throw AlgoError("Variable '" + name + "' non définie", line);
    }

    void set(const std::string& name, const Value& val, int line = -1) {
        std::string n = normalize(name);
        auto it = bindings_.find(n);
        if (it != bindings_.end()) {
            it->second.set(val);
            return;
        }
        if (parent_ && parent_->has(name)) {
            parent_->set(name, val, line);
            return;
        }
        // Auto-define if not exists
        define(name, val);
    }

    VariableBinding getBinding(const std::string& name, int line = -1) {
        std::string n = normalize(name);
        auto it = bindings_.find(n);
        if (it != bindings_.end()) {
            return it->second;
        }
        if (parent_) {
            return parent_->getBinding(name, line);
        }
        // If not found in parent, define in this environment
        define(name, Value());
        return bindings_[n];
    }

    std::shared_ptr<Environment> parent() const { return parent_; }

    std::map<std::string, Value> getAllVars() const {
        std::map<std::string, Value> res;
        for (const auto& kv : bindings_) {
            res[kv.first] = kv.second.get();
        }
        return res;
    }

private:
    std::shared_ptr<Environment> parent_;
    std::unordered_map<std::string, VariableBinding> bindings_;
};

} // namespace algo
