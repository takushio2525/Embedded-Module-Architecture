// IModule.h — モジュールインターフェース（プロジェクト非依存）
#pragma once

// SystemDataの前方宣言
// 各プロジェクトでSystemData構造体を定義すること
struct SystemData;

class IModule {
public:
    // 仮想デストラクタ
    virtual ~IModule() {}

    // モジュールの初期化
    // 戻り値: true=成功, false=失敗
    virtual bool init() = 0;

    // 入力フェーズ更新（センサー読み取り等）
    // 入力を持つモジュールのみオーバーライドする
    virtual void updateInput(SystemData& data) {}

    // 出力フェーズ更新（アクチュエータ制御・画面描画等）
    // 出力を持つモジュールのみオーバーライドする
    virtual void updateOutput(SystemData& data) {}

    // モジュールの終了処理（リソース解放）
    // デフォルトは空実装。解放が必要なモジュールのみオーバーライド
    virtual void deinit() {}

    // 動的有効/無効化フラグ
    // false の場合、ループ内で updateInput()/updateOutput() がスキップされる
    bool enabled = true;
};
