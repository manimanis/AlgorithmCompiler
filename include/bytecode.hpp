#pragma once

#include "value.hpp"
#include "ast.hpp"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <iostream>

namespace algo {

enum class OpCode : uint8_t {
    OP_NOP = 0,
    // Constants & Stack
    OP_CONST,               // [uint32 constIdx] -> push constants[constIdx]
    OP_POP,                 // pop 1 value
    OP_DUP,                 // duplicate top of stack

    // Variables (Globals)
    OP_GET_GLOBAL,          // [uint32 nameIdx] -> push global
    OP_SET_GLOBAL,          // [uint32 nameIdx] -> peek top -> set global
    OP_POP_GLOBAL,          // [uint32 nameIdx] -> pop top -> set global

    // Variables (Locals)
    OP_GET_LOCAL,           // [uint32 slot] -> push local
    OP_SET_LOCAL,           // [uint32 slot] -> peek top -> set local
    OP_POP_LOCAL,           // [uint32 slot] -> pop top -> set local

    // References (By-ref parameters)
    OP_GET_REF,             // [uint32 slot] -> dereference and push
    OP_SET_REF,             // [uint32 slot] -> peek top -> set through ref
    OP_POP_REF,             // [uint32 slot] -> pop top -> set through ref

    // Arithmetic
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV_REAL,
    OP_DIV_INT,
    OP_MOD,
    OP_NEG,

    // Logical
    OP_NOT,
    OP_AND,
    OP_OR,

    // Comparison
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_IN,

    // Jumps & Control flow
    OP_JUMP,                // [int32 offset] -> ip += offset
    OP_JUMP_IF_FALSE,       // [int32 offset] -> if (!pop().asBool()) ip += offset
    OP_JUMP_IF_TRUE,        // [int32 offset] -> if (pop().asBool()) ip += offset
    OP_JUMP_IF_FALSE_PEEK,  // [int32 offset] -> if (!peek().asBool()) ip += offset else pop()
    OP_JUMP_IF_TRUE_PEEK,   // [int32 offset] -> if (peek().asBool()) ip += offset else pop()
    OP_LOOP,                // [uint32 offset] -> ip -= offset

    // Data structures
    OP_ARRAY_GET,           // [target, idx] -> arr[idx]
    OP_ARRAY_SET,           // [target, idx, val] -> arr[idx] = val
    OP_ARRAY2D_GET,         // [target, r, c] -> arr[r][c]
    OP_ARRAY2D_SET,         // [target, r, c, val] -> arr[r][c] = val
    OP_RECORD_GET,          // [uint32 fieldNameIdx] -> rec[field]
    OP_RECORD_SET,          // [uint32 fieldNameIdx] -> rec[field] = val
    OP_MAKE_SET,            // [uint32 count] -> array of count elements

    // I/O
    OP_WRITE,               // [uint32 argCount, uint8 hasNl, uint32 sepIdx]
    OP_READ_VAR,            // [uint32 nameIdx]
    OP_READ_LOCAL,          // [uint32 slot]
    OP_READ_REF,            // [uint32 slot]
    OP_READ_ARRAY,          // [uint32 nameIdx] (reads index from stack)
    OP_READ_ARRAY2D,        // [uint32 nameIdx] (reads col, row from stack)
    OP_READ_RECORD,         // [uint32 nameIdx, uint32 fieldIdx]

    // Subroutines
    OP_CALL,                // [uint32 nameIdx, uint32 argCount]
    OP_CALL_BUILTIN,        // [uint32 builtinId, uint32 argCount]
    OP_RETURN,              // pop return val and return
    OP_RETURN_VOID,         // return Null
    OP_HALT                 // stop execution
};

const char* opCodeName(OpCode op);

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<std::string> names;
    std::vector<int> lines;

    size_t writeByte(uint8_t byte, int line);
    size_t writeOp(OpCode op, int line);
    size_t writeUint32(uint32_t val, int line);
    size_t writeInt32(int32_t val, int line);

    void patchInt32(size_t offset, int32_t val);
    void patchUint32(size_t offset, uint32_t val);

    uint32_t addConstant(const Value& val);
    uint32_t addName(const std::string& name);

    uint8_t readByte(size_t& ip) const;
    uint32_t readUint32(size_t& ip) const;
    int32_t readInt32(size_t& ip) const;

    void disassemble(const std::string& title) const;
    size_t disassembleInstruction(size_t offset) const;
};

struct BytecodeParam {
    std::string name;
    bool byRef = false;
};

struct BytecodeFunction {
    std::string name;
    bool isProcedure = false;
    std::string returnType;
    std::vector<BytecodeParam> params;
    std::vector<VarDecl> localVars;
    Chunk chunk;
};

struct BytecodeProgram {
    std::string name;
    Chunk mainChunk;
    std::vector<TypeDecl> typeDecls;
    std::vector<VarDecl> varDecls;
    std::unordered_map<std::string, BytecodeFunction> subroutines;
};

} // namespace algo
