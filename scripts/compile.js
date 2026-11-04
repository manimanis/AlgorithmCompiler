const { spawnSync } = require('child_process');
const fs = require('fs');

const isWin = process.platform === 'win32';
const env = isWin
  ? { ...process.env, PATH: `C:\\msys64\\ucrt64\\bin;${process.env.PATH}` }
  : process.env;

const gxx = isWin
  ? (process.env.CXX || 'C:\\msys64\\ucrt64\\bin\\g++.exe')
  : (process.env.CXX || 'g++');

const algoOut = isWin ? 'algo.exe' : 'algo';
const testOut = isWin ? 'run_tests.exe' : 'run_tests';

const commonFlags = ['-std=c++20', '-O2', '-Wall', '-Wextra', '-Iinclude'];
if (isWin) {
  commonFlags.push('-static');
}

console.log(`Compilation avec ${gxx}...`);

const res = spawnSync(gxx, [
  ...commonFlags,
  'src/lexer.cpp',
  'src/value.cpp',
  'src/parser.cpp',
  'src/interpreter.cpp',
  'src/python_transpiler.cpp',
  'src/semantic_analyzer.cpp',
  'src/main.cpp',
  '-o',
  algoOut
], { encoding: 'utf8', env });

console.log(`[${algoOut}] Status:`, res.status);
if (res.stderr) console.error(res.stderr);

const res2 = spawnSync(gxx, [
  ...commonFlags,
  'src/lexer.cpp',
  'src/value.cpp',
  'src/parser.cpp',
  'src/interpreter.cpp',
  'src/semantic_analyzer.cpp',
  'src/test_runner.cpp',
  '-o',
  testOut
], { encoding: 'utf8', env });

console.log(`[${testOut}] Status:`, res2.status);
if (res2.stderr) console.error(res2.stderr);

// On non-Windows, also maintain algo.exe and run_tests.exe symlinks/copies for convenience
if (!isWin && res.status === 0 && res2.status === 0) {
  try {
    if (!fs.existsSync('algo.exe')) fs.copyFileSync('algo', 'algo.exe');
    if (!fs.existsSync('run_tests.exe')) fs.copyFileSync('run_tests', 'run_tests.exe');
  } catch (_) {}
}
