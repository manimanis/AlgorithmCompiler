#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <functional>
#include <map>
#include <unordered_map>
#include <set>
#include <variant>
#include <filesystem>
#include <string_view>

namespace algo {

// Convert UTF-8 std::string to std::filesystem::path in C++20 without deprecated u8path
inline std::filesystem::path utf8Path(const std::string& p) {
    return std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(p.data()), p.size()));
}

// Structure représentant un niveau de la pile d'appels d'exécution
struct StackFrame {
    std::string functionName;
    int line = -1;
    int column = -1;

    StackFrame() = default;
    StackFrame(std::string name, int l = -1, int c = -1)
        : functionName(std::move(name)), line(l), column(c) {}
};

// Custom exception for runtime and syntax errors
class AlgoError : public std::runtime_error {
public:
    int line;
    std::string rawMessage;
    std::vector<StackFrame> callStack;

    AlgoError(const std::string& msg, int line = -1)
        : std::runtime_error(line > 0 ? msg + " (ligne " + std::to_string(line) + ")" : msg),
          line(line), rawMessage(msg) {}

    AlgoError(const std::string& msg, int line, const std::vector<StackFrame>& stack)
        : std::runtime_error(line > 0 ? msg + " (ligne " + std::to_string(line) + ")" : msg),
          line(line), rawMessage(msg), callStack(stack) {}

    std::string formatStackTrace(const std::string& sourceCode = "") const {
        std::ostringstream oss;
        oss << "==============================================================================\n"
            << " ERREUR D'EXÉCUTION : " << rawMessage;
        if (line > 0) {
            oss << " (ligne " << line << ")";
        }
        oss << "\n==============================================================================\n";

        if (!sourceCode.empty() && line > 0) {
            std::istringstream iss(sourceCode);
            std::string curLineText;
            int curNum = 1;
            while (std::getline(iss, curLineText)) {
                if (!curLineText.empty() && curLineText.back() == '\r') curLineText.pop_back();
                if (curNum == line) {
                    oss << " -> Ligne " << line << " :\n";
                    oss << "    " << curLineText << "\n";
                    // Afficher le pointeur sous le code
                    size_t firstNonSpace = curLineText.find_first_not_of(" \t");
                    if (firstNonSpace != std::string::npos) {
                        oss << "    " << std::string(firstNonSpace, ' ') << "^~~~~~\n";
                    }
                    break;
                }
                curNum++;
            }
        }

        if (!callStack.empty()) {
            oss << "\nTrace de la pile d'appels (Call Stack) :\n";
            for (size_t i = 0; i < callStack.size(); ++i) {
                const auto& frame = callStack[callStack.size() - 1 - i];
                oss << "  #" << i << "  " << frame.functionName;
                if (frame.line > 0) {
                    oss << " (ligne " << frame.line << ")";
                }
                oss << "\n";
            }
        }
        oss << "==============================================================================\n";
        return oss.str();
    }
};

// Information sur une allocation dynamique pour le Memory Leak Checker
struct MemoryAllocation {
    uint64_t id = 0;
    void* address = nullptr;
    std::string typeName;
    int line = -1;
};

// Gestionnaire de détection des fuites de mémoire (Memory Leak Checker)
class MemoryTracker {
public:
    void recordAlloc(void* addr, const std::string& typeName, int line) {
        if (!addr) return;
        allocations_[addr] = {++nextId_, addr, typeName, line};
    }

    void recordFree(void* addr) {
        if (!addr) return;
        allocations_.erase(addr);
    }

    bool hasLeaks() const {
        return !allocations_.empty();
    }

    size_t leakCount() const {
        return allocations_.size();
    }

    std::vector<MemoryAllocation> getLeaks() const {
        std::vector<MemoryAllocation> res;
        res.reserve(allocations_.size());
        for (const auto& kv : allocations_) {
            res.push_back(kv.second);
        }
        std::sort(res.begin(), res.end(), [](const MemoryAllocation& a, const MemoryAllocation& b) {
            return a.id < b.id;
        });
        return res;
    }

    std::string formatLeakReport() const {
        if (allocations_.empty()) return "";
        std::ostringstream oss;
        oss << "==============================================================================\n"
            << " AVERTISSEMENT : FUITE DE MÉMOIRE DÉTECTÉE (MEMORY LEAK CHECKER)\n"
            << "==============================================================================\n"
            << " Nombre de bloc(s) dynamique(s) alloué(s) non libéré(s) : " << allocations_.size() << "\n\n";
        auto leaks = getLeaks();
        for (size_t i = 0; i < leaks.size(); ++i) {
            oss << "  #" << (i + 1) << "  Bloc de type '" << (leaks[i].typeName.empty() ? "inconnu" : leaks[i].typeName) << "'";
            if (leaks[i].line > 0) {
                oss << " alloué à la ligne " << leaks[i].line;
            }
            oss << " (non libéré)\n";
        }
        oss << "\n Conseil pédagogique : Pensez à libérer chaque variable allouée avec 'Liberer(...)'.\n"
            << "==============================================================================\n";
        return oss.str();
    }

    void reset() {
        allocations_.clear();
        nextId_ = 0;
    }

private:
    uint64_t nextId_ = 0;
    std::unordered_map<void*, MemoryAllocation> allocations_;
};

// Normalize a string by converting to lowercase and stripping French accents
inline std::string normalize(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = (unsigned char)s[i];
        if (c < 0x80) {
            result.push_back((char)std::tolower(c));
            i++;
        } else if (c == 0xC3 && i + 1 < s.size()) {
            unsigned char c2 = (unsigned char)s[i + 1];
            switch (c2) {
                // a: à, á, â, ã, ä, À, Á, Â, Ã, Ä
                case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4:
                case 0x80: case 0x81: case 0x82: case 0x83: case 0x84:
                    result.push_back('a'); break;
                // e: è, é, ê, ë, È, É, Ê, Ë
                case 0xA8: case 0xA9: case 0xAA: case 0xAB:
                case 0x88: case 0x89: case 0x8A: case 0x8B:
                    result.push_back('e'); break;
                // i: ì, í, î, ï, Ì, Í, Î, Ï
                case 0xAC: case 0xAD: case 0xAE: case 0xAF:
                case 0x8C: case 0x8D: case 0x8E: case 0x8F:
                    result.push_back('i'); break;
                // o: ò, ó, ô, õ, ö, Ò, Ó, Ô, Õ, Ö
                case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6:
                case 0x92: case 0x93: case 0x94: case 0x95: case 0x96:
                    result.push_back('o'); break;
                // u: ù, ú, û, ü, Ù, Ú, Û, Ü
                case 0xB9: case 0xBA: case 0xBB: case 0xBC:
                case 0x99: case 0x9A: case 0x9B: case 0x9C:
                    result.push_back('u'); break;
                // c: ç, Ç
                case 0xA7: case 0x87:
                    result.push_back('c'); break;
                default:
                    result.push_back((char)c);
                    result.push_back((char)c2);
                    break;
            }
            i += 2;
        } else {
            result.push_back((char)c);
            i++;
        }
    }
    return result;
}

// Case-insensitive comparison using normalized strings
inline bool iequals(const std::string& a, const std::string& b) {
    return normalize(a) == normalize(b);
}

// Convert string to uppercase (ASCII)
inline std::string toUpper(std::string s) {
    for (char& c : s) c = (char)std::toupper((unsigned char)c);
    return s;
}

// Convert string to lowercase (ASCII)
inline std::string toLower(std::string s) {
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

// UTF-8 string length in characters (codepoints)
inline size_t utf8Length(const std::string& s) {
    size_t len = 0;
    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = (unsigned char)s[i];
        if (c < 0x80) i += 1;
        else if ((c & 0xE0) == 0xC0) i += 2;
        else if ((c & 0xF0) == 0xE0) i += 3;
        else if ((c & 0xF8) == 0xF0) i += 4;
        else i += 1;
        len++;
    }
    return len;
}

// Extract UTF-8 character at character index
inline std::string utf8CharAt(const std::string& s, size_t charIdx) {
    size_t curIdx = 0;
    for (size_t i = 0; i < s.size(); ) {
        size_t start = i;
        unsigned char c = (unsigned char)s[i];
        if (c < 0x80) i += 1;
        else if ((c & 0xE0) == 0xC0) i += 2;
        else if ((c & 0xF0) == 0xE0) i += 3;
        else if ((c & 0xF8) == 0xF0) i += 4;
        else i += 1;

        if (curIdx == charIdx) {
            return s.substr(start, i - start);
        }
        curIdx++;
    }
    return "";
}

// UTF-8 substring by character indices [start, end)
inline std::string utf8Substr(const std::string& s, size_t startChar, size_t endChar) {
    if (startChar >= endChar) return "";
    size_t curIdx = 0;
    size_t byteStart = s.size();
    size_t byteEnd = s.size();

    for (size_t i = 0; i < s.size(); ) {
        if (curIdx == startChar) byteStart = i;
        if (curIdx == endChar) { byteEnd = i; break; }

        unsigned char c = (unsigned char)s[i];
        if (c < 0x80) i += 1;
        else if ((c & 0xE0) == 0xC0) i += 2;
        else if ((c & 0xF0) == 0xE0) i += 3;
        else if ((c & 0xF8) == 0xF0) i += 4;
        else i += 1;

        curIdx++;
    }

    if (curIdx == startChar) byteStart = s.size();
    if (byteStart >= s.size() || byteStart >= byteEnd) return "";
    return s.substr(byteStart, byteEnd - byteStart);
}

// Check if character can start an identifier
inline bool isIdentStart(unsigned char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c >= 0x80;
}

// Check if character can continue an identifier
inline bool isIdentPart(unsigned char c) {
    return isIdentStart(c) || (c >= '0' && c <= '9') || c == '\'';
}

inline void initConsoleUtf8() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

} // namespace algo
