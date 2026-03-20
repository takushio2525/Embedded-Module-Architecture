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

    // モジュールの更新（毎ループ呼び出される）
    // 引数: data - プロジェクト共有データへの参照
    virtual void update(SystemData& data) = 0;

    // モジュールの終了処理（リソース解放）
    // デフォルトは空実装。解放が必要なモジュールのみオーバーライド
    virtual void deinit() {}

    // 動的有効/無効化フラグ
    // false の場合、ループ内で update() がスキップされる
    bool enabled = true;
};
