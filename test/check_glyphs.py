#!/usr/bin/env python3
"""Every character the renderer prints must be in the glyph list of the font it prints with (packages/fonts.yaml): a missing glyph shows
up as an empty rectangle on the display. Usage (from test/, after building build/dump_scenario, see dump_scenario.cpp):
    python3 check_glyphs.py scenarios/*.json"""
import re, subprocess, sys, os
import yaml

here = os.path.dirname(os.path.abspath(__file__))
class Loader(yaml.SafeLoader): pass
Loader.add_multi_constructor('!', lambda loader, suffix, node: None)
fonts = yaml.load(open(os.path.join(here, '..', 'packages', 'fonts.yaml'), encoding='utf-8'), Loader=Loader)['font']
glyphs = {f['id'][2:]: set(f['glyphs']) for f in fonts}
bad = 0
for scenario in sys.argv[1:]:
    run = subprocess.run([os.path.join(here, 'build', 'dump_scenario'), scenario], capture_output=True, text=True)
    if run.returncode == 3:
        continue  # unknown message version, nothing is drawn
    run.check_returncode()
    for line in run.stdout.splitlines():
        m = re.match(r'print -?\d+,-?\d+ (\w+) c\d+ a\d \[(.*)\]$', line)
        if not m:
            continue
        font, text = m.groups()
        if font not in glyphs:
            print(f'{scenario}: font {font} is not in packages/fonts.yaml'); bad += 1; continue
        missing = sorted(set(text) - glyphs[font])
        if missing:
            print(f'{scenario}: "{text}" in font {font} lacks {"".join(missing)!r}'); bad += 1
print('glyph check: ' + ('FAILED, %d problem(s)' % bad if bad else 'all texts covered'))
sys.exit(1 if bad else 0)
