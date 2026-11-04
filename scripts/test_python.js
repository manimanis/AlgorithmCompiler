const { spawnSync } = require('child_process');
const py = process.platform === 'win32'
  ? (process.env.PYTHON || 'C:\\msys64\\ucrt64\\bin\\python3.exe')
  : (process.env.PYTHON || 'python3');

const res = spawnSync(py, ['-c', 'import numpy as np; print("NumPy version:", np.__version__)'], { encoding: 'utf8' });
console.log('STDOUT:', res.stdout);
console.log('STDERR:', res.stderr);
console.log('Status:', res.status);
