#pragma once

#include "bytecode.hpp"
#include "interpreter.hpp"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <chrono>

namespace algo {

struct VMRefSlot {
    Value* ptr = nullptr;
};

struct VMCallFrame {
    const BytecodeFunction* function = nullptr;
    const Chunk* chunk = nullptr;
    size_t ip = 0;
    size_t stackBase = 0;
    std::vector<VMRefSlot> refSlots;
    std::vector<Value*> globalSlots;
};

class VM {
public:
    explicit VM(InterpreterOptions options = InterpreterOptions());

    void setOutputCallback(std::function<void(const std::string&)> cb) { outputCallback_ = std::move(cb); }
    void setInputCallback(std::function<std::string(const std::string&)> cb) { inputCallback_ = std::move(cb); }

    void addInput(const std::string& input) { inputBuffer_.push_back(input); }
    const std::vector<std::string>& getOutput() const { return output_; }

    void run(const BytecodeProgram& prog);

    Value getDefaultValue(const std::string& typeName, const std::string& varName = "") const;
    Value getDefaultValue(const TypeSpec& ts, const std::string& varName = "") const;

private:
    InterpreterOptions options_;

    std::vector<Value> stack_;
    std::vector<VMCallFrame> frames_;
    std::map<std::string, Value> globals_;
    std::unordered_map<std::string, TypeDecl> typeDefs_;
    std::unordered_map<std::string, BytecodeFunction> subroutines_;
    std::map<std::string, std::shared_ptr<AlgoFile>> openFiles_;

    std::vector<std::string> output_;
    std::vector<std::string> inputBuffer_;
    size_t inputBufferIdx_ = 0;

    std::function<void(const std::string&)> outputCallback_;
    std::function<std::string(const std::string&)> inputCallback_;

    uint64_t iterationCount_ = 0;
    uint64_t instructionCount_ = 0;
    std::chrono::steady_clock::time_point startTime_;
    Mulberry32RNG rng_;

    void push(Value val);
    Value pop();
    Value& peek(size_t distance = 0);

    void checkIterationLimit(int line = -1);
    void checkTimeout(int line = -1);
    void addOutput(const std::string& str);
    std::string requestInput(const std::string& prompt);
    Value convertInput(const std::string& inputStr, const Value& currentVal);

    void executeBuiltin(uint32_t builtinId, uint32_t argCount, int line);
    void writeFileData(AlgoFile& f, const Value& val);
    Value readFileData(AlgoFile& f);
};

} // namespace algo
