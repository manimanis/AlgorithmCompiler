#include "../include/vm.hpp"
#include <regex>
#include <filesystem>
#include <cmath>

namespace algo {

VM::VM(InterpreterOptions options)
    : options_(options), rng_(options.seed) {
    stack_.reserve(4096);
    frames_.reserve(256);
}

void VM::push(Value val) {
    stack_.push_back(std::move(val));
}

Value VM::pop() {
    if (stack_.empty()) return Value();
    Value val = std::move(stack_.back());
    stack_.pop_back();
    return val;
}

Value& VM::peek(size_t distance) {
    return stack_[stack_.size() - 1 - distance];
}

void VM::checkIterationLimit(int line) {
    iterationCount_++;
    if (iterationCount_ > options_.maxIterations) {
        throw AlgoError("Limite d'itérations atteinte (" + std::to_string(options_.maxIterations) + " itérations) - boucle infinie potentielle", line);
    }
}

void VM::checkTimeout(int line) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_).count();
    if ((uint64_t)elapsed > options_.timeoutMs) {
        throw AlgoError("Timeout: Délai d'exécution dépassé (" + std::to_string(options_.timeoutMs) + " ms) - boucle infinie potentielle", line);
    }
}

void VM::addOutput(const std::string& str) {
    output_.push_back(str);
    if (outputCallback_) {
        outputCallback_(str);
    }
}

std::string VM::requestInput(const std::string& prompt) {
    if (inputBufferIdx_ < inputBuffer_.size()) {
        return inputBuffer_[inputBufferIdx_++];
    }
    if (inputCallback_) {
        return inputCallback_(prompt);
    }
    std::string line;
    std::getline(std::cin, line);
    return line;
}

Value VM::convertInput(const std::string& inputStr, const Value& currentVal) {
    std::string trimmed = inputStr;
    size_t first = trimmed.find_first_not_of(" \t\r\n");
    if (first != std::string::npos) {
        size_t last = trimmed.find_last_not_of(" \t\r\n");
        trimmed = trimmed.substr(first, (last - first + 1));
    } else {
        trimmed = "";
    }

    if (currentVal.isInt()) {
        try {
            return Value((int64_t)std::stoll(trimmed));
        } catch (...) {
            return Value((int64_t)0);
        }
    }
    if (currentVal.isReal()) {
        try {
            return Value((double)std::stod(trimmed));
        } catch (...) {
            return Value((double)0.0);
        }
    }
    if (currentVal.isBool()) {
        std::string lower = toLower(trimmed);
        return Value(lower == "vrai" || lower == "true" || lower == "1");
    }
    return Value(inputStr);
}

Value VM::getDefaultValue(const TypeSpec& ts, const std::string& varName) const {
    if (ts.kind == TypeKind::Array1D) {
        Value elemDef = getDefaultValue(ts.elemType, varName);
        std::vector<Value> arr(ts.rows, elemDef);
        return Value(std::move(arr));
    }
    if (ts.kind == TypeKind::Array2D) {
        Value elemDef = getDefaultValue(ts.elemType, varName);
        std::vector<Value> row(ts.cols, elemDef);
        std::vector<Value> matrix(ts.rows, Value(row));
        return Value(std::move(matrix));
    }
    if (ts.kind == TypeKind::Record) {
        std::map<std::string, Value> fields;
        for (const auto& f : ts.fields) {
            for (const auto& fname : f->names) {
                fields[fname] = getDefaultValue(f->typeSpec, fname);
            }
        }
        return Value(std::move(fields));
    }
    if (ts.kind == TypeKind::FileText || ts.kind == TypeKind::FileData) {
        Value v;
        v.type = ValueType::FileHandle;
        v.fileLogicalName = varName;
        return v;
    }
    return getDefaultValue(ts.name.empty() ? ts.elemType : ts.name, varName);
}

Value VM::getDefaultValue(const std::string& typeName, const std::string& varName) const {
    std::string norm = normalize(typeName);
    if (norm == "entier") return Value((int64_t)0);
    if (norm == "reel") return Value((double)0.0);
    if (norm == "booleen") return Value(false);
    if (norm == "caractere") return Value("");
    if (norm == "chaine") return Value("");

    auto it = typeDefs_.find(norm);
    if (it != typeDefs_.end()) {
        const auto& td = it->second;
        if (td.typeSpec.kind != TypeKind::Simple) {
            return getDefaultValue(td.typeSpec, varName);
        }
        Value elemDef = getDefaultValue(td.elemType, varName);
        std::vector<Value> arr(td.arraySize, elemDef);
        return Value(std::move(arr));
    }
    return Value();
}

void VM::writeFileData(AlgoFile& f, const Value& val) {
    std::streampos curPos = f.stream.tellp();
    if (curPos != (std::streampos)-1) {
        if (f.currentRecordIndex >= (int64_t)f.recordOffsets.size()) {
            f.recordOffsets.push_back(curPos);
        } else {
            f.recordOffsets[f.currentRecordIndex] = curPos;
        }
        f.currentRecordIndex++;
    }
    if (val.isInt()) {
        f.stream << "I " << val.asInt() << "\n";
    } else if (val.isReal()) {
        f.stream << "R " << val.asReal() << "\n";
    } else if (val.isBool()) {
        f.stream << "B " << (val.asBool() ? "1" : "0") << "\n";
    } else if (val.isString()) {
        f.stream << "S " << val.asString() << "\n";
    } else if (val.isArray()) {
        f.stream << "ARR " << val.arrVal.size() << "\n";
        for (const auto& item : val.arrVal) {
            writeFileData(f, item);
        }
        f.stream << "ENDARR\n";
    } else if (val.isRecord()) {
        f.stream << "REC " << val.recVal.size() << "\n";
        for (const auto& kv : val.recVal) {
            f.stream << "FLD " << kv.first << " ";
            writeFileData(f, kv.second);
        }
        f.stream << "ENDREC\n";
    } else {
        f.stream << "N 0\n";
    }
}

Value VM::readFileData(AlgoFile& f) {
    std::string tag;
    f.stream >> std::ws;
    if (!(f.stream >> tag)) {
        return Value();
    }
    if (tag == "I") {
        int64_t v;
        f.stream >> v;
        return Value(v);
    }
    if (tag == "R") {
        double v;
        f.stream >> v;
        return Value(v);
    }
    if (tag == "B") {
        int v;
        f.stream >> v;
        return Value(v != 0);
    }
    if (tag == "S") {
        std::string s;
        std::getline(f.stream >> std::ws, s);
        return Value(s);
    }
    if (tag == "REC") {
        size_t count;
        f.stream >> count;
        std::map<std::string, Value> fields;
        for (size_t i = 0; i < count; i++) {
            std::string fldTag, fldName;
            f.stream >> fldTag >> fldName;
            fields[fldName] = readFileData(f);
        }
        std::string endTag;
        f.stream >> endTag;
        return Value(fields);
    }
    if (tag == "ARR") {
        size_t count;
        f.stream >> count;
        std::vector<Value> items;
        items.reserve(count);
        for (size_t i = 0; i < count; i++) {
            items.push_back(readFileData(f));
        }
        std::string endTag;
        f.stream >> endTag;
        return Value(items);
    }
    return Value();
}

void VM::executeBuiltin(uint32_t builtinId, uint32_t argCount, int line) {
    std::vector<Value> args(argCount);
    for (int i = static_cast<int>(argCount) - 1; i >= 0; --i) {
        args[i] = pop();
    }

    switch (builtinId) {
        case BI_RACINE: {
            if (args.empty()) throw AlgoError("racine attend 1 argument", line);
            push(Value(std::sqrt(args[0].asReal())));
            break;
        }
        case BI_CARRE: {
            if (args.empty()) throw AlgoError("carre attend 1 argument", line);
            if (args[0].isInt()) push(Value(args[0].asInt() * args[0].asInt()));
            else push(Value(args[0].asReal() * args[0].asReal()));
            break;
        }
        case BI_ABS: {
            if (args.empty()) throw AlgoError("abs attend 1 argument", line);
            if (args[0].isInt()) push(Value(std::abs(args[0].asInt())));
            else push(Value(std::abs(args[0].asReal())));
            break;
        }
        case BI_EXP: {
            if (args.empty()) throw AlgoError("exp attend 1 argument", line);
            push(Value(std::exp(args[0].asReal())));
            break;
        }
        case BI_LN:
        case BI_LOG: {
            if (args.empty()) throw AlgoError("ln attend 1 argument", line);
            push(Value(std::log(args[0].asReal())));
            break;
        }
        case BI_ENT: {
            if (args.empty()) throw AlgoError("ent attend 1 argument", line);
            push(Value((int64_t)std::floor(args[0].asReal())));
            break;
        }
        case BI_ARRONDI: {
            if (args.empty()) throw AlgoError("arrondi attend 1 argument", line);
            double x = args[0].asReal();
            double ix = std::floor(x);
            if (x - ix == 0.5) {
                push(Value((int64_t)ix % 2 == 0 ? (int64_t)ix : (int64_t)ix + 1));
            } else {
                push(Value((int64_t)std::round(x)));
            }
            break;
        }
        case BI_LONG: {
            if (args.empty()) throw AlgoError("long attend 1 argument", line);
            if (args[0].isArray()) push(Value((int64_t)args[0].arrVal.size()));
            else push(Value((int64_t)utf8Length(args[0].asString())));
            break;
        }
        case BI_CHR: {
            if (args.empty()) throw AlgoError("chr attend 1 argument", line);
            push(Value(std::string(1, static_cast<char>(args[0].asInt()))));
            break;
        }
        case BI_ORD: {
            if (args.empty()) throw AlgoError("ord attend 1 argument", line);
            if (args[0].isInt()) {
                push(args[0]);
                break;
            }
            std::string s = args[0].asString();
            std::string normS = normalize(s);
            bool foundEnum = false;
            for (const auto& kv : typeDefs_) {
                if (kv.second.typeSpec.kind == TypeKind::Enum) {
                    for (size_t i = 0; i < kv.second.typeSpec.enumValues.size(); i++) {
                        if (normalize(kv.second.typeSpec.enumValues[i]) == normS) {
                            push(Value((int64_t)i));
                            foundEnum = true;
                            break;
                        }
                    }
                }
                if (foundEnum) break;
            }
            if (!foundEnum) {
                push(Value((int64_t)(s.empty() ? 0 : (unsigned char)s[0])));
            }
            break;
        }
        case BI_CONVCH: {
            if (args.empty()) throw AlgoError("convch attend 1 argument", line);
            push(Value(args[0].asString()));
            break;
        }
        case BI_VALEUR: {
            if (args.empty()) throw AlgoError("valeur attend 1 argument", line);
            std::string s = args[0].asString();
            try {
                if (s.find('.') != std::string::npos || s.find('e') != std::string::npos || s.find('E') != std::string::npos) {
                    push(Value(std::stod(s)));
                } else {
                    push(Value((int64_t)std::stoll(s)));
                }
            } catch (...) {
                push(Value(0.0));
            }
            break;
        }
        case BI_SOUS_CHAINE: {
            if (args.size() < 3) throw AlgoError("sous_chaine attend 3 arguments", line);
            std::string s = args[0].asString();
            int64_t d = args[1].asInt();
            int64_t p3 = args[2].asInt();
            size_t len = utf8Length(s);
            if (d < 0) d = 0;
            if (d >= (int64_t)len) {
                push(Value(""));
                break;
            }
            int64_t end;
            if (p3 < d) {
                end = d + p3;
            } else if (d + p3 == (int64_t)len && p3 != (int64_t)len) {
                end = len;
            } else {
                end = p3;
            }
            if (end < d) {
                push(Value(""));
                break;
            }
            if (end > (int64_t)len) end = len;
            push(Value(utf8Substr(s, static_cast<size_t>(d), static_cast<size_t>(end))));
            break;
        }
        case BI_POS: {
            if (args.size() < 2) throw AlgoError("pos attend 2 arguments", line);
            std::string s1 = args[0].asString();
            std::string s2 = args[1].asString();
            size_t p = s2.find(s1);
            if (p == std::string::npos) push(Value((int64_t)-1));
            else push(Value((int64_t)utf8Length(s2.substr(0, p))));
            break;
        }
        case BI_MAJUS: {
            if (args.empty()) throw AlgoError("majus attend 1 argument", line);
            push(Value(toUpper(args[0].asString())));
            break;
        }
        case BI_MINUS: {
            if (args.empty()) throw AlgoError("minus attend 1 argument", line);
            push(Value(toLower(args[0].asString())));
            break;
        }
        case BI_EFFACER: {
            if (args.size() < 3) throw AlgoError("effacer attend 3 arguments", line);
            std::string s = args[0].asString();
            int64_t d = args[1].asInt();
            int64_t f = args[2].asInt();
            size_t len = utf8Length(s);
            int64_t end = (f < d) ? (d + f) : f;
            std::string part1 = utf8Substr(s, 0, static_cast<size_t>(std::max((int64_t)0, d)));
            std::string part2 = (end < (int64_t)len) ? utf8Substr(s, static_cast<size_t>(end), len) : "";
            push(Value(part1 + part2));
            break;
        }
        case BI_ESTNUM: {
            if (args.empty()) throw AlgoError("estnum attend 1 argument", line);
            static const std::regex numRegex("^-?\\d+(\\.\\d+)?$");
            push(Value(std::regex_match(args[0].asString(), numRegex)));
            break;
        }
        case BI_ALEA: {
            if (args.size() < 2) throw AlgoError("alea attend 2 arguments", line);
            int64_t minVal = args[0].asInt();
            int64_t maxVal = args[1].asInt();
            push(Value(rng_.randomInt(minVal, maxVal)));
            break;
        }
        case BI_GRAINE: {
            if (args.empty()) throw AlgoError("graine attend 1 argument", line);
            int64_t seed = args[0].asInt();
            rng_.reseed(static_cast<uint32_t>(seed));
            push(Value(seed));
            break;
        }
        case BI_OUVRIR: {
            if (args.size() < 3) throw AlgoError("Ouvrir attend 3 arguments", line);
            std::string path = args[0].asString();
            std::string fVar = args[1].fileLogicalName.empty() ? args[1].asString() : args[1].fileLogicalName;
            std::string mode = args[2].asString();
            std::string fNorm = normalize(fVar);

            auto fileIt = openFiles_.find(fNorm);
            if (fileIt != openFiles_.end() && fileIt->second->isOpen) {
                fileIt->second->stream.close();
                fileIt->second->isOpen = false;
            }

            auto file = std::make_shared<AlgoFile>();
            file->path = path;
            file->mode = mode;
            file->isText = (mode == "r" || mode == "w" || mode == "a");

            std::ios_base::openmode m;
            if (mode == "r") m = std::ios_base::in;
            else if (mode == "w") m = std::ios_base::out | std::ios_base::trunc;
            else if (mode == "a") m = std::ios_base::out | std::ios_base::app;
            else if (mode == "rb") m = std::ios_base::in | std::ios_base::binary;
            else if (mode == "wb") m = std::ios_base::out | std::ios_base::trunc | std::ios_base::binary;
            else if (mode == "ab") m = std::ios_base::out | std::ios_base::app | std::ios_base::binary;
            else throw AlgoError("Mode de fichier invalide: '" + mode + "'", line);

#ifdef _WIN32
            file->stream.open(std::filesystem::u8path(path), m);
#else
            file->stream.open(path, m);
#endif
            file->isOpen = file->stream.is_open();
            file->recordOffsets.clear();
            file->currentRecordIndex = 0;
            if (mode == "rb") {
                while (true) {
                    file->stream >> std::ws;
                    std::streampos p = file->stream.tellg();
                    if (p == (std::streampos)-1 || file->stream.peek() == std::char_traits<char>::eof()) break;
                    file->recordOffsets.push_back(p);
                    readFileData(*file);
                    if (file->stream.fail()) break;
                }
                file->stream.clear();
                file->stream.seekg(0, std::ios::beg);
                file->currentRecordIndex = 0;
            }
            openFiles_[fNorm] = file;
            push(Value());
            break;
        }
        case BI_FERMER: {
            if (args.empty()) throw AlgoError("Fermer attend 1 argument", line);
            std::string fVar = args[0].fileLogicalName.empty() ? args[0].asString() : args[0].fileLogicalName;
            std::string fNorm = normalize(fVar);
            auto it = openFiles_.find(fNorm);
            if (it != openFiles_.end() && it->second->isOpen) {
                it->second->stream.close();
                it->second->isOpen = false;
            }
            push(Value());
            break;
        }
        case BI_LIRE_LIGNE: {
            if (args.empty()) throw AlgoError("Lire_Ligne attend au moins 1 argument", line);
            std::string fVar = args[0].fileLogicalName.empty() ? args[0].asString() : args[0].fileLogicalName;
            std::string fNorm = normalize(fVar);
            auto it = openFiles_.find(fNorm);
            if (it == openFiles_.end() || !it->second->isOpen) {
                throw AlgoError("Fichier '" + fVar + "' non ouvert", line);
            }
            std::string lineContent;
            if (std::getline(it->second->stream, lineContent)) {
                if (!lineContent.empty() && lineContent.back() == '\r') lineContent.pop_back();
            }
            push(Value(lineContent));
            break;
        }
        case BI_FIN_FICHIER: {
            if (args.empty()) throw AlgoError("Fin_fichier attend 1 argument", line);
            std::string fVar = args[0].fileLogicalName.empty() ? args[0].asString() : args[0].fileLogicalName;
            std::string fNorm = normalize(fVar);
            auto it = openFiles_.find(fNorm);
            if (it == openFiles_.end() || !it->second->isOpen) {
                push(Value(true));
                break;
            }
            auto& stream = it->second->stream;
            if (!stream.good()) {
                push(Value(true));
                break;
            }
            if (!it->second->isText) {
                stream >> std::ws;
            }
            int c = stream.peek();
            push(Value(c == std::char_traits<char>::eof()));
            break;
        }
        case BI_SIN: {
            if (args.empty()) throw AlgoError("sin attend 1 argument", line);
            push(Value(std::sin(args[0].asReal())));
            break;
        }
        case BI_COS: {
            if (args.empty()) throw AlgoError("cos attend 1 argument", line);
            push(Value(std::cos(args[0].asReal())));
            break;
        }
        case BI_TAN: {
            if (args.empty()) throw AlgoError("tan attend 1 argument", line);
            push(Value(std::tan(args[0].asReal())));
            break;
        }
        case BI_ARCTAN: {
            if (args.empty()) throw AlgoError("arctan attend 1 argument", line);
            push(Value(std::atan(args[0].asReal())));
            break;
        }
        case BI_FRAC: {
            if (args.empty()) throw AlgoError("frac attend 1 argument", line);
            double intp;
            push(Value(std::modf(args[0].asReal(), &intp)));
            break;
        }
        case BI_PUISSANCE: {
            if (args.size() < 2) throw AlgoError("puissance attend 2 arguments", line);
            double res = std::pow(args[0].asReal(), args[1].asReal());
            if (args[0].isInt() && args[1].isInt() && args[1].asInt() >= 0) {
                push(Value((int64_t)std::round(res)));
            } else {
                push(Value(res));
            }
            break;
        }
        case BI_INSERER: {
            if (args.size() < 3) throw AlgoError("inserer attend 3 arguments", line);
            std::string ch1 = args[0].asString();
            std::string ch2 = args[1].asString();
            int64_t pos = args[2].asInt();
            size_t len = utf8Length(ch2);
            if (pos < 0) pos = 0;
            if (pos > (int64_t)len) pos = len;
            push(Value(utf8Substr(ch2, 0, (size_t)pos) + ch1 + utf8Substr(ch2, (size_t)pos, len)));
            break;
        }
        case BI_TAILLE_FICHIER: {
            if (args.empty()) throw AlgoError("taille_fichier attend 1 argument", line);
            std::string fVar = args[0].fileLogicalName.empty() ? args[0].asString() : args[0].fileLogicalName;
            auto it = openFiles_.find(normalize(fVar));
            if (it == openFiles_.end() || !it->second->isOpen) push(Value((int64_t)0));
            else push(Value((int64_t)it->second->recordOffsets.size()));
            break;
        }
        case BI_POSITION_FICHIER: {
            if (args.empty()) throw AlgoError("position_fichier attend 1 argument", line);
            std::string fVar = args[0].fileLogicalName.empty() ? args[0].asString() : args[0].fileLogicalName;
            auto it = openFiles_.find(normalize(fVar));
            if (it == openFiles_.end() || !it->second->isOpen) push(Value((int64_t)0));
            else push(Value(it->second->currentRecordIndex));
            break;
        }
        case BI_ALLER_A: {
            if (args.size() < 2) throw AlgoError("aller_a attend 2 arguments", line);
            std::string fVar = args[0].fileLogicalName.empty() ? args[0].asString() : args[0].fileLogicalName;
            auto it = openFiles_.find(normalize(fVar));
            if (it == openFiles_.end() || !it->second->isOpen) throw AlgoError("Fichier non ouvert", line);
            int64_t targetPos = args[1].asInt();
            auto& f = *(it->second);
            if (targetPos < 0 || targetPos > (int64_t)f.recordOffsets.size()) throw AlgoError("Position fichier hors bornes", line);
            f.currentRecordIndex = targetPos;
            f.stream.clear();
            if (targetPos < (int64_t)f.recordOffsets.size()) {
                f.stream.seekg(f.recordOffsets[targetPos]);
                f.stream.seekp(f.recordOffsets[targetPos]);
            } else {
                f.stream.seekg(0, std::ios::end);
                f.stream.seekp(0, std::ios::end);
            }
            push(Value());
            break;
        }
        default:
            push(Value());
            break;
    }
}

void VM::run(const BytecodeProgram& prog) {
    stack_.clear();
    stack_.reserve(500000);
    frames_.clear();
    globals_.clear();
    typeDefs_.clear();
    subroutines_ = prog.subroutines;
    openFiles_.clear();
    output_.clear();
    inputBufferIdx_ = 0;
    iterationCount_ = 0;
    instructionCount_ = 0;
    startTime_ = std::chrono::steady_clock::now();

    for (const auto& td : prog.typeDecls) {
        typeDefs_[normalize(td.name)] = td;
        if (td.typeSpec.kind == TypeKind::Enum) {
            for (const auto& ev : td.typeSpec.enumValues) {
                globals_[normalize(ev)] = Value(ev);
            }
        }
    }

    // Initialize globals
    for (const auto& vd : prog.varDecls) {
        std::string vtNorm = normalize(vd.varType);
        if (vtNorm == "procedure" || vtNorm == "fonction" || vd.typeSpec.name == "procedure" || vd.typeSpec.name == "fonction") continue;
        for (const auto& name : vd.names) {
            Value defVal;
            if (vd.typeSpec.kind != TypeKind::Simple || !vd.typeSpec.name.empty()) {
                defVal = getDefaultValue(vd.typeSpec, name);
            } else {
                defVal = getDefaultValue(vd.varType, name);
            }
            globals_[normalize(name)] = std::move(defVal);
        }
    }

    // Push initial main frame
    VMCallFrame mainFrame;
    mainFrame.chunk = &prog.mainChunk;
    mainFrame.ip = 0;
    mainFrame.stackBase = 0;
    mainFrame.globalSlots.assign(prog.mainChunk.names.size(), nullptr);
    frames_.push_back(std::move(mainFrame));

    VMCallFrame* frame = &frames_.back();

    while (true) {
        instructionCount_++;
        if ((instructionCount_ & 4095) == 0) {
            checkTimeout(frame->chunk->lines.empty() ? -1 : frame->chunk->lines[frame->ip]);
        }

        uint8_t instruction = frame->chunk->readByte(frame->ip);
        OpCode op = static_cast<OpCode>(instruction);
        int curLine = frame->chunk->lines[frame->ip - 1];

        switch (op) {
            case OpCode::OP_NOP:
                break;

            case OpCode::OP_CONST: {
                uint32_t constIdx = frame->chunk->readUint32(frame->ip);
                push(frame->chunk->constants[constIdx]);
                break;
            }

            case OpCode::OP_POP:
                pop();
                break;

            case OpCode::OP_DUP:
                push(peek());
                break;

            case OpCode::OP_GET_GLOBAL: {
                uint32_t nameIdx = frame->chunk->readUint32(frame->ip);
                Value* slot = (nameIdx < frame->globalSlots.size()) ? frame->globalSlots[nameIdx] : nullptr;
                if (slot) {
                    push(*slot);
                } else {
                    const std::string& name = frame->chunk->names[nameIdx];
                    std::string norm = normalize(name);
                    auto it = globals_.find(norm);
                    if (it != globals_.end()) {
                        if (nameIdx < frame->globalSlots.size()) frame->globalSlots[nameIdx] = &it->second;
                        push(it->second);
                    } else {
                        throw AlgoError("Variable '" + name + "' non définie", curLine);
                    }
                }
                break;
            }

            case OpCode::OP_SET_GLOBAL: {
                uint32_t nameIdx = frame->chunk->readUint32(frame->ip);
                Value* slot = (nameIdx < frame->globalSlots.size()) ? frame->globalSlots[nameIdx] : nullptr;
                if (slot) {
                    *slot = peek();
                } else {
                    const std::string& name = frame->chunk->names[nameIdx];
                    std::string norm = normalize(name);
                    auto it = globals_.find(norm);
                    if (it != globals_.end()) {
                        it->second = peek();
                        if (nameIdx < frame->globalSlots.size()) frame->globalSlots[nameIdx] = &it->second;
                    } else {
                        globals_[norm] = peek();
                        if (nameIdx < frame->globalSlots.size()) frame->globalSlots[nameIdx] = &globals_[norm];
                    }
                }
                break;
            }

            case OpCode::OP_POP_GLOBAL: {
                uint32_t nameIdx = frame->chunk->readUint32(frame->ip);
                Value* slot = (nameIdx < frame->globalSlots.size()) ? frame->globalSlots[nameIdx] : nullptr;
                if (slot) {
                    *slot = pop();
                } else {
                    const std::string& name = frame->chunk->names[nameIdx];
                    std::string norm = normalize(name);
                    auto it = globals_.find(norm);
                    if (it != globals_.end()) {
                        it->second = pop();
                        if (nameIdx < frame->globalSlots.size()) frame->globalSlots[nameIdx] = &it->second;
                    } else {
                        globals_[norm] = pop();
                        if (nameIdx < frame->globalSlots.size()) frame->globalSlots[nameIdx] = &globals_[norm];
                    }
                }
                break;
            }

            case OpCode::OP_GET_LOCAL: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                push(stack_[frame->stackBase + slot]);
                break;
            }

            case OpCode::OP_SET_LOCAL: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                stack_[frame->stackBase + slot] = peek();
                break;
            }

            case OpCode::OP_POP_LOCAL: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                stack_[frame->stackBase + slot] = pop();
                break;
            }

            case OpCode::OP_GET_REF: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                if (slot < frame->refSlots.size() && frame->refSlots[slot].ptr != nullptr) {
                    push(*(frame->refSlots[slot].ptr));
                } else {
                    push(stack_[frame->stackBase + slot]);
                }
                break;
            }

            case OpCode::OP_SET_REF: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                if (slot < frame->refSlots.size() && frame->refSlots[slot].ptr != nullptr) {
                    *(frame->refSlots[slot].ptr) = peek();
                } else {
                    stack_[frame->stackBase + slot] = peek();
                }
                break;
            }

            case OpCode::OP_POP_REF: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                if (slot < frame->refSlots.size() && frame->refSlots[slot].ptr != nullptr) {
                    *(frame->refSlots[slot].ptr) = pop();
                } else {
                    stack_[frame->stackBase + slot] = pop();
                }
                break;
            }

            case OpCode::OP_PUSH_REF_GLOBAL: {
                uint32_t nameIdx = frame->chunk->readUint32(frame->ip);
                const std::string& name = frame->chunk->names[nameIdx];
                std::string norm = normalize(name);
                auto it = globals_.find(norm);
                if (it != globals_.end()) {
                    push(Value::makeRef(&it->second));
                } else {
                    globals_[norm] = Value();
                    push(Value::makeRef(&globals_[norm]));
                }
                break;
            }

            case OpCode::OP_PUSH_REF_LOCAL: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                push(Value::makeRef(&stack_[frame->stackBase + slot]));
                break;
            }

            case OpCode::OP_PUSH_REF_REF: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                Value* ptr = (slot < frame->refSlots.size() && frame->refSlots[slot].ptr != nullptr)
                    ? frame->refSlots[slot].ptr : &stack_[frame->stackBase + slot];
                push(Value::makeRef(ptr));
                break;
            }

            case OpCode::OP_REF_ARRAY: {
                int64_t idx = pop().asInt();
                Value refVal = pop();
                if (!refVal.isRef() || !refVal.refPtr || !refVal.refPtr->isArray()) {
                    throw AlgoError("Référence de tableau invalide pour l'indexation", curLine);
                }
                Value* arrPtr = refVal.refPtr;
                if (idx < 0 || (size_t)idx >= arrPtr->arrVal.size()) {
                    throw AlgoError("Indice tableau hors bornes", curLine);
                }
                push(Value::makeRef(&arrPtr->arrVal[idx]));
                break;
            }

            case OpCode::OP_REF_ARRAY2D: {
                int64_t c = pop().asInt();
                int64_t r = pop().asInt();
                Value refVal = pop();
                if (!refVal.isRef() || !refVal.refPtr || !refVal.refPtr->isArray()) {
                    throw AlgoError("Référence de matrice invalide pour l'indexation", curLine);
                }
                Value* matPtr = refVal.refPtr;
                if (r < 0 || (size_t)r >= matPtr->arrVal.size()) {
                    throw AlgoError("Indice ligne matrice hors bornes", curLine);
                }
                Value& row = matPtr->arrVal[r];
                if (!row.isArray() || c < 0 || (size_t)c >= row.arrVal.size()) {
                    throw AlgoError("Indice colonne matrice hors bornes", curLine);
                }
                push(Value::makeRef(&row.arrVal[c]));
                break;
            }

            case OpCode::OP_REF_MEMBER: {
                uint32_t fIdx = frame->chunk->readUint32(frame->ip);
                const std::string& fieldName = frame->chunk->names[fIdx];
                Value refVal = pop();
                if (!refVal.isRef() || !refVal.refPtr || !refVal.refPtr->isRecord()) {
                    throw AlgoError("Référence d'enregistrement invalide pour l'accès au champ", curLine);
                }
                Value* recPtr = refVal.refPtr;
                auto it = recPtr->recVal.find(fieldName);
                if (it != recPtr->recVal.end()) {
                    push(Value::makeRef(&it->second));
                } else {
                    std::string norm = normalize(fieldName);
                    bool found = false;
                    for (auto& kv : recPtr->recVal) {
                        if (normalize(kv.first) == norm) {
                            push(Value::makeRef(&kv.second));
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        push(Value::makeRef(&recPtr->recVal[fieldName]));
                    }
                }
                break;
            }

            case OpCode::OP_SET_LVALUE: {
                Value val = pop();
                Value refVal = pop();
                if (refVal.isRef() && refVal.refPtr) {
                    *(refVal.refPtr) = val;
                } else {
                    throw AlgoError("Cible d'affectation non valide", curLine);
                }
                break;
            }

            case OpCode::OP_ADD: {
                if (stack_.size() >= 2) {
                    Value& b = stack_.back();
                    Value& a = stack_[stack_.size() - 2];
                    if (a.type == ValueType::Int && b.type == ValueType::Int) {
                        a.intVal += b.intVal;
                        a.realVal = static_cast<double>(a.intVal);
                        stack_.pop_back();
                        break;
                    }
                    if (a.type == ValueType::Real && b.type == ValueType::Real) {
                        a.realVal += b.realVal;
                        a.intVal = static_cast<int64_t>(a.realVal);
                        stack_.pop_back();
                        break;
                    }
                }
                Value b = pop();
                Value a = pop();
                push(a.add(b));
                break;
            }

            case OpCode::OP_SUB: {
                if (stack_.size() >= 2) {
                    Value& b = stack_.back();
                    Value& a = stack_[stack_.size() - 2];
                    if (a.type == ValueType::Int && b.type == ValueType::Int) {
                        a.intVal -= b.intVal;
                        a.realVal = static_cast<double>(a.intVal);
                        stack_.pop_back();
                        break;
                    }
                    if (a.type == ValueType::Real && b.type == ValueType::Real) {
                        a.realVal -= b.realVal;
                        a.intVal = static_cast<int64_t>(a.realVal);
                        stack_.pop_back();
                        break;
                    }
                }
                Value b = pop();
                Value a = pop();
                push(a.sub(b));
                break;
            }

            case OpCode::OP_MUL: {
                if (stack_.size() >= 2) {
                    Value& b = stack_.back();
                    Value& a = stack_[stack_.size() - 2];
                    if (a.type == ValueType::Int && b.type == ValueType::Int) {
                        a.intVal *= b.intVal;
                        a.realVal = static_cast<double>(a.intVal);
                        stack_.pop_back();
                        break;
                    }
                    if (a.type == ValueType::Real && b.type == ValueType::Real) {
                        a.realVal *= b.realVal;
                        a.intVal = static_cast<int64_t>(a.realVal);
                        stack_.pop_back();
                        break;
                    }
                }
                Value b = pop();
                Value a = pop();
                push(a.mul(b));
                break;
            }

            case OpCode::OP_POW: {
                Value b = pop();
                Value a = pop();
                double res = std::pow(a.asReal(), b.asReal());
                if (a.isInt() && b.isInt() && b.asInt() >= 0) {
                    push(Value((int64_t)std::round(res)));
                } else {
                    push(Value(res));
                }
                break;
            }

            case OpCode::OP_DIV_REAL: {
                Value b = pop();
                Value a = pop();
                push(a.divReal(b));
                break;
            }

            case OpCode::OP_DIV_INT: {
                if (stack_.size() >= 2) {
                    Value& b = stack_.back();
                    Value& a = stack_[stack_.size() - 2];
                    if (a.type == ValueType::Int && b.type == ValueType::Int) {
                        if (b.intVal == 0) throw AlgoError("Division entière par zéro", curLine);
                        double res = std::floor(static_cast<double>(a.intVal) / static_cast<double>(b.intVal));
                        a.intVal = static_cast<int64_t>(res);
                        a.realVal = static_cast<double>(a.intVal);
                        stack_.pop_back();
                        break;
                    }
                }
                Value b = pop();
                Value a = pop();
                push(a.divInt(b));
                break;
            }

            case OpCode::OP_MOD: {
                if (stack_.size() >= 2) {
                    Value& b = stack_.back();
                    Value& a = stack_[stack_.size() - 2];
                    if (a.type == ValueType::Int && b.type == ValueType::Int) {
                        if (b.intVal == 0) throw AlgoError("Modulo par zéro", curLine);
                        a.intVal %= b.intVal;
                        a.realVal = static_cast<double>(a.intVal);
                        stack_.pop_back();
                        break;
                    }
                }
                Value b = pop();
                Value a = pop();
                push(a.mod(b));
                break;
            }

            case OpCode::OP_NEG: {
                if (!stack_.empty() && stack_.back().type == ValueType::Int) {
                    stack_.back().intVal = -stack_.back().intVal;
                    stack_.back().realVal = static_cast<double>(stack_.back().intVal);
                    break;
                }
                Value val = pop();
                if (val.isInt()) push(Value(-val.asInt()));
                else push(Value(-val.asReal()));
                break;
            }

            case OpCode::OP_NOT: {
                if (!stack_.empty() && stack_.back().type == ValueType::Bool) {
                    stack_.back().boolVal = !stack_.back().boolVal;
                    stack_.back().intVal = stack_.back().boolVal ? 1 : 0;
                    break;
                }
                Value val = pop();
                push(Value(!val.asBool()));
                break;
            }

            case OpCode::OP_AND: {
                Value b = pop();
                Value a = pop();
                push(Value(a.asBool() && b.asBool()));
                break;
            }

            case OpCode::OP_OR: {
                Value b = pop();
                Value a = pop();
                push(Value(a.asBool() || b.asBool()));
                break;
            }

            case OpCode::OP_EQ: {
                if (stack_.size() >= 2) {
                    Value& b = stack_.back();
                    Value& a = stack_[stack_.size() - 2];
                    if (a.type == ValueType::Int && b.type == ValueType::Int) {
                        bool res = (a.intVal == b.intVal);
                        a.type = ValueType::Bool;
                        a.boolVal = res;
                        a.intVal = res ? 1 : 0;
                        stack_.pop_back();
                        break;
                    }
                }
                Value b = pop();
                Value a = pop();
                push(Value(a.equals(b)));
                break;
            }

            case OpCode::OP_NE: {
                if (stack_.size() >= 2) {
                    Value& b = stack_.back();
                    Value& a = stack_[stack_.size() - 2];
                    if (a.type == ValueType::Int && b.type == ValueType::Int) {
                        bool res = (a.intVal != b.intVal);
                        a.type = ValueType::Bool;
                        a.boolVal = res;
                        a.intVal = res ? 1 : 0;
                        stack_.pop_back();
                        break;
                    }
                }
                Value b = pop();
                Value a = pop();
                push(Value(a.notEquals(b)));
                break;
            }

            case OpCode::OP_LT: {
                if (stack_.size() >= 2) {
                    Value& b = stack_.back();
                    Value& a = stack_[stack_.size() - 2];
                    if (a.type == ValueType::Int && b.type == ValueType::Int) {
                        bool res = (a.intVal < b.intVal);
                        a.type = ValueType::Bool;
                        a.boolVal = res;
                        a.intVal = res ? 1 : 0;
                        stack_.pop_back();
                        break;
                    }
                }
                Value b = pop();
                Value a = pop();
                push(Value(a.lessThan(b)));
                break;
            }

            case OpCode::OP_LE: {
                if (stack_.size() >= 2) {
                    Value& b = stack_.back();
                    Value& a = stack_[stack_.size() - 2];
                    if (a.type == ValueType::Int && b.type == ValueType::Int) {
                        bool res = (a.intVal <= b.intVal);
                        a.type = ValueType::Bool;
                        a.boolVal = res;
                        a.intVal = res ? 1 : 0;
                        stack_.pop_back();
                        break;
                    }
                }
                Value b = pop();
                Value a = pop();
                push(Value(a.lessOrEqual(b)));
                break;
            }

            case OpCode::OP_GT: {
                if (stack_.size() >= 2) {
                    Value& b = stack_.back();
                    Value& a = stack_[stack_.size() - 2];
                    if (a.type == ValueType::Int && b.type == ValueType::Int) {
                        bool res = (a.intVal > b.intVal);
                        a.type = ValueType::Bool;
                        a.boolVal = res;
                        a.intVal = res ? 1 : 0;
                        stack_.pop_back();
                        break;
                    }
                }
                Value b = pop();
                Value a = pop();
                push(Value(a.greaterThan(b)));
                break;
            }

            case OpCode::OP_GE: {
                if (stack_.size() >= 2) {
                    Value& b = stack_.back();
                    Value& a = stack_[stack_.size() - 2];
                    if (a.type == ValueType::Int && b.type == ValueType::Int) {
                        bool res = (a.intVal >= b.intVal);
                        a.type = ValueType::Bool;
                        a.boolVal = res;
                        a.intVal = res ? 1 : 0;
                        stack_.pop_back();
                        break;
                    }
                }
                Value b = pop();
                Value a = pop();
                push(Value(a.greaterOrEqual(b)));
                break;
            }

            case OpCode::OP_IN: {
                Value container = pop();
                Value elem = pop();
                push(Value(container.contains(elem)));
                break;
            }

            case OpCode::OP_JUMP: {
                int32_t offset = frame->chunk->readInt32(frame->ip);
                frame->ip += offset;
                break;
            }

            case OpCode::OP_JUMP_IF_FALSE: {
                int32_t offset = frame->chunk->readInt32(frame->ip);
                if (!pop().asBool()) {
                    frame->ip += offset;
                }
                break;
            }

            case OpCode::OP_JUMP_IF_TRUE: {
                int32_t offset = frame->chunk->readInt32(frame->ip);
                if (pop().asBool()) {
                    frame->ip += offset;
                }
                break;
            }

            case OpCode::OP_JUMP_IF_FALSE_PEEK: {
                int32_t offset = frame->chunk->readInt32(frame->ip);
                if (!peek().asBool()) {
                    frame->ip += offset;
                } else {
                    pop();
                }
                break;
            }

            case OpCode::OP_JUMP_IF_TRUE_PEEK: {
                int32_t offset = frame->chunk->readInt32(frame->ip);
                if (peek().asBool()) {
                    frame->ip += offset;
                } else {
                    pop();
                }
                break;
            }

            case OpCode::OP_LOOP: {
                uint32_t offset = frame->chunk->readUint32(frame->ip);
                frame->ip -= offset;
                checkIterationLimit(curLine);
                break;
            }

            case OpCode::OP_ARRAY_GET: {
                Value idxVal = pop();
                Value arr = pop();
                if (!arr.isArray() && !arr.isString()) {
                    throw AlgoError("L'expression n'est pas un tableau", curLine);
                }
                int64_t idx = idxVal.asInt();
                if (arr.isString()) {
                    size_t len = utf8Length(arr.asString());
                    if (idx < 0 || (size_t)idx >= len) {
                        throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", curLine);
                    }
                    push(Value(utf8Substr(arr.asString(), static_cast<size_t>(idx), static_cast<size_t>(idx + 1))));
                } else {
                    if (idx < 0 || (size_t)idx >= arr.arrVal.size()) {
                        throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", curLine);
                    }
                    push(arr.arrVal[idx]);
                }
                break;
            }

            case OpCode::OP_ARRAY_SET: {
                Value val = pop();
                Value idxVal = pop();
                Value arr = pop();
                if (!arr.isArray()) {
                    throw AlgoError("L'expression n'est pas un tableau", curLine);
                }
                int64_t idx = idxVal.asInt();
                if (idx < 0 || (size_t)idx >= arr.arrVal.size()) {
                    throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", curLine);
                }
                arr.arrVal[idx] = val;
                push(arr);
                break;
            }

            case OpCode::OP_ARRAY_GET_GLOBAL: {
                uint32_t nameIdx = frame->chunk->readUint32(frame->ip);
                Value* slot = (nameIdx < frame->globalSlots.size()) ? frame->globalSlots[nameIdx] : nullptr;
                if (!slot) {
                    const std::string& name = frame->chunk->names[nameIdx];
                    auto it = globals_.find(normalize(name));
                    if (it == globals_.end()) throw AlgoError("Variable '" + name + "' non définie", curLine);
                    slot = &it->second;
                    if (nameIdx < frame->globalSlots.size()) frame->globalSlots[nameIdx] = slot;
                }
                int64_t idx = pop().asInt();
                if (slot->isArray()) {
                    if (idx < 0 || (size_t)idx >= slot->arrVal.size()) {
                        throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", curLine);
                    }
                    push(slot->arrVal[idx]);
                } else if (slot->isString()) {
                    size_t len = utf8Length(slot->asString());
                    if (idx < 0 || (size_t)idx >= len) {
                        throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", curLine);
                    }
                    push(Value(utf8Substr(slot->asString(), static_cast<size_t>(idx), static_cast<size_t>(idx + 1))));
                } else {
                    throw AlgoError("L'expression n'est pas un tableau", curLine);
                }
                break;
            }

            case OpCode::OP_ARRAY_SET_GLOBAL: {
                uint32_t nameIdx = frame->chunk->readUint32(frame->ip);
                Value* slot = (nameIdx < frame->globalSlots.size()) ? frame->globalSlots[nameIdx] : nullptr;
                if (!slot) {
                    const std::string& name = frame->chunk->names[nameIdx];
                    auto it = globals_.find(normalize(name));
                    if (it == globals_.end()) throw AlgoError("Variable '" + name + "' non définie", curLine);
                    slot = &it->second;
                    if (nameIdx < frame->globalSlots.size()) frame->globalSlots[nameIdx] = slot;
                }
                Value val = pop();
                int64_t idx = pop().asInt();
                if (!slot->isArray()) throw AlgoError("L'expression n'est pas un tableau", curLine);
                if (idx < 0 || (size_t)idx >= slot->arrVal.size()) {
                    throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", curLine);
                }
                slot->arrVal[idx] = std::move(val);
                break;
            }

            case OpCode::OP_ARRAY_GET_LOCAL: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                Value& arr = stack_[frame->stackBase + slot];
                int64_t idx = pop().asInt();
                if (arr.isArray()) {
                    if (idx < 0 || (size_t)idx >= arr.arrVal.size()) {
                        throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", curLine);
                    }
                    push(arr.arrVal[idx]);
                } else if (arr.isString()) {
                    size_t len = utf8Length(arr.asString());
                    if (idx < 0 || (size_t)idx >= len) {
                        throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", curLine);
                    }
                    push(Value(utf8Substr(arr.asString(), static_cast<size_t>(idx), static_cast<size_t>(idx + 1))));
                } else {
                    throw AlgoError("L'expression n'est pas un tableau", curLine);
                }
                break;
            }

            case OpCode::OP_ARRAY_SET_LOCAL: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                Value& arr = stack_[frame->stackBase + slot];
                Value val = pop();
                int64_t idx = pop().asInt();
                if (!arr.isArray()) throw AlgoError("L'expression n'est pas un tableau", curLine);
                if (idx < 0 || (size_t)idx >= arr.arrVal.size()) {
                    throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", curLine);
                }
                arr.arrVal[idx] = std::move(val);
                break;
            }

            case OpCode::OP_ARRAY_GET_REF: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                Value* arrPtr = (slot < frame->refSlots.size() && frame->refSlots[slot].ptr != nullptr)
                    ? frame->refSlots[slot].ptr : &stack_[frame->stackBase + slot];
                int64_t idx = pop().asInt();
                if (arrPtr->isArray()) {
                    if (idx < 0 || (size_t)idx >= arrPtr->arrVal.size()) {
                        throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", curLine);
                    }
                    push(arrPtr->arrVal[idx]);
                } else if (arrPtr->isString()) {
                    size_t len = utf8Length(arrPtr->asString());
                    if (idx < 0 || (size_t)idx >= len) {
                        throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", curLine);
                    }
                    push(Value(utf8Substr(arrPtr->asString(), static_cast<size_t>(idx), static_cast<size_t>(idx + 1))));
                } else {
                    throw AlgoError("L'expression n'est pas un tableau", curLine);
                }
                break;
            }

            case OpCode::OP_ARRAY_SET_REF: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                Value* arrPtr = (slot < frame->refSlots.size() && frame->refSlots[slot].ptr != nullptr)
                    ? frame->refSlots[slot].ptr : &stack_[frame->stackBase + slot];
                Value val = pop();
                int64_t idx = pop().asInt();
                if (!arrPtr->isArray()) throw AlgoError("L'expression n'est pas un tableau", curLine);
                if (idx < 0 || (size_t)idx >= arrPtr->arrVal.size()) {
                    throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", curLine);
                }
                arrPtr->arrVal[idx] = std::move(val);
                break;
            }

            case OpCode::OP_ARRAY2D_GET: {
                Value colVal = pop();
                Value rowVal = pop();
                Value mat = pop();
                if (!mat.isArray()) throw AlgoError("L'expression n'est pas une matrice", curLine);
                int64_t r = rowVal.asInt();
                if (r < 0 || (size_t)r >= mat.arrVal.size()) throw AlgoError("Indice ligne hors bornes", curLine);
                Value row = mat.arrVal[r];
                int64_t c = colVal.asInt();
                if (row.isString()) {
                    size_t len = utf8Length(row.asString());
                    if (c < 0 || (size_t)c >= len) throw AlgoError("Indice colonne hors bornes", curLine);
                    push(Value(utf8Substr(row.asString(), static_cast<size_t>(c), static_cast<size_t>(c + 1))));
                } else {
                    if (!row.isArray()) throw AlgoError("Ligne n'est pas un tableau", curLine);
                    if (c < 0 || (size_t)c >= row.arrVal.size()) throw AlgoError("Indice colonne hors bornes", curLine);
                    push(row.arrVal[c]);
                }
                break;
            }

            case OpCode::OP_ARRAY2D_SET: {
                Value val = pop();
                Value colVal = pop();
                Value rowVal = pop();
                Value mat = pop();
                if (!mat.isArray()) throw AlgoError("L'expression n'est pas une matrice", curLine);
                int64_t r = rowVal.asInt();
                if (r < 0 || (size_t)r >= mat.arrVal.size()) throw AlgoError("Indice ligne hors bornes", curLine);
                Value row = mat.arrVal[r];
                int64_t c = colVal.asInt();
                if (!row.isArray()) throw AlgoError("Ligne n'est pas un tableau", curLine);
                if (c < 0 || (size_t)c >= row.arrVal.size()) throw AlgoError("Indice colonne hors bornes", curLine);
                row.arrVal[c] = val;
                mat.arrVal[r] = row;
                push(mat);
                break;
            }

            case OpCode::OP_RECORD_GET: {
                uint32_t fIdx = frame->chunk->readUint32(frame->ip);
                const std::string& fieldName = frame->chunk->names[fIdx];
                Value obj = pop();
                if (!obj.isRecord()) throw AlgoError("Accès à un champ sur un non-enregistrement", curLine);
                auto it = obj.recVal.find(fieldName);
                if (it != obj.recVal.end()) {
                    push(it->second);
                } else {
                    std::string norm = normalize(fieldName);
                    bool found = false;
                    for (const auto& kv : obj.recVal) {
                        if (normalize(kv.first) == norm) {
                            push(kv.second);
                            found = true;
                            break;
                        }
                    }
                    if (!found) throw AlgoError("Champ '" + fieldName + "' introuvable", curLine);
                }
                break;
            }

            case OpCode::OP_RECORD_SET: {
                uint32_t fIdx = frame->chunk->readUint32(frame->ip);
                const std::string& fieldName = frame->chunk->names[fIdx];
                Value val = pop();
                Value obj = pop();
                if (!obj.isRecord()) throw AlgoError("Accès à un champ sur un non-enregistrement", curLine);
                obj.recVal[fieldName] = val;
                push(obj);
                break;
            }

            case OpCode::OP_MAKE_SET: {
                uint32_t count = frame->chunk->readUint32(frame->ip);
                std::vector<Value> elements(count);
                for (int i = static_cast<int>(count) - 1; i >= 0; --i) {
                    elements[i] = pop();
                }
                push(Value(std::move(elements)));
                break;
            }

            case OpCode::OP_WRITE: {
                uint32_t argCount = frame->chunk->readUint32(frame->ip);
                uint8_t hasNl = frame->chunk->readByte(frame->ip);
                uint32_t sepIdx = frame->chunk->readUint32(frame->ip);
                uint32_t finIdx = frame->chunk->readUint32(frame->ip);
                const std::string& sep = frame->chunk->names[sepIdx];
                const std::string& fin = frame->chunk->names[finIdx];

                std::vector<Value> args(argCount);
                for (int i = static_cast<int>(argCount) - 1; i >= 0; --i) {
                    args[i] = pop();
                }

                bool isFileWrite = false;
                std::string fileLogicalName;
                if (!args.empty() && args[0].isFile()) {
                    isFileWrite = true;
                    fileLogicalName = normalize(args[0].fileLogicalName);
                }

                if (isFileWrite) {
                    auto it = openFiles_.find(fileLogicalName);
                    if (it == openFiles_.end() || !it->second->isOpen) {
                        throw AlgoError("Fichier '" + fileLogicalName + "' non ouvert", curLine);
                    }
                    auto& f = *(it->second);
                    if (f.isText) {
                        for (size_t i = 1; i < args.size(); ++i) {
                            f.stream << args[i].asString();
                        }
                        if (hasNl) f.stream << "\n";
                        f.stream.flush();
                    } else {
                        for (size_t i = 1; i < args.size(); ++i) {
                            writeFileData(f, args[i]);
                        }
                        f.stream.flush();
                    }
                } else {
                    std::string lineStr;
                    for (size_t i = 0; i < args.size(); ++i) {
                        if (args[i].isBool()) {
                            lineStr += args[i].asBool() ? "Vrai" : "Faux";
                        } else {
                            lineStr += args[i].asString();
                        }
                        if (i + 1 < args.size()) {
                            lineStr += sep;
                        }
                    }
                    if (hasNl) lineStr += "\n";
                    else lineStr += fin;
                    addOutput(lineStr);
                }
                break;
            }

            case OpCode::OP_READ_VAR: {
                uint32_t nameIdx = frame->chunk->readUint32(frame->ip);
                const std::string& name = frame->chunk->names[nameIdx];
                std::string norm = normalize(name);
                Value currentVal = globals_[norm];
                std::string inputStr = requestInput("Saisir " + name + ":");
                globals_[norm] = convertInput(inputStr, currentVal);
                break;
            }

            case OpCode::OP_READ_LOCAL: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                Value& target = stack_[frame->stackBase + slot];
                std::string inputStr = requestInput("Saisir une valeur:");
                target = convertInput(inputStr, target);
                break;
            }

            case OpCode::OP_READ_REF: {
                uint32_t slot = frame->chunk->readUint32(frame->ip);
                Value* targetPtr = (slot < frame->refSlots.size() && frame->refSlots[slot].ptr != nullptr)
                    ? frame->refSlots[slot].ptr : &stack_[frame->stackBase + slot];
                std::string inputStr = requestInput("Saisir une valeur:");
                *targetPtr = convertInput(inputStr, *targetPtr);
                break;
            }

            case OpCode::OP_READ_ARRAY: {
                uint32_t nameIdx = frame->chunk->readUint32(frame->ip);
                const std::string& name = frame->chunk->names[nameIdx];
                int64_t idx = pop().asInt();
                std::string norm = normalize(name);
                Value& arr = globals_[norm];
                if (!arr.isArray() || idx < 0 || (size_t)idx >= arr.arrVal.size()) {
                    throw AlgoError("Indice hors bornes pour la lecture", curLine);
                }
                std::string inputStr = requestInput("Saisir " + name + "[" + std::to_string(idx) + "]:");
                arr.arrVal[idx] = convertInput(inputStr, arr.arrVal[idx]);
                break;
            }

            case OpCode::OP_READ_ARRAY2D: {
                uint32_t nameIdx = frame->chunk->readUint32(frame->ip);
                const std::string& name = frame->chunk->names[nameIdx];
                int64_t c = pop().asInt();
                int64_t r = pop().asInt();
                std::string norm = normalize(name);
                Value& mat = globals_[norm];
                if (!mat.isArray() || r < 0 || (size_t)r >= mat.arrVal.size()) {
                    throw AlgoError("Indice ligne hors bornes", curLine);
                }
                Value& row = mat.arrVal[r];
                if (!row.isArray() || c < 0 || (size_t)c >= row.arrVal.size()) {
                    throw AlgoError("Indice colonne hors bornes", curLine);
                }
                std::string inputStr = requestInput("Saisir " + name + "[" + std::to_string(r) + "," + std::to_string(c) + "]:");
                row.arrVal[c] = convertInput(inputStr, row.arrVal[c]);
                break;
            }

            case OpCode::OP_READ_RECORD: {
                uint32_t nameIdx = frame->chunk->readUint32(frame->ip);
                uint32_t fIdx = frame->chunk->readUint32(frame->ip);
                const std::string& name = frame->chunk->names[nameIdx];
                const std::string& member = frame->chunk->names[fIdx];
                std::string norm = normalize(name);
                Value& obj = globals_[norm];
                if (!obj.isRecord()) throw AlgoError("L'objet n'est pas un enregistrement", curLine);
                std::string inputStr = requestInput("Saisir " + name + "." + member + ":");
                obj.recVal[member] = convertInput(inputStr, obj.recVal[member]);
                break;
            }

            case OpCode::OP_READ_FILE: {
                uint32_t fIdx = frame->chunk->readUint32(frame->ip);
                const std::string& fVar = frame->chunk->names[fIdx];
                std::string fNorm = normalize(fVar);
                auto it = openFiles_.find(fNorm);
                if (it == openFiles_.end() || !it->second->isOpen) {
                    throw AlgoError("Fichier '" + fVar + "' non ouvert", curLine);
                }
                auto& f = *(it->second);
                if (f.isText) {
                    std::ostringstream ss;
                    ss << f.stream.rdbuf();
                    push(Value(ss.str()));
                } else {
                    Value val = readFileData(f);
                    f.currentRecordIndex++;
                    push(val);
                }
                break;
            }

            case OpCode::OP_READ_LVALUE: {
                Value refVal = pop();
                if (!refVal.isRef() || !refVal.refPtr) {
                    throw AlgoError("Référence invalide pour la lecture", curLine);
                }
                std::string inputStr = requestInput("Saisir une valeur:");
                *(refVal.refPtr) = convertInput(inputStr, *(refVal.refPtr));
                break;
            }

            case OpCode::OP_CALL: {
                uint32_t nameIdx = frame->chunk->readUint32(frame->ip);
                uint32_t arity = frame->chunk->readUint32(frame->ip);
                const std::string& subName = frame->chunk->names[nameIdx];
                std::string norm = normalize(subName);

                auto it = subroutines_.find(norm);
                if (it == subroutines_.end()) {
                    throw AlgoError("Sous-programme '" + subName + "' non défini", curLine);
                }

                const BytecodeFunction& func = it->second;
                size_t argBase = stack_.size() - arity;

                VMCallFrame newFrame;
                newFrame.function = &func;
                newFrame.chunk = &func.chunk;
                newFrame.ip = 0;
                newFrame.stackBase = argBase;

                // Setup reference parameters
                newFrame.refSlots.resize(func.params.size());
                for (size_t p = 0; p < func.params.size(); ++p) {
                    if (func.params[p].byRef) {
                        Value& argVal = stack_[argBase + p];
                        if (argVal.isRef()) {
                            newFrame.refSlots[p].ptr = argVal.refPtr;
                            argVal = *(argVal.refPtr);
                        } else {
                            newFrame.refSlots[p].ptr = &stack_[argBase + p];
                        }
                    }
                }

                // Allocate default local variables beyond params
                for (size_t l = func.params.size(); l < func.chunk.lines.size(); ++l) {
                    // Locals will be pushed as default values
                }
                for (const auto& vd : func.localVars) {
                    std::string vtNorm = normalize(vd.varType);
                    if (vtNorm == "procedure" || vtNorm == "fonction") continue;
                    for (const auto& vName : vd.names) {
                        bool isP = false;
                        for (const auto& p : func.params) {
                            if (normalize(p.name) == normalize(vName)) { isP = true; break; }
                        }
                        if (isP) continue;
                        Value defVal;
                        if (vd.typeSpec.kind != TypeKind::Simple || !vd.typeSpec.name.empty()) {
                            defVal = getDefaultValue(vd.typeSpec, vName);
                        } else {
                            defVal = getDefaultValue(vd.varType, vName);
                        }
                        push(std::move(defVal));
                    }
                }

                newFrame.globalSlots.assign(func.chunk.names.size(), nullptr);
                frames_.push_back(std::move(newFrame));
                frame = &frames_.back();
                break;
            }

            case OpCode::OP_CALL_BUILTIN: {
                uint32_t bId = frame->chunk->readUint32(frame->ip);
                uint32_t arity = frame->chunk->readUint32(frame->ip);
                executeBuiltin(bId, arity, curLine);
                break;
            }

            case OpCode::OP_RETURN: {
                Value retVal = pop();
                size_t base = frame->stackBase;
                frames_.pop_back();
                if (frames_.empty()) return;
                frame = &frames_.back();
                stack_.resize(base);
                push(std::move(retVal));
                break;
            }

            case OpCode::OP_RETURN_VOID: {
                size_t base = frame->stackBase;
                frames_.pop_back();
                if (frames_.empty()) return;
                frame = &frames_.back();
                stack_.resize(base);
                push(Value());
                break;
            }

            case OpCode::OP_HALT:
                return;
        }
    }
}

} // namespace algo
