#!/usr/bin/env python3
"""Preview mockups of the DARK_ADMIN menu (Obsidian+cyan theme), faithful to jni/menu.h.

Renders the REAL menu UI (not a redesign):
  preview_login.png  - Admin Server login card (strings copied from DrawLogin)
  preview_draw.png   - Draw tab: Draw Lines / Draw Pockets / Line Thickness /
                       Fix Menu Size / Save Config
  preview_play.png   - Play tab: Enable AutoPlay + info card
  preview_queue.png  - Queue tab: Enable AutoQueue + stake cards (100/100M/200M)
                       + Table grid (17 tables)
  preview_user.png   - User tab: MOD INFORMATION card (uses the project's real
                       mod_*.png icons) + JOIN NOW

Menu structure mirrored from menu.h:
  - 4 top icon tabs: Draw, Play, Queue, User + close X
  - content titles: DRAW SETTINGS / AUTO PLAY / AUTO QUEUE / USER INFO
Behavior/struts untouched; this script only renders images. No emojis.
"""
import os
from PIL import Image, ImageDraw, ImageFilter, ImageFont

OUT = os.path.dirname(os.path.abspath(__file__))
PROJECT_ICONS = os.path.normpath(os.path.join(OUT, '..', 'app', 'src', 'main', 'jni', 'icons'))
if not os.path.isdir(PROJECT_ICONS):
    PROJECT_ICONS = os.path.normpath(os.path.join(OUT, '..', '..', 'DARK_ADMIN', 'app', 'src', 'main', 'jni', 'icons'))

# --- palette (mirrors jni/menu.h UI_* tokens) ---
FRAME       = (11, 18, 32, 250)     # #0B1220
PANEL       = (19, 28, 42, 255)     # #131C2A
CARD        = (24, 35, 53, 255)     # #182335
SIDEBAR     = (16, 26, 38, 245)     # #101A26
ACCENT      = (34, 211, 238, 255)   # #22D3EE
ACCENT_DARK = (21, 94, 117, 255)    # #155E75
ACCENT_LT   = (165, 243, 252, 255)  # #A5F3FC
ACCENT_HOV  = (8, 145, 178, 255)    # #0891B2
TEXT        = (232, 238, 245, 255)  # #E8EEF5
TEXT95      = (240, 250, 252, 255)  # ImGuiCol_Text on AQ table buttons
MUTED       = (154, 168, 184, 255)  # #9AA8B8
LABEL       = (154, 168, 184, 255)  # TextColored(154,168,184)
SUCCESS     = (52, 211, 153, 255)   # #34D399 license ACTIVE
DANGER      = (248, 113, 113, 255)  # #F87171 expiry
BORDER1     = (46, 74, 94, 255)     # #2E4A5E
BORDER2     = (34, 56, 74, 255)     # #22384A
INK         = (6, 32, 42, 255)      # dark ink on accent
SLIDER_BG   = (27, 40, 56, 255)     # #1B2836 frame bg

FONT_DIR = "/usr/share/fonts/truetype/dejavu/"
def font(size, bold=False):
    try:
        return ImageFont.truetype(FONT_DIR + ("DejaVuSans-Bold.ttf" if bold else "DejaVuSans.ttf"), size)
    except Exception:
        return ImageFont.load_default()

def rounded(d, box, r, fill=None, outline=None, width=1):
    d.rounded_rectangle(box, radius=r, fill=fill, outline=outline, width=width)

def shadow(img, box, r, blur=14, alpha=90, offset=(6, 10)):
    layer = Image.new('RGBA', img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    d.rounded_rectangle([box[0]+offset[0], box[1]+offset[1], box[2]+offset[0], box[3]+offset[1]], radius=r, fill=(0, 0, 0, alpha))
    img.alpha_composite(layer.filter(ImageFilter.GaussianBlur(blur)))

def ctext(d, cx, cy, s, f, fill):
    b = d.textbbox((0, 0), s, font=f)
    d.text((cx - (b[2]-b[0])/2, cy - (b[3]-b[1])/2), s, font=f, fill=fill)

def tw(d, s, f):
    b = d.textbbox((0, 0), s, font=f)
    return b[2]-b[0]

def load_icon(name, size):
    p = os.path.join(PROJECT_ICONS, name)
    if os.path.exists(p):
        return Image.open(p).convert('RGBA').resize((size, size))
    return None

# ---------------------------------------------------------------- chrome
WW, WH = 700, 560
MX, MY = 40, 26
TABS = ["Draw", "Play", "Queue", "User"]
TITLES = {"Draw": "DRAW SETTINGS", "Play": "AUTO PLAY", "Queue": "AUTO QUEUE", "User": "USER INFO"}

def glyph(d, name, cx, cy, size, ink):
    """Simple icons for the 4 tab buttons (drawn, not emoji)."""
    s = size / 2.0
    if name == "Draw":      # pencil
        d.line([(cx - s*0.5, cy + s*0.6), (cx + s*0.55, cy - s*0.5)], fill=ink, width=3)
        d.polygon([(cx - s*0.55, cy + s*0.62), (cx - s*0.35, cy + s*0.45), (cx - s*0.45, cy + s*0.75)], fill=ink)
    elif name == "Play":    # play triangle
        d.polygon([(cx - s*0.4, cy - s*0.55), (cx + s*0.6, cy), (cx - s*0.4, cy + s*0.55)], fill=ink)
    elif name == "Queue":   # circular Q
        d.ellipse([cx - s*0.55, cy - s*0.55, cx + s*0.55, cy + s*0.55], outline=ink, width=3)
        d.line([(cx + s*0.25, cy + s*0.25), (cx + s*0.6, cy + s*0.6)], fill=ink, width=3)
    else:                   # User bust
        d.ellipse([cx - s*0.35, cy - s*0.6, cx + s*0.35, cy + s*0.1], fill=ink)
        d.pieslice([cx - s*0.6, cy, cx + s*0.6, cy + s*1.2], 180, 360, fill=ink)

def window_shell(active):
    global _img
    _img = Image.new('RGBA', (WW+2*MX, WH+2*MY), (7, 11, 19, 255))
    shadow(_img, (MX, MY, MX+WW, MY+WH), 22, blur=18, alpha=90, offset=(8, 14))
    d = ImageDraw.Draw(_img)
    rounded(d, (MX, MY, MX+WW, MY+WH), 20, fill=FRAME, outline=BORDER1, width=2)
    rounded(d, (MX+3, MY+3, MX+WW-3, MY+WH-3), 18, outline=BORDER2, width=1)

    # --- top sidebar strip (menu.h DrawSidebar: icon tabs + close X) ---
    sb_x0, sb_y0, sb_x1 = MX+10, MY+10, MX+WW-10
    sb_h = 66
    rounded(d, (sb_x0, sb_y0, sb_x1, sb_y0+sb_h), 18, fill=SIDEBAR, outline=BORDER1, width=1)
    closeW = 70
    tabsW = (sb_x1 - sb_x0) - closeW
    btnW = tabsW / 4
    for i, t in enumerate(TABS):
        bx = sb_x0 + i*btnW
        sel = (t == active)
        if sel:
            rounded(d, (bx+4, sb_y0+8, bx+btnW-4, sb_y0+sb_h-8), 12, fill=ACCENT)
        ink = INK if sel else TEXT
        gly = INK if sel else ACCENT_LT
        cxi, cyi = bx + btnW/2, sb_y0 + 22
        glyph(d, t, cxi, cyi, 22, gly)
        ctext(d, cxi, sb_y0+sb_h-16, t, font(12, sel), ink)
    # separator + close X
    sepX = sb_x0 + tabsW
    d.line([(sepX, sb_y0+sb_h*0.22), (sepX, sb_y0+sb_h*0.78)], fill=BORDER1, width=1)
    cxC, cyC = sepX + closeW/2, sb_y0 + sb_h/2
    k = 9
    d.line([(cxC-k, cyC-k), (cxC+k, cyC+k)], fill=MUTED, width=3)
    d.line([(cxC+k, cyC-k), (cxC-k, cyC+k)], fill=MUTED, width=3)

    # --- content panel (DrawThemedPanel -> UI_PANEL) ---
    p_y0 = sb_y0 + sb_h + 12
    rounded(d, (MX+10, p_y0, MX+WW-10, MY+WH-10), 18, fill=PANEL, outline=BORDER2, width=1)

    # centered tab title + separator (menu.h DrawContentArea)
    ctext(d, MX+WW/2, p_y0+30, TITLES[active], font(19, True), TEXT)
    d.line([(MX+30, p_y0+56), (MX+WW-30, p_y0+56)], fill=BORDER1, width=1)
    return d, (MX+26, p_y0+66, MX+WW-26, MY+WH-16)

def toggle_row(d, x, y, label, on):
    rounded(d, (x, y, x+54, y+28), 14, fill=(ACCENT if on else BORDER2))
    if on:
        d.ellipse([x+30, y+4, x+50, y+24], fill=INK)
    else:
        d.ellipse([x+4, y+4, x+24, y+24], fill=MUTED)
    d.text((x+66, y+4), label, font=font(16), fill=TEXT)

def slider(d, x0, x1, y, label, val_norm, val_txt):
    d.text((x0, y), label, font=font(14), fill=LABEL)
    ty = y + 32
    rounded(d, (x0, ty, x1, ty+8), 4, fill=SLIDER_BG)
    fx = x0 + (x1-x0)*val_norm
    rounded(d, (x0, ty, fx, ty+8), 4, fill=ACCENT)
    d.ellipse([fx-9, ty-5, fx+9, ty+13], fill=ACCENT, outline=ACCENT_LT, width=2)
    d.text((x1+10, ty-6), val_txt, font=font(13), fill=LABEL)
    return ty + 26

# ------------------------------------------------------------- previews
def mock_draw_tab():
    d, (x0, y0, x1, y1) = window_shell("Draw")
    toggle_row(d, x0, y0+6, "Draw Lines", True)
    toggle_row(d, x0, y0+56, "Draw Pockets", True)
    y = slider(d, x0, x1-70, y0+110, "Line Thickness", 4/10.0, "4")
    y = slider(d, x0, x1-70, y+18, "Menu Scale", 0.9, "90%")
    y2 = slider(d, x0, x1-70, y+18, "Fix Menu Size", 0.5, "Normal")
    # features chip
    d.text((x0, y2+22), "Features active: 2 / 4", font=font(13), fill=LABEL)
    # Save Config
    by0, by1 = y2+46, y2+46+55
    rounded(d, (x0, by0, x1, by1), 12, fill=ACCENT_DARK)
    ctext(d, (x0+x1)/2, (by0+by1)/2, "Save Config", font(16, True), TEXT95)
    save("preview_draw.png")

def mock_play_tab():
    d, (x0, y0, x1, y1) = window_shell("Play")
    toggle_row(d, x0, y0+6, "Enable AutoPlay", True)
    # info card (menu.h: UI_CARD 112px, border + inner ACCENT_LT line)
    cy0, cy1 = y0+52, y0+52+112
    rounded(d, (x0, cy0, x1, cy1), 14, fill=CARD, outline=BORDER1, width=2)
    rounded(d, (x0+3, cy0+3, x1-3, cy1-3), 11, outline=(165, 243, 252, 60), width=1)
    # target glyph (DrawAutoPlayTarget)
    tgx, tgy, r = x0+48, cy0+56, 22
    d.ellipse([tgx-r, tgy-r, tgx+r, tgy+r], outline=ACCENT, width=2)
    d.ellipse([tgx-r*0.55, tgy-r*0.55, tgx+r*0.55, tgy+r*0.55], outline=ACCENT, width=2)
    d.ellipse([tgx-3, tgy-3, tgx+3, tgy+3], fill=ACCENT)
    d.line([(tgx-r, tgy), (tgx+r, tgy)], fill=ACCENT_LT, width=2)
    d.line([(tgx, tgy-r), (tgx, tgy+r)], fill=ACCENT_LT, width=2)
    d.text((x0+86, cy0+18), "Auto Play will automatically", font=font(15), fill=TEXT)
    d.text((x0+86, cy0+43), "aim and shoot for you", font=font(15), fill=TEXT)
    d.text((x0+86, cy0+68), "in every match.", font=font(15), fill=TEXT)
    save("preview_play.png")

def mock_queue_tab():
    d, (x0, y0, x1, y1) = window_shell("Queue")
    toggle_row(d, x0, y0+6, "Enable AutoQueue", True)
    # stake cards (menu.h: 3 cards, coin tex, labels 100 / 100M / 200M)
    gap = 7
    cw = (x1-x0-gap*2)//3
    ch = 76
    coin = load_icon("coin_100.png", 38)
    labels = ["100", "100M", "200M"]
    for i in range(3):
        cx0 = x0 + i*(cw+gap)
        rounded(d, (cx0, y0+52, cx0+cw, y0+52+ch), 9, fill=CARD, outline=BORDER2, width=2)
        rounded(d, (cx0+2, y0+54, cx0+cw-2, y0+52+ch-2), 7, outline=BORDER1, width=1)
        if coin is not None:
            _img.alpha_composite(coin, (cx0+cw//2-19, int(y0+56)))
        ctext(d, cx0+cw/2, y0+52+49, labels[i], font(12), TEXT)
    # table grid (menu.h: 17 entries, 4 cols)
    ty = y0+52+ch+10
    d.text((x0, ty), "Table", font=font(14), fill=LABEL)
    tables = ["100","200","1k","2.5k","10k","50k","100k","500k","1M","2M","5M","8M","10M","20M","50M","100M","200M"]
    gy = ty+26
    cols, g2 = 4, 6
    bw = (x1-x0-g2*(cols-1))//cols
    bh, sel = 36, 0
    for i, t in enumerate(tables):
        r_, c_ = divmod(i, cols)
        bx = x0 + c_*(bw+g2)
        by = gy + r_*(bh+g2)
        rounded(d, (bx, by, bx+bw, by+bh), 7, fill=(ACCENT if i == sel else ACCENT_DARK))
        ctext(d, bx+bw/2, by+bh/2, t, font(13, i == sel), INK if i == sel else TEXT95)
    save("preview_queue.png")

def mock_user_tab():
    d, (x0, y0, x1, y1) = window_shell("User")
    ch = 300
    rounded(d, (x0, y0+4, x1, y0+4+ch), 16, fill=CARD, outline=BORDER2, width=2)
    rounded(d, (x0+3, y0+7, x1-3, y0+4+ch-3), 13, outline=BORDER1, width=1)
    rows = [
        ("mod_india.png", "PROUDLY MADE FOR INDIA", None),
        ("mod_lightning.png", "DARK OWNER EDITION", None),
        ("mod_telegram.png", "@DARK_OWNER_VIP", None),
        ("mod_lock.png", "LICENSE STATUS  •  ", "ACTIVE"),
        ("mod_calendar.png", "EXPIRES  •  ", "31 Dec 2026"),
    ]
    ay = [y0+4+18, y0+4+60, y0+4+102, y0+4+144, y0+4+186]
    for (icon, txt, suffix), ry in zip(rows, ay):
        im = load_icon(icon, 38)
        if im is not None:
            _img.alpha_composite(im, (x0+18, ry))
        xT = x0+18+54
        f17 = font(17)
        d.text((xT, ry+9), txt, font=f17, fill=TEXT if suffix == "ACTIVE" or suffix is None else TEXT)
        if suffix:
            col = SUCCESS if suffix == "ACTIVE" else DANGER
            d.text((xT+tw(d, txt, f17), ry+9), suffix, font=f17, fill=col)
    # JOIN NOW button
    by0, by1 = y0+4+224, y0+4+224+46
    rounded(d, (x0+18, by0, x1-18, by1), 11, fill=ACCENT_DARK)
    ctext(d, (x0+x1)/2, (by0+by1)/2, "JOIN NOW", font(17, True), (234, 251, 255, 255))
    save("preview_user.png")

def mock_login():
    img = Image.new('RGBA', (WW+2*MX, WH+2*MY), (9, 14, 24, 255))
    CW, CH = 460, 500
    bx, by = (img.width-CW)//2, (img.height-CH)//2
    shadow(img, (bx, by, bx+CW, by+CH), 26, blur=26, alpha=96, offset=(8, 14))
    d = ImageDraw.Draw(img)
    rounded(d, (bx, by, bx+CW, by+CH), 24, fill=CARD)
    # gradient header band 118px (menu.h DrawGradientRect ACCENT_DARK -> ACCENT)
    gh = 118
    grad = Image.new('RGBA', (CW, gh))
    gd = ImageDraw.Draw(grad)
    for x in range(CW):
        t = x/max(CW-1, 1)
        gd.line([(x, 0), (x, gh)], fill=tuple(int(ACCENT_DARK[i]+(ACCENT[i]-ACCENT_DARK[i])*t) for i in range(4)))
    mask = Image.new('L', (CW, gh), 0)
    md = ImageDraw.Draw(mask)
    md.rounded_rectangle([0, 0, CW, gh], radius=24, fill=255)
    md.rectangle([0, gh-24, CW, gh], fill=255)
    img.paste(grad, (bx, by), mask)
    d = ImageDraw.Draw(img)
    rounded(d, (bx+1, by+1, bx+CW-1, by+CH-1), 24, outline=ACCENT_LT, width=2)
    # header texts (real strings from DrawLogin)
    ctext(d, bx+CW/2, by+34, "8 BALL POOL", font(13, True), (26, 68, 86, 255))
    ctext(d, bx+CW/2, by+66, "ADMIN SERVER LOGIN", font(24, True), INK)
    ctext(d, bx+CW/2, by+94, "Secure login  |  Server verified license key", font(12), (26, 68, 86, 255))
    # key label + field
    fy = by + 240
    d.text((bx+34, fy-26), "LICENSE KEY", font=font(12, True), fill=ACCENT_LT)
    rounded(d, (bx+34, fy, bx+CW-34, fy+60), 14, fill=(14, 22, 34, 255), outline=ACCENT_LT, width=2)
    ctext(d, bx+CW/2, fy+30, "Copy your license key and tap login", font(13), MUTED)
    # ENTER KEY button
    by0, by1 = fy+80, fy+80+54
    rounded(d, (bx+34, by0, bx+CW-34, by1), 14, fill=ACCENT_HOV)
    ctext(d, bx+CW/2, (by0+by1)/2, "ENTER KEY", font(17, True), INK)
    ctext(d, bx+CW/2, by1+26, "Authenticating...", font(12), MUTED)
    ctext(d, bx+CW/2, by1+48, "Default master key: annati", font(12), LABEL)
    ctext(d, bx+CW/2, by+CH-26, "DARK OWNER ADMIN SERVER", font(11), MUTED)
    save("preview_login.png", img)

# ------------------------------------------------------------- driver
_img = None
def save(name, img=None):
    im = img or _img
    im.convert('RGB').save(os.path.join(OUT, name), quality=95)
    print(name)

if __name__ == '__main__':
    mock_login()
    mock_draw_tab()
    mock_play_tab()
    mock_queue_tab()
    mock_user_tab()
    print("done ->", OUT)
