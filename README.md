# JoyaGameVST

除夜の鐘をタイミング良く鳴らすゲームとして遊べる VST プラグインです。

DAW 上でプラグインとして起動し、画面内に飛んでくる棒（撞木）が鐘の縁に当たる瞬間に合わせて入力すると、本物の鐘の音が鳴りスコアが加算されます。108回叩くことが出来たらゲームクリアです。

## 遊び方

1. プラグインの GUI を開く
2. クリック、または `F` / `J` キーでスタート
3. カウントダウン後、棒が鐘の縁（ヒットゾーン）に差しかかった瞬間に入力する
4. 成功すると鐘が鳴り、スコアが +1
5. タイミングを外すか、棒が鐘の内側まで入り込むと Game Over

### 操作

| 操作 | 内容 |
|------|------|
| クリック / `F` / `J` | スタート・ヒット・タイトルへ戻る |

## インストール

ビルド不要です。[Releases](https://github.com/kosora623/JoyaGameVST/releases) から最新版をダウンロードし、VST3 フォルダに入れるだけで使えます。

例（Windows）:

```
C:\Program Files\Common Files\VST3\
```

配置後、DAW でプラグインを再スキャンして読み込んでください。

## ビルド手順（開発者向け）

ソースからビルドする場合:

- [JUCE](https://juce.com/)（このリポジトリでは `../JUCE/modules` を参照）
- [Projucer](https://juce.com/learn/documentation/)
- Visual Studio 2022（Windows ビルド用）

1. リポジトリをクローンする
2. 同階層、または `JoyaGameVST.jucer` 内のモジュールパスが指す場所に JUCE を配置する
3. Projucer で `JoyaGameVST.jucer` を開き、Visual Studio 2022 向けにエクスポートする
4. `Builds/VisualStudio2022` のソリューションを開き、Debug / Release でビルドする
5. 生成された VST3 を DAW のプラグインフォルダに配置して読み込む

## プロジェクト構成

```
JoyaGameVST/
├── JoyaGameVST.jucer      # Projucer プロジェクト
├── Source/
│   ├── PluginProcessor.*  # オーディオ処理・鐘サンプル再生
│   ├── PluginEditor.*     # ゲーム UI / 操作
│   ├── Bell.wav           # 鐘のサウンド
│   └── gameover.png       # Game Over 用画像
└── README.md
```

## 技術概要

- **フレームワーク**: JUCE（Audio Plugin / Synth）
- **フォーマット**: VST3（Projucer 設定に依存）
- **音声**: 組み込みの `Bell.wav` を Sampler で再生（`triggerBellSound()`）
- **ゲームループ**: エディタ側の 60 FPS タイマーで棒の生成・判定を更新