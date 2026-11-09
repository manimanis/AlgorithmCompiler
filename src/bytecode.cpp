#include "../include/bytecode.hpp"
#include "../include/vm.hpp"
#include <iomanip>
#include <cstring>
#include <fstream>
#include <sstream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#endif

namespace algo {

const char* opCodeName(OpCode op) {
    switch (op) {
        case OpCode::OP_NOP: return "OP_NOP";
        case OpCode::OP_CONST: return "OP_CONST";
        case OpCode::OP_POP: return "OP_POP";
        case OpCode::OP_DUP: return "OP_DUP";
        case OpCode::OP_GET_GLOBAL: return "OP_GET_GLOBAL";
        case OpCode::OP_SET_GLOBAL: return "OP_SET_GLOBAL";
        case OpCode::OP_POP_GLOBAL: return "OP_POP_GLOBAL";
        case OpCode::OP_GET_LOCAL: return "OP_GET_LOCAL";
        case OpCode::OP_SET_LOCAL: return "OP_SET_LOCAL";
        case OpCode::OP_POP_LOCAL: return "OP_POP_LOCAL";
        case OpCode::OP_GET_REF: return "OP_GET_REF";
        case OpCode::OP_SET_REF: return "OP_SET_REF";
        case OpCode::OP_POP_REF: return "OP_POP_REF";
        case OpCode::OP_PUSH_REF_GLOBAL: return "OP_PUSH_REF_GLOBAL";
        case OpCode::OP_PUSH_REF_LOCAL: return "OP_PUSH_REF_LOCAL";
        case OpCode::OP_PUSH_REF_REF: return "OP_PUSH_REF_REF";
        case OpCode::OP_REF_ARRAY: return "OP_REF_ARRAY";
        case OpCode::OP_REF_ARRAY2D: return "OP_REF_ARRAY2D";
        case OpCode::OP_REF_MEMBER: return "OP_REF_MEMBER";
        case OpCode::OP_REF_DEREF: return "OP_REF_DEREF";
        case OpCode::OP_DEREF: return "OP_DEREF";
        case OpCode::OP_ALLOC_PTR: return "OP_ALLOC_PTR";
        case OpCode::OP_FREE_PTR: return "OP_FREE_PTR";
        case OpCode::OP_SET_LVALUE: return "OP_SET_LVALUE";
        case OpCode::OP_ADD: return "OP_ADD";
        case OpCode::OP_SUB: return "OP_SUB";
        case OpCode::OP_MUL: return "OP_MUL";
        case OpCode::OP_POW: return "OP_POW";
        case OpCode::OP_DIV_REAL: return "OP_DIV_REAL";
        case OpCode::OP_DIV_INT: return "OP_DIV_INT";
        case OpCode::OP_MOD: return "OP_MOD";
        case OpCode::OP_NEG: return "OP_NEG";
        case OpCode::OP_LOOP_STEP: return "OP_LOOP_STEP";
        case OpCode::OP_NOT: return "OP_NOT";
        case OpCode::OP_AND: return "OP_AND";
        case OpCode::OP_OR: return "OP_OR";
        case OpCode::OP_EQ: return "OP_EQ";
        case OpCode::OP_NE: return "OP_NE";
        case OpCode::OP_LT: return "OP_LT";
        case OpCode::OP_LE: return "OP_LE";
        case OpCode::OP_GT: return "OP_GT";
        case OpCode::OP_GE: return "OP_GE";
        case OpCode::OP_IN: return "OP_IN";
        case OpCode::OP_JUMP: return "OP_JUMP";
        case OpCode::OP_JUMP_IF_FALSE: return "OP_JUMP_IF_FALSE";
        case OpCode::OP_JUMP_IF_TRUE: return "OP_JUMP_IF_TRUE";
        case OpCode::OP_JUMP_IF_FALSE_PEEK: return "OP_JUMP_IF_FALSE_PEEK";
        case OpCode::OP_JUMP_IF_TRUE_PEEK: return "OP_JUMP_IF_TRUE_PEEK";
        case OpCode::OP_LOOP: return "OP_LOOP";
        case OpCode::OP_ARRAY_GET: return "OP_ARRAY_GET";
        case OpCode::OP_ARRAY_SET: return "OP_ARRAY_SET";
        case OpCode::OP_ARRAY_GET_GLOBAL: return "OP_ARRAY_GET_GLOBAL";
        case OpCode::OP_ARRAY_SET_GLOBAL: return "OP_ARRAY_SET_GLOBAL";
        case OpCode::OP_ARRAY_GET_LOCAL: return "OP_ARRAY_GET_LOCAL";
        case OpCode::OP_ARRAY_SET_LOCAL: return "OP_ARRAY_SET_LOCAL";
        case OpCode::OP_ARRAY_GET_REF: return "OP_ARRAY_GET_REF";
        case OpCode::OP_ARRAY_SET_REF: return "OP_ARRAY_SET_REF";
        case OpCode::OP_ARRAY2D_GET: return "OP_ARRAY2D_GET";
        case OpCode::OP_ARRAY2D_SET: return "OP_ARRAY2D_SET";
        case OpCode::OP_RECORD_GET: return "OP_RECORD_GET";
        case OpCode::OP_RECORD_SET: return "OP_RECORD_SET";
        case OpCode::OP_MAKE_SET: return "OP_MAKE_SET";
        case OpCode::OP_WRITE: return "OP_WRITE";
        case OpCode::OP_READ_VAR: return "OP_READ_VAR";
        case OpCode::OP_READ_LOCAL: return "OP_READ_LOCAL";
        case OpCode::OP_READ_REF: return "OP_READ_REF";
        case OpCode::OP_READ_ARRAY: return "OP_READ_ARRAY";
        case OpCode::OP_READ_ARRAY2D: return "OP_READ_ARRAY2D";
        case OpCode::OP_READ_RECORD: return "OP_READ_RECORD";
        case OpCode::OP_READ_FILE: return "OP_READ_FILE";
        case OpCode::OP_READ_LVALUE: return "OP_READ_LVALUE";
        case OpCode::OP_CALL: return "OP_CALL";
        case OpCode::OP_CALL_BUILTIN: return "OP_CALL_BUILTIN";
        case OpCode::OP_RETURN: return "OP_RETURN";
        case OpCode::OP_RETURN_VOID: return "OP_RETURN_VOID";
        case OpCode::OP_HALT: return "OP_HALT";
    }
    return "UNKNOWN_OP";
}

size_t Chunk::writeByte(uint8_t byte, int line) {
    code.push_back(byte);
    lines.push_back(line);
    return code.size() - 1;
}

size_t Chunk::writeOp(OpCode op, int line) {
    return writeByte(static_cast<uint8_t>(op), line);
}

size_t Chunk::writeUint32(uint32_t val, int line) {
    size_t start = code.size();
    writeByte(static_cast<uint8_t>(val & 0xFF), line);
    writeByte(static_cast<uint8_t>((val >> 8) & 0xFF), line);
    writeByte(static_cast<uint8_t>((val >> 16) & 0xFF), line);
    writeByte(static_cast<uint8_t>((val >> 24) & 0xFF), line);
    return start;
}

size_t Chunk::writeInt32(int32_t val, int line) {
    return writeUint32(static_cast<uint32_t>(val), line);
}

void Chunk::patchInt32(size_t offset, int32_t val) {
    patchUint32(offset, static_cast<uint32_t>(val));
}

void Chunk::patchUint32(size_t offset, uint32_t val) {
    code[offset] = static_cast<uint8_t>(val & 0xFF);
    code[offset + 1] = static_cast<uint8_t>((val >> 8) & 0xFF);
    code[offset + 2] = static_cast<uint8_t>((val >> 16) & 0xFF);
    code[offset + 3] = static_cast<uint8_t>((val >> 24) & 0xFF);
}

uint32_t Chunk::addConstant(const Value& val) {
    for (size_t i = 0; i < constants.size(); ++i) {
        if (constants[i].equals(val)) return static_cast<uint32_t>(i);
    }
    constants.push_back(val);
    return static_cast<uint32_t>(constants.size() - 1);
}

uint32_t Chunk::addName(const std::string& name) {
    std::string norm = normalize(name);
    for (size_t i = 0; i < names.size(); ++i) {
        if (normalize(names[i]) == norm) return static_cast<uint32_t>(i);
    }
    names.push_back(name);
    return static_cast<uint32_t>(names.size() - 1);
}

uint8_t Chunk::readByte(size_t& ip) const {
    return code[ip++];
}

uint32_t Chunk::readUint32(size_t& ip) const {
    uint32_t b0 = code[ip++];
    uint32_t b1 = code[ip++];
    uint32_t b2 = code[ip++];
    uint32_t b3 = code[ip++];
    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

int32_t Chunk::readInt32(size_t& ip) const {
    return static_cast<int32_t>(readUint32(ip));
}

void Chunk::disassemble(const std::string& title) const {
    std::cout << "========== " << title << " ==========\n";
    for (size_t offset = 0; offset < code.size();) {
        offset = disassembleInstruction(offset);
    }
    std::cout << "========================================\n";
}

size_t Chunk::disassembleInstruction(size_t offset) const {
    std::cout << std::setw(4) << std::setfill('0') << offset << "  ";
    if (offset > 0 && lines[offset] == lines[offset - 1]) {
        std::cout << "   | ";
    } else {
        std::cout << std::setw(4) << std::setfill(' ') << lines[offset] << " ";
    }

    uint8_t instruction = code[offset];
    OpCode op = static_cast<OpCode>(instruction);
    std::cout << std::left << std::setw(22) << std::setfill(' ') << opCodeName(op);

    size_t nextOffset = offset + 1;
    switch (op) {
        case OpCode::OP_CONST: {
            size_t tempIp = nextOffset;
            uint32_t idx = readUint32(tempIp);
            nextOffset = tempIp;
            std::cout << "idx: " << idx << " ('" << (idx < constants.size() ? constants[idx].asString() : "?") << "')";
            break;
        }
        case OpCode::OP_GET_GLOBAL:
        case OpCode::OP_SET_GLOBAL:
        case OpCode::OP_POP_GLOBAL:
        case OpCode::OP_PUSH_REF_GLOBAL:
        case OpCode::OP_REF_MEMBER:
        case OpCode::OP_READ_VAR:
        case OpCode::OP_READ_ARRAY:
        case OpCode::OP_READ_ARRAY2D:
        case OpCode::OP_ARRAY_GET_GLOBAL:
        case OpCode::OP_ARRAY_SET_GLOBAL: {
            size_t tempIp = nextOffset;
            uint32_t idx = readUint32(tempIp);
            nextOffset = tempIp;
            std::cout << "name: " << idx << " ('" << (idx < names.size() ? names[idx] : "?") << "')";
            break;
        }
        case OpCode::OP_GET_LOCAL:
        case OpCode::OP_SET_LOCAL:
        case OpCode::OP_POP_LOCAL:
        case OpCode::OP_GET_REF:
        case OpCode::OP_SET_REF:
        case OpCode::OP_POP_REF:
        case OpCode::OP_PUSH_REF_LOCAL:
        case OpCode::OP_PUSH_REF_REF:
        case OpCode::OP_READ_LOCAL:
        case OpCode::OP_READ_REF:
        case OpCode::OP_ARRAY_GET_LOCAL:
        case OpCode::OP_ARRAY_SET_LOCAL:
        case OpCode::OP_ARRAY_GET_REF:
        case OpCode::OP_ARRAY_SET_REF: {
            size_t tempIp = nextOffset;
            uint32_t slot = readUint32(tempIp);
            nextOffset = tempIp;
            std::cout << "slot: " << slot;
            break;
        }
        case OpCode::OP_JUMP:
        case OpCode::OP_JUMP_IF_FALSE:
        case OpCode::OP_JUMP_IF_TRUE:
        case OpCode::OP_JUMP_IF_FALSE_PEEK:
        case OpCode::OP_JUMP_IF_TRUE_PEEK: {
            size_t tempIp = nextOffset;
            int32_t jmp = readInt32(tempIp);
            nextOffset = tempIp;
            std::cout << "offset: " << jmp << " -> target: " << (int32_t)nextOffset + jmp;
            break;
        }
        case OpCode::OP_LOOP: {
            size_t tempIp = nextOffset;
            uint32_t jmp = readUint32(tempIp);
            nextOffset = tempIp;
            std::cout << "back: " << jmp << " -> target: " << (int32_t)nextOffset - (int32_t)jmp;
            break;
        }
        case OpCode::OP_RECORD_GET:
        case OpCode::OP_RECORD_SET: {
            size_t tempIp = nextOffset;
            uint32_t fIdx = readUint32(tempIp);
            nextOffset = tempIp;
            std::cout << "field: " << (fIdx < names.size() ? names[fIdx] : "?");
            break;
        }
        case OpCode::OP_ALLOC_PTR: {
            size_t tempIp = nextOffset;
            uint32_t tIdx = readUint32(tempIp);
            nextOffset = tempIp;
            std::cout << "type: " << (tIdx < names.size() ? names[tIdx] : "?");
            break;
        }
        case OpCode::OP_READ_RECORD: {
            size_t tempIp = nextOffset;
            uint32_t nIdx = readUint32(tempIp);
            uint32_t fIdx = readUint32(tempIp);
            nextOffset = tempIp;
            std::cout << "var: " << (nIdx < names.size() ? names[nIdx] : "?") << ", field: " << (fIdx < names.size() ? names[fIdx] : "?");
            break;
        }
        case OpCode::OP_WRITE: {
            size_t tempIp = nextOffset;
            uint32_t count = readUint32(tempIp);
            uint8_t hasNl = readByte(tempIp);
            uint32_t sepIdx = readUint32(tempIp);
            uint32_t finIdx = readUint32(tempIp);
            nextOffset = tempIp;
            std::cout << "count: " << count << ", nl: " << (int)hasNl
                      << ", sep: '" << (sepIdx < names.size() ? names[sepIdx] : " ")
                      << "', fin: '" << (finIdx < names.size() ? names[finIdx] : "\\n") << "'";
            break;
        }
        case OpCode::OP_CALL: {
            size_t tempIp = nextOffset;
            uint32_t nameIdx = readUint32(tempIp);
            uint32_t arity = readUint32(tempIp);
            nextOffset = tempIp;
            std::cout << "sub: '" << (nameIdx < names.size() ? names[nameIdx] : "?") << "', arity: " << arity;
            break;
        }
        case OpCode::OP_CALL_BUILTIN: {
            size_t tempIp = nextOffset;
            uint32_t bId = readUint32(tempIp);
            uint32_t arity = readUint32(tempIp);
            nextOffset = tempIp;
            std::cout << "builtin: " << (bId < names.size() ? names[bId] : "?") << ", arity: " << arity;
            break;
        }
        default:
            break;
    }
    std::cout << "\n";
    return nextOffset;
}

// ==============================================================================
// Primitives de Sérialisation Binaire
// ==============================================================================

static void write8(std::ostream& os, uint8_t v) {
    os.write(reinterpret_cast<const char*>(&v), 1);
}

static uint8_t read8(std::istream& is) {
    uint8_t v = 0;
    is.read(reinterpret_cast<char*>(&v), 1);
    return v;
}

static void write32(std::ostream& os, uint32_t v) {
    os.write(reinterpret_cast<const char*>(&v), 4);
}

static uint32_t read32(std::istream& is) {
    uint32_t v = 0;
    is.read(reinterpret_cast<char*>(&v), 4);
    return v;
}

static void write64(std::ostream& os, uint64_t v) {
    os.write(reinterpret_cast<const char*>(&v), 8);
}

static uint64_t read64(std::istream& is) {
    uint64_t v = 0;
    is.read(reinterpret_cast<char*>(&v), 8);
    return v;
}

static void writeString(std::ostream& os, const std::string& s) {
    write32(os, static_cast<uint32_t>(s.size()));
    if (!s.empty()) {
        os.write(s.data(), s.size());
    }
}

static std::string readString(std::istream& is) {
    uint32_t sz = read32(is);
    std::string s(sz, '\0');
    if (sz > 0) {
        is.read(&s[0], sz);
    }
    return s;
}

// Forward declarations pour les types imbriqués
static void serializeVarDecl(std::ostream& os, const VarDecl& vd);
static VarDecl deserializeVarDecl(std::istream& is);

static void serializeValue(std::ostream& os, const Value& val) {
    write8(os, static_cast<uint8_t>(val.type));
    switch (val.type) {
        case ValueType::Null:
            break;
        case ValueType::Int:
            write64(os, static_cast<uint64_t>(val.intVal));
            break;
        case ValueType::Real: {
            uint64_t u = 0;
            std::memcpy(&u, &val.realVal, sizeof(double));
            write64(os, u);
            break;
        }
        case ValueType::Bool:
            write8(os, val.boolVal ? 1 : 0);
            break;
        case ValueType::String:
            writeString(os, val.strVal);
            break;
        case ValueType::Array: {
            write32(os, static_cast<uint32_t>(val.arrVal.size()));
            for (const auto& elem : val.arrVal) {
                serializeValue(os, elem);
            }
            break;
        }
        case ValueType::Record: {
            write32(os, static_cast<uint32_t>(val.recVal.size()));
            for (const auto& kv : val.recVal) {
                writeString(os, kv.first);
                serializeValue(os, kv.second);
            }
            break;
        }
        case ValueType::FileHandle:
            writeString(os, val.fileLogicalName);
            break;
        case ValueType::Pointer: {
            if (val.ptrVal) {
                write8(os, 1);
                serializeValue(os, *val.ptrVal);
            } else {
                write8(os, 0);
            }
            break;
        }
        case ValueType::Ref: {
            if (val.refPtr) {
                write8(os, 1);
                serializeValue(os, *val.refPtr);
            } else {
                write8(os, 0);
            }
            break;
        }
    }
}

static Value deserializeValue(std::istream& is) {
    uint8_t t = read8(is);
    ValueType type = static_cast<ValueType>(t);
    Value v;
    v.type = type;
    switch (type) {
        case ValueType::Null:
            break;
        case ValueType::Int: {
            int64_t iv = static_cast<int64_t>(read64(is));
            v.intVal = iv;
            v.realVal = static_cast<double>(iv);
            break;
        }
        case ValueType::Real: {
            uint64_t u = read64(is);
            double rv = 0.0;
            std::memcpy(&rv, &u, sizeof(double));
            v.realVal = rv;
            v.intVal = static_cast<int64_t>(rv);
            break;
        }
        case ValueType::Bool: {
            bool b = read8(is) != 0;
            v.boolVal = b;
            v.intVal = b ? 1 : 0;
            break;
        }
        case ValueType::String:
            v.strVal = readString(is);
            break;
        case ValueType::Array: {
            uint32_t count = read32(is);
            v.arrVal.resize(count);
            for (uint32_t i = 0; i < count; i++) {
                v.arrVal[i] = deserializeValue(is);
            }
            break;
        }
        case ValueType::Record: {
            uint32_t count = read32(is);
            for (uint32_t i = 0; i < count; i++) {
                std::string k = readString(is);
                v.recVal[k] = deserializeValue(is);
            }
            break;
        }
        case ValueType::FileHandle:
            v.fileLogicalName = readString(is);
            break;
        case ValueType::Pointer: {
            uint8_t hasPtr = read8(is);
            if (hasPtr) {
                v.ptrVal = std::make_shared<Value>(deserializeValue(is));
            } else {
                v.ptrVal = nullptr;
            }
            break;
        }
        case ValueType::Ref: {
            uint8_t hasRef = read8(is);
            if (hasRef) {
                v = deserializeValue(is);
            }
            break;
        }
    }
    return v;
}

static void serializeTypeSpec(std::ostream& os, const TypeSpec& ts) {
    write8(os, static_cast<uint8_t>(ts.kind));
    writeString(os, ts.name);
    write32(os, static_cast<uint32_t>(ts.rows));
    write32(os, static_cast<uint32_t>(ts.cols));
    writeString(os, ts.elemType);
    write64(os, static_cast<uint64_t>(ts.minInt));
    write64(os, static_cast<uint64_t>(ts.maxInt));

    write32(os, static_cast<uint32_t>(ts.enumValues.size()));
    for (const auto& ev : ts.enumValues) {
        writeString(os, ev);
    }

    write32(os, static_cast<uint32_t>(ts.fields.size()));
    for (const auto& f : ts.fields) {
        if (f) {
            write8(os, 1);
            serializeVarDecl(os, *f);
        } else {
            write8(os, 0);
        }
    }
}

static TypeSpec deserializeTypeSpec(std::istream& is) {
    TypeSpec ts;
    ts.kind = static_cast<TypeKind>(read8(is));
    ts.name = readString(is);
    ts.rows = static_cast<int>(read32(is));
    ts.cols = static_cast<int>(read32(is));
    ts.elemType = readString(is);
    ts.minInt = static_cast<int64_t>(read64(is));
    ts.maxInt = static_cast<int64_t>(read64(is));

    uint32_t enumCount = read32(is);
    ts.enumValues.resize(enumCount);
    for (uint32_t i = 0; i < enumCount; i++) {
        ts.enumValues[i] = readString(is);
    }

    uint32_t fieldCount = read32(is);
    ts.fields.reserve(fieldCount);
    for (uint32_t i = 0; i < fieldCount; i++) {
        uint8_t hasField = read8(is);
        if (hasField) {
            ts.fields.push_back(std::make_shared<VarDecl>(deserializeVarDecl(is)));
        }
    }
    return ts;
}

static void serializeVarDecl(std::ostream& os, const VarDecl& vd) {
    write32(os, static_cast<uint32_t>(vd.names.size()));
    for (const auto& n : vd.names) {
        writeString(os, n);
    }
    writeString(os, vd.varType);
    serializeTypeSpec(os, vd.typeSpec);
    write32(os, static_cast<uint32_t>(vd.line));
}

static VarDecl deserializeVarDecl(std::istream& is) {
    VarDecl vd;
    uint32_t nameCount = read32(is);
    vd.names.resize(nameCount);
    for (uint32_t i = 0; i < nameCount; i++) {
        vd.names[i] = readString(is);
    }
    vd.varType = readString(is);
    vd.typeSpec = deserializeTypeSpec(is);
    vd.line = static_cast<int>(read32(is));
    return vd;
}

static void serializeTypeDecl(std::ostream& os, const TypeDecl& td) {
    writeString(os, td.name);
    serializeTypeSpec(os, td.typeSpec);
    write32(os, static_cast<uint32_t>(td.arraySize));
    writeString(os, td.elemType);
    write32(os, static_cast<uint32_t>(td.line));
}

static TypeDecl deserializeTypeDecl(std::istream& is) {
    TypeDecl td;
    td.name = readString(is);
    td.typeSpec = deserializeTypeSpec(is);
    td.arraySize = static_cast<int>(read32(is));
    td.elemType = readString(is);
    td.line = static_cast<int>(read32(is));
    return td;
}

static void serializeConstDecl(std::ostream& os, const ConstDecl& cd) {
    writeString(os, cd.name);
    serializeValue(os, cd.value);
    write32(os, static_cast<uint32_t>(cd.line));
}

static ConstDecl deserializeConstDecl(std::istream& is) {
    ConstDecl cd;
    cd.name = readString(is);
    cd.value = deserializeValue(is);
    cd.line = static_cast<int>(read32(is));
    return cd;
}

static void serializeChunk(std::ostream& os, const Chunk& chunk) {
    write32(os, static_cast<uint32_t>(chunk.code.size()));
    if (!chunk.code.empty()) {
        os.write(reinterpret_cast<const char*>(chunk.code.data()), chunk.code.size());
    }

    write32(os, static_cast<uint32_t>(chunk.lines.size()));
    for (int l : chunk.lines) {
        write32(os, static_cast<uint32_t>(l));
    }

    write32(os, static_cast<uint32_t>(chunk.names.size()));
    for (const auto& n : chunk.names) {
        writeString(os, n);
    }

    write32(os, static_cast<uint32_t>(chunk.constants.size()));
    for (const auto& c : chunk.constants) {
        serializeValue(os, c);
    }
}

static Chunk deserializeChunk(std::istream& is) {
    Chunk chunk;
    uint32_t codeSize = read32(is);
    chunk.code.resize(codeSize);
    if (codeSize > 0) {
        is.read(reinterpret_cast<char*>(chunk.code.data()), codeSize);
    }

    uint32_t linesSize = read32(is);
    chunk.lines.resize(linesSize);
    for (uint32_t i = 0; i < linesSize; i++) {
        chunk.lines[i] = static_cast<int>(read32(is));
    }

    uint32_t namesSize = read32(is);
    chunk.names.resize(namesSize);
    for (uint32_t i = 0; i < namesSize; i++) {
        chunk.names[i] = readString(is);
    }

    uint32_t constsSize = read32(is);
    chunk.constants.resize(constsSize);
    for (uint32_t i = 0; i < constsSize; i++) {
        chunk.constants[i] = deserializeValue(is);
    }
    return chunk;
}

void serializeProgram(std::ostream& os, const BytecodeProgram& prog) {
    os.write(reinterpret_cast<const char*>(ALGB_MAGIC), sizeof(ALGB_MAGIC));
    writeString(os, prog.name);

    write32(os, static_cast<uint32_t>(prog.typeDecls.size()));
    for (const auto& td : prog.typeDecls) {
        serializeTypeDecl(os, td);
    }

    write32(os, static_cast<uint32_t>(prog.constDecls.size()));
    for (const auto& cd : prog.constDecls) {
        serializeConstDecl(os, cd);
    }

    write32(os, static_cast<uint32_t>(prog.varDecls.size()));
    for (const auto& vd : prog.varDecls) {
        serializeVarDecl(os, vd);
    }

    serializeChunk(os, prog.mainChunk);

    write32(os, static_cast<uint32_t>(prog.subroutines.size()));
    for (const auto& kv : prog.subroutines) {
        writeString(os, kv.first);
        const auto& fn = kv.second;
        writeString(os, fn.name);
        write8(os, fn.isProcedure ? 1 : 0);
        writeString(os, fn.returnType);

        write32(os, static_cast<uint32_t>(fn.params.size()));
        for (const auto& p : fn.params) {
            writeString(os, p.name);
            write8(os, p.byRef ? 1 : 0);
        }

        write32(os, static_cast<uint32_t>(fn.localVars.size()));
        for (const auto& lv : fn.localVars) {
            serializeVarDecl(os, lv);
        }

        serializeChunk(os, fn.chunk);
    }
}

BytecodeProgram deserializeProgram(std::istream& is) {
    char magic[sizeof(ALGB_MAGIC)];
    is.read(magic, sizeof(ALGB_MAGIC));
    if (std::memcmp(magic, ALGB_MAGIC, sizeof(ALGB_MAGIC)) != 0) {
        throw AlgoError("Format de bytecode invalide ou incompatible (en-tête ALGB incorrect)");
    }

    BytecodeProgram prog;
    prog.name = readString(is);

    uint32_t typeCount = read32(is);
    prog.typeDecls.resize(typeCount);
    for (uint32_t i = 0; i < typeCount; i++) {
        prog.typeDecls[i] = deserializeTypeDecl(is);
    }

    uint32_t constCount = read32(is);
    prog.constDecls.resize(constCount);
    for (uint32_t i = 0; i < constCount; i++) {
        prog.constDecls[i] = deserializeConstDecl(is);
    }

    uint32_t varCount = read32(is);
    prog.varDecls.resize(varCount);
    for (uint32_t i = 0; i < varCount; i++) {
        prog.varDecls[i] = deserializeVarDecl(is);
    }

    prog.mainChunk = deserializeChunk(is);

    uint32_t subCount = read32(is);
    for (uint32_t i = 0; i < subCount; i++) {
        std::string key = readString(is);
        BytecodeFunction fn;
        fn.name = readString(is);
        fn.isProcedure = read8(is) != 0;
        fn.returnType = readString(is);

        uint32_t paramCount = read32(is);
        fn.params.resize(paramCount);
        for (uint32_t p = 0; p < paramCount; p++) {
            fn.params[p].name = readString(is);
            fn.params[p].byRef = read8(is) != 0;
        }

        uint32_t locCount = read32(is);
        fn.localVars.resize(locCount);
        for (uint32_t l = 0; l < locCount; l++) {
            fn.localVars[l] = deserializeVarDecl(is);
        }

        fn.chunk = deserializeChunk(is);
        prog.subroutines[key] = std::move(fn);
    }

    return prog;
}

// ==============================================================================
// Support des Exécutables Autonomes (.exe)
// ==============================================================================

std::string getSelfExecutablePath() {
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        return std::string(buf, len);
    }
    return "";
#elif defined(__APPLE__)
    char buf[1024];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        return std::string(buf);
    }
    return "";
#else
    char buf[1024];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        return std::string(buf);
    }
    return "";
#endif
}

bool isStandaloneExecutable(const std::string& path) {
    std::string p = path;
    if (p.empty()) p = getSelfExecutablePath();
    if (p.empty()) return false;

    std::ifstream is(p, std::ios::binary | std::ios::ate);
    if (!is.is_open()) return false;
    std::streampos fileSize = is.tellg();
    if (fileSize < 16) return false;

    is.seekg(-16, std::ios::end);
    uint64_t payloadSize = 0;
    is.read(reinterpret_cast<char*>(&payloadSize), 8);
    char magic[8];
    is.read(magic, 8);

    if (std::memcmp(magic, ALGB_EXEC_MAGIC, 8) != 0) {
        return false;
    }
    if (payloadSize == 0 || payloadSize > static_cast<uint64_t>(fileSize) - 16) {
        return false;
    }
    return true;
}

int runStandaloneExecutable(const std::string& path, int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    std::string p = path;
    if (p.empty()) p = getSelfExecutablePath();

    std::ifstream is(p, std::ios::binary | std::ios::ate);
    if (!is.is_open()) {
        std::cerr << "Erreur: Impossible d'ouvrir l'exécutable pour charger le bytecode.\n";
        return 1;
    }
    is.seekg(-16, std::ios::end);
    uint64_t payloadSize = 0;
    is.read(reinterpret_cast<char*>(&payloadSize), 8);

    is.seekg(-16 - static_cast<std::streamoff>(payloadSize), std::ios::end);
    try {
        BytecodeProgram prog = deserializeProgram(is);
        VM vm;
        vm.setOutputCallback([](const std::string& text) {
            std::cout << text;
        });
        vm.run(prog);
        return 0;
    } catch (const AlgoError& e) {
        std::cerr << "Erreur d'exécution : " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Exception : " << e.what() << "\n";
        return 1;
    }
}

bool buildStandaloneExecutable(const BytecodeProgram& prog, const std::string& outputPath, std::string& outError) {
    std::string selfPath = getSelfExecutablePath();
    if (selfPath.empty()) {
        outError = "Impossible de déterminer le chemin de l'exécutable hôte";
        return false;
    }

    std::filesystem::path hostPath(selfPath);
    std::filesystem::path dir = hostPath.parent_path();
    std::filesystem::path stubPath = dir / "algo_stub.exe";
    if (!std::filesystem::exists(stubPath)) {
        stubPath = dir / "algo_stub";
    }

    std::filesystem::path runnerTemplate = std::filesystem::exists(stubPath) ? stubPath : hostPath;

    std::ifstream src(runnerTemplate, std::ios::binary | std::ios::ate);
    if (!src.is_open()) {
        outError = "Impossible de lire le modèle exécutable : " + runnerTemplate.string();
        return false;
    }

    std::streampos srcSize = src.tellg();
    uint64_t baseBinarySize = static_cast<uint64_t>(srcSize);
    if (srcSize >= 16) {
        src.seekg(-16, std::ios::end);
        uint64_t oldPayload = 0;
        src.read(reinterpret_cast<char*>(&oldPayload), 8);
        char magic[8];
        src.read(magic, 8);
        if (std::memcmp(magic, ALGB_EXEC_MAGIC, 8) == 0 && oldPayload > 0 && oldPayload < static_cast<uint64_t>(srcSize) - 16) {
            baseBinarySize = static_cast<uint64_t>(srcSize) - 16 - oldPayload;
        }
    }

    // Sérialiser le bytecode en mémoire
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    serializeProgram(ss, prog);
    std::string payload = ss.str();
    uint64_t payloadSize = payload.size();

    // Écrire le nouvel exécutable
    std::ofstream dst(outputPath, std::ios::binary);
    if (!dst.is_open()) {
        outError = "Impossible d'écrire l'exécutable de sortie : " + outputPath;
        return false;
    }

    // Copier le binaire hôte
    src.seekg(0, std::ios::beg);
    constexpr size_t BUFFER_SIZE = 64 * 1024;
    std::vector<char> buffer(BUFFER_SIZE);
    uint64_t remaining = baseBinarySize;
    while (remaining > 0) {
        size_t toRead = static_cast<size_t>(std::min<uint64_t>(remaining, BUFFER_SIZE));
        src.read(buffer.data(), toRead);
        dst.write(buffer.data(), toRead);
        remaining -= toRead;
    }

    // Concaténer le payload de bytecode
    dst.write(payload.data(), payload.size());

    // Concaténer le pied de page (8 octets taille + 8 octets signature ALGBEXEC)
    dst.write(reinterpret_cast<const char*>(&payloadSize), 8);
    dst.write(ALGB_EXEC_MAGIC, 8);
    dst.close();

#ifndef _WIN32
    chmod(outputPath.c_str(), 0755);
#endif

    return true;
}

} // namespace algo
