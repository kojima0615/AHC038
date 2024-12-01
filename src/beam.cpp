#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <cmath>
#include <algorithm>
#include <random>
#include <sstream>
#include <stdexcept>

using namespace std;

const int INF = 1000000000;                                // コード内で用いるとても大きな数
const int DIRS[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}}; // dirと対応する方向を表す配列
const int ROTATIONS[4] = {0, 1, 3, 2};                     // rotationの値とdirに加算する数の対応を表す配列

// 最小値を返す関数
int minInt(int a, int b)
{
    if (a < b)
    {
        return a;
    }
    return b;
}

// 最大値を返す関数
int maxInt(int a, int b)
{
    if (a > b)
    {
        return a;
    }
    return b;
}

// たこ焼き盤面を保存する構造体
struct TakoyakiPlate
{
    int Size;          // 縦横のサイズ
    int NumTakoyaki;   // たこ焼き総数
    vector<int> Field; // 各マスにたこ焼きが含まれているか

    TakoyakiPlate(int size)
    {
        NumTakoyaki = 0;
        Size = size;
        Field.resize(size);
    }

    // たこ焼きが置かれているかを判定する
    bool IsEmpty(int i, int j) const
    {
        if (IsOutOfField(i, j))
        {
            return true;
        }
        // 対象のbitが0か確認
        return (Field[i] & (1 << j)) == 0;
    }

    // 盤面外かどうかを判定する
    bool IsOutOfField(int i, int j) const
    {
        return i < 0 || j < 0 || i >= Size || j >= Size; // 盤面外のインデックスかどうか
    }
    // たこ焼きを追加する関数
    void AddTakoyaki(int i, int j)
    {
        // すでにたこ焼きがある場合、エラーを表示して処理を中断
        if (!IsEmpty(i, j))
        {
            std::cerr << "ERROR: not empty (" << i << ", " << j << ")" << std::endl;
            throw std::runtime_error("Not empty error");
        }

        // 盤面外の場合、エラーを表示して処理を中断
        if (IsOutOfField(i, j))
        {
            std::cerr << "ERROR: out of field (" << i << ", " << j << ")" << std::endl;
            throw std::runtime_error("Out of field error");
        }

        NumTakoyaki++;
        Field[i] ^= 1 << j; // j番目のbitを反転させる
    }

    void DeleteTakoyaki(int i, int j)
    {
        NumTakoyaki--;
        Field[i] ^= 1 << j;
    }

    TakoyakiPlate *Copy()
    {
        TakoyakiPlate *newPlate = new TakoyakiPlate(Size);
        newPlate->Load(this);
        return newPlate;
    }

    void Load(TakoyakiPlate *plate2)
    {
        Size = plate2->Size;
        NumTakoyaki = plate2->NumTakoyaki;
        for (int i = 0; i < plate2->Field.size(); i++)
        {
            Field[i] = plate2->Field[i];
        }
    }
};

// 探索実行時に変わらないような定数を保存する構造体
struct GameConst
{
    int PlateSize;                       // プレートのサイズ
    int NumTakoyaki;                     // たこ焼きの総数
    int MaxVertex;                       // 最大頂点数
    int InitialNumComplete;              // 初期完了たこ焼き数
    TakoyakiPlate *InitialTakoyakiPlate; // 初期のたこ焼き盤面
    TakoyakiPlate *GoalTakoyakiPlate;    // 目標のたこ焼き盤面
};

struct Action
{
    int Move;           // 根の移動
    vector<int> Rotate; // 各頂点の回転量
    vector<bool> Pick;  // 各頂点がたこ焼きをつかむ、あるいは離すならtrue

    Action(GameConst *gameConst)
    {
        Rotate = vector<int>(gameConst->MaxVertex, 0);
        Pick = vector<bool>(gameConst->MaxVertex, false);
    }

    void Load(Action *a2)
    {
        Move = a2->Move;
        for (int i = 0; i < Rotate.size(); i++)
        {
            Rotate[i] = a2->Rotate[i];
        }
        for (int i = 0; i < Pick.size(); i++)
        {
            Pick[i] = a2->Pick[i];
        }
    }
};

struct Tree
{
    int numBase;         // 葉になっていない頂点の数
    vector<int> parents; // 各頂点の親を格納するベクター
    vector<int> lengths; // 各辺の長さを格納するベクター
};

// GameState構造体の定義
struct GameState
{
    vector<int> RootPos = vector<int>(2, 0); // 根の座標
    vector<int> Angles;                      // 各頂点の親における相対的な回転角
    vector<bool> HoldingTakoyaki;            // 各頂点がたこ焼きを持っているか
    TakoyakiPlate *NowPlate;                 // 現在の盤面
    int Progress;                            // これまで拾ったたこ焼き数 + 置いてきたたこ焼き数
    int Turn;                                // 操作回数

    // beamSearch用
    float BeamValue;    // 盤面評価値
    int PrevIndex;      // ひとつ前の深さでどのStateだったか
    Action *PrevAction; // 直前のStateからどのアクションで到達できるか

    GameState(GameConst *gameConst)
    {
        Angles = vector<int>(gameConst->MaxVertex, 0);
        HoldingTakoyaki = vector<bool>(gameConst->MaxVertex, false);
        NowPlate = gameConst->InitialTakoyakiPlate->Copy();
        PrevAction = new Action(gameConst);
    }

    // デストラクタで手動でメモリ解放
    ~GameState()
    {
        delete NowPlate;   // NowPlateが指すメモリを解放
        delete PrevAction; // PrevActionが指すメモリを解放
    }

    bool Completed(GameConst *gameConst)
    {
        return Progress + 2 * gameConst->InitialNumComplete == 2 * gameConst->NumTakoyaki;
    }

    GameState *Copy(GameConst *gameConst, Tree *tree)
    {
        GameState *newg = new GameState(gameConst);
        newg->Load(this);
        return newg;
    }

    void Load(GameState *g2)
    {
        RootPos = g2->RootPos;
        for (int i = 0; i < Angles.size(); i++)
        {
            Angles[i] = g2->Angles[i];
        }
        for (int i = 0; i < HoldingTakoyaki.size(); i++)
        {
            HoldingTakoyaki[i] = g2->HoldingTakoyaki[i];
        }
        NowPlate->Load(g2->NowPlate);
        Turn = g2->Turn;
        Progress = g2->Progress;

        BeamValue = g2->BeamValue;
        PrevIndex = g2->PrevIndex;
        PrevAction->Load(g2->PrevAction);
    }

    void ApplyAction(GameConst *gameConst, Action *action, Tree *tree)
    {
        // greedilyで行っている方法と同じ方法で回転の計算などを行う
        vector<vector<int>> poss = vector<vector<int>>(gameConst->MaxVertex, vector<int>(2));
        vector<int> dirs = vector<int>(gameConst->MaxVertex);

        poss[0] = RootPos;
        int move = action->Move;
        if (move >= 0)
        {
            poss[0][0] += DIRS[move][0];
            poss[0][1] += DIRS[move][1];
            RootPos = poss[0];
        }
        // 頂点の座標を順に決定
        for (int i = 1; i < gameConst->MaxVertex; i++)
        {
            int parent = tree->parents[i];
            int dir = dirs[parent];
            // 今回は親の角度からの相対回転角情報を更新する必要がある
            int nAngle = Angles[i];
            int rotation = action->Rotate[i];
            nAngle += ROTATIONS[rotation]; // 今回曲げる角度
            nAngle %= 4;
            Angles[i] = nAngle;

            dir += nAngle;
            dir %= 4;
            dirs[i] = dir;
            poss[i][0] = poss[parent][0] + tree->lengths[i] * DIRS[dir][0];
            poss[i][1] = poss[parent][1] + tree->lengths[i] * DIRS[dir][1];
        }
        // たこ焼きを拾う・置く操作の適用
        for (int i = 0; i < gameConst->MaxVertex; i++)
        {
            if (action->Pick[i])
            {
                if (!HoldingTakoyaki[i])
                {
                    // 拾う
                    Pick(i, poss[i][0], poss[i][1]);
                }
                else
                {
                    // 置く（はなす）
                    Release(i, poss[i][0], poss[i][1]);
                }
            }
        }
        Turn++;
    }

    void Pick(int index, int y, int x)
    {
        HoldingTakoyaki[index] = true;
        NowPlate->DeleteTakoyaki(y, x);
        Progress++;
    }

    void Release(int index, int y, int x)
    {
        HoldingTakoyaki[index] = false;
        NowPlate->AddTakoyaki(y, x);
        Progress++;
    }
};

struct BeamSearchQueue
{
    vector<GameState *> items; // アイテムを保持するベクター
    int popIndex;              // ポップするインデックス
    int pushIndex;             // プッシュするインデックス
    int width;                 // ビーム幅
    bool sorted;               // ソートされているかどうか
    float boarder;

    BeamSearchQueue(int w)
    {
        items = vector<GameState *>(2 * w);
        popIndex = 0;
        pushIndex = 0;
        boarder = -INF;
        width = w;
    }

    bool Push(GameState *item)
    {
        bool pushed = false;
        // 配列の長さが限界ならsortして、上位width個以外は消し去る
        if (pushIndex >= items.size())
        {
            Sort();
        }
        if (!Promising(item->BeamValue))
        {
            // 上位に入り得ないのでこの段階で棄却
            delete item;
            return pushed;
        }
        pushed = true;
        sorted = false;
        items[pushIndex] = item;
        pushIndex++;
        return pushed;
    }

    void Sort()
    {
        // pushIndex (有効なデータが格納されていないインデックスで最小のもの)の手前までをソートする
        sort(items.begin(), items.begin() + pushIndex, [](const GameState *a, const GameState *b)
             { return a->BeamValue > b->BeamValue; });
        if (pushIndex > width)
        {
            for (int i = width; i < pushIndex; i++)
            {
                delete items[i];
            }
            // 上位width分以外は捨てる
            pushIndex = width;
            // width番目の評価値をボーダーとして登録
            boarder = items[width - 1]->BeamValue;
        }
        sorted = true;
    }

    bool Promising(double v)
    {
        return boarder <= v;
    }

    int Len()
    {
        //     // 正しくはこれだが、安全性を無視するなら二つ目のほうが高速
        // m := minInt(q.width, q.pushIndex)
        // l := maxInt(m-q.popIndex, 0)
        // // l := q.pushIndex - q.popIndex
        return pushIndex - popIndex;
    }

    GameState *Pop()
    {
        GameState *ret = items[popIndex];
        popIndex++;
        return ret;
    }

    void Init()
    {
        popIndex = 0;
        pushIndex = 0;
        width = items.size() / 2;
        boarder = -INF;
        sorted = false;
    }
};

// BeamSearch構造体の定義
struct BeamSearch
{
    int Width;                                  // ビーム幅
    vector<vector<GameState *>> PossibleStates; // 有効な状態を保持する
    vector<int> numPossibleState;               // 各深さごとのPossibleStateの数

    // 解を復元するときに必要になる情報
    int BestDepth; // 最善解が入っているdepth
    int BestIndex; // 最善解が入っているindex

    void Init(int width, int depthCap)
    {
        Width = width;
        PossibleStates = vector<vector<GameState *>>(depthCap);
        for (int i = 0; i < depthCap; i++)
        {
            PossibleStates[i] = vector<GameState *>(width);
        }
        numPossibleState = vector<int>(depthCap, 0);
        BestDepth = -1;
        BestIndex = -1;
    }

    void Calculate(GameConst *gameConst, GameState *initialState, Tree *tree)
    {
        // 上位width個を保持するためPriorityQueueを用意する。ここでは自前のBeamSearch専用queueを用いる。
        BeamSearchQueue *bsQueue = new BeamSearchQueue(Width);
        bsQueue->Push(initialState);
        int depth = 0;
        while (true)
        {
            // この自前のものの場合はSortされてない場合があり明示的にSortしておく必要がある
            // この自前のものでは、この段階で上位widthに入らなかったものは存在を消されています
            bsQueue->Sort();
            // 上位width個の状態をpossibleに保存しておく
            int index = 0;
            while (bsQueue->Len() > 0)
            {
                GameState *PossibleState = bsQueue->Pop();
                PossibleStates[depth][index] = PossibleState;
                numPossibleState[depth]++;
                if (PossibleState->Completed(gameConst))
                {
                    BestDepth = depth;
                    BestIndex = index;
                    return;
                }
                index++;
            }
            if (depth == PossibleStates.size())
            {
                // depthの想定を超える
                // 次の手を探索しても、格納先が用意できていない
                // capacityを大きめに設定したのに超えてしまう場合は正しく探索できていないかも？
                // 難しいケースはここに到達する可能性あり
                cout << "ERROR: depth exceeds cap" << "\n";
                exit(1);
            }
            if (numPossibleState[depth] == 0)
            {
                // 状態候補がなくなってしまった
                // 制約条件の厳しい問題では、有効な手が取れない状態になったのかも？
                // AHC038に限っては、ランダムウォークがあるのでこの状態にはならないはず
                cout << "ERROR: no possibleStates" << "\n";
                exit(1);
            }
            // 次の候補手のためのPriorityQueueを用意する
            bsQueue->Init();
            // 各候補盤面について有効手を検討する
            for (int i = 0; i < numPossibleState[depth]; i++)
            {
                GameState *PossibleState = PossibleStates[depth][i];
                TryPossibleActions(PossibleState, tree, i, gameConst, bsQueue);
            }
            depth++;
        }
    }

    void TryPossibleActions(GameState *gameState, Tree *tree, int prevIndex, GameConst *gameConst, BeamSearchQueue *bsQueue)
    {
        // この関数内で良さげな手をいくつか考え、それらを試す（ここでの有効手の考え方の違いが差をつけるチャンスの一つ）
        // 移動、回転、拾うかどうかをすべて試すと 5 * 3^V * 2 ^ V となり現実的でない
        // 今回は複雑な指標は考えず、根の移動、葉でない頂点は全探索する 5*3*3=45通り
        // 葉の動きは、回転を順番に試したこ焼きを拾うor置くことができたらその回転で決定する 別々に3通りずつ見て決定するので3*3*3*3*...通り見るということにはならない
        // すなわち、今の実装では大量の候補手から一定の法則により45通りの候補手を列挙していることになる。より賢い候補手の決め方があれば、改善が期待できる

        // move -1:wait 0,1,2,3,4: DIRS[move]に移動する
        for (int move = -1; move < 4; move++)
        {
            // 根の移動先がエリア外なら対象でない
            if (move == 0 && gameState->RootPos[1] >= gameConst->PlateSize - 1)
            {
                continue;
            }
            if (move == 1 && gameState->RootPos[0] >= gameConst->PlateSize - 1)
            {
                continue;
            }
            if (move == 2 && gameState->RootPos[1] <= 0)
            {
                continue;
            }
            if (move == 3 && gameState->RootPos[0] <= 0)
            {
                continue;
            }
            for (int rotateindex0 = 0; rotateindex0 < 1; rotateindex0++)
            {
                for (int rotateindex1 = 0; rotateindex1 < 3; rotateindex1++)
                {
                    for (int rotateindex2 = 0; rotateindex2 < 3; rotateindex2++)
                    {
                        Action *action = decideGreedily(gameState, tree, gameConst, move, vector<int>{rotateindex0, rotateindex1, rotateindex2});

                        int count = 0;
                        for (int i = 0; i < action->Pick.size(); i++)
                        {
                            if (action->Pick[i])
                                count++;
                        }
                        double value = double(gameState->Progress + count) + (double(rand()) / RAND_MAX) * 0.1;
                        if (!bsQueue->Promising(value))
                        {
                            delete action;
                            continue;
                        }

                        // actionを試すが、元のgameStateが破壊されるとよくないのでしっかりコピー
                        GameState *cGameState = gameState->Copy(gameConst, tree);
                        cGameState->ApplyAction(gameConst, action, tree);

                        // 進捗を評価値にする
                        // 同点がある程度考えられる場合は、同点同士の差をつけるためにわずかな乱数項を入れることを強くお勧めします
                        // 評価指標が素朴な場合ほど乱数項の存在が重要になります
                        value = double(cGameState->Progress) + (double(rand()) / RAND_MAX) * 0.1;
                        cGameState->BeamValue = value;
                        // どの状態から来たかを保存
                        cGameState->PrevIndex = prevIndex;
                        // どの行動で到達できるかを保存
                        cGameState->PrevAction = action;
                        bsQueue->Push(cGameState);
                    }
                }
            }
        }
    }
    Action *decideGreedily(GameState *gameState, Tree *tree, GameConst *gameConst, int move, vector<int> rotates)
    {
        // 各頂点の座標、方向を保存するため配列を用意する
        vector<vector<int>> poss = vector<vector<int>>(gameConst->MaxVertex, vector<int>(2));
        vector<int> dirs = vector<int>(gameConst->MaxVertex);

        Action *action = new Action(gameConst);
        // 指定されている移動、葉以外の回転を固定する
        action->Move = move;
        for (int i = 0; i < 3; i++)
        {
            action->Rotate[i] = rotates[i];
        }
        // 根の位置を登録
        poss[0] = gameState->RootPos;
        if (move >= 0)
        {
            // 根の位置を移動させる
            poss[0][0] += DIRS[move][0];
            poss[0][1] += DIRS[move][1];
        }
        // 葉でないものについて頂点番号の小さいものから順に場所を計算していく
        for (int jointIndex = 1; jointIndex < tree->numBase; jointIndex++)
        {
            // 辺のもう片方の端点（木の親）を取得
            int parent = tree->parents[jointIndex];
            int dir = dirs[parent];                       // 親が向いている方向
            dir += gameState->Angles[jointIndex];         // 現段階で曲がっている分
            dir += ROTATIONS[action->Rotate[jointIndex]]; // 今回新たに曲げる分
            dir %= 4;                                     // 1が90°相当なので、4で一回転する

            // 伸びている方向が分かったので、座標を求める
            poss[jointIndex][0] = poss[parent][0] + tree->lengths[jointIndex] * DIRS[dir][0];
            poss[jointIndex][1] = poss[parent][1] + tree->lengths[jointIndex] * DIRS[dir][1];

            dirs[jointIndex] = dir;
        }

        // 各葉ごとにたこ焼きをつかむ・離すができるか考える
        // 現在のtreeは(に限らずある程度性質の良い木については)複数の葉が同じたこ焼きをつかんでしまう可能性はないので独立して決定していく
        for (int jointIndex = tree->numBase; jointIndex < gameConst->MaxVertex; jointIndex++)
        {
            bool exist = false;

            int maxRotateIndex = 3; // .,L,Rの3通り
            for (int rotateIndex = 0; rotateIndex < maxRotateIndex; rotateIndex++)
            {
                // 各回転方向で葉がどの座標になるか求める
                int ndir = dirs[tree->parents[jointIndex]];
                ndir += gameState->Angles[jointIndex];
                ndir += ROTATIONS[rotateIndex];
                ndir %= 4;
                vector<int> nPos = poss[tree->parents[jointIndex]];
                nPos[0] += tree->lengths[jointIndex] * DIRS[ndir][0];
                nPos[1] += tree->lengths[jointIndex] * DIRS[ndir][1];
                if (gameState->NowPlate->IsOutOfField(nPos[0], nPos[1]))
                {
                    // 盤面外なのでつかめる可能性がない
                    continue;
                }
                // 対象座標で拾う・置くができるか確認
                if (gameState->HoldingTakoyaki[jointIndex])
                {
                    // この頂点はたこ焼きを持っているので、置けるかを判定する
                    if (gameConst->GoalTakoyakiPlate->Field[nPos[0]] & (1 << nPos[1]))
                    {
                        if ((gameState->NowPlate->Field[nPos[0]] & (1 << nPos[1])) == 0)
                        {
                            // 置ける
                            action->Pick[jointIndex] = true;
                            action->Rotate[jointIndex] = rotateIndex;
                            poss[jointIndex][0] = nPos[0];
                            poss[jointIndex][1] = nPos[1];
                            exist = true;
                            break;
                        }
                    }
                }
                else
                {
                    // この頂点はたこ焼きを持っていないので、拾えるかを判定する
                    if ((gameConst->GoalTakoyakiPlate->Field[nPos[0]] & (1 << nPos[1])) == 0)
                    {
                        if (gameState->NowPlate->Field[nPos[0]] & (1 << nPos[1]))
                        {
                            // 拾える
                            action->Pick[jointIndex] = true;
                            action->Rotate[jointIndex] = rotateIndex;
                            poss[jointIndex][0] = nPos[0];
                            poss[jointIndex][1] = nPos[1];
                            exist = true;
                            break;
                        }
                    }
                }
            }
            if (!exist)
            {
                // 置くことができなかったので、適当に回転させておく
                action->Pick[jointIndex] = false;
                action->Rotate[jointIndex] = rand() % 3;
                // 回転させない場合はこちら
                // action.Rotate[jointIndex] = 0
            }
        }
        return action;
    }

    Action *GetAction(int depth)
    {
        // 終了状態から一手ずつ戻して、depth+1の時点でどの状態だったか確認
        int index = 0;
        for (int d = BestDepth; d > depth + 1; d--)
        {
            index = PossibleStates[d][index]->PrevIndex;
        }
        // depthからdepth+1の状態へはどのactionを適用したらよいのかを取得
        return PossibleStates[depth + 1][index]->PrevAction;
    }
};
GameConst *getInput()
{
    GameConst *gConst = new GameConst();
    int n, m, v;
    vector<vector<int>> field;
    vector<vector<int>> supply;
    vector<vector<int>> demand;
    cin >> n >> m >> v;
    gConst->PlateSize = n;
    gConst->NumTakoyaki = m;
    gConst->MaxVertex = v;
    gConst->InitialTakoyakiPlate = new TakoyakiPlate(gConst->PlateSize);
    for (int i = 0; i < n; i++)
    {
        string s;
        cin >> s;
        for (int j = 0; j < s.size(); j++)
        {
            if (s[j] == '1')
            {
                gConst->InitialTakoyakiPlate->AddTakoyaki(i, j);
            }
        }
    }
    gConst->GoalTakoyakiPlate = new TakoyakiPlate(gConst->PlateSize);
    gConst->InitialNumComplete = 0;
    for (int i = 0; i < n; i++)
    {
        string s;
        cin >> s;
        for (int j = 0; j < s.size(); j++)
        {
            if (s[j] == '1')
            {
                if (gConst->InitialTakoyakiPlate->IsEmpty(i, j) == false)
                {
                    gConst->InitialNumComplete++;
                }
                gConst->GoalTakoyakiPlate->AddTakoyaki(i, j);
            }
        }
    }
    return gConst;
}

Tree *getDefaultTree(GameConst *gameConst)
{
    Tree *tree = new Tree();
    tree->numBase = 3; // 0-1-2とまっすぐつないで、2からたくさん伸ばす形を想定。葉でないのは0,1,2の3つ
    tree->parents = vector<int>(gameConst->MaxVertex, 0);
    tree->lengths = vector<int>(gameConst->MaxVertex, 0);
    for (int i = 0; i < tree->numBase; i++)
    {
        tree->parents[i] = i - 1;
    }
    tree->lengths[1] = gameConst->PlateSize / 3 - 2;
    tree->lengths[2] = gameConst->PlateSize / 3;
    for (int i = tree->numBase; i < gameConst->MaxVertex; i++)
    {
        tree->parents[i] = tree->numBase - 1;
        tree->lengths[i] = i - tree->numBase + 1;
    }
    return tree;
}

// 木を出力する関数
void printTree(Tree *tree)
{
    // 頂点数
    cout << tree->parents.size() << "\n";
    // 各辺の情報 indexが1以降のもののみ有効であることに注意
    for (int i = 1; i < tree->parents.size(); i++)
    {
        cout << tree->parents[i] << " " << tree->lengths[i] << "\n";
    }
}

// actionを出力する関数
void printAction(Action *action, GameConst *gameConst)
{
    // actionを愚直に文字列へ変換
    string message = "";
    string character = ".";
    if (action->Move == 0)
    {
        character = "R";
    }
    else if (action->Move == 1)
    {
        character = "D";
    }
    else if (action->Move == 2)
    {
        character = "L";
    }
    else if (action->Move == 3)
    {
        character = "U";
    }
    message = message + character;
    for (int i = 0; i < gameConst->MaxVertex; i++)
    {
        if (i == 0)
        {
            continue;
        }
        int rotation = action->Rotate[i];
        character = ".";
        if (rotation == 1)
        {
            character = "R";
        }
        else if (rotation == 2)
        {
            character = "L";
        }
        message = message + character;
    }
    for (int i = 0; i < gameConst->MaxVertex; i++)
    {
        character = ".";
        if (action->Pick[i])
        {
            character = "P";
        }
        message = message + character;
    }
    cout << message << "\n";
}

int main()
{
    // デバッグのために乱数のシードを固定
    srand(0);

    int width = 100;
    int depthCap = 10000;

    // 入力を受け取り、実行中の実質的定数とする
    GameConst *gameConst = getInput();
    GameState *gameState = new GameState(gameConst);
    gameState->RootPos = {gameConst->PlateSize / 2, gameConst->PlateSize / 2};
    gameState->Angles = vector<int>(gameConst->MaxVertex, 0);

    // ここでは全V,Nに対して似たような形の木を用いる
    Tree *tree = getDefaultTree(gameConst);
    BeamSearch *bs = new BeamSearch();
    // ビーム幅を100とし、10000ターン以内に終わることを願う
    bs->Init(width, depthCap);
    // ビームサーチ実行
    bs->Calculate(gameConst, gameState, tree);
    // // 完了したので、出力フェーズ
    // 木
    printTree(tree);
    cout << gameState->RootPos[0] << " " << gameState->RootPos[1] << "\n";
    while (!gameState->Completed(gameConst))
    {
        Action *action = bs->GetAction(gameState->Turn);
        printAction(action, gameConst);
        gameState->ApplyAction(gameConst, action, tree);
    }
}
