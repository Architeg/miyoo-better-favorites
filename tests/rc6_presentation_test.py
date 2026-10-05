#!/usr/bin/env python3
from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'src/menu_renderer.cpp').read_text()
assert '{"Stock Favorites replacement",true}' in s and '{"Automatic return",true}' in s
assert "Enable 'Replace stock Favorites' in Settings to open Better Favorites from the Home Favorites tile." in s
assert "Enable 'Automatic return' in Settings to return to Better Favorites when you leave GameSwitcher with [B] or [START]." in s
welcome=s[s.index("const std::vector<Block> blocks=page==MenuPage::Welcome"):s.index("}:page==MenuPage::HomeInfo")]
assert "Open Apps" not in welcome and 'Home access: Available' not in s and 'Integration: Available' not in s
assert 'Home access: Not installed' in s and 'Integration: unavailable (optional patch required)' in s
print('RC6 requested wording/headings and unavailable explanations: PASS')
