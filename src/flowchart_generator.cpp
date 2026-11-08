#include "../include/flowchart_generator.hpp"
#include <iomanip>
#include <regex>

namespace algo {

FlowchartGenerator::FlowchartGenerator(const Program& prog)
    : prog_(prog) {}

std::string FlowchartGenerator::allocNodeId(const std::string& prefix) const {
    return prefix + "_" + std::to_string(nextNodeId_++);
}

std::string FlowchartGenerator::escapeLabel(const std::string& text) const {
    std::string res;
    res.reserve(text.size() * 2);
    for (char c : text) {
        if (c == '"') {
            res += "#quot;";
        } else if (c == '<') {
            res += "&lt;";
        } else if (c == '>') {
            res += "&gt;";
        } else if (c == '\n') {
            res += " ";
        } else if (c == '{' || c == '}' || c == '[' || c == ']' || c == '(' || c == ')') {
            res += c;
        } else {
            res += c;
        }
    }
    return res;
}

std::string FlowchartGenerator::exprToString(const std::shared_ptr<Expr>& expr) const {
    if (!expr) return "";
    switch (expr->kind) {
        case ExprKind::Number:
            return std::to_string(static_cast<NumberExpr*>(expr.get())->value);
        case ExprKind::Real: {
            double v = static_cast<RealExpr*>(expr.get())->value;
            std::ostringstream ss;
            ss << v;
            std::string s = ss.str();
            if (s.find('.') == std::string::npos && s.find('e') == std::string::npos && s.find('E') == std::string::npos) {
                s += ".0";
            }
            return s;
        }
        case ExprKind::String:
            return "\"" + static_cast<StringExpr*>(expr.get())->value + "\"";
        case ExprKind::Bool:
            return static_cast<BoolExpr*>(expr.get())->value ? "Vrai" : "Faux";
        case ExprKind::Variable:
            return static_cast<VariableExpr*>(expr.get())->name;
        case ExprKind::ArrayAccess: {
            auto* ae = static_cast<ArrayAccessExpr*>(expr.get());
            std::string base = ae->target ? exprToString(ae->target) : ae->name;
            std::string res = base + "[" + exprToString(ae->index);
            if (ae->index2) res += ", " + exprToString(ae->index2);
            res += "]";
            return res;
        }
        case ExprKind::MemberAccess: {
            auto* me = static_cast<MemberAccessExpr*>(expr.get());
            return exprToString(me->object) + "." + me->member;
        }
        case ExprKind::Unary: {
            auto* ue = static_cast<UnaryExpr*>(expr.get());
            return ue->op + " " + exprToString(ue->operand);
        }
        case ExprKind::Binary: {
            auto* be = static_cast<BinaryExpr*>(expr.get());
            return "(" + exprToString(be->left) + " " + be->op + " " + exprToString(be->right) + ")";
        }
        case ExprKind::Call: {
            auto* ce = static_cast<CallExpr*>(expr.get());
            std::string res = ce->name + "(";
            for (size_t i = 0; i < ce->args.size(); ++i) {
                if (i > 0) res += ", ";
                res += exprToString(ce->args[i]);
            }
            res += ")";
            return res;
        }
        case ExprKind::Deref: {
            auto* de = static_cast<DerefExpr*>(expr.get());
            return exprToString(de->target) + "^";
        }
        case ExprKind::Nil:
            return "NIL";
        case ExprKind::SetLiteral: {
            auto* se = static_cast<SetLiteralExpr*>(expr.get());
            std::string res = "[";
            for (size_t i = 0; i < se->elements.size(); ++i) {
                if (i > 0) res += ", ";
                res += exprToString(se->elements[i]);
            }
            res += "]";
            return res;
        }
        default:
            return "";
    }
}

FlowchartGenerator::FlowNode FlowchartGenerator::generateStmt(std::ostringstream& oss,
                                                             const std::shared_ptr<Stmt>& stmt,
                                                             const std::string& prefix) const {
    if (!stmt) return {};

    switch (stmt->kind) {
        case StmtKind::Assign: {
            auto* s = static_cast<AssignStmt*>(stmt.get());
            std::string id = allocNodeId(prefix + "_assign");
            std::string label = escapeLabel(s->target + " ← " + exprToString(s->value));
            oss << "        " << id << "[\"" << label << "\"]\n";
            return {id, {id}};
        }
        case StmtKind::ArrayAssign: {
            auto* s = static_cast<ArrayAssignStmt*>(stmt.get());
            std::string id = allocNodeId(prefix + "_arr_assign");
            std::string base = s->targetExpr ? exprToString(s->targetExpr) : s->target;
            std::string idxs = exprToString(s->index);
            if (s->index2) idxs += ", " + exprToString(s->index2);
            std::string label = escapeLabel(base + "[" + idxs + "] ← " + exprToString(s->value));
            oss << "        " << id << "[\"" << label << "\"]\n";
            return {id, {id}};
        }
        case StmtKind::MemberAssign: {
            auto* s = static_cast<MemberAssignStmt*>(stmt.get());
            std::string id = allocNodeId(prefix + "_mem_assign");
            std::string base = exprToString(s->target);
            std::string label = escapeLabel(base + "." + s->member + " ← " + exprToString(s->value));
            oss << "        " << id << "[\"" << label << "\"]\n";
            return {id, {id}};
        }
        case StmtKind::DerefAssign: {
            auto* s = static_cast<DerefAssignStmt*>(stmt.get());
            std::string id = allocNodeId(prefix + "_deref_assign");
            std::string label = escapeLabel(exprToString(s->target) + "^ ← " + exprToString(s->value));
            oss << "        " << id << "[\"" << label << "\"]\n";
            return {id, {id}};
        }
        case StmtKind::Alloc: {
            auto* s = static_cast<AllocStmt*>(stmt.get());
            std::string id = allocNodeId(prefix + "_alloc");
            std::string label = escapeLabel("Allouer(" + exprToString(s->target) + ")");
            oss << "        " << id << "[\"" << label << "\"]\n";
            return {id, {id}};
        }
        case StmtKind::Free: {
            auto* s = static_cast<FreeStmt*>(stmt.get());
            std::string id = allocNodeId(prefix + "_free");
            std::string label = escapeLabel("Liberer(" + exprToString(s->target) + ")");
            oss << "        " << id << "[\"" << label << "\"]\n";
            return {id, {id}};
        }
        case StmtKind::Write: {
            auto* s = static_cast<WriteStmt*>(stmt.get());
            std::string id = allocNodeId(prefix + "_write");
            std::string args;
            for (size_t i = 0; i < s->args.size(); ++i) {
                if (i > 0) args += ", ";
                args += exprToString(s->args[i]);
            }
            std::string label = escapeLabel("Ecrire(" + args + ")");
            // Forme sortie Mermaid : [/label/]
            oss << "        " << id << "[/\"" << label << "\"/]\n";
            return {id, {id}};
        }
        case StmtKind::Read: {
            auto* s = static_cast<ReadStmt*>(stmt.get());
            std::string id = allocNodeId(prefix + "_read");
            std::string targets;
            for (size_t i = 0; i < s->targets.size(); ++i) {
                if (i > 0) targets += ", ";
                targets += exprToString(s->targets[i].targetExpr);
            }
            std::string label = escapeLabel("Lire(" + targets + ")");
            // Forme entrée Mermaid : [/label/]
            oss << "        " << id << "[/\"" << label << "\"/]\n";
            return {id, {id}};
        }
        case StmtKind::Call: {
            auto* s = static_cast<CallStmt*>(stmt.get());
            std::string id = allocNodeId(prefix + "_call");
            std::string args;
            for (size_t i = 0; i < s->args.size(); ++i) {
                if (i > 0) args += ", ";
                args += exprToString(s->args[i]);
            }
            std::string label = escapeLabel(s->name + "(" + args + ")");
            // Forme sous-programme Mermaid : [[label]]
            oss << "        " << id << "[[\"" << label << "\"]]\n";
            return {id, {id}};
        }
        case StmtKind::Return: {
            auto* s = static_cast<ReturnStmt*>(stmt.get());
            std::string id = allocNodeId(prefix + "_ret");
            std::string label = escapeLabel("Retourner " + exprToString(s->value));
            oss << "        " << id << "([\"" << label << "\"])\n";
            return {id, {id}};
        }
        case StmtKind::If: {
            auto* s = static_cast<IfStmt*>(stmt.get());
            std::string condId = allocNodeId(prefix + "_if_cond");
            std::string mergeId = allocNodeId(prefix + "_if_join");

            oss << "        " << condId << "{\"" << escapeLabel(exprToString(s->condition) + " ?") << "\"}\n";

            // Branche Vrai (Then)
            FlowNode thenFlow = generateBlock(oss, s->thenBlock, prefix + "_then");
            if (!thenFlow.entryId.empty()) {
                oss << "        " << condId << " -->|Vrai| " << thenFlow.entryId << "\n";
                for (const auto& exitId : thenFlow.exitIds) {
                    oss << "        " << exitId << " --> " << mergeId << "\n";
                }
            } else {
                oss << "        " << condId << " -->|Vrai| " << mergeId << "\n";
            }

            // Branche Faux (Else-If / Else)
            if (!s->elseIfBlocks.empty() || !s->elseBlock.empty()) {
                std::string prevCondId = condId;
                for (size_t i = 0; i < s->elseIfBlocks.size(); ++i) {
                    const auto& eb = s->elseIfBlocks[i];
                    std::string elifCondId = allocNodeId(prefix + "_elif_cond_" + std::to_string(i));
                    oss << "        " << elifCondId << "{\"" << escapeLabel(exprToString(eb.condition) + " ?") << "\"}\n";
                    oss << "        " << prevCondId << " -->|Faux| " << elifCondId << "\n";

                    FlowNode elifFlow = generateBlock(oss, eb.body, prefix + "_elif_" + std::to_string(i));
                    if (!elifFlow.entryId.empty()) {
                        oss << "        " << elifCondId << " -->|Vrai| " << elifFlow.entryId << "\n";
                        for (const auto& exitId : elifFlow.exitIds) {
                            oss << "        " << exitId << " --> " << mergeId << "\n";
                        }
                    } else {
                        oss << "        " << elifCondId << " -->|Vrai| " << mergeId << "\n";
                    }
                    prevCondId = elifCondId;
                }

                if (!s->elseBlock.empty()) {
                    FlowNode elseFlow = generateBlock(oss, s->elseBlock, prefix + "_else");
                    if (!elseFlow.entryId.empty()) {
                        oss << "        " << prevCondId << " -->|Faux| " << elseFlow.entryId << "\n";
                        for (const auto& exitId : elseFlow.exitIds) {
                            oss << "        " << exitId << " --> " << mergeId << "\n";
                        }
                    } else {
                        oss << "        " << prevCondId << " -->|Faux| " << mergeId << "\n";
                    }
                } else {
                    oss << "        " << prevCondId << " -->|Faux| " << mergeId << "\n";
                }
            } else {
                oss << "        " << condId << " -->|Faux| " << mergeId << "\n";
            }

            oss << "        " << mergeId << "(( ))\n";
            return {condId, {mergeId}};
        }
        case StmtKind::While: {
            auto* s = static_cast<WhileStmt*>(stmt.get());
            std::string condId = allocNodeId(prefix + "_while_cond");
            std::string exitId = allocNodeId(prefix + "_while_exit");

            oss << "        " << condId << "{\"" << escapeLabel("Tant que " + exprToString(s->condition)) << "\"}\n";

            FlowNode bodyFlow = generateBlock(oss, s->body, prefix + "_while_body");
            if (!bodyFlow.entryId.empty()) {
                oss << "        " << condId << " -->|Vrai| " << bodyFlow.entryId << "\n";
                for (const auto& ex : bodyFlow.exitIds) {
                    oss << "        " << ex << " --> " << condId << "\n";
                }
            } else {
                oss << "        " << condId << " -->|Vrai| " << condId << "\n";
            }

            oss << "        " << condId << " -->|Faux| " << exitId << "(( ))\n";
            return {condId, {exitId}};
        }
        case StmtKind::Repeat: {
            auto* s = static_cast<RepeatStmt*>(stmt.get());
            std::string condId = allocNodeId(prefix + "_repeat_cond");
            std::string exitId = allocNodeId(prefix + "_repeat_exit");

            FlowNode bodyFlow = generateBlock(oss, s->body, prefix + "_repeat_body");
            oss << "        " << condId << "{\"" << escapeLabel("Jusqu'à " + exprToString(s->condition) + " ?") << "\"}\n";

            std::string entryId = bodyFlow.entryId.empty() ? condId : bodyFlow.entryId;
            if (!bodyFlow.entryId.empty()) {
                for (const auto& ex : bodyFlow.exitIds) {
                    oss << "        " << ex << " --> " << condId << "\n";
                }
            }
            oss << "        " << condId << " -->|Faux| " << entryId << "\n";
            oss << "        " << condId << " -->|Vrai| " << exitId << "(( ))\n";
            return {entryId, {exitId}};
        }
        case StmtKind::For: {
            auto* s = static_cast<ForStmt*>(stmt.get());
            std::string headerId = allocNodeId(prefix + "_for_head");
            std::string exitId = allocNodeId(prefix + "_for_exit");

            std::string forLabel = "Pour " + s->varName + " de " + exprToString(s->start) + " à " + exprToString(s->end);
            if (s->step) {
                forLabel += " pas " + exprToString(s->step);
            }
            oss << "        " << headerId << "[\"" << escapeLabel(forLabel) << "\"]\n";

            FlowNode bodyFlow = generateBlock(oss, s->body, prefix + "_for_body");
            if (!bodyFlow.entryId.empty()) {
                oss << "        " << headerId << " -->|Itération| " << bodyFlow.entryId << "\n";
                for (const auto& ex : bodyFlow.exitIds) {
                    oss << "        " << ex << " --> " << headerId << "\n";
                }
            }
            oss << "        " << headerId << " -->|Fin de boucle| " << exitId << "(( ))\n";
            return {headerId, {exitId}};
        }
        case StmtKind::Switch: {
            auto* s = static_cast<SwitchStmt*>(stmt.get());
            std::string switchId = allocNodeId(prefix + "_switch");
            std::string exitId = allocNodeId(prefix + "_switch_exit");

            oss << "        " << switchId << "{\"" << escapeLabel("Selon " + exprToString(s->expr)) << "\"}\n";

            for (size_t i = 0; i < s->cases.size(); ++i) {
                const auto& c = s->cases[i];
                std::string caseLabel;
                if (c.isDefault) {
                    caseLabel = "Autrement";
                } else {
                    for (size_t v = 0; v < c.items.size(); ++v) {
                        if (v > 0) caseLabel += ", ";
                        if (c.items[v].isRange()) {
                            caseLabel += exprToString(c.items[v].start) + " .. " + exprToString(c.items[v].end);
                        } else {
                            caseLabel += exprToString(c.items[v].start);
                        }
                    }
                    if (caseLabel.empty()) {
                        for (size_t v = 0; v < c.values.size(); ++v) {
                            if (v > 0) caseLabel += ", ";
                            caseLabel += exprToString(c.values[v]);
                        }
                    }
                    caseLabel = "Cas " + caseLabel;
                }
                FlowNode caseFlow = generateBlock(oss, c.body, prefix + "_case_" + std::to_string(i));
                if (!caseFlow.entryId.empty()) {
                    oss << "        " << switchId << " -->|\"" << escapeLabel(caseLabel) << "\"| " << caseFlow.entryId << "\n";
                    for (const auto& ex : caseFlow.exitIds) {
                        oss << "        " << ex << " --> " << exitId << "\n";
                    }
                } else {
                    oss << "        " << switchId << " -->|\"" << escapeLabel(caseLabel) << "\"| " << exitId << "\n";
                }
            }

            oss << "        " << exitId << "(( ))\n";
            return {switchId, {exitId}};
        }
        default:
            return {};
    }
}

FlowchartGenerator::FlowNode FlowchartGenerator::generateBlock(std::ostringstream& oss,
                                                               const std::vector<std::shared_ptr<Stmt>>& stmts,
                                                               const std::string& prefix) const {
    if (stmts.empty()) return {};

    std::string firstEntry;
    std::vector<std::string> prevExits;

    for (size_t i = 0; i < stmts.size(); ++i) {
        FlowNode nodeFlow = generateStmt(oss, stmts[i], prefix + "_" + std::to_string(i));
        if (nodeFlow.entryId.empty()) continue;

        if (firstEntry.empty()) {
            firstEntry = nodeFlow.entryId;
        }

        if (!prevExits.empty()) {
            for (const auto& ex : prevExits) {
                oss << "        " << ex << " --> " << nodeFlow.entryId << "\n";
            }
        }

        prevExits = nodeFlow.exitIds;

        // Si instruction Return, fin du flux séquentiel
        if (stmts[i]->kind == StmtKind::Return) {
            break;
        }
    }

    return {firstEntry, prevExits};
}

std::string FlowchartGenerator::generateMermaid() const {
    nextNodeId_ = 1;
    std::ostringstream oss;
    oss << "flowchart TD\n";

    // Sous-programmes (Procédures)
    for (const auto& proc : prog_.procedures) {
        std::string subId = "sub_proc_" + proc.name;
        oss << "\n    subgraph " << subId << " [\"Procédure " << escapeLabel(proc.name) << "\"]\n";
        std::string startId = allocNodeId("proc_start_" + proc.name);
        std::string endId = allocNodeId("proc_end_" + proc.name);
        oss << "        " << startId << "([\"Début " << escapeLabel(proc.name) << "\"])\n";

        FlowNode bodyFlow = generateBlock(oss, proc.body, "p_" + proc.name);
        if (!bodyFlow.entryId.empty()) {
            oss << "        " << startId << " --> " << bodyFlow.entryId << "\n";
            for (const auto& ex : bodyFlow.exitIds) {
                oss << "        " << ex << " --> " << endId << "\n";
            }
        } else {
            oss << "        " << startId << " --> " << endId << "\n";
        }
        oss << "        " << endId << "([\"Fin " << escapeLabel(proc.name) << "\"])\n";
        oss << "    end\n";
    }

    // Sous-programmes (Fonctions)
    for (const auto& func : prog_.functions) {
        std::string subId = "sub_func_" + func.name;
        oss << "\n    subgraph " << subId << " [\"Fonction " << escapeLabel(func.name) << " : " << escapeLabel(func.returnType) << "\"]\n";
        std::string startId = allocNodeId("func_start_" + func.name);
        std::string endId = allocNodeId("func_end_" + func.name);
        oss << "        " << startId << "([\"Début " << escapeLabel(func.name) << "\"])\n";

        FlowNode bodyFlow = generateBlock(oss, func.body, "f_" + func.name);
        if (!bodyFlow.entryId.empty()) {
            oss << "        " << startId << " --> " << bodyFlow.entryId << "\n";
            for (const auto& ex : bodyFlow.exitIds) {
                oss << "        " << ex << " --> " << endId << "\n";
            }
        } else {
            oss << "        " << startId << " --> " << endId << "\n";
        }
        oss << "        " << endId << "([\"Fin " << escapeLabel(func.name) << "\"])\n";
        oss << "    end\n";
    }

    // Algorithme principal
    std::string mainTitle = prog_.name.empty() ? "Programme Principal" : prog_.name;
    oss << "\n    subgraph main_algo [\"Algorithme " << escapeLabel(mainTitle) << "\"]\n";
    std::string mainStartId = allocNodeId("main_start");
    std::string mainEndId = allocNodeId("main_end");
    oss << "        " << mainStartId << "([\"Début " << escapeLabel(mainTitle) << "\"])\n";

    FlowNode mainFlow = generateBlock(oss, prog_.mainBody, "main");
    if (!mainFlow.entryId.empty()) {
        oss << "        " << mainStartId << " --> " << mainFlow.entryId << "\n";
        for (const auto& ex : mainFlow.exitIds) {
            oss << "        " << ex << " --> " << mainEndId << "\n";
        }
    } else {
        oss << "        " << mainStartId << " --> " << mainEndId << "\n";
    }
    oss << "        " << mainEndId << "([\"Fin " << escapeLabel(mainTitle) << "\"])\n";
    oss << "    end\n";

    return oss.str();
}

std::string FlowchartGenerator::generateHtml(const std::string& title) const {
    std::string docTitle = title.empty() ? ("Organigramme - " + (prog_.name.empty() ? "Algorithme" : prog_.name)) : title;
    std::string mermaidCode = generateMermaid();

    std::ostringstream oss;
    oss << "<!DOCTYPE html>\n"
        << "<html lang=\"fr\">\n"
        << "<head>\n"
        << "  <meta charset=\"UTF-8\">\n"
        << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        << "  <title>" << docTitle << "</title>\n"
        << "  <script src=\"https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.min.js\"></script>\n"
        << "  <script src=\"https://cdn.jsdelivr.net/npm/svg-pan-zoom@3.6.1/dist/svg-pan-zoom.min.js\"></script>\n"
        << "  <style>\n"
        << "    :root {\n"
        << "      --bg-color: #0f172a;\n"
        << "      --panel-bg: #1e293b;\n"
        << "      --text-color: #f8fafc;\n"
        << "      --accent-color: #38bdf8;\n"
        << "      --btn-bg: #334155;\n"
        << "      --btn-hover: #475569;\n"
        << "      --border-color: #334155;\n"
        << "    }\n"
        << "    body.light {\n"
        << "      --bg-color: #f8fafc;\n"
        << "      --panel-bg: #ffffff;\n"
        << "      --text-color: #0f172a;\n"
        << "      --accent-color: #0284c7;\n"
        << "      --btn-bg: #e2e8f0;\n"
        << "      --btn-hover: #cbd5e1;\n"
        << "      --border-color: #cbd5e1;\n"
        << "    }\n"
        << "    * { box-sizing: border-box; margin: 0; padding: 0; }\n"
        << "    body {\n"
        << "      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;\n"
        << "      background-color: var(--bg-color);\n"
        << "      color: var(--text-color);\n"
        << "      display: flex;\n"
        << "      flex-direction: column;\n"
        << "      height: 100vh;\n"
        << "      overflow: hidden;\n"
        << "    }\n"
        << "    header {\n"
        << "      background: var(--panel-bg);\n"
        << "      border-bottom: 1px solid var(--border-color);\n"
        << "      padding: 12px 24px;\n"
        << "      display: flex;\n"
        << "      justify-content: space-between;\n"
        << "      align-items: center;\n"
        << "      box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);\n"
        << "    }\n"
        << "    h1 {\n"
        << "      font-size: 1.25rem;\n"
        << "      font-weight: 700;\n"
        << "      color: var(--accent-color);\n"
        << "      display: flex;\n"
        << "      align-items: center;\n"
        << "      gap: 10px;\n"
        << "    }\n"
        << "    .controls {\n"
        << "      display: flex;\n"
        << "      gap: 10px;\n"
        << "    }\n"
        << "    button {\n"
        << "      background: var(--btn-bg);\n"
        << "      color: var(--text-color);\n"
        << "      border: 1px solid var(--border-color);\n"
        << "      padding: 8px 14px;\n"
        << "      border-radius: 6px;\n"
        << "      cursor: pointer;\n"
        << "      font-size: 0.875rem;\n"
        << "      font-weight: 500;\n"
        << "      transition: all 0.2s;\n"
        << "    }\n"
        << "    button:hover {\n"
        << "      background: var(--btn-hover);\n"
        << "    }\n"
        << "    #diagram-container {\n"
        << "      flex: 1;\n"
        << "      overflow: hidden;\n"
        << "      position: relative;\n"
        << "      display: flex;\n"
        << "      justify-content: center;\n"
        << "      align-items: center;\n"
        << "      padding: 20px;\n"
        << "    }\n"
        << "    .mermaid {\n"
        << "      width: 100%;\n"
        << "      height: 100%;\n"
        << "      display: flex;\n"
        << "      justify-content: center;\n"
        << "      align-items: center;\n"
        << "    }\n"
        << "    .mermaid svg {\n"
        << "      max-width: 100%;\n"
        << "      max-height: 100%;\n"
        << "    }\n"
        << "  </style>\n"
        << "</head>\n"
        << "<body>\n"
        << "  <header>\n"
        << "    <h1>⚡ " << docTitle << "</h1>\n"
        << "    <div class=\"controls\">\n"
        << "      <button onclick=\"zoomIn()\">🔍 Zoom +</button>\n"
        << "      <button onclick=\"zoomOut()\">🔍 Zoom -</button>\n"
        << "      <button onclick=\"resetZoom()\">↺ Réinitialiser</button>\n"
        << "      <button onclick=\"toggleTheme()\">🌓 Thème</button>\n"
        << "      <button onclick=\"downloadSvg()\">💾 Télécharger SVG</button>\n"
        << "    </div>\n"
        << "  </header>\n"
        << "  <main id=\"diagram-container\">\n"
        << "    <div class=\"mermaid\" id=\"mermaid-diagram\">\n"
        << mermaidCode
        << "    </div>\n"
        << "  </main>\n"
        << "  <script>\n"
        << "    let panZoomInstance = null;\n"
        << "    function isDark() {\n"
        << "      return !document.body.classList.contains('light');\n"
        << "    }\n"
        << "    function initMermaid() {\n"
        << "      mermaid.initialize({\n"
        << "        startOnLoad: true,\n"
        << "        theme: isDark() ? 'dark' : 'default',\n"
        << "        securityLevel: 'loose',\n"
        << "        flowchart: {\n"
        << "          useMaxWidth: false,\n"
        << "          htmlLabels: true,\n"
        << "          curve: 'basis'\n"
        << "        }\n"
        << "      });\n"
        << "    }\n"
        << "    initMermaid();\n"
        << "    window.addEventListener('load', () => {\n"
        << "      setTimeout(() => {\n"
        << "        const svg = document.querySelector('#mermaid-diagram svg');\n"
        << "        if (svg && window.svgPanZoom) {\n"
        << "          panZoomInstance = svgPanZoom(svg, {\n"
        << "            zoomEnabled: true,\n"
        << "            controlIconsEnabled: false,\n"
        << "            fit: true,\n"
        << "            center: true,\n"
        << "            minZoom: 0.2,\n"
        << "            maxZoom: 10\n"
        << "          });\n"
        << "        }\n"
        << "      }, 500);\n"
        << "    });\n"
        << "    function zoomIn() { if (panZoomInstance) panZoomInstance.zoomIn(); }\n"
        << "    function zoomOut() { if (panZoomInstance) panZoomInstance.zoomOut(); }\n"
        << "    function resetZoom() { if (panZoomInstance) { panZoomInstance.reset(); panZoomInstance.fit(); panZoomInstance.center(); } }\n"
        << "    function toggleTheme() {\n"
        << "      document.body.classList.toggle('light');\n"
        << "      location.reload();\n"
        << "    }\n"
        << "    function downloadSvg() {\n"
        << "      const svg = document.querySelector('#mermaid-diagram svg');\n"
        << "      if (!svg) return;\n"
        << "      const data = new XMLSerializer().serializeToString(svg);\n"
        << "      const blob = new Blob([data], { type: 'image/svg+xml;charset=utf-8' });\n"
        << "      const url = URL.createObjectURL(blob);\n"
        << "      const a = document.createElement('a');\n"
        << "      a.href = url;\n"
        << "      a.download = '" << (prog_.name.empty() ? "organigramme" : prog_.name) << ".svg';\n"
        << "      document.body.appendChild(a);\n"
        << "      a.click();\n"
        << "      document.body.removeChild(a);\n"
        << "      URL.revokeObjectURL(url);\n"
        << "    }\n"
        << "  </script>\n"
        << "</body>\n"
        << "</html>\n";

    return oss.str();
}

std::string FlowchartGenerator::generate(FlowchartFormat format) const {
    switch (format) {
        case FlowchartFormat::Mermaid:
            return generateMermaid();
        case FlowchartFormat::Markdown:
            return "```mermaid\n" + generateMermaid() + "```\n";
        case FlowchartFormat::Html:
            return generateHtml();
    }
    return generateMermaid();
}

std::string FlowchartGenerator::toFlowchart(const Program& prog, FlowchartFormat format) {
    FlowchartGenerator gen(prog);
    return gen.generate(format);
}

} // namespace algo
