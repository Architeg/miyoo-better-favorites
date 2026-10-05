#!/usr/bin/env python3
"""Check repository documentation paths and GitHub-style Markdown anchors offline."""
import html
import re
from collections import Counter
from pathlib import Path
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
PAGES = [ROOT / n for n in ('README.md', 'CONTRIBUTING.md', 'THIRD_PARTY_NOTICES.md', 'CHANGELOG.md')]
PAGES += sorted((ROOT / 'docs').rglob('*.md'))


def readable(text):
    return re.sub(r'```.*?```|~~~.*?~~~', '', text, flags=re.S)


def anchors(path):
    text = readable(path.read_text())
    found = set(re.findall(r'(?:id|name)=["\']([^"\']+)', text))
    counts = Counter()
    for line in text.splitlines():
        if not re.match(r'^#{1,6}\s', line):
            continue
        heading = re.sub(r'^#+\s*|\s+#+\s*$', '', line)
        heading = re.sub(r'\[([^\]]+)\]\([^)]*\)', r'\1', heading)
        heading = html.unescape(re.sub(r'<[^>]+>', '', heading)).lower()
        heading = re.sub(r'[^\w\- ]', '', heading).replace(' ', '-')
        index = counts[heading]
        counts[heading] += 1
        found.add(heading + ('-' + str(index) if index else ''))
    return found


errors = []
for p in PAGES:
    text = readable(p.read_text())
    targets = re.findall(r'\]\((<[^>]+>|[^\s)]+)', text)
    targets += re.findall(r'(?:href|src)=["\']([^"\']+)', text)
    for target in targets:
        target = html.unescape(target.strip('<>'))
        u = urlsplit(target)
        if u.scheme or u.netloc:
            continue
        q = (p.parent / unquote(u.path)).resolve() if u.path else p
        if not q.exists():
            errors.append(f'{p.relative_to(ROOT)}: missing {target}')
        elif u.fragment and q.suffix == '.md' and unquote(u.fragment) not in anchors(q):
            errors.append(f'{p.relative_to(ROOT)}: missing anchor {target}')
if errors:
    raise SystemExit('\n'.join(sorted(set(errors))))
print(f'PASS: relative targets and Markdown/HTML anchors in {len(PAGES)} current and historical documents; external URLs require separate verification')
