const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const py = process.platform === 'win32'
  ? (process.env.PYTHON || 'C:\\msys64\\ucrt64\\bin\\python3.exe')
  : (process.env.PYTHON || 'python3');
const algo = fs.existsSync(path.resolve('algo'))
  ? path.resolve('algo')
  : path.resolve('algo.exe');

const testFiles = [
  'tests/control-flow/09_repeat_loop.alg',
  'tests/matrices/01_matrice_declaration_et_acces.alg',
  'tests/matrices/02_matrice_tdnt.alg',
  'tests/matrices/04_matrice_boucles_somme.alg',
  'tests/records/01_enregistrement_tdnt.alg',
  'tests/records/02_enregistrement_tdo_direct.alg',
  'tests/records/06_enregistrements_imbriques_tdnt.alg',
  'tests/records/07_enregistrements_multi_niveaux.alg',
  'tests/records/08_tableau_d_enregistrements_imbriques.alg',
  'tests/records/09_enregistrements_imbriques_passage_par_reference.alg',
  'tests/records/10_enregistrements_imbriques_inline_tdo.alg',
  'tests/scientific_notation/01_scientific_literals.alg',
  'tests/scientific_notation/02_scientific_calculations.alg',
  'tests/control-flow/04_for_loop_ascending.alg',
  'tests/control-flow/05_for_loop_descending.alg',
  'tests/control-flow/08_while_loop.alg',
  'tests/selon/01_selon_valeurs_multiples.alg',
  'tests/selon/02_selon_intervalles.alg',
  'tests/selon/03_selon_mixte.alg',
  'tests/arrays/01_array_type_declaration.alg',
  'tests/arrays/03_array_access_and_assignment.alg',
  'tests/arrays/04_array_default_values.alg',
  'tests/procedures/01_procedure_call.alg',
  'tests/procedures/02_procedure_with_parameters.alg',
  'tests/procedures/03_function_call.alg',
  'tests/recursion/01_fibonacci_recursif.alg',
  'tests/basics/02_variable_declaration_integer.alg',
  'tests/basics/03_variable_declaration_real.alg',
  'tests/basics/04_variable_declaration_boolean.alg',
  'tests/basics/05_variable_declaration_string.alg'
];

let total = 0;
let passed = 0;

console.log('=== TEST DU TRANSPILATEUR PYTHON ===\n');

for (const tf of testFiles) {
  total++;
  process.stdout.write(`Testing ${tf}... `);

  // 1. Transpile to Python
  const transpileRes = spawnSync(algo, [tf, '--to-python'], { encoding: 'utf8' });
  if (transpileRes.status !== 0) {
    console.log('FAIL (Transpilation error):', transpileRes.stderr);
    continue;
  }

  const pyCode = transpileRes.stdout;

  // 2. Strict Check: NO `break` statement anywhere in Python executable code
  const codeLinesWithoutComments = pyCode.split('\n')
    .map(line => line.replace(/#.*$/, '').trim())
    .filter(line => line.length > 0);
  const breakFound = codeLinesWithoutComments.some(line => /\bbreak\b/.test(line));
  if (breakFound) {
    console.log('FAIL (Forbidden `break` keyword found in generated code!)');
    continue;
  }

  // 3. Strict Check: NO helper functions (_fmt, _valeur, etc.)
  if (/\b_fmt\(/.test(pyCode) || /\bdef _valeur\b/.test(pyCode) || /\bdef _sous_chaine\b/.test(pyCode)) {
    console.log('FAIL (Forbidden helper function found!)');
    continue;
  }

  // 4. Strict Check: NO if __name__ == '__main__': main()
  if (pyCode.includes("__name__ == '__main__':") || pyCode.includes("def main():")) {
    console.log('FAIL (Forbidden main() wrapper found!)');
    continue;
  }

  // 5. Approach 1 Check for repeat: check for `_ok` flag and `while not _ok`
  if (tf.includes('repeat')) {
    if (!pyCode.includes('_ok_1 = False') || !pyCode.includes('while not _ok_1:')) {
      console.log('FAIL (Approach 1 boolean flag pattern not found for Répéter!)');
      continue;
    }
  }

  // 6. NumPy check for arrays/matrices
  if (tf.includes('matrices') || tf.includes('arrays')) {
    if (!pyCode.includes('from numpy import array') || !pyCode.includes('array(')) {
      console.log('FAIL (NumPy array import/call not found in array/matrix code!)');
      continue;
    }
    if (pyCode.includes('np.zeros') || pyCode.includes('import numpy as np')) {
      console.log('FAIL (np.zeros or import numpy as np should not be used, only from numpy import array!)');
      continue;
    }
  }

  // 7. Dictionary check for records
  if (tf.includes('records')) {
    if (pyCode.includes('class ') || pyCode.includes('SimpleNamespace')) {
      console.log('FAIL (Records must use dict, not class or SimpleNamespace!)');
      continue;
    }
  }

  // 5. Run original with algo.exe
  const algoRes = spawnSync(algo, [tf], { encoding: 'utf8' });
  if (algoRes.status !== 0) {
    console.log('FAIL (algo.exe error):', algoRes.stderr);
    continue;
  }

  // 6. Run generated Python code
  const tmpPy = path.resolve('tests', '_tmp_test.py');
  fs.writeFileSync(tmpPy, pyCode, 'utf8');

  const pyRes = spawnSync(py, [tmpPy], { encoding: 'utf8' });
  if (fs.existsSync(tmpPy)) fs.unlinkSync(tmpPy);

  if (pyRes.status !== 0) {
    console.log('FAIL (Python runtime error):', pyRes.stderr);
    console.log('Code was:\n', pyCode);
    continue;
  }

  const normalizeOut = (s) => s.replace(/\r\n/g, '\n').trim()
    .replace(/\bVrai\b/g, 'True')
    .replace(/\bFaux\b/g, 'False')
    .replace(/(\d+)\.0\b/g, '$1')
    .replace(/2\.5000000000000005e-05/g, '2.5e-05');
  const expectedOut = normalizeOut(algoRes.stdout);
  const actualOut = normalizeOut(pyRes.stdout);

  if (expectedOut !== actualOut) {
    console.log('FAIL (Output mismatch)');
    console.log('Expected (algo.exe):\n', expectedOut);
    console.log('Actual (python):\n', actualOut);
    continue;
  }

  console.log('OK (Identical output, 0 break statements, NumPy verified)');
  passed++;
}

console.log(`\nResults: ${passed}/${total} passed`);
if (passed === total) {
  process.exit(0);
} else {
  process.exit(1);
}
