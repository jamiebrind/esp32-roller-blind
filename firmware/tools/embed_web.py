from pathlib import Path
root=Path(__file__).resolve().parents[1]
files={'INDEX_HTML':'index.html','APP_CSS':'app.css','APP_JS':'app.js','CONTROL_JS':'control.mjs','FIRMWARE_JS':'firmware.mjs','FLEET_JS':'fleet.mjs'}
parts=['#pragma once\n']
for name,file in files.items():
    value=(root/'web'/file).read_text(encoding='utf-8')
    assert ')WEBASSET"' not in value
    parts.append(f'const char {name}[] PROGMEM = R"WEBASSET({value})WEBASSET";\n')
(root/'ScheduledBlind'/'WebAssets.h').write_text(''.join(parts),encoding='utf-8')
print('Embedded local web assets')
