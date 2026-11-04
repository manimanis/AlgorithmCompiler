const fs = require('fs');
const { execSync } = require('child_process');

const tests = [
  'tests/bac_2026_principale/01_inconnue_et_remplir',
  'tests/bac_2026_principale/02_tri_compagnies',
  'tests/bac_2026_principale/03_nombres_forts',
  'tests/bac_2026_principale/04_jeu_fidele_fibonacci_stern'
];

for (const t of tests) {
  const alg = t + '.alg';
  const inFile = t + '.in';
  const outFile = t + '.out';
  const input = fs.existsSync(inFile) ? fs.readFileSync(inFile) : '';
  const out = execSync(`.\\algo.exe "${alg}" --seed 42`, { input });
  fs.writeFileSync(outFile, out);
  console.log('Generated out for', t);
}
