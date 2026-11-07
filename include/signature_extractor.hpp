#pragma once

#include "ast.hpp"
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <map>

namespace algo {

// Représentation de la signature d'un sous-programme (Procédure ou Fonction)
struct SubprogramSignature {
    bool isFunction = false;
    std::string name;
    std::vector<ParamDecl> params;
    std::string returnType; // vide pour une procédure
    int line = -1;

    std::string formatSignature() const;
};

// Entrée de déclaration dans une table TDO / TDOL
struct TdoEntry {
    std::vector<std::string> names; // ex: {"i", "j"}
    std::string typeOrNature;       // ex: "entier", "tab", "Constante = 10"
    bool isConst = false;
    std::string constValueStr;
};

// Informations complètes sur un sous-programme : Signature, TDNT respectif, TDOL
struct SubprogramInfo {
    SubprogramSignature signature;
    std::vector<TypeDecl> respectiveTdnt;
    std::vector<TdoEntry> tdol;
    bool isTdolGenerated = false;

    std::string formatText() const;
};

// Résultat global d'analyse pour un fichier / algorithme
struct ProgramSignaturesResult {
    std::string programName;
    std::vector<SubprogramInfo> subprograms;
    std::vector<TypeDecl> globalTdnt;
    bool isGlobalTdntGenerated = false;
    std::vector<TdoEntry> mainTdo;
    bool isMainTdoGenerated = false;

    std::string formatText() const;
    std::string formatJson() const;
};

// Extracteur et générateur de signatures et tables TDO / TDNT
class SignatureExtractor {
public:
    explicit SignatureExtractor(const Program& prog);

    ProgramSignaturesResult extract() const;

    static ProgramSignaturesResult extractFromProgram(const Program& prog);
    static ProgramSignaturesResult extractFromSource(const std::string& source, const std::string& filePath = "");

private:
    const Program& prog_;

    std::vector<TypeDecl> inferMissingTypeDecls(
        std::map<std::string, std::string>& outVarToCustomType) const;

    std::vector<TdoEntry> extractOrInferTdo(
        const std::vector<std::shared_ptr<Stmt>>& body,
        const std::vector<ParamDecl>& params,
        const std::vector<VarDecl>& declaredVars,
        const std::vector<ConstDecl>& declaredConsts,
        bool isSubprogram,
        bool& outGenerated,
        const std::map<std::string, std::string>& varToCustomType,
        const std::vector<TypeDecl>& allTdnt) const;

    std::vector<TypeDecl> extractRespectiveTdnt(
        const std::vector<ParamDecl>& params,
        const std::string& returnType,
        const std::vector<TdoEntry>& tdol,
        const std::vector<TypeDecl>& allTdnt) const;

    std::string inferExprType(
        const std::shared_ptr<Expr>& expr,
        const std::map<std::string, std::string>& knownTypes,
        const std::vector<TypeDecl>& allTdnt) const;
};

} // namespace algo
