# Runs before every build (see platformio.ini). Draws the MoonPixel app icon and writes
# src/web_icons.h with:
#   ICON_PNG      180x180 PNG for the iPhone/Android home screen (iOS ignores SVG there)
#   ICON_BIG_PNG  256x256 PNG for the Mac Dock web app (Safari "Add to Dock") and the manifest.
#                 Full colour: dropping bits shows as rings in the glow at this size.
#   ICON_SVG      small vector favicon for browser tabs
#   MANIFEST_JSON web-app manifest (opens full-screen when added to the home screen)
# Pure Python (no image libraries needed). Edit the design below and rebuild.
import hashlib, math, pathlib, struct, zlib

try:
    Import("env")  # noqa: F821  (PlatformIO/SCons)
    root = pathlib.Path(env["PROJECT_DIR"])  # noqa: F821
except Exception:
    root = pathlib.Path(__file__).resolve().parent.parent

BG_IN, BG_OUT = (27, 33, 51), (10, 13, 20)       # night sky, lighter in the middle
GLOW = (255, 210, 122)                           # warm halo, same accent as the page
MOON_DARK, MOON_LIGHT = (196, 150, 72), (255, 247, 222)
CRATER = (165, 124, 58)
CRATERS = [(-0.30, -0.18, 0.17), (0.24, 0.06, 0.23), (-0.04, 0.38, 0.12),
           (0.30, -0.34, 0.09), (-0.42, 0.22, 0.08)]   # (x, y, radius) in moon radii


def mix(a, b, t):
    return tuple(a[i] + (b[i] - a[i]) * t for i in range(3))


def clamp(x, lo=0.0, hi=1.0):
    return lo if x < lo else hi if x > hi else x


def shade(x, y, S):
    c = S / 2
    r = S * 0.31
    dx, dy = x - c, y - c
    d = math.hypot(dx, dy)
    col = mix(BG_IN, BG_OUT, clamp(d / (S * 0.75)))
    if d > r - 3:                                               # soft halo outside the disc
        h = math.exp(-((d - r) / (S * 0.12)) ** 2)
        col = mix(col, GLOW, 0.42 * h)
    a = clamp(r - d + 0.5)                                      # anti-aliased disc edge
    if a > 0:
        lit = clamp(1 - math.hypot(dx / r + 0.38, dy / r + 0.38) / 1.65)   # light from top-left
        m = mix(MOON_DARK, MOON_LIGHT, lit ** 0.8)
        for ux, uy, ur in CRATERS:
            cd = math.hypot(dx / r - ux, dy / r - uy)
            if cd < ur:
                m = mix(m, CRATER, 0.24 * clamp((ur - cd) / (ur * 0.4)))
        col = mix(col, m, a)
    return col


def render(S, mask):
    rows = []
    for y in range(S):
        row = bytearray()
        for x in range(S):
            acc = [0.0, 0.0, 0.0]
            for sy in (0.25, 0.75):                              # 2x2 supersampling
                for sx in (0.25, 0.75):
                    p = shade(x + sx, y + sy, S)
                    acc = [acc[i] + p[i] / 4 for i in range(3)]
            row += bytes(int(round(v)) & mask for v in acc)      # mask drops low bits (small icon only)
        rows.append(bytes(row))
    return rows


def png(rows):
    def paeth(a, b, c):
        p = a + b - c
        pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
        return a if pa <= pb and pa <= pc else b if pb <= pc else c

    raw, prev, bpp = bytearray(), bytes(len(rows[0])), 3
    for row in rows:                                             # pick the best filter per row
        cands = []
        for ft in range(5):
            out = bytearray()
            for i, v in enumerate(row):
                a = row[i - bpp] if i >= bpp else 0
                b = prev[i]
                c = prev[i - bpp] if i >= bpp else 0
                pred = (0, a, b, (a + b) // 2, paeth(a, b, c))[ft]
                out.append((v - pred) & 0xFF)
            cands.append((sum(x if x < 128 else 256 - x for x in out), ft, out))
        _, ft, out = min(cands)
        raw += bytes([ft]) + out
        prev = row

    def chunk(t, data):
        return struct.pack(">I", len(data)) + t + data + struct.pack(">I", zlib.crc32(t + data) & 0xFFFFFFFF)

    ihdr = struct.pack(">IIBBBBB", len(rows[0]) // 3, len(rows), 8, 2, 0, 0, 0)
    return b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", zlib.compress(bytes(raw), 9)) + chunk(b"IEND", b"")


SVG = ('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64"><defs>'
       '<radialGradient id="b" r=".75"><stop offset="0" stop-color="#1b2133"/><stop offset="1" stop-color="#0a0d14"/></radialGradient>'
       '<radialGradient id="g" r=".5"><stop offset=".6" stop-color="#ffd27a" stop-opacity=".5"/><stop offset="1" stop-color="#ffd27a" stop-opacity="0"/></radialGradient>'
       '<radialGradient id="m" cx=".33" cy=".33" r=".8"><stop offset="0" stop-color="#fff7de"/><stop offset=".55" stop-color="#f1d28a"/><stop offset="1" stop-color="#c49648"/></radialGradient>'
       '</defs><rect width="64" height="64" rx="14" fill="url(#b)"/><circle cx="32" cy="32" r="31" fill="url(#g)"/>'
       '<circle cx="32" cy="32" r="20" fill="url(#m)"/><g fill="#a57c3a" opacity=".28">'
       '<circle cx="26" cy="28.4" r="3.4"/><circle cx="36.8" cy="33.2" r="4.6"/><circle cx="31.2" cy="39.6" r="2.4"/></g></svg>')

MANIFEST = ('{"name":"MoonPixel","short_name":"MoonPixel","start_url":"/","display":"standalone",'
            '"background_color":"#0d0f14","theme_color":"#0d0f14","icons":['
            '{"src":"/apple-touch-icon.png","sizes":"180x180","type":"image/png","purpose":"any"},'
            '{"src":"/icon-256.png","sizes":"256x256","type":"image/png","purpose":"any"},'
            '{"src":"/icon.svg","sizes":"any","type":"image/svg+xml","purpose":"any"}]}')

out_h = root / "src" / "web_icons.h"
design = hashlib.sha1((root / "tools" / "gen_icons.py").read_bytes()).hexdigest()[:12]   # no __file__ under PlatformIO
if out_h.exists() and f"design-hash: {design}" in out_h.read_text():
    print("[gen_icons] icons up to date")                 # rendering takes a few seconds: skip it
else:
    def carray(name, data):
        rows = [", ".join(f"0x{b:02x}" for b in data[i:i + 20]) for i in range(0, len(data), 20)]
        return (f"static const uint8_t {name}[] PROGMEM = {{\n  " + ",\n  ".join(rows) + "\n};\n"
                f"static const size_t {name}_LEN = {len(data)};\n")
    small = png(render(180, 0xFC))
    big = png(render(256, 0xFF))
    out_h.write_text(f"// GENERATED by tools/gen_icons.py - do not edit. design-hash: {design}\n"
                     "#pragma once\n#include <Arduino.h>\n"
                     + carray("ICON_PNG", small) + carray("ICON_BIG_PNG", big)
                     + f'static const char ICON_SVG[] PROGMEM = R"SVG({SVG})SVG";\n'
                     + f'static const char MANIFEST_JSON[] PROGMEM = R"JSON({MANIFEST})JSON";\n')
    (root / ".pio").mkdir(exist_ok=True)
    (root / ".pio" / "icon-preview.png").write_bytes(small)
    (root / ".pio" / "icon-256-preview.png").write_bytes(big)
    print(f"[gen_icons] icons: 180 px = {len(small)} bytes, 256 px = {len(big)} bytes, favicon SVG = {len(SVG)} bytes")
