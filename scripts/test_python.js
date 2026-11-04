const { spawnSync } = require('child_process');
const py = 'C:\\msys64\\ucrt64\\bin\\python3.exe';

const res = spawnSync(py, ['-c', 'import numpy as np; print("NumPy version:", np.__version__)'], { encoding: 'utf8' });
console.log('STDOUT:', res.stdout);
console.log('STDERR:', res.stderr);
console.log('Status:', res.status);
