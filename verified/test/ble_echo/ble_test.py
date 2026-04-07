#!/usr/bin/env python3
"""BLE エコーバック検証スクリプト
ESP32側で test-ble (ble_echo) を動かした状態で実行する。

使い方:
  pip install bleak
  python ble_test.py

動作:
  1. "ESP32-TestBench" をスキャンして接続
  2. テスト文字列を送信 → Notify で返ってきたデータと比較
  3. 結果を表示
"""
import asyncio
import sys
from bleak import BleakClient, BleakScanner

# ProjectConfig.h の BLE_CONFIG と一致させる
DEVICE_NAME   = "ESP32-TestBench"
SERVICE_UUID  = "12345678-1234-1234-1234-123456789abc"
RX_CHAR_UUID  = "12345678-1234-1234-1234-123456789abd"  # ESP32側の受信（PC→ESP32）
TX_CHAR_UUID  = "12345678-1234-1234-1234-123456789abe"  # ESP32側の送信（ESP32→PC）

SCAN_TIMEOUT  = 10.0   # スキャン秒数
ECHO_TIMEOUT  = 5.0    # エコー待ち秒数

# 送信するテストデータ
TEST_MESSAGES = [
    "Hello BLE",
    "12345",
    "abcdefghijklmnopqrstuvwxyz",
    "",           # 空文字（BLEライブラリが無視する可能性あり）
    "A" * 100,    # 長めのデータ
]


async def main():
    # --- スキャン ---
    print(f"[スキャン] '{DEVICE_NAME}' を検索中 ({SCAN_TIMEOUT}秒)...")
    device = await BleakScanner.find_device_by_name(
        DEVICE_NAME, timeout=SCAN_TIMEOUT
    )
    if device is None:
        print(f"[エラー] '{DEVICE_NAME}' が見つかりません。ESP32が起動しているか確認してください。")
        sys.exit(1)

    print(f"[発見] {device.name}  addr={device.address}")

    # --- 接続 ---
    async with BleakClient(device, timeout=20.0) as client:
        print(f"[接続] 成功 (MTU={client.mtu_size})")

        passed = 0
        failed = 0
        skipped = 0

        for i, msg in enumerate(TEST_MESSAGES):
            if len(msg) == 0:
                print(f"\n--- テスト {i+1}/{len(TEST_MESSAGES)}: (空文字 → スキップ) ---")
                skipped += 1
                continue

            print(f"\n--- テスト {i+1}/{len(TEST_MESSAGES)}: \"{msg[:40]}{'...' if len(msg)>40 else ''}\" ({len(msg)}bytes) ---")

            # Notify受信用のイベントとバッファ
            echo_event = asyncio.Event()
            echo_data = bytearray()

            def on_notify(_sender, data: bytearray):
                nonlocal echo_data
                echo_data = data
                echo_event.set()

            # Notify購読開始
            await client.start_notify(TX_CHAR_UUID, on_notify)

            # 送信
            payload = msg.encode("utf-8")
            await client.write_gatt_char(RX_CHAR_UUID, payload)
            print(f"  TX → {len(payload)}bytes")

            # エコー待ち
            try:
                await asyncio.wait_for(echo_event.wait(), timeout=ECHO_TIMEOUT)
                print(f"  RX ← {len(echo_data)}bytes")

                if echo_data == payload:
                    print("  結果: OK（一致）")
                    passed += 1
                else:
                    print(f"  結果: NG（不一致）")
                    print(f"    期待: {payload}")
                    print(f"    実際: {echo_data}")
                    failed += 1
            except asyncio.TimeoutError:
                print(f"  結果: NG（{ECHO_TIMEOUT}秒タイムアウト）")
                failed += 1

            await client.stop_notify(TX_CHAR_UUID)

            # 次のテストの前に少し待つ（ESP32側のloop周期を考慮）
            await asyncio.sleep(0.3)

        # --- 結果サマリー ---
        print("\n" + "=" * 40)
        print(f"結果: {passed} passed / {failed} failed / {skipped} skipped")
        if failed == 0:
            print("全テスト合格！")
        else:
            print("失敗があります。シリアルモニターのログを確認してください。")
            sys.exit(1)


if __name__ == "__main__":
    asyncio.run(main())
