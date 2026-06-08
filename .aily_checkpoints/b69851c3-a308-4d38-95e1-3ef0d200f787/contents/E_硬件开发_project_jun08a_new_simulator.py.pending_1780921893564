#!/usr/bin/env python3
"""
Clawd Pixel Crab Monitor - Hardware Simulator
Preview Clawd pixel crab animations without hardware.
Matches ESP32 firmware pixel-for-pixel (scaled 2x).
SVG source: https://github.com/rullerzhou-afk/clawd-on-desk

Run: python simulator.py
"""

import tkinter as tk
from tkinter import ttk
import threading
import time
import sys
import random

try:
    import winsound
    HAS_WINSOUND = True
except ImportError:
    HAS_WINSOUND = False

# ─── Colors matching ESP32 RGB565 ───
CC      = '#DE886D'   # tft.color565(222,136,109) Clawd salmon
BLACK   = '#000000'
WHITE   = '#FFFFFF'
SHADOW  = '#424242'   # 0x4208
CYAN    = '#0ADFE0'   # 0x04DF
YELLOW  = '#FFE000'   # 0xFFE0
GOLD_BG = '#C8A000'   # RGB(200,160,0) warm gold - permission bg
RED     = '#F80000'   # 0xF800
GREEN   = '#07E007'   # 0x07E0
DOTC    = '#256891'   # tft.color565(37,104,145) thought dots
SPARKC  = '#FFE066'   # tft.color565(255,224,102) sparkle
SCR_BG  = '#1E1E2E'   # tft.color565(30,30,46) screen bg
SCR_BR  = '#2D2D3D'   # tft.color565(45,45,61) screen border
CODE_G  = '#4CAF50'   # tft.color565(76,175,80)
CODE_B  = '#40C4FF'   # tft.color565(64,196,255)
CODE_Y  = '#FFC107'   # tft.color565(255,193,7)
KB_C    = '#455A64'   # tft.color565(69,90,100)
KB_K    = '#78909C'   # tft.color565(120,144,156)
GOLD    = '#FFD700'   # tft.color565(255,215,0)
AMBER   = '#FFC107'   # tft.color565(255,193,7)
LYELLOW = '#FFF99D'   # tft.color565(255,249,157)
BOOK_C  = '#8B4513'   # tft.color565(139,69,19) book cover
BOOK_SB = '#64320A'   # tft.color565(100,50,10) book spine
ZZZ     = '#07FF07'   # 0x07FF sleeping z's
LAVENDER= '#8410A0'   # 0x8410 sleeping text

SCALE = 2

class HardwareSimulator:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("🦀 Clawd Pixel Crab Monitor - Hardware Simulator")
        self.root.configure(bg='#1a1a1a')
        self.root.resizable(False, False)

        self.current_state = "idle"
        self.anim_frame = 0
        self.muted = False
        self.solenoid_active = False
        self.anim_running = True
        self.button_press_time = 0
        self.demo_active = False

        # Idle sub-state: 0=basic, 1=reading, 2=sleeping
        self.idle_st = 0
        self.idle_sub = 0
        self.idle_start = time.time()
        # Transition: 0=none, 1=from_reading, 2=from_sleeping
        self.trans = 0
        self.trans_frame = 0

        self._create_gui()
        self.anim_thread = threading.Thread(target=self._animation_loop, daemon=True)
        self.anim_thread.start()

    # ─── Drawing primitives ───

    def _rect(self, x, y, w, h, color):
        sx, sy = x * SCALE, y * SCALE
        self.canvas.create_rectangle(sx, sy, sx + w * SCALE, sy + h * SCALE, fill=color, outline='')

    def _round_rect(self, x, y, w, h, r, color):
        sx, sy, sw, sh, sr = x*SCALE, y*SCALE, w*SCALE, h*SCALE, r*SCALE
        self.canvas.create_rectangle(sx+sr, sy, sx+sw-sr, sy+sh, fill=color, outline='')
        self.canvas.create_rectangle(sx, sy+sr, sx+sw, sy+sh-sr, fill=color, outline='')
        for cx, cy in [(sx+sr,sy+sr),(sx+sw-sr,sy+sr),(sx+sr,sy+sh-sr),(sx+sw-sr,sy+sh-sr)]:
            self.canvas.create_oval(cx-sr, cy-sr, cx+sr, cy+sr, fill=color, outline='')

    def _circle(self, cx, cy, r, color):
        sx, sy, sr = cx*SCALE, cy*SCALE, r*SCALE
        self.canvas.create_oval(sx-sr, sy-sr, sx+sr, sy+sr, outline=color, width=SCALE)

    def _text(self, x, y, txt, color, size=1):
        sx, sy = x*SCALE, y*SCALE
        fs = max(8, size * 8 * SCALE)
        font = ('Courier', fs, 'bold') if size >= 2 else ('Courier', fs)
        self.canvas.create_text(sx, sy, text=txt, fill=color, font=font, anchor='nw')

    def _clear(self, color=BLACK):
        self.canvas.create_rectangle(0, 0, 160*SCALE, 80*SCALE, fill=color, outline='')

    # ─── Crab body helpers ───

    def _crab_body(self, ox, yO, legs_h=8, arm_y_off=0, eyes=True, eye_h=4):
        """Draw standard crab body at offset ox, y offset yO"""
        # Shadow
        self._rect(ox+2, 46, 18, 2, SHADOW)
        # Legs
        self._rect(ox+2, 38, 2, legs_h, CC)
        self._rect(ox+6, 38, 2, legs_h, CC)
        self._rect(ox+14, 38, 2, legs_h, CC)
        self._rect(ox+18, 38, 2, legs_h, CC)
        # Torso
        self._rect(ox, 28+yO, 22, 14, CC)
        # Arms
        self._rect(ox-4, 34+yO+arm_y_off, 4, 4, CC)
        self._rect(ox+22, 34+yO+arm_y_off, 4, 4, CC)
        # Eyes
        if eyes:
            self._rect(ox+4, 32+yO, 2, eye_h, BLACK)
            self._rect(ox+16, 32+yO, 2, eye_h, BLACK)

    # ─── State Animations ───

    def _draw_idle(self):
        if self.idle_st == 0:
            self._draw_idle_basic()
        elif self.idle_st == 1:
            self._draw_idle_reading()
        else:
            self._draw_idle_sleeping()

    def _draw_idle_basic(self):
        f = self.anim_frame % 20
        self._clear()
        yO = 1 if (f % 6 < 3) else 0
        blink = (16 <= f % 20 <= 17)
        self._crab_body(69, yO, eyes=not blink)
        self._text(5, 70, "idle", CYAN, 1)
        # After 10s for testing (60s on real hardware)
        if time.time() - self.idle_start > 10:
            self.idle_st = random.choice([1, 2])
            self.idle_sub = 0
            self.anim_frame = 0

    def _draw_idle_reading(self):
        f = self.idle_sub % 20
        self._clear()
        yO = 1 if (f % 8 < 4) else 0
        # Crab with legs 5px (shorter), no arms in normal pos
        self._rect(71, 47, 18, 2, SHADOW)
        # Shorter legs
        self._rect(71, 40, 2, 5, CC); self._rect(75, 40, 2, 5, CC)
        self._rect(83, 40, 2, 5, CC); self._rect(87, 40, 2, 5, CC)
        # Torso
        self._rect(69, 30+yO, 22, 12, CC)
        # Arms (shorter, holding book)
        self._rect(65, 38+yO, 4, 3, CC); self._rect(91, 38+yO, 4, 3, CC)
        # Eyes squinted (2px)
        self._rect(73, 34+yO, 2, 2, BLACK); self._rect(85, 34+yO, 2, 2, BLACK)
        # Book (held by claws) - two pages with spine
        self._rect(68, 39+yO, 11, 7, BOOK_C)  # Left page cover
        self._rect(81, 39+yO, 11, 7, BOOK_C)  # Right page cover
        self._rect(69, 40+yO, 9, 5, WHITE)     # Left page white
        self._rect(82, 40+yO, 9, 5, WHITE)     # Right page white
        self._rect(79, 39+yO, 2, 7, BOOK_SB)   # Spine
        # Text lines on pages
        tl = f % 6
        self._rect(70, 41+yO, 7, 1, BLACK)     # Left page line 1
        self._rect(70, 43+yO, 5, 1, BLACK)     # Left page line 2
        self._rect(83, 41+yO, 4+tl, 1, BLACK)  # Right page growing line
        self._rect(83, 43+yO, 3+((tl+2)%5), 1, BLACK)
        # Page turn every 10 frames
        if f % 10 >= 8:
            self._rect(79, 39+yO, 4, 7, WHITE)  # Page flipping
        self._text(5, 70, "reading", YELLOW, 1)

    def _draw_idle_sleeping(self):
        f = self.idle_sub % 20
        self._clear()
        yO = 1 if (f % 10 < 5) else 0
        self._crab_body(69, yO, eyes=True, eye_h=1)  # Tiny slit eyes
        # Sleeping Z's
        zp = f % 10
        self._text(94, 24-zp, "z", ZZZ, 1)
        if zp >= 3:
            self._text(100, 18-zp, "z", ZZZ, 1)
        if zp >= 6:
            self._text(106, 12-zp, "z", ZZZ, 1)
        self._text(5, 70, "sleeping", LAVENDER, 1)

    def _draw_thinking(self):
        self._clear()
        if self.trans > 0 and self.trans_frame < 4:
            self._draw_thinking_transition()
            self.trans_frame += 1
            if self.trans_frame >= 4:
                self.trans = 0
        else:
            self.trans = 0
            self._draw_thinking_normal()

    def _draw_thinking_transition(self):
        tf = self.trans_frame
        if self.trans == 1:
            # From reading → eyes widen, book drops
            self._rect(71, 46, 18, 2, SHADOW)
            self._rect(71, 38, 2, 8, CC); self._rect(75, 38, 2, 8, CC)
            self._rect(83, 38, 2, 8, CC); self._rect(87, 38, 2, 8, CC)
            self._rect(69, 28, 22, 14, CC)
            self._rect(65, 34, 4, 4, CC); self._rect(91, 34, 4, 4, CC)
            # Eyes gradually open
            eH = min(2 + tf, 4)
            self._rect(73, 32, 2, eH, BLACK); self._rect(85, 32, 2, eH, BLACK)
            # Book drops away
            bY = 40 + tf * 3
            if bY < 48:
                self._rect(74, bY, 12, 5, BOOK_C)
                self._rect(76, bY+1, 8, 3, WHITE)
        else:
            # From sleeping → rise up, eyes open
            yU = -min(tf, 2)
            self._rect(71, 46, 18, 2, SHADOW)
            self._rect(71, 38, 2, 8, CC); self._rect(75, 38, 2, 8, CC)
            self._rect(83, 38, 2, 8, CC); self._rect(87, 38, 2, 8, CC)
            self._rect(69, 28+yU, 22, 14, CC)
            self._rect(65, 34+yU, 4, 4, CC); self._rect(91, 34+yU, 4, 4, CC)
            # Eyes open
            eH = min(1 + tf, 4)
            self._rect(73, 32+yU, 2, eH, BLACK); self._rect(85, 32+yU, 2, eH, BLACK)
        self._text(5, 70, "thinking", YELLOW, 1)

    def _draw_thinking_normal(self):
        f = self.anim_frame % 12
        xO = -4 if (f < 6) else 4
        ox = 65 + xO
        # Shadow
        self._rect(ox+6, 46, 18, 2, SHADOW)
        # Legs
        self._rect(ox+6, 38, 2, 8, CC); self._rect(ox+10, 38, 2, 8, CC)
        self._rect(ox+18, 38, 2, 8, CC); self._rect(ox+22, 38, 2, 8, CC)
        # Torso
        self._rect(ox+4, 28, 22, 14, CC)
        # Arms
        armOff = -4 if (f < 6) else 0
        self._rect(ox, 34+armOff, 4, 4, CC); self._rect(ox+26, 34, 4, 4, CC)
        # Eyes squint
        self._rect(ox+8, 32, 2, 3, BLACK); self._rect(ox+20, 32, 2, 3, BLACK)
        # Thought bubble
        bx, by = ox-8, 2
        self._round_rect(bx, by, 18, 16, 3, WHITE)
        # Chain puffs
        self._rect(ox+4, 22, 2, 2, WHITE); self._rect(ox+8, 25, 1, 1, WHITE)
        # Dots appear one by one
        phase = f % 6
        if phase >= 1: self._rect(bx+2, by+6, 2, 2, DOTC)
        if phase >= 3: self._rect(bx+6, by+6, 2, 2, DOTC)
        if phase >= 5:
            self._rect(bx+10, by+6, 2, 2, DOTC)
            # Sparkle +++
            self._rect(ox+12, 0, 2, 2, SPARKC)
            self._rect(ox+12, 2, 2, 2, SPARKC)
            self._rect(ox+10, 0, 2, 2, SPARKC)
            self._rect(ox+14, 0, 2, 2, SPARKC)
        self._text(5, 70, "thinking", YELLOW, 1)

    def _draw_coding(self):
        f = self.anim_frame % 24
        self._clear()
        yO = 2 if (f % 4 >= 2) else 0
        # Screen background
        self._rect(66, 4, 27, 22, SCR_BG)
        self._rect(66, 4, 27, 2, SCR_BR)
        # Traffic lights
        self._rect(67, 5, 2, 1, RED); self._rect(70, 5, 2, 1, YELLOW); self._rect(73, 5, 2, 1, GREEN)
        # Code lines
        li = f // 4
        self._rect(67, 8, 10, 1, CODE_G)
        self._rect(69, 11, 16, 1, CODE_G)
        self._rect(69, 14, (9+li)%16, 1, CODE_B)
        self._rect(67, 17, (4+li)%8, 1, CODE_Y)
        # Data particles
        if f % 6 < 3: self._rect(71, 1, 2, 2, CODE_B)
        else:         self._rect(83, 0, 2, 2, CODE_B)
        # Shadow
        self._rect(71, 46, 18, 2, SHADOW)
        # Legs
        self._rect(71, 38, 2, 8, CC); self._rect(75, 38, 2, 8, CC)
        self._rect(83, 38, 2, 8, CC); self._rect(87, 38, 2, 8, CC)
        # Torso
        self._rect(69, 28+yO, 22, 14, CC)
        # Keyboard
        self._rect(64, 40, 32, 6, KB_C)
        self._rect(66, 41, 28, 1, KB_K)
        self._rect(66, 43, 28, 1, KB_K)
        # Arms typing
        aL = -2 if (f % 2 == 0) else 0
        aR = -2 if (f % 2 == 1) else 0
        self._rect(65, 34+yO+aL, 4, 4, CC)
        self._rect(91, 34+yO+aR, 4, 4, CC)
        # Eyes squint at screen
        self._rect(73, 32+yO, 2, 3, BLACK); self._rect(85, 32+yO, 2, 3, BLACK)
        self._text(5, 70, "typing", GREEN, 1)

    def _draw_done(self):
        f = self.anim_frame % 16
        self._clear()
        phase = f % 8
        jump = 0
        if 1 <= phase <= 3:
            jump = -(phase * (4 - phase)) * 2
        # Sparkles (alternating sets)
        if f % 4 < 2:
            self._rect(57, 12, 2, 2, GOLD); self._rect(57, 10, 2, 2, GOLD)
            self._rect(57, 14, 2, 2, GOLD); self._rect(55, 12, 2, 2, GOLD)
            self._rect(59, 12, 2, 2, GOLD)
            self._rect(101, 8, 2, 2, AMBER); self._rect(101, 6, 2, 2, AMBER)
            self._rect(101, 10, 2, 2, AMBER); self._rect(99, 8, 2, 2, AMBER)
            self._rect(103, 8, 2, 2, AMBER)
        else:
            self._rect(105, 36, 2, 2, LYELLOW); self._rect(105, 34, 2, 2, LYELLOW)
            self._rect(105, 38, 2, 2, LYELLOW); self._rect(103, 36, 2, 2, LYELLOW)
            self._rect(107, 36, 2, 2, LYELLOW)
            self._rect(53, 40, 2, 2, AMBER); self._rect(53, 38, 2, 2, AMBER)
            self._rect(53, 42, 2, 2, AMBER); self._rect(51, 40, 2, 2, AMBER)
            self._rect(55, 40, 2, 2, AMBER)
        # Shadow
        self._rect(71, 46, 18, 2, SHADOW)
        # Torso
        self._rect(69, 28+jump, 22, 14, CC)
        # Legs (shorter 4px in happy)
        self._rect(71, 38+jump, 2, 4, CC); self._rect(75, 38+jump, 2, 4, CC)
        self._rect(83, 38+jump, 2, 4, CC); self._rect(87, 38+jump, 2, 4, CC)
        # Arms
        self._rect(65, 34+jump, 4, 4, CC); self._rect(91, 34+jump, 4, 4, CC)
        # Eyes
        if f % 8 != 4:
            self._rect(73, 32+jump, 2, 4, BLACK); self._rect(85, 32+jump, 2, 4, BLACK)
        self._text(5, 70, "done!", GREEN, 1)
        # Victory melody - one cycle only
        if self.anim_frame < 16:
            melody = [523,659,784,1047, 0,1319,1568,2093, 0,1047,1568, 0,2093, 0, 0, 0]
            if not self.muted:
                fq = melody[self.anim_frame]
                if fq > 0:
                    self._beep(fq, 150)

    def _draw_permission(self):
        f = self.anim_frame % 2
        bg = GOLD_BG if f else BLACK
        self._clear(bg)
        # Shadow
        self._rect(71, 46, 18, 2, SHADOW)
        # Legs
        self._rect(71, 38, 2, 8, CC); self._rect(75, 38, 2, 8, CC)
        self._rect(83, 38, 2, 8, CC); self._rect(87, 38, 2, 8, CC)
        # Torso
        self._rect(69, 28, 22, 14, CC)
        # Arms
        self._rect(65, 34, 4, 4, CC); self._rect(91, 34, 4, 4, CC)
        # Eyes wide
        self._rect(73, 32, 2, 4, BLACK); self._rect(85, 32, 2, 4, BLACK)
        # Question bubble
        self._round_rect(80, 2, 18, 16, 3, YELLOW)
        self._round_rect(82, 4, 14, 12, 2, BLACK)
        self._text(85, 5, "?", YELLOW, 2)
        tc = RED if f else YELLOW
        self._text(5, 70, "Waiting", tc, 1)

    # ─── Sound & Solenoid ───

    def _beep(self, freq, dur):
        if self.muted or not HAS_WINSOUND:
            return
        try:
            threading.Thread(target=lambda: winsound.Beep(freq, dur), daemon=True).start()
        except:
            pass
        self.buzzer.itemconfig('bz', fill='#00ff00')
        self.root.after(100, lambda: self.buzzer.itemconfig('bz', fill='gray'))

    def _solenoid(self, ms):
        if self.solenoid_active:
            return
        self.solenoid_active = True
        self.solenoid_cv.itemconfig('sol', fill='#ff6600')
        self.solenoid_lb.config(text="ON ⚡")
        self.root.after(ms, self._solenoid_off)

    def _solenoid_tap(self):
        """Single quick tap (40ms ON)"""
        self._solenoid(40)

    def _permission_taps(self, count=0):
        """Rapid 4 taps for permission state"""
        if count >= 4:
            return
        self._solenoid_tap()
        self.root.after(80, lambda: self._permission_taps(count + 1))

    def _solenoid_off(self):
        self.solenoid_active = False
        self.solenoid_cv.itemconfig('sol', fill='gray')
        self.solenoid_lb.config(text="OFF")

    # ─── Button ───

    def _btn_down(self, e):
        self.btn_cv.itemconfig('btn', fill='#00ff00')
        self.button_press_time = time.time()

    def _btn_up(self, e):
        self.btn_cv.itemconfig('btn', fill='#444')
        held = time.time() - self.button_press_time
        if held > 0.7:
            self.muted = not self.muted
            self.mute_var.set(self.muted)
            self.btn_log.config(text="🔇 Muted" if self.muted else "🔊 Unmuted")
            if not self.muted:
                self._beep(800, 100)
        else:
            self.btn_log.config(text="✓ Confirmed!")
            if not self.muted:
                self._beep(1000, 50)
            if self.current_state == "permission":
                self._set_state("idle")

    # ─── State Control ───

    def _set_state(self, state):
        self.current_state = state
        self.anim_frame = 0
        self.trans = 0
        self.trans_frame = 0

        if state == "idle":
            self.idle_st = 0
            self.idle_sub = 0
            self.idle_start = time.time()
        elif state == "thinking":
            # Set transition from idle sub-state
            self.trans = self.idle_st if self.idle_st > 0 else 0
            self.trans_frame = 0
            self.idle_st = 0
            if self.trans > 0:
                self.anim_frame = 0

        icons = {"idle": "💤 Idle", "thinking": "🤔 Thinking", "coding": "💻 Typing",
                 "done": "✅ Done", "permission": "⏳ Waiting"}
        self.status_var.set(icons.get(state, state))
        if state == "done":
            self._solenoid(300)
        elif state == "permission":
            if not self.muted:
                self._beep(800, 100)
            self._permission_taps()

    def _toggle_mute(self):
        self.muted = self.mute_var.get()
        self.btn_log.config(text="🔇 Muted" if self.muted else "🔊 Unmuted")

    # ─── Animation Loop ───

    def _animation_loop(self):
        while self.anim_running:
            self.root.after(0, self._render)
            if self.current_state == "idle":
                if self.idle_st == 0:
                    time.sleep(0.3)
                elif self.idle_st == 1:
                    time.sleep(0.5)
                else:
                    time.sleep(0.6)
            elif self.current_state == "thinking":
                if self.trans > 0:
                    time.sleep(0.5)
                else:
                    time.sleep(0.5)
            elif self.current_state == "coding":
                time.sleep(0.2)
            elif self.current_state == "done":
                time.sleep(0.18)
            elif self.current_state == "permission":
                time.sleep(0.4)
            else:
                time.sleep(0.05)

    def _render(self):
        self.canvas.delete('all')
        draw = {"idle": self._draw_idle, "thinking": self._draw_thinking,
                "coding": self._draw_coding, "done": self._draw_done,
                "permission": self._draw_permission}
        fn = draw.get(self.current_state)
        if fn:
            fn()

        # Advance frame counters
        if self.current_state == "idle":
            if self.idle_st == 0:
                self.anim_frame += 1
            else:
                self.idle_sub += 1
        else:
            self.anim_frame += 1

    # ─── Demo Mode ───

    def _toggle_demo(self):
        self.demo_active = self.demo_var.get()
        if self.demo_active:
            self._demo_step(0)

    def _demo_step(self, idx):
        seq = [("idle", 12), ("thinking", 4), ("coding", 5), ("done", 3),
               ("idle", 15), ("permission", 4), ("coding", 4), ("done", 3)]
        if not self.demo_active or not self.anim_running:
            return
        state, dur = seq[idx % len(seq)]
        self._set_state(state)
        self.root.after(int(dur * 1000), lambda: self._demo_step(idx + 1))

    # ─── GUI ───

    def _create_gui(self):
        style = ttk.Style()
        style.configure('TFrame', background='#1a1a1a')
        style.configure('TLabel', background='#1a1a1a', foreground='white')
        style.configure('TLabelframe', background='#1a1a1a', foreground='white')
        style.configure('TLabelframe.Label', background='#1a1a1a', foreground='white')

        main = ttk.Frame(self.root, padding=8)
        main.pack(fill=tk.BOTH, expand=True)

        # Title
        ttk.Label(main, text="🦀 Clawd Pixel Crab — ESP32 Monitor Simulator",
                   font=('Arial', 11, 'bold'), foreground='#DE886D').pack(pady=(0, 5))

        # TFT Display
        tft_frame = ttk.LabelFrame(main, text="📺 TFT 160×80 (×2)", padding=3)
        tft_frame.pack()
        self.canvas = tk.Canvas(tft_frame, width=160*SCALE, height=80*SCALE,
                                bg='black', highlightthickness=2, highlightbackground='#444')
        self.canvas.pack()

        # Status
        sf = ttk.Frame(main)
        sf.pack(fill=tk.X, pady=4)
        ttk.Label(sf, text="State:", font=('Arial', 10)).pack(side=tk.LEFT)
        self.status_var = tk.StringVar(value="💤 Idle")
        ttk.Label(sf, textvariable=self.status_var, font=('Arial', 11, 'bold'),
                  foreground='#DE886D').pack(side=tk.LEFT, padx=8)

        # Hardware row
        hw = ttk.Frame(main)
        hw.pack(fill=tk.X, pady=3)

        # Buzzer
        bz_f = ttk.LabelFrame(hw, text="🔊", padding=2)
        bz_f.pack(side=tk.LEFT, padx=4)
        self.buzzer = tk.Canvas(bz_f, width=20, height=20, bg='#1a1a1a', highlightthickness=0)
        self.buzzer.pack()
        self.buzzer.create_oval(2, 2, 18, 18, fill='gray', outline='#666', tags='bz')

        # Solenoid
        sol_f = ttk.LabelFrame(hw, text="⚡", padding=2)
        sol_f.pack(side=tk.LEFT, padx=4)
        self.solenoid_cv = tk.Canvas(sol_f, width=40, height=18, bg='#1a1a1a', highlightthickness=0)
        self.solenoid_cv.pack()
        self.solenoid_cv.create_rectangle(2, 2, 38, 16, fill='gray', outline='#666', tags='sol')
        self.solenoid_lb = ttk.Label(sol_f, text="OFF", font=('Arial', 8))
        self.solenoid_lb.pack()

        # Mute
        self.mute_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(hw, text="🔇 Mute", variable=self.mute_var,
                        command=self._toggle_mute).pack(side=tk.LEFT, padx=8)

        # Button
        btn_f = ttk.LabelFrame(hw, text="🔘 Button", padding=2)
        btn_f.pack(side=tk.LEFT, padx=4)
        self.btn_cv = tk.Canvas(btn_f, width=40, height=24, bg='#1a1a1a', highlightthickness=0)
        self.btn_cv.pack()
        self.btn_cv.create_rectangle(8, 4, 32, 20, fill='#444', outline='#888', tags='btn')
        self.btn_cv.bind('<Button-1>', self._btn_down)
        self.btn_cv.bind('<ButtonRelease-1>', self._btn_up)
        self.btn_log = ttk.Label(btn_f, text="click", font=('Arial', 7))
        self.btn_log.pack()

        # State buttons
        ctrl = ttk.LabelFrame(main, text="🎮 State Control", padding=4)
        ctrl.pack(fill=tk.X, pady=4)

        states = [("💤 Idle", "idle", CYAN), ("🤔 Thinking", "thinking", YELLOW),
                  ("💻 Typing", "coding", GREEN), ("✅ Done", "done", GREEN),
                  ("⏳ Waiting", "permission", '#C8A000')]
        for txt, st, fg in states:
            tk.Button(ctrl, text=txt, command=lambda s=st: self._set_state(s),
                      bg='#2a2a2a', fg=fg, font=('Arial', 9), width=13,
                      activebackground='#444', activeforeground=fg, relief=tk.RAISED, bd=2
                     ).pack(side=tk.LEFT, padx=1)

        # Bottom
        bot = ttk.Frame(main)
        bot.pack(fill=tk.X, pady=6)
        self.demo_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(bot, text="🔄 Auto Demo", variable=self.demo_var,
                        command=self._toggle_demo).pack(side=tk.LEFT)
        ttk.Button(bot, text="❌ Quit", command=self._quit).pack(side=tk.RIGHT)

        ttk.Label(main, text="SVG-accurate Clawd pixel crab — matches ESP32 firmware",
                  font=('Arial', 8), foreground='#666').pack()

    def _quit(self):
        self.anim_running = False
        self.root.quit()
        self.root.destroy()

    def run(self):
        self.root.update_idletasks()
        w, h = self.root.winfo_width(), self.root.winfo_height()
        x = (self.root.winfo_screenwidth() - w) // 2
        y = (self.root.winfo_screenheight() - h) // 2
        self.root.geometry(f'+{x}+{y}')
        self.root.mainloop()

def run_simulator():
    HardwareSimulator().run()

if __name__ == "__main__":
    run_simulator()
