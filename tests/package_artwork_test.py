#!/usr/bin/env python3
"""Exercise the actual packaging copy loop without building a release or executable."""
import ast
import hashlib
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
code = ast.parse((ROOT / 'tools/package-release.py').read_text())
main = next(n for n in code.body if isinstance(n, ast.FunctionDef) and n.name == 'main')
loop = next(n for n in main.body if isinstance(n, ast.For) and isinstance(n.iter, ast.Tuple)
            and [v.value for v in n.iter.elts] == ['config.json', 'launch.sh', 'icon.png'])
with tempfile.TemporaryDirectory(prefix='bf-artwork-test-') as td:
    base, stage = Path(td) / 'app-payload', Path(td) / 'installer'
    copied = {}
    def write(root, name, data):
        p = root / name
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_bytes(data)
    def payload(name, source, mode):
        data = source.read_bytes()
        copied[name] = (source, mode)
        write(stage, 'payload/' + name, data)
        return data
    exec(compile(ast.Module(body=[loop], type_ignores=[]), 'packaging-artwork-loop', 'exec'),
         dict(ROOT=ROOT, base=base, payload=payload, write=write))
    expected = (ROOT / 'assets/icon.png').read_bytes()
    assert expected.startswith(b'\x89PNG\r\n\x1a\n')
    assert copied['App/BetterFavorites/icon.png'] == (ROOT / 'assets/icon.png', 0o644)
    assert (base / 'App/BetterFavorites/icon.png').read_bytes() == expected
    assert (stage / 'payload/App/BetterFavorites/icon.png').read_bytes() == expected
    assert (ROOT / 'App/BetterFavorites/icon.png').read_bytes() == expected
    assert '/mnt/SDCARD/App/BetterFavorites/icon.png' in (ROOT / 'App/BetterFavorites/config.json').read_text()
    for n in ('config.json', 'launch.sh'):
        assert (base / 'App/BetterFavorites' / n).read_bytes() == (ROOT / 'App/BetterFavorites' / n).read_bytes()
    print('PASS: actual packaging loop copies exact assets/icon.png to both unchanged app destinations; config/launcher unchanged; SHA256=' + hashlib.sha256(expected).hexdigest())
