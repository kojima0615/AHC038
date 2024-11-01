# 提案手法

## 基本的な考え方

現在の状態から最も少ない操作でたこ焼きを掴むまたは離せる操作列を特定しこれを実行する手法をベースとする。上記に効率を改善するための工夫 1-4 を加えた手法を提案する。

- 工夫 1
- 工夫 2
- 工夫 3

## ベースとなる手法

### ロボットアームの設計の観点

深さ 2 の木を作成する。根からの距離が多様な指先を確保するために、距離 1 から n/2 までの距離で指先を配置する。距離 n/2 の指先を配置し終えたら再び距離 1 に指先を配置する。

### 操作の観点

全ての指先と目的地のペアに対して回転と移動を考慮して必要な操作回数を計算して、必要な操作回数が最小の操作を実行する。
上記を繰り返す。ここで、目的地は指先がたこ焼きを持っている場合はたこ焼きを配置すべき場所をさし、たこ焼きを持っていない場合にはたこ焼きの場所を指すことと簡単化のため操作回数が最小の操作を一つ選び実行することに注意されたい。

### ベースとなる手法の改善点

1. 操作する頂点以外が有効に動作していないこと

ベースとなる手法では、以下の場合に必要な操作回数が最小となる指先と目的地のペアに選ばれなかった指先は動作しない。

> 全ての指先と目的地のペアに対して回転と移動を考慮して必要な操作回数を計算して、必要な操作回数が最小の操作を実行する。

例えば以下の場合は最小の動作回数は 1 であるが、片方の動作のみ実行されるため効率が悪くなっている。

図

2. 根に対して斜め方向の目的地への対応

深さ 2 の木を設計しているため根に対して斜め方向に指先が存在しない。
例えば、以下のようなたこ焼きの配置の場合、45 回の操作が必要になる。

図

3. 目的地が偏った問題クラスへの対応

例えば、N=30,M=5 の場合最大距離 5 の指先が設定される。
以下のような例の場合、距離の短い指先しかないために多くの横移動が必要となる

図

4. 最終盤において、端の目的地が残り多くの移動を必要としている場合への対応

解を観察した際に、最終盤において中心から大きく離れた目的地が残ってしまったため、多くの上下左右の移動を必要としているケースが観察された。 \*コンテスト中には確認いたしました

## 工夫 1

## 工夫 2

## 工夫 3

## 工夫 4

## 全体の流れ
