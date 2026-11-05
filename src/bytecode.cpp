#include "../include/bytecode.hpp"
#include <iomanip>

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
        case OpCode::OP_SET_LVALUE: return "OP_SET_LVALUE";
        case OpCode::OP_ADD: return "OP_ADD";
        case OpCode::OP_SUB: return "OP_SUB";
        case OpCode::OP_MUL: return "OP_MUL";
        case OpCode::OP_POW: return "OP_POW";
        case OpCode::OP_DIV_REAL: return "OP_DIV_REAL";
        case OpCode::OP_DIV_INT: return "OP_DIV_INT";
        case OpCode::OP_MOD: return "OP_MOD";
        case OpCode::OP_NEG: return "OP_NEG";
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
        case OpCode::OP_READ_ARRAY2D: {
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
        case OpCode::OP_READ_REF: {
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

} // namespace algo
