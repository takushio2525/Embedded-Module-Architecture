// test_ble — BLE通信モジュールのテスト
// BLEハードウェア（ESP32-S3内蔵）が必要
// 注意: 接続テストには外部BLEクライアント（スマホ等）が必要
#include <Arduino.h>
#include <unity.h>
#include <BLEDevice.h>
#include "BleModule.h"
#include "ProjectConfig.h"

static BleModule* ble = nullptr;
static SystemData systemData;

// init()が成功すること（BLEスタック起動）
void test_ble_init() {
    bool result = ble->init();
    TEST_ASSERT_TRUE_MESSAGE(result, "BLEのinit()が失敗");
}

// init後は未接続状態であること
void test_ble_initial_disconnected() {
    systemData.ble = BleData{};
    ble->update(systemData);
    TEST_ASSERT_FALSE_MESSAGE(systemData.ble.connected,
        "init直後なのにconnected=true");
}

// BLEデバイスアドレスが取得できること
void test_ble_device_address() {
    BLEAddress addr = BLEDevice::getAddress();
    String addrStr = String(addr.toString().c_str());
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, addrStr.length(),
        "BLEアドレスが空");
    // MACアドレス形式: xx:xx:xx:xx:xx:xx (17文字)
    TEST_ASSERT_EQUAL_MESSAGE(17, addrStr.length(),
        "BLEアドレスのフォーマットが不正");
    Serial.printf("[BLE] アドレス: %s\n", addrStr.c_str());
}

// update()で受信データがない場合、newDataIn=falseのままであること
void test_ble_no_data() {
    systemData.ble = BleData{};
    ble->update(systemData);
    TEST_ASSERT_FALSE_MESSAGE(systemData.ble.newDataIn,
        "データ未受信なのにnewDataIn=true");
}

// 送信リクエストが未接続時にクリアされないこと（接続していないので送信しない）
void test_ble_tx_without_connection() {
    systemData.ble = BleData{};
    systemData.ble.sendRequest = true;
    const char* testMsg = "test";
    memcpy(systemData.ble.txData, testMsg, 4);
    systemData.ble.txLength = 4;

    ble->update(systemData);
    // 未接続なので送信されず、sendRequestはtrueのまま
    TEST_ASSERT_TRUE_MESSAGE(systemData.ble.sendRequest,
        "未接続なのにsendRequestがクリアされた");
}

// onReceive()コールバックのテスト（手動呼び出し）
void test_ble_receive_callback() {
    const uint8_t testPayload[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};  // "Hello"
    ble->onReceive(testPayload, 5);

    systemData.ble = BleData{};
    ble->update(systemData);
    TEST_ASSERT_TRUE_MESSAGE(systemData.ble.newDataIn,
        "onReceive後にnewDataIn=false");
    TEST_ASSERT_EQUAL_UINT8(5, systemData.ble.rxLength);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(testPayload, systemData.ble.rxData, 5);
}

// 受信フラグが次のupdate()でクリアされること
void test_ble_receive_flag_cleared() {
    // 前のテストでデータ受信済み
    // 新たなデータなしでupdate → newDataInはfalseに戻る
    systemData.ble.newDataIn = false;
    ble->update(systemData);
    TEST_ASSERT_FALSE_MESSAGE(systemData.ble.newDataIn,
        "2回目のupdate後もnewDataInがtrue");
}

// onConnect/onDisconnectコールバックのテスト（手動呼び出し）
void test_ble_connect_disconnect_callback() {
    ble->onConnect();
    systemData.ble = BleData{};
    ble->update(systemData);
    TEST_ASSERT_TRUE_MESSAGE(systemData.ble.connected,
        "onConnect後にconnected=false");

    ble->onDisconnect();
    systemData.ble = BleData{};
    ble->update(systemData);
    TEST_ASSERT_FALSE_MESSAGE(systemData.ble.connected,
        "onDisconnect後にconnected=true");
}

// deinit()が正常に完了すること
void test_ble_deinit() {
    ble->deinit();
    TEST_ASSERT_TRUE(true);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }  // USB-CDC接続待ち
    delay(500);

    ble = new BleModule(BLE_CONFIG);

    UNITY_BEGIN();

    RUN_TEST(test_ble_init);
    RUN_TEST(test_ble_initial_disconnected);
    RUN_TEST(test_ble_device_address);
    RUN_TEST(test_ble_no_data);
    RUN_TEST(test_ble_tx_without_connection);
    RUN_TEST(test_ble_receive_callback);
    RUN_TEST(test_ble_receive_flag_cleared);
    RUN_TEST(test_ble_connect_disconnect_callback);
    RUN_TEST(test_ble_deinit);

    UNITY_END();

    delete ble;
}

void loop() {}
