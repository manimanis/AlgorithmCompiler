#pragma once

#include "ast.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <memory>

namespace algo {

enum class FlowchartFormat {
    Mermaid,        // Pure Mermaid flowchart text (flowchart TD ...)
    Markdown,       // Markdown with ```mermaid ... ```
    Html            // Standalone interactive HTML page with Mermaid.js rendering
};

class FlowchartGenerator {
public:
    explicit FlowchartGenerator(const Program& prog);

    // Génère le code de l'organigramme selon le format demandé
    std::string generate(FlowchartFormat format = FlowchartFormat::Mermaid) const;

    // Raccourci pour obtenir le code Mermaid brut
    std::string generateMermaid() const;

    // Raccourci pour obtenir une page HTML autonome interactive
    std::string generateHtml(const std::string& title = "") const;

    // Méthode statique utilitaire
    static std::string toFlowchart(const Program& prog, FlowchartFormat format = FlowchartFormat::Mermaid);

private:
    const Program& prog_;
    mutable int nextNodeId_ = 1;

    std::string allocNodeId(const std::string& prefix = "n") const;
    std::string escapeLabel(const std::string& text) const;
    std::string exprToString(const std::shared_ptr<Expr>& expr) const;

    struct FlowNode {
        std::string entryId;
        std::vector<std::string> exitIds;
    };

    FlowNode generateBlock(std::ostringstream& oss,
                           const std::vector<std::shared_ptr<Stmt>>& stmts,
                           const std::string& prefix) const;

    FlowNode generateStmt(std::ostringstream& oss,
                          const std::shared_ptr<Stmt>& stmt,
                          const std::string& prefix) const;
};

} // namespace algo
