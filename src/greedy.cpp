#include <bits/stdc++.h>
using namespace std;
// 型エイリアス
using ll = long long;
using ld = long double;
using pii = pair<int, int>;
using pll = pair<ll, ll>;
template <typename T>
using vec = vector<T>;
template <typename T>
using vvec = vector<vector<T>>;
// 定数
constexpr int INF = 1e9;
constexpr ll LINF = 1e18;
constexpr ld EPS = 1e-9;
// 入出力の高速化
#define FAST_IO()                \
    ios::sync_with_stdio(false); \
    cin.tie(nullptr);
// 乱数生成
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
int random(int a, int b) { return uniform_int_distribution<int>(a, b)(rng); }
double random_double(double a, double b) { return uniform_real_distribution<double>(a, b)(rng); }

namespace marathon
{
    mt19937 engine(0);
    clock_t start_time;
    double now()
    {
        return 1000.0 * (clock() - start_time) / CLOCKS_PER_SEC;
    }
    void marathon_init()
    {
        start_time = clock();
        random_device seed_gen;
        engine.seed(seed_gen());
    }
    int randint(int mn, int mx)
    {
        int rng = mx - mn + 1;
        return mn + (engine() % rng);
    }
    double uniform(double x, double y)
    {
        const int RND = 1e8;
        double mean = (x + y) / 2.0;
        double dif = y - mean;
        double p = double(engine() % RND) / RND;
        return mean + dif * (1.0 - 2.0 * p);
    }
    template <typename T>
    T random_choice(vector<T> &vec)
    {
        return vec[engine() % vec.size()];
    }
    bool anneal_accept(double new_score, double old_score, double cur_time, double begin_time, double end_time, double begin_temp, double end_temp)
    {
        const int ANNEAL_RND = 1e8;
        const double ANNEAL_EPS = 1e-6;
        double temp = (begin_temp * (end_time - cur_time) + end_temp * (cur_time - begin_time)) / (end_time - begin_time);
        return (exp((new_score - old_score) / temp) > double(engine() % ANNEAL_RND) / ANNEAL_RND + ANNEAL_EPS);
    }
    bool anneal_accept2(double new_score, double old_score, double cur_time, double begin_time, double end_time, double begin_temp, double end_temp)
    {
        static int called = 0;
        static double temp = 0;
        if ((called & 127) == 0)
        {
            double ratio = (cur_time - begin_time) / (end_time - begin_time);
            ratio = min(ratio, 1.0);
            temp = pow(begin_temp, 1.0 - ratio) * pow(end_temp, ratio);
        }
        called++;

        const int ANNEAL_RND = 1e8;
        const double ANNEAL_EPS = 1e-6;

        return (exp((new_score - old_score) / temp) > double(engine() % ANNEAL_RND) / ANNEAL_RND + ANNEAL_EPS);
    }

}

// デバッグ用マクロ
#define DEBUG(var) cerr << #var << ": " << var << endl;
#define rep(i, n) for (int i = 0; i < n; i++)
#define ALL(v) v.begin(), v.end()
// 方向ベクトル（上下左右）
const int dx[4] = {0, 1, 0, -1};
const int dy[4] = {1, 0, -1, 0};
#define debug(v)          \
    cout << #v << ":";    \
    for (auto x : v)      \
    {                     \
        cout << x << ' '; \
    }                     \
    cout << endl;
template <class T>
bool chmax(T &a, const T &b)
{
    if (a < b)
    {
        a = b;
        return 1;
    }
    return 0;
}
template <class T>
bool chmin(T &a, const T &b)
{
    if (b < a)
    {
        a = b;
        return 1;
    }
    return 0;
}
// cout<<fixed<<setprecision(15);有効数字15桁
ll gcd(ll a, ll b) { return b ? gcd(b, a % b) : a; }
ll lcm(ll a, ll b) { return a / gcd(a, b) * b; }

// 入力処理
int n, m, v;
vector<vector<int>> field;
vector<vector<int>> supply;
vector<vector<int>> demand;
vector<string> best;
int bestSize = 30000;
vector<string> ans;

struct ops
{
    bool ops = false; // falseなら置きにいく
    double costOpt = INF;
    int node = -1;
    vector<int> spot = {-1, -1};
    int dir = -1;
    int xdiff = 0;
    int ydiff = 0;
    // supplyとdemandのindex、処理が終わったら削除する
    int supIndex = -1;

    void print()
    {
        cout << "ops:" << ops << "\n";
        cout << "costOpt:" << costOpt << "\n";
        cout << "node:" << node << "\n";
        cout << "spot:" << spot[0] << "," << spot[1] << "\n";
        cout << "xdiff:" << xdiff << "\n";
        cout << "ydiff:" << ydiff << "\n";
        cout << "dir:" << dir << "\n";
    }
};

struct graph
{
    int thres;
    vector<vector<int>> child;
    vector<int> par;
    vector<bool> tako;
    // ノードの座標
    vector<vector<int>> coor;
    vector<int> dir;
    vector<int> armL;
    int takoNum = 0;
    // int param = n / 4;
    int param = 5;
    double paramd = 1.0;
    string input_param;
    // 関節であるか
    vector<bool> joint;
    // 深さ2の葉であるか
    vector<bool> dep2;

    graph(int v, int n, string paramIn, int paramIn2)
    {

        string input_param = paramIn;
        param = paramIn2;
        thres = n;
        child.resize(v);
        par.resize(v);
        tako.resize(v);
        coor.resize(v);
        dir.resize(v);
        armL.resize(v);
        joint.resize(v, false);
        dep2.resize(v, false);
        int stepsize = 3;
        // dir右向き0,下向き1,左向き2,上むき3
        if (input_param == "crossmax")
        {
            bool cc = false;
            int offset = 0;
            int cur = n / 2;
            int i = 0;
            while (i < v)
            {
                joint[i] = false;
                dep2[i] = false;
                if (i == 0)
                {
                    armL[i] = 0;
                    joint[i] = true;
                    par[i] = 0;
                    tako[i] = false;
                    coor[i].resize(2);
                    coor[i][0] = n / 2;
                    coor[i][1] = n / 2 - 1 + armL[i];
                    dir[i] = 0;
                    i += 1;
                }
                else
                {
                    if (cc || v - i < 2 || cur <= n / 4)
                    {
                        armL[i] = cur;
                        cur -= stepsize;
                        if (cur <= 0)
                        {
                            offset++;
                            offset %= 3;
                            cur = n / 2 - offset;
                        }
                        par[i] = 0;
                        tako[i] = false;
                        coor[i].resize(2);
                        coor[i][0] = n / 2;
                        coor[i][1] = n / 2 - 1 + armL[i];
                        dir[i] = 0;
                        i += 1;
                        cc = false;
                    }
                    else
                    {
                        armL[i] = cur;
                        armL[i + 1] = cur;
                        cc = true;
                        par[i] = 0;
                        par[i + 1] = i;
                        tako[i] = false;
                        tako[i + 1] = false;
                        coor[i].resize(2);
                        coor[i + 1].resize(2);
                        coor[i][0] = n / 2;
                        coor[i][1] = n / 2 - 1 + armL[i];
                        coor[i + 1][0] = n / 2;
                        coor[i + 1][1] = n / 2 - 1 + armL[i] + armL[i + 1];
                        dir[i] = 0;
                        dir[i + 1] = 0;
                        joint[i] = true;
                        dep2[i] = false;
                        joint[i + 1] = false;
                        dep2[i + 1] = true;
                        child[i].push_back(i + 1);
                        i += 2;
                    }
                }
            }
        }
        else if (input_param == "crossmid")
        {
            int offset = 1;
            int curp = n / 4;
            int curm = n / 4 - 1;
            int cur = n / 4;
            int i = 0;
            bool cc = false;
            int limcount = 0;
            while (i < v)
            {
                joint[i] = false;
                dep2[i] = false;
                if (i == 0)
                {
                    armL[i] = 0;
                    joint[i] = true;
                    par[i] = 0;
                    tako[i] = false;
                    coor[i].resize(2);
                    coor[i][0] = n / 2;
                    coor[i][1] = n / 2 - 1 + armL[i];
                    dir[i] = 0;
                    i += 1;
                }
                else
                {
                    if (cc || v - i < 2 || cur <= n / 8)
                    {
                        armL[i] = cur;
                        if (curp == cur)
                        {
                            cur = curm;
                            curp += stepsize;
                            if (curp > n / 2)
                            {
                                curp = n / 4 + offset;
                                limcount++;
                            }
                        }
                        else
                        {
                            cur = curp;
                            curm -= stepsize;
                            if (curm < 0)
                            {
                                curm = n / 4 - 1 - offset;
                                limcount++;
                            }
                        }
                        if (limcount > 1)
                        {
                            offset++;
                            offset %= 3;
                        }
                        par[i] = 0;
                        tako[i] = false;
                        coor[i].resize(2);
                        coor[i][0] = n / 2;
                        coor[i][1] = n / 2 - 1 + armL[i];
                        dir[i] = 0;
                        i += 1;
                        cc = false;
                    }
                    else
                    {
                        armL[i] = cur;
                        armL[i + 1] = cur;
                        cc = true;
                        par[i] = 0;
                        par[i + 1] = i;
                        tako[i] = false;
                        tako[i + 1] = false;
                        coor[i].resize(2);
                        coor[i + 1].resize(2);
                        coor[i][0] = n / 2;
                        coor[i][1] = n / 2 - 1 + armL[i];
                        coor[i + 1][0] = n / 2;
                        coor[i + 1][1] = n / 2 - 1 + armL[i] + armL[i + 1];
                        dir[i] = 0;
                        dir[i + 1] = 0;
                        joint[i] = true;
                        dep2[i] = false;
                        joint[i + 1] = false;
                        dep2[i + 1] = true;
                        child[i].push_back(i + 1);
                        i += 2;
                    }
                }
            }
        }
        else if (input_param == "maxstep")
        {
            int offset = 0;
            int cur = n / 2;
            rep(i, v)
            {
                if (i == 0)
                {
                    armL[i] = 0;
                }
                else
                {
                    armL[i] = cur;
                    cur -= 3;
                    // if (cur > n / 2)
                    // if (cur > n * 3 / 4)
                    // 長いedgeを入れるときは折り返し判定に注意
                    if (cur <= 0)
                    {
                        offset++;
                        offset %= 3;
                        cur = n / 2 - offset;
                    }
                }
                par[i] = 0;
                tako[i] = false;
                coor[i].resize(2);
                coor[i][0] = n / 2;
                coor[i][1] = n / 2 - 1 + armL[i];
                dir[i] = 0;
            }
        }
        else if (input_param == "minstep")
        {
            int offset = 3;
            int cur = 3;
            rep(i, v)
            {
                if (i == 0)
                {
                    armL[i] = 0;
                }
                else
                {
                    armL[i] = cur;
                    cur += 3;
                    // if (cur > n / 2)
                    // if (cur > n * 3 / 4)
                    // 長いedgeを入れるときは折り返し判定に注意
                    if (cur > n / 2)
                    {
                        offset--;
                        if (offset == 0)
                        {
                            offset = 3;
                        }
                        cur = offset;
                    }
                }
                par[i] = 0;
                tako[i] = false;
                coor[i].resize(2);
                coor[i][0] = n / 2;
                coor[i][1] = n / 2 - 1 + armL[i];
                dir[i] = 0;
            }
        }
        else if (input_param == "maxiter")
        {
            int cc = 0;
            int cur = n / 2;
            rep(i, v)
            {
                if (i == 0)
                {
                    armL[i] = 0;
                }
                else
                {
                    armL[i] = cur;
                    if (cc == 1)
                    {
                        cur -= 1;
                        cc -= 1;
                        if (cur == 0)
                        {
                            cur = n / 2;
                        }
                    }
                    else
                    {
                        cc += 1;
                    }
                }
                par[i] = 0;
                tako[i] = false;
                coor[i].resize(2);
                coor[i][0] = n / 2;
                coor[i][1] = n / 2 - 1 + armL[i];
                dir[i] = 0;
            }
        }
        else if (input_param == "miditer")
        {
            int cc = 0;
            int cur = n / 4;
            rep(i, v)
            {
                if (i == 0)
                {
                    armL[i] = 0;
                }
                else
                {
                    if (i % 2)
                    {
                        if (cur + cc > n / 2)
                            cc = 0;
                        armL[i] = cur + cc;
                        cc += 1;
                    }
                    else
                    {
                        if (cur - cc < 1)
                            cc = 0;
                        armL[i] = cur - cc;
                    }
                }
                par[i] = 0;
                tako[i] = false;
                coor[i].resize(2);
                coor[i][0] = n / 2;
                coor[i][1] = n / 2 - 1 + armL[i];
                dir[i] = 0;
            }
        }
        else if (input_param == "miniter")
        {
            int cc = 0;
            int cur = 1;
            rep(i, v)
            {
                if (i == 0)
                {
                    armL[i] = 0;
                }
                else
                {
                    armL[i] = cur;
                    if (cc == 1)
                    {
                        cur += 1;
                        cc -= 1;
                        if (cur > n / 2)
                        {
                            cur = 1;
                        }
                    }
                    else
                    {
                        cc += 1;
                    }
                }
                par[i] = 0;
                tako[i] = false;
                coor[i].resize(2);
                coor[i][0] = n / 2;
                coor[i][1] = n / 2 - 1 + armL[i];
                dir[i] = 0;
            }
        }
        else if (input_param == "flat")
        {
            int cur = 1;
            rep(i, v)
            {
                if (i == 0)
                {
                    armL[i] = 0;
                }
                else
                {
                    armL[i] = cur;
                    cur += 1;
                    if (cur > n / 2)
                    {
                        cur = 1;
                    }
                }
                par[i] = 0;
                tako[i] = false;
                coor[i].resize(2);
                coor[i][0] = n / 2;
                coor[i][1] = n / 2 - 1 + armL[i];
                dir[i] = 0;
            }
        }
        par[0] = -1;
        rep(i, v - 1)
        {
            if (!dep2[i + 1])
                child[0].push_back(i + 1);
        }
    }

    bool check_dist(int node, vector<int> spot)
    {
        if ((coor[node][0] == spot[0]) && (coor[node][1] == spot[1]))
            return true;
        else
            return false;
    }

    // 深さ2の木を曲げる
    void initRotate()
    {
        string s = ".";
        rep(i, v - 1)
        {
            if (dep2[i + 1])
            {
                rotate(i + 1, par[i + 1], 'R');
                s += 'R';
            }
            else
            {
                s += '.';
            }
        }
        rep(i, v) s += '.';
        ans.push_back(s);
    }

    // 配下全てをローテートした場合に置いたり、取ったりできるか
    vector<int> rotateCheck(int node, int parn, char r)
    {

        vector<int> res = {};
        int newX = -1;
        int newY = -1;
        if (r == 'R')
        {
            newX = (coor[node][1] - coor[parn][1]) + coor[parn][0];
            newY = -(coor[node][0] - coor[parn][0]) + coor[parn][1];
        }
        else if (r == 'L')
        {
            newX = -(coor[node][1] - coor[parn][1]) + coor[parn][0];
            newY = (coor[node][0] - coor[parn][0]) + coor[parn][1];
        }
        else
        {
            newX = coor[node][0];
            newY = coor[node][1];
        }
        if (newX < 0 || newY < 0 || newX >= thres || newY >= thres)
        {
            return {};
        }

        int a = field[newX][newY];
        if (child[node].size() == 0)
        {
            if (a == 1 && tako[node] == false)
            {
                res.push_back(node);
            }
            else if (a == -1 && tako[node] == true)
            {
                res.push_back(node);
            }
        }

        for (auto x : child[node])
        {
            vector<int> b = rotateCheck(x, parn, r);
            res.insert(res.end(), b.begin(), b.end());
        }
        return res;
    }

    // 配下全てをローテート
    // check_rotateは初回のみ
    bool rotate(int node, int parn, char r)
    {

        int newX = -1;
        int newY = -1;
        if (r == 'R')
        {
            newX = (coor[node][1] - coor[parn][1]) + coor[parn][0];
            newY = -(coor[node][0] - coor[parn][0]) + coor[parn][1];
            dir[node] += 1;
        }
        else
        {
            newX = -(coor[node][1] - coor[parn][1]) + coor[parn][0];
            newY = (coor[node][0] - coor[parn][0]) + coor[parn][1];
            dir[node] -= 1;
        }
        coor[node][0] = newX;
        coor[node][1] = newY;
        dir[node] = (dir[node] + 4) % 4;
        for (auto x : child[node])
        {
            rotate(x, parn, r);
        }
        return true;
    }

    bool shift(char r)
    {
        int dx[] = {0, 0};
        if (r == 'U')
        {
            dx[0] -= 1;
        }
        else if (r == 'D')
        {
            dx[0] += 1;
        }
        else if (r == 'R')
        {
            dx[1] += 1;
        }
        else
        {
            dx[1] -= 1;
        }
        // TODO:枝分かれしているとrotateできない場合があるため注意
        rep(i, v)
        {
            int newX = coor[i][0] + dx[0];
            int newY = coor[i][1] + dx[1];
            if (i == 0)
            {
                if (newX < 0 || newY < 0 || newX >= thres || newY >= thres)
                {
                    // cout << "error" << "\n";
                    return false;
                }
            }
            coor[i][0] = newX;
            coor[i][1] = newY;
        }
        return true;
    }

    int calc_outbound(vector<int> dist)
    {
        return -abs(dist[0] - n / 2) - abs(dist[1] - n / 2);
    }
    bool checkValidRoot(int newX, int newY)
    {
        if (newX < 0 || newY < 0 || newX >= thres || newY >= thres)
        {
            return false;
        }
        return true;
    }
    ops calcCostCross(vector<int> src, vector<int> dist, int armlen, int node)
    {
        ops ops;
        int xdiff = dist[0] - src[0];
        int ydiff = dist[1] - src[1];
        rep(tdir, 4)
        {
            // tdirは親のノードのdirction
            int distNodeX, distNodeY;
            if (tdir == 0)
            {
                distNodeX = dist[0] - armlen;
                distNodeY = dist[1] - armlen;
            }
            else if (tdir == 1)
            {
                distNodeX = dist[0] - armlen;
                distNodeY = dist[1] + armlen;
            }
            else if (tdir == 2)
            {
                distNodeX = dist[0] + armlen;
                distNodeY = dist[1] + armlen;
            }
            else
            {
                distNodeX = dist[0] + armlen;
                distNodeY = dist[1] - armlen;
            }
            if (!checkValidRoot(distNodeX, distNodeY))
            {
                continue;
            }
            int moveX = distNodeX - src[0];
            int moveY = distNodeY - src[1];
            int diffdir = (tdir - dir[node] + 4) % 4;
            int cost = (abs(moveX) + abs(moveY)) * param + calc_outbound(dist);
            if (cost < ops.costOpt)
            {
                ops.costOpt = cost;
                ops.xdiff = moveX;
                ops.ydiff = moveY;
                ops.dir = tdir;
            }
        }
        return ops;
    }
    ops calcCostSingle(vector<int> src, vector<int> dist, int armlen, int node)
    {
        ops ops;
        int xdiff = dist[0] - src[0];
        int ydiff = dist[1] - src[1];
        int txdiff = abs(abs(xdiff) - armlen);
        int tydiff = abs(abs(ydiff) - armlen);
        int subX = min(abs(xdiff), armlen);
        int subY = min(abs(ydiff), armlen);
        int cost = INF;
        int tdir = -1;
        // 要検討
        if (xdiff == 0)
        {
            if (ydiff >= 0 && (abs(ydiff) - armlen) >= 0)
            {
                ops.ydiff = tydiff;
                tdir = 0;
            }
            else if (ydiff >= 0 && (abs(ydiff) - armlen) < 0)
            {
                ops.ydiff = -tydiff;
                tdir = 0;
            }
            else if (ydiff < 0 && (abs(ydiff) - armlen) >= 0)
            {
                ops.ydiff = -tydiff;
                tdir = 2;
            }
            else
            {
                ops.ydiff = tydiff;
                tdir = 2;
            }
            ops.xdiff = 0;
        }
        else if (ydiff == 0)
        {
            // armの旋回以外に全体をどっちに動かせばいいか
            if (xdiff >= 0 && (abs(xdiff) - armlen) >= 0)
            {
                ops.xdiff = txdiff;
                tdir = 1;
            }
            else if (xdiff >= 0 && (abs(xdiff) - armlen) < 0)
            {
                ops.xdiff = -txdiff;
                tdir = 1;
            }
            else if (xdiff < 0 && (abs(xdiff) - armlen) >= 0)
            {
                ops.xdiff = -txdiff;
                tdir = 3;
            }
            else
            {
                ops.xdiff = txdiff;
                tdir = 3;
            }
            ops.ydiff = 0;
        }
        else if (subX > subY)
        {
            // armの旋回以外に全体をどっちに動かせばいいか
            if (xdiff >= 0 && (abs(xdiff) - armlen) >= 0)
            {
                ops.xdiff = txdiff;
                tdir = 1;
            }
            else if (xdiff >= 0 && (abs(xdiff) - armlen) < 0)
            {
                ops.xdiff = -txdiff;
                tdir = 1;
            }
            else if (xdiff < 0 && (abs(xdiff) - armlen) >= 0)
            {
                ops.xdiff = -txdiff;
                tdir = 3;
            }
            else
            {
                ops.xdiff = txdiff;
                tdir = 3;
            }
            ops.ydiff = ydiff;
        }
        else
        {
            if (ydiff >= 0 && (abs(ydiff) - armlen) >= 0)
            {
                ops.ydiff = tydiff;
                tdir = 0;
            }
            else if (ydiff >= 0 && (abs(ydiff) - armlen) < 0)
            {
                ops.ydiff = -tydiff;
                tdir = 0;
            }
            else if (ydiff < 0 && (abs(ydiff) - armlen) >= 0)
            {
                ops.ydiff = -tydiff;
                tdir = 2;
            }
            else
            {
                ops.ydiff = tydiff;
                tdir = 2;
            }
            ops.xdiff = xdiff;
        }
        int distX = ops.xdiff + src[0];
        int distY = ops.ydiff + src[1];
        if (distX < 0 || distX >= thres)
        {
            if (distX < 0)
            {
                ops.xdiff = abs(xdiff) + armlen;
                tdir = 3;
            }
            else
            {
                ops.xdiff = -(abs(xdiff) + armlen);
                tdir = 1;
            }
        }
        if (distY < 0 || distY >= thres)
        {
            if (distY < 0)
            {
                ops.ydiff = abs(ydiff) + armlen;
                tdir = 2;
            }
            else
            {
                ops.ydiff = -(abs(ydiff) + armlen);
                tdir = 0;
            }
        }
        int diffdir = (tdir - dir[node] + 4) % 4;
        ops.costOpt = max(abs(ops.xdiff) + abs(ops.ydiff), diffdir) * param + calc_outbound(dist);
        // ops.costOpt = (abs(ops.xdiff) + abs(ops.ydiff)) * param + calc_outbound(dist);
        ops.dir = tdir;
        return ops;
    }

    ops calcCost(vector<int> src, vector<int> dist, int armlen, int node, bool crossFlag)
    {
        if (crossFlag)
        {
            return calcCostCross(src, dist, armlen, par[node]);
        }
        else
        {
            return calcCostSingle(src, dist, armlen, node);
        }
    }

    ops getOperate()
    {
        ops opsOut;
        opsOut.costOpt = INF;
        int takoCount = 0;
        rep(i, v)
        {
            if (tako[i])
                takoCount += 1;
        }
        // double d = double(takoCount) / v;
        double d = double(supply.size()) / m;
        // cout << d << "\n";
        for (int i = 1; i < v; i++)
        {
            if (!joint[i])
            {
                if (tako[i])
                {
                    // armlen:とりあえず深さ1なのでindexと同じ
                    for (int j = 0; j < demand.size(); j++)
                    {
                        ops tops = calcCost(coor[0], demand[j], armL[i], i, dep2[i]);
                        if (tops.costOpt < opsOut.costOpt)
                        {
                            opsOut = tops;
                            opsOut.ops = false;
                            opsOut.node = i;
                            opsOut.spot = demand[j];
                            opsOut.supIndex = j;
                        }
                    }
                }
                else
                {
                    // armlen:とりあえず深さ1なのでindexと同じ
                    for (int j = 0; j < supply.size(); j++)
                    {
                        ops tops = calcCost(coor[0], supply[j], armL[i], i, dep2[i]);
                        if (tops.costOpt < opsOut.costOpt)
                        {
                            opsOut = tops;
                            opsOut.ops = true;
                            opsOut.node = i;
                            opsOut.spot = supply[j];
                            opsOut.supIndex = j;
                        }
                    }
                }
            }
        }
        return opsOut;
    }
};

void execOps(ops opsIn, graph &g)
{
    while (true)
    {
        string s = "";
        char shiftOps = '.';
        if (opsIn.xdiff > 0)
        {
            shiftOps = 'D';
            opsIn.xdiff -= 1;
        }
        else if (opsIn.xdiff < 0)
        {
            shiftOps = 'U';
            opsIn.xdiff += 1;
        }
        else if (opsIn.ydiff > 0)
        {
            shiftOps = 'R';
            opsIn.ydiff -= 1;
        }
        else if (opsIn.ydiff < 0)
        {
            shiftOps = 'L';
            opsIn.ydiff += 1;
        }
        if (shiftOps != '.')
        {
            bool c = g.shift(shiftOps);
            if (!c)
            {
                return;
            }
        }
        s += shiftOps;
        string rotStr = string(v, '.');
        // 適当に回して上手くいったnodeを詰める
        vector<int> other = {};
        rep(i, v - 1)
        {
            if (g.joint[i + 1])
                continue;
            // 目標の腕のみを動作
            // その他の腕についても動きを加える
            if (i + 1 == opsIn.node)
            {
                if (g.dep2[i + 1])
                {
                    int diff = opsIn.dir - g.dir[g.par[i + 1]];
                    if (diff == 0)
                        continue;
                    else if (diff == 1 || diff == -3)
                    {
                        g.rotate(g.par[i + 1], 0, 'R');
                        rotStr[g.par[i + 1]] = 'R';
                    }
                    else if (diff == -1 || diff == 3)
                    {
                        g.rotate(g.par[i + 1], 0, 'L');
                        rotStr[g.par[i + 1]] = 'L';
                    }
                    else
                    {
                        g.rotate(g.par[i + 1], 0, 'R');
                        rotStr[g.par[i + 1]] = 'R';
                    }
                }
                else
                {
                    int diff = opsIn.dir - g.dir[opsIn.node];
                    if (diff == 0)
                        continue;
                    else if (diff == 1 || diff == -3)
                    {
                        g.rotate(opsIn.node, g.par[opsIn.node], 'R');
                        rotStr[opsIn.node] = 'R';
                    }
                    else if (diff == -1 || diff == 3)
                    {
                        g.rotate(opsIn.node, g.par[opsIn.node], 'L');
                        rotStr[opsIn.node] = 'L';
                    }
                    else
                    {
                        bool c = g.rotate(opsIn.node, g.par[opsIn.node], 'R');
                        rotStr[opsIn.node] = 'R';
                    }
                }
            }
            else
            {
                if (g.dep2[i + 1])
                {
                    // 深さ2以上にするときは関係するノードを含めないように注意恐らく、親で判定すれば良い
                    vector<int> tmp = g.rotateCheck(g.par[i + 1], 0, 'C');
                    if (tmp.size() > 0)
                    {
                        other.insert(other.end(), tmp.begin(), tmp.end());
                        continue;
                    }
                    tmp = g.rotateCheck(g.par[i + 1], 0, 'L');
                    if (tmp.size() > 0)
                    {
                        rotStr[g.par[i + 1]] = 'L';
                        g.rotate(g.par[i + 1], 0, 'L');
                        other.insert(other.end(), tmp.begin(), tmp.end());
                        continue;
                    }
                    tmp = g.rotateCheck(g.par[i + 1], 0, 'R');
                    if (tmp.size() > 0)
                    {
                        rotStr[g.par[i + 1]] = 'R';
                        g.rotate(g.par[i + 1], 0, 'R');
                        other.insert(other.end(), tmp.begin(), tmp.end());
                        continue;
                    }
                    rotStr[g.par[i + 1]] = 'R';
                    g.rotate(g.par[i + 1], 0, 'R');
                }
                else
                {
                    // 深さ2以上にするときは関係するノードを含めないように注意恐らく、親で判定すれば良い
                    vector<int> tmp = g.rotateCheck(i + 1, g.par[i + 1], 'C');
                    if (tmp.size() > 0)
                    {
                        other.insert(other.end(), tmp.begin(), tmp.end());
                        continue;
                    }
                    tmp = g.rotateCheck(i + 1, g.par[i + 1], 'L');
                    if (tmp.size() > 0)
                    {
                        rotStr[i + 1] = 'L';
                        g.rotate(i + 1, g.par[i + 1], 'L');
                        other.insert(other.end(), tmp.begin(), tmp.end());
                        continue;
                    }
                    tmp = g.rotateCheck(i + 1, g.par[i + 1], 'R');
                    if (tmp.size() > 0)
                    {
                        rotStr[i + 1] = 'R';
                        g.rotate(i + 1, g.par[i + 1], 'R');
                        other.insert(other.end(), tmp.begin(), tmp.end());
                        continue;
                    }
                    rotStr[i + 1] = 'R';
                    g.rotate(i + 1, g.par[i + 1], 'R');
                }
            }
        }
        s += rotStr.substr(1, rotStr.size());
        string takoStr = string(v, '.');
        // debug(other);
        rep(i, v)
        {
            // その他付け替え動作を加える
            if (opsIn.node == i && g.check_dist(opsIn.node, opsIn.spot))
            {
                takoStr[opsIn.node] = 'P';
                // opsIn.opsがtrueなら回収
                if (opsIn.ops)
                {
                    field[opsIn.spot[0]][opsIn.spot[1]] -= 1;
                    bool f = false;
                    rep(j, supply.size())
                    {
                        if ((supply[j][0] == g.coor[i][0]) && (supply[j][1] == g.coor[i][1]))
                        {
                            f = true;
                            supply.erase(supply.begin() + j);
                            break;
                        }
                    }
                    // if (!f)
                    // {
                    //     cout << "error" << "\n";
                    //     exit(1);
                    // }
                    g.tako[opsIn.node] = true;
                }
                else
                {
                    field[opsIn.spot[0]][opsIn.spot[1]] += 1;
                    bool f = false;
                    rep(j, demand.size())
                    {
                        if ((demand[j][0] == g.coor[i][0]) && (demand[j][1] == g.coor[i][1]))
                        {
                            demand.erase(demand.begin() + j);
                            f = true;
                            break;
                        }
                    }
                    // if (!f)
                    // {
                    //     cout << "error" << "\n";
                    //     exit(1);
                    // }
                    g.tako[opsIn.node] = false;
                }
            }
            else if (find(other.begin(), other.end(), i) != other.end())
            {
                if (field[g.coor[i][0]][g.coor[i][1]] == 0)
                    continue;
                if (g.coor[i][0] == opsIn.spot[0] && g.coor[i][1] == opsIn.spot[1])
                    continue;
                if (g.tako[i])
                {
                    int indextemp = -1;
                    rep(j, demand.size())
                    {
                        if ((demand[j][0] == g.coor[i][0]) && (demand[j][1] == g.coor[i][1]))
                        {
                            indextemp = j;
                            break;
                        }
                    }
                    if (indextemp == -1)
                        continue;
                    demand.erase(demand.begin() + indextemp);
                    takoStr[i] = 'P';
                    field[g.coor[i][0]][g.coor[i][1]] += 1;
                    g.tako[i] = false;
                }
                else
                {
                    int indextemp = -1;
                    rep(j, supply.size())
                    {
                        if ((supply[j][0] == g.coor[i][0]) && (supply[j][1] == g.coor[i][1]))
                        {
                            indextemp = j;
                            break;
                        }
                    }
                    if (indextemp == -1)
                        continue;
                    takoStr[i] = 'P';
                    field[g.coor[i][0]][g.coor[i][1]] -= 1;
                    supply.erase(supply.begin() + indextemp);
                    g.tako[i] = true;
                }
            }
        }
        s += takoStr;
        ans.push_back(s);
        // cout << s << "\n";
        if (!(g.tako[opsIn.node] ^ opsIn.ops))
            break;
        if (ans.size() > bestSize)
            break;
    }
}

// 現在の時間を返す関数（ms単位）
inline double get_time()
{
    return (double)clock() / CLOCKS_PER_SEC * 1000;
}

// パラメータ
const double TIME_LIMIT = 2.0;    // 制限時間 (秒)
const double START_TEMP = 1000.0; // 初期温度
const double END_TEMP = 10.0;     // 最終温度

int main()
{
    clock_t start = clock();
    FAST_IO();
    bool debugFlag = false;
    cin >> n >> m >> v;
    // 長すぎるedgeは扱いづらいので一旦少ないノード数で制御する
    // v = min(n / 2, v);
    vector<string> ss;
    rep(i, n)
    {
        string s;
        cin >> s;
        ss.push_back(s);
    }
    rep(i, n)
    {
        string s;
        cin >> s;
        ss.push_back(s);
    }
    // vector<string> initP = {"minstep", "maxstep", "miniter", "maxiter", "flat"};
    // vector<string> initP = {"crossmid"};
    vector<string> initP = {"minstep", "maxstep", "crossmax", "maxiter", "miditer", "flat", "crossmid", "miniter"};
    vector<int> pList = {n / 5, 1000, n / 10, n / 2, n / 3};
    clock_t end = clock();
    double time = static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000.0;
    for (string x : initP)
    {
        for (int x2 : pList)
        {
            // 初期化
            field.clear();
            supply.clear();
            demand.clear();
            ans.clear();
            field.resize(n);
            rep(i, n) field[i].resize(n);
            rep(i, n)
            {
                string s = ss[i];
                rep(j, n)
                    field[i][j] = s[j] - '0';
            }
            rep(i, n)
            {
                string s = ss[i + n];
                rep(j, n)
                    field[i][j] -= s[j] - '0';
            }
            rep(i, n)
            {
                rep(j, n)
                {
                    if (field[i][j] == 1)
                        supply.push_back(vector<int>{i, j});
                    else if (field[i][j] == -1)
                    {
                        demand.push_back(vector<int>{i, j});
                    }
                }
            }
            // グラフの作成
            graph g = graph(v, n, x, x2);
            ans.push_back(to_string(v));
            // cout << v << "\n";
            // ノード番号と親
            rep(i, v - 1)
            {
                ans.push_back(to_string(g.par[i + 1]) + " " + to_string(g.armL[i + 1]));
                // cout << g.par[i + 1] << " " << g.armL[i + 1] << "\n";
            }
            // 初期位置なるべく真ん中に
            ans.push_back(to_string(g.coor[0][0]) + " " + to_string(g.coor[0][1]));
            // cout << g.coor[0][0] << " " << g.coor[0][1] << "\n";

            if (x.find("cross") != string::npos)
            {
                g.initRotate();
                if (debugFlag)
                {
                    cout << g.coor[0][0] << " " << g.coor[0][1] << "\n";
                }
            }
            while ((supply.size() != 0) || (demand.size() != 0))
            {
                ops o = g.getOperate();
                if (debugFlag)
                    o.print();
                execOps(o, g);
                if (debugFlag)
                {
                    cout << g.coor[0][0] << " " << g.coor[0][1] << "\n";
                }
                if (ans.size() > bestSize)
                {
                    break;
                }
            }
            if (best.size() == 0)
            {
                best = ans;
                bestSize = ans.size();
            }
            else if (bestSize > ans.size())
            {
                best = ans;
                bestSize = ans.size();
            }
            end = clock();
            time = static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000.0;
            if (time > 2800)
            {
                break;
            }
        }
        if (time > 2800)
        {
            break;
        }
    }

    rep(i, best.size())
    {
        cout << best[i] << "\n";
    }
    return 0;
}