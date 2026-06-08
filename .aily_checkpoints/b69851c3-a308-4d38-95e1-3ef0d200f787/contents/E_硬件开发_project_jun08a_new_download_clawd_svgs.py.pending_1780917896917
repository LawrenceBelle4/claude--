#!/usr/bin/env python3
"""Download Clawd SVG files from assets/svg directory"""
import os
import requests

BASE_URL = "https://raw.githubusercontent.com/rullerzhou-afk/clawd-on-desk/main/assets/svg/"
OUTPUT_DIR = "E:/硬件开发/clawd_svgs"

# Core animation files
SVG_FILES = [
    "clawd-idle-follow.svg",      # 基础idle状态
    "clawd-idle-doze.svg",        # 打瞌睡
    "clawd-idle-yawn.svg",        # 打哈欠
    "clawd-idle-look.svg",        # 看左右
    "clawd-idle-bubble.svg",      # 冒泡思考
    "clawd-idle-reading.svg",     # 阅读
    "clawd-working-typing.svg",   # 打字工作
    "clawd-working-thinking.svg", # 思考
    "clawd-working-building.svg", # 构建
    "clawd-working-juggling.svg", # 杂耍
    "clawd-headphones-groove.svg", # 戴耳机摇摆
    "clawd-happy.svg",            # 开心
    "clawd-error.svg",            # 错误
    "clawd-notification.svg",     # 通知
    "clawd-collapse-sleep.svg",   # 睡觉
    "clawd-sleeping.svg",         # 深睡
    "clawd-wake.svg",             # 醒来
]

def download_svgs():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    for svg_file in SVG_FILES:
        url = BASE_URL + svg_file
        try:
            resp = requests.get(url, timeout=10)
            if resp.status_code == 200:
                path = os.path.join(OUTPUT_DIR, svg_file)
                with open(path, 'w', encoding='utf-8') as f:
                    f.write(resp.text)
                print(f"OK {svg_file} ({len(resp.text)} bytes)")
            else:
                print(f"FAIL {svg_file} (status {resp.status_code})")
        except Exception as e:
            print(f"FAIL {svg_file} - {e}")
    
    print(f"\nDownloaded to: {OUTPUT_DIR}")

if __name__ == "__main__":
    download_svgs()