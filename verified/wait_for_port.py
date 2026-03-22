"""
USB-CDC再接続待ちスクリプト
ESP32-S3はアップロード後のリセットでUSBポートが一時消失する。
PlatformIOがシリアル読み取りを開始する前にポート復帰を待つ。
"""
import time
import glob

Import("env")

def wait_for_usb_cdc(source, target, env):
    port_pattern = "/dev/cu.usbmodem*"
    print("USB-CDCポート再接続を待機中...")

    # ポートが消えるのを待つ（最大5秒）
    for _ in range(50):
        if not glob.glob(port_pattern):
            break
        time.sleep(0.1)

    # ポートが再出現するのを待つ（最大10秒）
    for i in range(100):
        ports = glob.glob(port_pattern)
        if ports:
            print(f"USB-CDCポート復帰: {ports[0]}")
            time.sleep(1)  # 安定化待ち
            return
        time.sleep(0.1)

    print("警告: USB-CDCポートが見つかりません。リセットボタンを押してください。")

env.AddPostAction("upload", wait_for_usb_cdc)
