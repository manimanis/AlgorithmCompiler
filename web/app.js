// AlgorithmCompiler Web Studio - Application JavaScript

let algoModule = null;
let panZoomInstance = null;

const PRESETS = {
  circuit_rc: `Fonction Calcul_Tau(R: réel, C: réel): réel
Début
    Retourner R * C * 1e-9
Fin

Algorithme Etude_Circuit_RC
Début
    R ← 1000.0
    C ← 2200.0
    tau ← Calcul_Tau(R, C)
    Ecrire("Résistance R =", R, "ohms")
    Ecrire("Capacité C =", C, "nF")
    Ecrire("Constante de temps tau =", tau, "secondes")
Fin

TDO
Objet : Type/Nature
R, C, tau : réel
Calcul_Tau : Fonction`,

  tri_selection: `Procédure Tri_Selection(@t: Tableau de 5 entier)
Début
    Pour i de 0 à 3 Faire
        minIdx ← i
        Pour j de i + 1 à 4 Faire
            Si t[j] < t[minIdx] Alors
                minIdx ← j
            Fin Si
        Fin Pour
        Si minIdx <> i Alors
            tmp ← t[i]
            t[i] ← t[minIdx]
            t[minIdx] ← tmp
        Fin Si
    Fin Pour
Fin

Algorithme Demo_Tri
Début
    t[0] ← 64
    t[1] ← 25
    t[2] ← 12
    t[3] ← 22
    t[4] ← 11

    Ecrire("Tableau initial :")
    Pour i de 0 à 4 Faire
        Ecrire(t[i])
    Fin Pour

    Tri_Selection(t)

    Ecrire("Tableau trié :")
    Pour i de 0 à 4 Faire
        Ecrire(t[i])
    Fin Pour
Fin

TDO
Objet : Type/Nature
t : Tableau de 5 entier
i : entier
Tri_Selection : Procédure`,

  pile_pointeurs: `Procédure Empiler(@sommet : ^Cellule, val : Entier)
Début
    Allouer(nouveau)
    nouveau^.valeur ← val
    nouveau^.prec ← sommet
    sommet ← nouveau
Fin

TDOL
Objet : Type/Nature
nouveau : ^Cellule

Procédure Depiler(@sommet : ^Cellule, @val : Entier)
Début
    Si sommet <> NIL Alors
        val ← sommet^.valeur
        pASuppr ← sommet
        sommet ← sommet^.prec
        Liberer(pASuppr)
    Fin Si
Fin

TDOL
Objet : Type/Nature
pASuppr : ^Cellule

Algorithme Demo_Pile_Pointeurs
Début
    pile ← NIL
    Empiler(pile, 10)
    Empiler(pile, 20)
    Empiler(pile, 30)

    Tant Que pile <> NIL Faire
        Depiler(pile, element)
        Ecrire("Dépilé :", element)
    Fin Tant Que
Fin

TDNT
Cellule = Enregistrement
    valeur : Entier
    prec : ^Cellule
Fin

TDO
Objet : Type/Nature
pile : ^Cellule
element : Entier
Empiler, Depiler : Procédure`,

  etudiants: `Algorithme Gestion_Etudiants
Début
    promo[0].nom ← "Sami"
    promo[0].note ← 14.5
    promo[1].nom ← "Mouna"
    promo[1].note ← 17.0
    promo[2].nom ← "Karim"
    promo[2].note ← 12.0

    somme ← 0.0
    Pour i de 0 à 2 Faire
        Ecrire(promo[i].nom, "a obtenu", promo[i].note)
        somme ← somme + promo[i].note
    Fin Pour

    moyenne ← somme / 3.0
    Ecrire("Moyenne générale de la classe =", moyenne)
Fin

TDNT
Etudiant = Enregistrement
    nom : chaîne
    note : réel
Fin

TDO
Objet : Type/Nature
promo : Tableau de 3 Etudiant
i : entier
somme, moyenne : réel`,

  matrice: `Algorithme Demo_Matrice
Début
    Pour i de 0 à 2 Faire
        Pour j de 0 à 2 Faire
            Si i = j Alors
                M[i, j] ← 1
            Sinon
                M[i, j] ← 0
            Fin Si
        Fin Pour
    Fin Pour

    Ecrire("Matrice Identité 3x3 :")
    Pour i de 0 à 2 Faire
        Ecrire(M[i, 0], M[i, 1], M[i, 2])
    Fin Pour
Fin

TDO
Objet : Type/Nature
M : Tableau de 3, 3 de entier
i, j : entier`
};

// Initialisation au chargement de la page
window.addEventListener('DOMContentLoaded', () => {
  loadPresetExample('circuit_rc');
  initMermaid();
  loadWasm();
});

function initMermaid() {
  if (window.mermaid) {
    mermaid.initialize({
      startOnLoad: false,
      theme: 'dark',
      securityLevel: 'loose',
      flowchart: {
        useMaxWidth: false,
        htmlLabels: true,
        curve: 'basis'
      }
    });
  }
}

async function loadWasm() {
  const statusEl = document.getElementById('wasm-status');
  try {
    if (typeof createAlgoModule === 'function') {
      algoModule = await createAlgoModule();
      statusEl.innerHTML = '<span class="status-ready">● Moteur WebAssembly natif chargé (C++20 Bytecode VM actif)</span>';
      console.log("Wasm module loaded successfully");
    } else {
      statusEl.innerHTML = '<span style="color: #f59e0b;">● Mode Web Client (Wasm non compilé - Utilisez make wasm pour activer la VM Wasm)</span>';
    }
  } catch (err) {
    console.warn("Wasm load error:", err);
    statusEl.innerHTML = '<span style="color: #f59e0b;">● Mode Studio Web</span>';
  }
}

function loadPresetExample(presetKey) {
  const select = document.getElementById('example-select');
  const key = presetKey || select.value;
  if (PRESETS[key]) {
    document.getElementById('code-input').value = PRESETS[key];
    if (key === 'circuit_rc') {
      document.getElementById('stdin-input').value = '';
    }
  }
}

function switchTab(tabId) {
  document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
  document.querySelectorAll('.tab-content').forEach(content => content.classList.remove('active'));

  const tabBtn = Array.from(document.querySelectorAll('.tab-btn')).find(b => b.textContent.toLowerCase().includes(tabId));
  if (tabBtn) tabBtn.classList.add('active');

  const content = document.getElementById('tab-' + tabId);
  if (content) content.classList.add('active');

  if (tabId === 'flowchart') {
    setTimeout(setupPanZoom, 200);
  }
}

function getSourceCode() {
  return document.getElementById('code-input').value;
}

function getStdin() {
  return document.getElementById('stdin-input').value;
}

// 1. Exécution de l'algorithme via Wasm
function runCode() {
  switchTab('console');
  const outputEl = document.getElementById('console-output');
  outputEl.textContent = "Exécution en cours...\n";

  const code = getSourceCode();
  const stdin = getStdin();

  if (algoModule && algoModule._algo_run) {
    try {
      const runFn = algoModule.cwrap('algo_run', 'string', ['string', 'string', 'number', 'number']);
      const res = runFn(code, stdin, 1000000, 30000);
      outputEl.textContent = res || "[Programme terminé sans sortie]";
      outputEl.style.color = res.includes("Erreur") ? "#f87171" : "#10b981";
    } catch (e) {
      outputEl.textContent = "Erreur WebAssembly: " + e.message;
      outputEl.style.color = "#f87171";
    }
  } else {
    outputEl.textContent = "[Simulation locale]\nLe module WebAssembly (algo.wasm) n'est pas encore compilé.\nPour le compiler avec Emscripten :\n  make wasm\n\nEn attendant, vous pouvez utiliser les onglets 'Organigramme', 'Python', et 'Signatures'.";
    outputEl.style.color = "#fde047";
  }
}

// 2. Vérification sémantique
function checkCode() {
  switchTab('semantics');
  const container = document.getElementById('semantics-output');
  container.innerHTML = "Analyse sémantique en cours...";

  const code = getSourceCode();

  if (algoModule && algoModule._algo_check) {
    try {
      const checkFn = algoModule.cwrap('algo_check', 'string', ['string']);
      const rawJson = checkFn(code);
      const data = JSON.parse(rawJson);

      container.innerHTML = "";
      if (data.status === "success") {
        container.innerHTML = '<div class="diag-item" style="color: #34d399; background: rgba(52, 211, 153, 0.1);">' + data.message + '</div>';
      } else if (data.diagnostics) {
        data.diagnostics.forEach(d => {
          const div = document.createElement('div');
          div.className = 'diag-item ' + (d.isError ? 'error' : 'warning');
          div.innerHTML = `<strong>${d.isError ? 'ERREUR' : 'AVERTISSEMENT'}</strong> (Ligne ${d.line}, Col ${d.column}) : ${d.message}<br><pre style="margin-top:6px; font-size:0.8rem;">${d.formatted}</pre>`;
          container.appendChild(div);
        });
      } else {
        container.innerHTML = '<div class="diag-item error">' + (data.message || rawJson) + '</div>';
      }
    } catch (e) {
      container.innerHTML = '<div class="diag-item error">Erreur d\'analyse: ' + e.message + '</div>';
    }
  } else {
    container.innerHTML = '<div class="diag-item warning">Module Wasm requis pour l\'analyse sémantique complète (tapez `make wasm`).</div>';
  }
}

// 3. Génération d'organigramme (Mermaid)
async function generateFlowchart() {
  switchTab('flowchart');
  const code = getSourceCode();
  const renderDiv = document.getElementById('mermaid-render');

  let mermaidText = "";
  if (algoModule && algoModule._algo_flowchart) {
    try {
      const flowFn = algoModule.cwrap('algo_flowchart', 'string', ['string', 'number']);
      mermaidText = flowFn(code, 0);
    } catch (e) {
      mermaidText = "flowchart TD\n    err[\"Erreur Wasm: " + e.message + "\"]";
    }
  } else {
    // Générateur d'organigramme client de secours
    mermaidText = generateClientFlowchart(code);
  }

  try {
    renderDiv.removeAttribute('data-processed');
    renderDiv.innerHTML = mermaidText;
    await mermaid.run({ nodes: [renderDiv] });
    setupPanZoom();
  } catch (err) {
    console.error("Mermaid error:", err);
    renderDiv.innerHTML = "<pre style='color:#f87171; padding:20px;'>Erreur Mermaid:\n" + mermaidText + "</pre>";
  }
}

function generateClientFlowchart(source) {
  let lines = source.split('\n');
  let mmd = "flowchart TD\n";
  mmd += "    start([Début]) --> main_body[Bloc Principal]\n";
  let count = 0;
  for (let l of lines) {
    let tr = l.trim();
    if (tr.startsWith('Ecrire')) {
      count++;
      let safeLabel = tr.replace(/"/g, '#quot;');
      mmd += `    main_body --> out_${count}[/"${safeLabel}"/]\n`;
    } else if (tr.startsWith('Lire')) {
      count++;
      let safeLabel = tr.replace(/"/g, '#quot;');
      mmd += `    main_body --> in_${count}[/"${safeLabel}"/]\n`;
    }
  }
  mmd += `    main_body --> fin([Fin])\n`;
  return mmd;
}

function setupPanZoom() {
  const svg = document.querySelector('#mermaid-render svg');
  if (svg && window.svgPanZoom) {
    if (panZoomInstance) {
      panZoomInstance.destroy();
    }
    panZoomInstance = svgPanZoom(svg, {
      zoomEnabled: true,
      controlIconsEnabled: false,
      fit: true,
      center: true,
      minZoom: 0.1,
      maxZoom: 10
    });
  }
}

function zoomFlowchartIn() { if (panZoomInstance) panZoomInstance.zoomIn(); }
function zoomFlowchartOut() { if (panZoomInstance) panZoomInstance.zoomOut(); }
function resetFlowchartZoom() { if (panZoomInstance) { panZoomInstance.reset(); panZoomInstance.fit(); panZoomInstance.center(); } }

function exportFlowchartSvg() {
  const svg = document.querySelector('#mermaid-render svg');
  if (!svg) return;
  const data = new XMLSerializer().serializeToString(svg);
  const blob = new Blob([data], { type: 'image/svg+xml;charset=utf-8' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = 'organigramme.svg';
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}

// 4. Transpilation Python
function transpilePython() {
  switchTab('python');
  const code = getSourceCode();
  const outputEl = document.getElementById('python-output');

  if (algoModule && algoModule._algo_to_python) {
    try {
      const pyFn = algoModule.cwrap('algo_to_python', 'string', ['string']);
      const pyCode = pyFn(code);
      outputEl.textContent = pyCode || "# Aucun code généré";
    } catch (e) {
      outputEl.textContent = "# Erreur de transpilation: " + e.message;
    }
  } else {
    outputEl.textContent = "# Transpilation locale : installez les modules Wasm pour un rendu direct.";
  }
}

// 5. Signatures et TDO
function extractSignatures() {
  switchTab('signatures');
  const code = getSourceCode();
  const outputEl = document.getElementById('signatures-output');

  if (algoModule && algoModule._algo_signatures) {
    try {
      const sigFn = algoModule.cwrap('algo_signatures', 'string', ['string', 'number']);
      outputEl.textContent = sigFn(code, 0);
    } catch (e) {
      outputEl.textContent = "Erreur d'extraction: " + e.message;
    }
  }
}

function clearConsole() {
  document.getElementById('console-output').textContent = "";
}

function copyConsole() {
  navigator.clipboard.writeText(document.getElementById('console-output').textContent);
}

function copyPython() {
  navigator.clipboard.writeText(document.getElementById('python-output').textContent);
}
