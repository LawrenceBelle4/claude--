#!/usr/bin/env python3
"""
Claude Code Status Monitor - PC Side Serial Bridge
===================================================
Monitors Claude Code activity and sends status commands
to ESP32-C3 via serial port. Also listens for button events
from ESP32 (e.g., permission confirm, mute toggle).

Status codes sent to ESP32:
    idle       - AI is resting
    thinking   - AI is reasoning
    coding     - AI is writing code
    done       - Task completed
    permission - Waiting for user permission

Status codes received from ESP32:
    btn        - Button short press (permission confirm)
    muted      - Sound muted
    unmuted    - Sound unmuted
"""

import serial
import serial.tools.list_ports
import time
import json
import threading
import queue
import os
import sys
import argparse
from datetime import datetime


class StatusBridge:
    """Serial bridge between PC and ESP32 status monitor"""
    
    def __init__(self, port=None, baudrate=115200, monitor_mode="cli"):
        self.port = port
        self.baudrate = baudrate
        self.monitor_mode = monitor_mode
        self.ser = None
        self.running = False
        self.rx_queue = queue.Queue()
        self.last_status = None
        self._status_file = os.path.join(os.path.dirname(__file__), ".claude_status")
        
        # CLI mode patterns for detecting Claude Code states
        self.patterns = {
            "thinking": [
                "thinking", "reasoning", "analyzing", "reading", "searching",
                "Thought", "Let me", "Looking at", "I'll", "First"
            ],
            "coding": [
                "writing", "creating", "editing", "applying",
                "replace", "write", "create_file", "edit_file",
                "I'll create", "I'll write", "Let me code"
            ],
            "permission": [
                "permission", "approve", "confirm", "allow",
                "ask_user", "Do you want", "Can I", "May I",
                "Proceed?"
            ],
            "done": [
                "completed", "success", "done", "finished", "all set",
                "Anything else", "that helps"
            ]
        }
    
    def find_port(self):
        """Auto-detect ESP32 serial port"""
        ports = serial.tools.list_ports.comports()
        for p in ports:
            # ESP32 often identified by these VID/PID
            if "USB" in p.description or "340" in p.description or \
               "esp" in p.description.lower() or "CP210" in p.description or \
               "CH9102" in p.description or "CH343" in p.description:
                print(f"[OK] Found: {p.device} - {p.description}")
                return p.device
        
        if ports:
            print("[WARN] No ESP32 auto-detected. Available ports:")
            for p in ports:
                print(f"  {p.device} - {p.description}")
        
        return None
    
    def connect(self):
        """Connect to ESP32 serial port"""
        if not self.port:
            self.port = self.find_port()
            if not self.port:
                print("[ERR] No serial port found. Specify with --port")
                return False
        
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=1)
            time.sleep(1.5)  # ESP32 resets on serial connect
            self.ser.reset_input_buffer()
            print(f"[OK] Connected to {self.port} @ {self.baudrate}")
            return True
        except Exception as e:
            print(f"[ERR] Failed to connect: {e}")
            return False
    
    def send_status(self, status):
        """Send status command to ESP32"""
        if self.ser and self.ser.is_open and status in ("idle", "thinking", "coding", "done", "permission"):
            try:
                self.ser.write(f"{status}\n".encode())
                self.ser.flush()
                self.last_status = status
                ts = datetime.now().strftime("%H:%M:%S")
                print(f"  [{ts}] PC -> HW: {status}")
            except Exception as e:
                print(f"[ERR] Write failed: {e}")
    
    def _reader_thread(self):
        """Background thread to read from serial"""
        while self.running:
            try:
                if self.ser and self.ser.is_open and self.ser.in_waiting:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        self.rx_queue.put(line)
                else:
                    time.sleep(0.05)
            except Exception:
                time.sleep(0.1)
    
    def start(self):
        """Start serial communication"""
        if not self.connect():
            return False
        
        self.running = True
        self.reader = threading.Thread(target=self._reader_thread, daemon=True)
        self.reader.start()
        
        # Send initial idle
        self.send_status("idle")
        return True
    
    def stop(self):
        """Stop serial communication"""
        self.running = False
        self.send_status("idle")
        time.sleep(0.2)
        if self.ser and self.ser.is_open:
            self.ser.close()
        print("[OK] Disconnected")
    
    def check_events(self):
        """Check for events from ESP32 (button presses)"""
        events = []
        while not self.rx_queue.empty():
            try:
                msg = self.rx_queue.get_nowait()
                events.append(msg)
                ts = datetime.now().strftime("%H:%M:%S")
                if msg == "btn":
                    print(f"  [{ts}] HW -> PC: BUTTON PRESSED (permission confirm)")
                elif msg == "muted":
                    print(f"  [{ts}] HW -> PC: SOUND MUTED")
                elif msg == "unmuted":
                    print(f"  [{ts}] HW -> PC: SOUND UNMUTED")
            except queue.Empty:
                break
        return events
    
    def detect_claude_state(self, text):
        """Heuristic detection of Claude Code state from output text"""
        text_lower = text.lower()
        
        # Check patterns in priority order
        for pattern in self.patterns["permission"]:
            if pattern.lower() in text_lower:
                return "permission"
        
        for pattern in self.patterns["coding"]:
            if pattern.lower() in text_lower:
                return "coding"
        
        for pattern in self.patterns["thinking"]:
            if pattern.lower() in text_lower:
                return "thinking"
        
        for pattern in self.patterns["done"]:
            if pattern.lower() in text_lower:
                return "done"
        
        return None
    
    def set_status(self, status):
        """Public API to set and send status"""
        if status != self.last_status:
            self.send_status(status)
    
    def write_status_file(self, status):
        """Write status to a file for external tools to read"""
        try:
            with open(self._status_file, 'w') as f:
                f.write(status)
        except:
            pass


def cli_mode(bridge):
    """Command-line interactive mode - manually control status"""
    print("""
╔══════════════════════════════════════╗
║   Claude Monitor - CLI Control       ║
╠══════════════════════════════════════╣
║ Keys:                                ║
║   i = idle (resting)                 ║
║   t = thinking                       ║
║   c = coding                         ║
║   d = done                           ║
║   p = permission                     ║
║   q = quit                           ║
╚══════════════════════════════════════╝
""")
    
    key_map = {
        'i': 'idle', 't': 'thinking', 'c': 'coding',
        'd': 'done', 'p': 'permission'
    }
    
    try:
        import msvcrt  # Windows
        getch = lambda: msvcrt.getch().decode('utf-8', errors='ignore').lower()
    except ImportError:
        try:
            import tty, termios  # Unix
            def getch():
                fd = sys.stdin.fileno()
                old = termios.tcgetattr(fd)
                try:
                    tty.setraw(fd)
                    return sys.stdin.read(1).lower()
                finally:
                    termios.tcsetattr(fd, termios.TCSADRAIN, old)
        except ImportError:
            getch = lambda: input().strip().lower()[0] if input() else ''
    
    while bridge.running:
        bridge.check_events()
        
        if os.name == 'nt':
            import msvcrt
            if msvcrt.kbhit():
                key = getch()
                if key == 'q':
                    break
                status = key_map.get(key)
                if status:
                    bridge.set_status(status)
        else:
            # Unix - use select for non-blocking
            import select
            if select.select([sys.stdin], [], [], 0.1)[0]:
                key = getch()
                if key == 'q':
                    break
                status = key_map.get(key)
                if status:
                    bridge.set_status(status)
        
        time.sleep(0.1)


def pipe_mode(bridge):
    """
    Pipe mode - read from stdin (for piping Claude Code output).
    
    Usage:
        claude status | python bridge.py --mode pipe
    
    Each line from stdin is analyzed for Claude Code state keywords.
    """
    print("[INFO] Pipe mode - reading from stdin...")
    idle_timer = time.time()
    
    try:
        while bridge.running:
            # Check for HW events
            bridge.check_events()
            
            # Read from stdin line by line
            if os.name == 'nt':
                import msvcrt
                if msvcrt.kbhit():
                    line = sys.stdin.readline()
                    if not line:
                        break
                    line = line.strip()
                    if line:
                        detected = bridge.detect_claude_state(line)
                        if detected:
                            bridge.set_status(detected)
                            idle_timer = time.time()
            else:
                import select
                if select.select([sys.stdin], [], [], 0.1)[0]:
                    line = sys.stdin.readline()
                    if not line:
                        break
                    line = line.strip()
                    if line:
                        detected = bridge.detect_claude_state(line)
                        if detected:
                            bridge.set_status(detected)
                            idle_timer = time.time()
            
            # Auto-idle after 5 seconds of no activity
            if time.time() - idle_timer > 5:
                if bridge.last_status != "idle":
                    bridge.set_status("idle")
            
            time.sleep(0.05)
    except KeyboardInterrupt:
        pass


def watch_mode(bridge):
    """
    Watch mode - read status from a file (for integration with other tools).
    Other programs write status to .claude_status file, this bridge reads and forwards.
    """
    status_file = os.path.join(os.path.dirname(__file__), ".claude_status")
    print(f"[INFO] Watch mode - monitoring {status_file}")
    
    last_file_status = None
    
    try:
        while bridge.running:
            bridge.check_events()
            
            try:
                if os.path.exists(status_file):
                    with open(status_file, 'r') as f:
                        status = f.read().strip()
                    if status in ("idle", "thinking", "coding", "done", "permission"):
                        if status != last_file_status:
                            bridge.set_status(status)
                            last_file_status = status
            except:
                pass
            
            time.sleep(0.2)
    except KeyboardInterrupt:
        pass


def simulate_mode():
    """
    Simulator mode - no hardware needed. Shows a GUI preview of the status monitor.
    """
    print("[INFO] Starting SIMULATOR mode (no hardware required)...")
    
    try:
        from simulator import run_simulator
        run_simulator()
    except ImportError:
        print("[ERR] simulator.py not found. Make sure it's in the same directory.")
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description="Claude Code Status Monitor - PC Bridge")
    parser.add_argument("--port", "-p", help="Serial port (e.g., COM3, /dev/ttyUSB0)")
    parser.add_argument("--baud", "-b", type=int, default=115200, help="Baud rate")
    parser.add_argument("--mode", "-m", default="cli",
                        choices=["cli", "pipe", "watch", "simulate"],
                        help="Operation mode: cli=manual control, pipe=stdin, watch=file, simulate=preview")
    parser.add_argument("--noserial", action="store_true",
                        help="Disable serial (useful for pipe/watch test)")
    args = parser.parse_args()
    
    # Simulator mode doesn't need serial
    if args.mode == "simulate":
        simulate_mode()
        return
    
    bridge = StatusBridge(port=args.port, baudrate=args.baud)
    
    if not args.noserial:
        if not bridge.start():
            print("[WARN] Serial connection failed. Use --noserial for test mode.")
            ans = input("Continue without hardware? (y/n): ").strip().lower()
            if ans != 'y':
                return
            bridge.running = True
    else:
        bridge.running = True
        print("[INFO] No-serial mode - status will be printed only")
    
    try:
        if args.mode == "cli":
            cli_mode(bridge)
        elif args.mode == "pipe":
            pipe_mode(bridge)
        elif args.mode == "watch":
            watch_mode(bridge)
    except KeyboardInterrupt:
        print("\n[INFO] Shutting down...")
    finally:
        bridge.stop()


if __name__ == "__main__":
    main()
