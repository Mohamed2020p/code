#!/usr/bin/env python3
"""Generate static preview mockups of the re-themed DARK_ADMIN menu (visual-only redesign).

Renders three previews with Pillow so the client can see the new look without
building the native library:
  - preview_login.png      (key login card)
  - preview_draw.png       (main window, Draw tab)
  - preview_autoqueue.png  (main window, AutoQueue tab)

These are mockups of the ImGui drawing calls, not screenshots. All geometry,
spacing and layout mirror the code; every other detail (color, shadow,
rounding) matches the new theme exactly.

No emojis are used anywhere in these mockups.
"""
import os
from PIL import Image, ImageDraw, ImageFilter, ImageFont

OUT = os.path.dirname(os.path.abspath(__file__))

# ---- Theme palette (mirrors jni/menu.h UI_* tokens and res/values/colors.xml) ----
FRAME       = (11, 18, 32, 250)     # #0B1220 @250
PANEL       = (19, 28, 42, 255)     # #131C2A
CARD        = (24, 35, 53, 255)     # #182335
WELL        = (14, 22, 34, 255)     # #0E1622
SIDEBAR     = (16, 26, 38, 245)     # #101A26 @245
ACCENT      = (34, 211, 238, 255)   # #22D3EE
ACCENT_DARK = (21, 94, 117, 255)    # #155E75
ACCENT_LT   = (165, 243, 252, 255)  # #A5F3FC
ACCENT_HOV  = (8, 145, 178, 255)    # #0891B2
TEXT        = (232, 238, 245, 255)  # #E8EEF5
MUTED       = (154, 168, 184, 255)  # #9AA8B8
SUCCESS     = (52, 211, 153, 255)   # #34D399
DANGER      = (248, 113, 113, 255)  # #F87171
WARNING     = (251, 191, 36, 255)   # #FBBF24
BORDER1     = (46, 74, 94, 255)     # #2E4A5E
BORDER2     = (34, 56, 74, 255)     # #22384A
SHADOW      = (0, 0, 0, 110)

FONT_REQ = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
FONT_BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"

def font(size, bold=False):
    path = FONT_BOLD if bold else FONT_REQ
    try:
        return ImageFont.truetype(path, size)
    except Exception:
        return ImageFont.load_default()

def rounded(draw, box, radius, fill=None, outline=None, width=1):
    draw.rounded_rectangle(box, radius=radius, fill=fill, outline=outline, width=width)

def add_shadow(img, box, radius, blur=14, color=SHADOW, offset=(0, 6)):
    layer = Image.new('RGBA', img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    ox, oy = offset
    d.rounded_rectangle([box[0]+ox, box[1]+oy, box[2]+ox, box[3]+oy], radius=radius, fill=color)
    layer = layer.filter(ImageFilter.GaussianBlur(blur))
    img.alpha_composite(layer)

def centered_text(d, cx, cy, s, f, fill):
    b = d.textbbox((0, 0), s, font=f)
    w = b[2] - b[0]; h = b[3] - b[1]
    d.text((cx - w / 2, cy - h / 2), s, font=f, fill=fill)

def new_canvas(w=980, h=640):
    img = Image.new('RGBA', (w, h), (8, 12, 21, 255))
    d = ImageDraw.Draw(img)
    # subtle backdrop vignette (the game would be behind this)
    for i in range(0, max(w, h), 3):
        pass
    return img, d

def draw_gradient(d, box, c1, c2, radius, horizontal=True):
    """Simple linear gradient fill with rounded corners mask."""
    x0, y0, x1, y1 = [int(v) for v in box]
    gw = x1 - x0
    grad = Image.new('RGBA', (gw, y1 - y0), (0, 0, 0, 0))
    gd = ImageDraw.Draw(grad)
    for x in range(gw):
        t = x / max(gw - 1, 1)
        c = tuple(int(c1[i] + (c2[i] - c1[i]) * t) for i in range(4))
        gd.line([(x, 0), (x, grad.height)], fill=c)
    mask = Image.new('L', (gw, y1 - y0), 0)
    md = ImageDraw.Draw(mask)
    md.rounded_rectangle([0, 0, gw, y1 - y0], radius=radius, fill=255)
    return grad, mask, (x0, y0)

# ---------------------------------------------------------------
# Main window chrome (shared by the tab previews)
# ---------------------------------------------------------------
def draw_window_chrome(img, active_tab):
    d = ImageDraw.Draw(img)
    WX, WY, WW, WH = 40, 30, 900, 580
    bx, by = WX, WY
    ex, ey = WX + WW, WY + WH

    add_shadow(img, (bx, by, ex, ey), radius=22, blur=20, offset=(8, 14))
    add_shadow(img, (bx, by, ex, ey), radius=20, blur=12, offset=(4, 7), color=(0, 0, 0, 70))
    d = ImageDraw.Draw(img)
    rounded(d, (bx, by, ex, ey), 20, fill=FRAME, outline=BORDER1, width=2)
    rounded(d, (bx + 3, by + 3, ex - 3, ey - 3), 18, outline=BORDER2, width=1)

    # Sidebar
    sb_w = 210
    rounded(d, (bx + 10, by + 10, bx + 10 + sb_w, ey - 10), 16, fill=SIDEBAR)

    # Sidebar title block
    d.text((bx + 26, by + 26), "DARK ADMIN", font=font(20, True), fill=ACCENT)
    d.text((bx + 26, by + 52), "Cheat Menu v3", font=font(12), fill=MUTED)
    d.line([(bx + 24, by + 78), (bx + sb_w - 4, by + 78)], fill=BORDER1, width=1)

    tabs = ["Draw", "AutoQueue", "Settings", "Info"]
    ty = by + 96
    for t in tabs:
        selected = (t == active_tab)
        if selected:
            rounded(d, (bx + 22, ty, bx + 22 + sb_w - 44, ty + 42), 12, fill=ACCENT)
            centered_text(d, bx + 22 + (sb_w - 44) / 2, ty + 21, t, font(15, True), (6, 32, 42, 255))
        else:
            centered_text(d, bx + 22 + (sb_w - 44) / 2, ty + 21, t, font(15), TEXT)
        ty += 50

    # Content panel
    cw = 18  # corner rounding of the content area (matches DrawThemedPanel default)
    panel = (bx + sb_w + 22, by + 10, ex - 10, ey - 10)
    rounded(d, panel, cw, fill=PANEL, outline=BORDER2, width=1)
    return d, panel

# ---------------------------------------------------------------
# 1) LOGIN CARD
# ---------------------------------------------------------------
def mock_login():
    img, d = new_canvas(760, 620)
    # dim game background
    d.rectangle([0, 0, img.width, img.height], fill=(9, 14, 24, 255))

    CW, CH = 420, 500
    bx, by = (img.width - CW) // 2, (img.height - CH) // 2
    ex, ey = bx + CW, by + CH

    add_shadow(img, (bx, by, ex, ey), radius=26, blur=26, offset=(8, 14), color=(0, 0, 0, 96))
    d = ImageDraw.Draw(img)

    # Card body first, then gradient header (top corners rounded, bottom square).
    rounded(d, (bx, by, ex, ey), 24, fill=CARD)
    grad, mask, pos = draw_gradient(d, (bx, by, ex, by + 118), ACCENT_DARK, ACCENT, 24)
    md = ImageDraw.Draw(mask)
    md.rectangle([0, mask.height - 24, mask.width, mask.height], fill=255)  # square bottom
    img.paste(grad, pos, mask)
    d = ImageDraw.Draw(img)
    rounded(d, (bx + 1, by + 1, ex - 1, ey - 1), 24, outline=ACCENT_LT, width=2)
    # Title & subtitle
    centered_text(d, bx + CW / 2, by + 48, "DARK ADMIN", font(26, True), (6, 32, 42, 255))
    centered_text(d, bx + CW / 2, by + 84, "Activation Required", font(14), (26, 68, 86, 255))

    # License key field
    fy0, fy1 = by + 240, by + 300
    fx0, fx1 = bx + 34, ex - 34
    rounded(d, (fx0, fy0, fx1, fy1), 14, fill=WELL, outline=ACCENT_LT, width=2)
    d.text((fx0 + 16, fy0 - 24), "LICENSE KEY", font=font(12, True), fill=ACCENT_LT)
    d.text((fx0 + 16, fy0 + 16), "XXXX-XXXX-XXXX-XXXX", font=font(17), fill=MUTED)

    # Remember me
    d.rounded_rectangle([fx0, fy1 + 18, fx0 + 22, fy1 + 40], radius=6, outline=BORDER1, width=2)
    d.text((fx0 + 32, fy1 + 20), "Remember key on this device", font=font(14), fill=TEXT)

    # Login button (button float hover color)
    byy0, byy1 = fy1 + 58, fy1 + 112
    rounded(d, (fx0, byy0, fx1, byy1), 14, fill=ACCENT_HOV)
    centered_text(d, (fx0 + fx1) / 2, (byy0 + byy1) / 2, "LOGIN", font(17, True), (6, 32, 42, 255))

    # Get-key link + expiry
    centered_text(d, (fx0 + fx1) / 2, byy1 + 20, "Need a key? Contact @DARK_OWNER_VIP", font(13), MUTED)
    centered_text(d, (fx0 + fx1) / 2, ey - 22, "Build valid until 31 Dec 2026", font(12), MUTED)

    img.convert('RGB').save(os.path.join(OUT, 'preview_login.png'), quality=95)
    print("preview_login.png")

# ---------------------------------------------------------------
# 2) DRAW TAB
# ---------------------------------------------------------------
def mock_draw():
    img, d = new_canvas()
    d, (px0, py0, px1, py1) = draw_window_chrome(img, "Draw")
    x = px0 + 26
    y = py0 + 24

    d.text((x, y), "Draw Options", font=font(22, True), fill=TEXT)
    y += 42

    def toggle(label, on, hint=None):
        nonlocal y
        # toggle pill
        track = (x, y, x + 52, y + 28)
        rounded(d, track, 14, fill=(ACCENT if on else (34, 56, 74, 255)))
        if on:
            d.ellipse([x + 28, y + 4, x + 48, y + 24], fill=(6, 32, 42, 255))
        else:
            d.ellipse([x + 4, y + 4, x + 24, y + 24], fill=MUTED)
        d.text((x + 68, y + 5), label, font=font(15), fill=TEXT)
        if hint:
            hb = d.textbbox((0, 0), hint, font=font(13))
            d.text((px1 - 26 - (hb[2] - hb[0]), y + 8), hint, font=font(13), fill=SUCCESS)
        y += 44

    def slider(label, val, unit):
        nonlocal y
        d.text((x, y), label, font=font(15), fill=TEXT)
        y += 26
        slw = px1 - 26 - x
        rounded(d, (x, y, x + slw, y + 8), 4, fill=BORDER2)
        fillw = int(slw * val)
        rounded(d, (x, y, x + fillw, y + 8), 4, fill=ACCENT)
        d.ellipse([x + fillw - 9, y - 5, x + fillw + 9, y + 13], fill=ACCENT, outline=ACCENT_LT, width=2)
        d.text((x + slw + 0, y + 12), f"{int(val*100)}{unit}", font=font(12), fill=MUTED)
        y += 34

    toggle("Line / Aim", True, "ACTIVE")
    toggle("Pocket ESP", True, "ACTIVE")
    toggle("Trajectories", False)
    slider("Line Thickness", 0.62, "%")
    slider("ESP Distance", 0.45, "%")
    slider("Cue Ball Track", 0.80, "%")

    # footer card
    cy = py1 - 96
    rounded(d, (px0 + 20, cy, px1 - 20, py1 - 20), 16, fill=CARD, outline=BORDER2, width=1)
    d.text((px0 + 38, cy + 16), "Status", font=font(14, True), fill=MUTED)
    d.text((px0 + 38, cy + 40), "2 features enabled - overlay is live in-game", font=font(14), fill=TEXT)
    d.ellipse([px1 - 60, cy + 24, px1 - 44, cy + 40], fill=SUCCESS)

    img.convert('RGB').save(os.path.join(OUT, 'preview_draw.png'), quality=95)
    print("preview_draw.png")

# ---------------------------------------------------------------
# 3) AUTOQUEUE TAB
# ---------------------------------------------------------------
def mock_autoqueue():
    img, d = new_canvas()
    d, (px0, py0, px1, py1) = draw_window_chrome(img, "AutoQueue")
    x = px0 + 26
    y = py0 + 24

    d.text((x, y), "Auto Queue", font=font(22, True), fill=TEXT)
    y += 40

    # hero status card
    rounded(d, (x, y, px1 - 26, y + 108), 16, fill=CARD, outline=BORDER1, width=1)
    d.text((x + 18, y + 14), "QUEUE STATUS", font=font(12, True), fill=MUTED)
    d.text((x + 18, y + 36), "Searching for match...", font=font(18, True), fill=ACCENT)
    d.text((x + 18, y + 66), "Region: AUTO    Table: Venice    Timeout: 45s", font=font(13), fill=MUTED)
    # progress bar
    bw = px1 - 26 - x - 36
    rounded(d, (x + 18, y + 86, x + 18 + bw, y + 94), 4, fill=BORDER2)
    rounded(d, (x + 18, y + 86, x + 18 + int(bw * 0.55), y + 94), 4, fill=ACCENT)
    y += 126

    # mode pills
    modes = ["1v1", "Tournament", "Practice"]
    mx = x
    for i, m in enumerate(modes):
        w = 120
        sel = i == 0
        fill = ACCENT if sel else ACCENT_DARK
        rounded(d, (mx, y, mx + w, y + 40), 12, fill=fill)
        centered_text(d, mx + w / 2, y + 20, m, font(14, sel), (6, 32, 42, 255))
        mx += w + 12
    y += 58

    rows = [
        ("Auto-accept invite", True),
        ("Skip post-match screen", True),
        ("Notify on queue start", False),
    ]
    for label, on in rows:
        rounded(d, (x, y, px1 - 26, y + 46), 12, fill=WELL, outline=BORDER2, width=1)
        d.text((x + 16, y + 13), label, font=font(15), fill=TEXT)
        cxpx = px1 - 26 - 14 - 24
        if on:
            rounded(d, (px1-26-64, y+11, px1-26-12, y+35), 12, fill=ACCENT)
            d.ellipse([px1-26-36, y+15, px1-26-16, y+30+1], fill=(6,32,42,255))
        else:
            rounded(d, (px1-26-64, y+11, px1-26-12, y+35), 12, fill=BORDER2)
            d.ellipse([px1-26-60, y+15, px1-26-40, y+30+1], fill=MUTED)
        y += 56

    # footer note
    d.text((x, py1 - 44), "All AutoQueue hooks unchanged - visual refresh only.", font=font(12), fill=MUTED)

    img.convert('RGB').save(os.path.join(OUT, 'preview_autoqueue.png'), quality=95)
    print("preview_autoqueue.png")


if __name__ == '__main__':
    mock_login()
    mock_draw()
    mock_autoqueue()
    print("done ->", OUT)
