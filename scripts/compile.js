const { spawnSync } = require('child_process');

const env = { ...process.env, PATH: `C:\\msys64\\ucrt64\\bin;${process.env.PATH}` };
const gxx = 'C:\\msys64\\ucrt64\\bin\\g++.exe';
const res = spawnSync(gxx, [
  '-std=c++20',
  '-O2',
  '-static',
  '-Iinclude',
  'src/lexer.cpp',
  'src/value.cpp',
  'src/parser.cpp',
  'src/interpreter.cpp',
  'src/python_transpiler.cpp',
  'src/semantic_analyzer.cpp',
  'src/main.cpp',
  '-o',
  'algo.exe'
], { encoding: 'utf8', env });

console.log('Status:', res.status);
console.log('Stderr:', res.stderr);

const res2 = spawnSync(gxx, [
  '-std=c++20',
  '-O2',
  '-static',
  '-Iinclude',
  'src/lexer.cpp',
  'src/value.cpp',
  'src/parser.cpp',
  'src/interpreter.cpp',
  'src/semantic_analyzer.cpp',
  'src/test_runner.cpp',
  '-o',
  'run_tests.exe'
], { encoding: 'utf8', env });

console.log('Status 2:', res2.status);
console.log('Stderr 2:', res2.stderr);
