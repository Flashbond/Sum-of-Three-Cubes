#include <iostream>
#include <fcntl.h>
#include <chrono>
#include <iomanip>
#include <thread>
#include <vector>
#include <cmath>
#include <atomic>
#include <cstdint>
#include <limits>
#include <algorithm>

#ifdef _WIN32
    #include <io.h>
#endif

using namespace std;

using u64 = unsigned long long;

std::chrono::time_point<std::chrono::system_clock> start_time;

static const unsigned THREADS = std::max(1u, (unsigned)(std::thread::hardware_concurrency() * 1.375));

std::atomic<int> state{1};
unsigned int o = 0;

u64 fk = 0;
u64 fl = 0;
u64 fm = 0;

// ============================================================
// Overflow-safe cube
// ============================================================

static constexpr u64 MAX_CUBE_ROOT_U64 = 2642245ULL;

inline bool cube_u64(u64 x, u64& out)
{
    if (x > MAX_CUBE_ROOT_U64)
        return false;

    out = x * x * x;
    return true;
}

// ============================================================
// Exact integer cube root
// ============================================================

inline bool cube_root(u64 n, u64& root)
{
    u64 lo = 0;
    u64 hi = MAX_CUBE_ROOT_U64;

    while (lo <= hi)
    {
        u64 mid = lo + (hi - lo) / 2;

        u64 c;
        cube_u64(mid, c);

        if (c == n)
        {
            root = mid;
            return true;
        }

        if (c < n)
            lo = mid + 1;
        else
            hi = mid - 1;
    }

    return false;
}

// ============================================================
// Print result
// ============================================================

void result(u64 k, u64 l, u64 m, int mode)
{
    if (mode == -1)
    { wcout << k << L"\u00B3+" << l << L"\u00B3+" << m << L"\u00B3 = " << o << L"\n"; }

    if (mode == -2)
    { wcout << l << L"\u00B3-" << k << L"\u00B3+" << m << L"\u00B3 = " << o << L"\n"; }

    if (mode == -3)
    { wcout << k << L"\u00B3-" << l << L"\u00B3-" << m << L"\u00B3 = " << o << L"\n"; }

    auto elapsed = std::chrono::system_clock::now() - start_time;

    std::chrono::duration<double> sec = elapsed;

    wcout << L"Time elapsed: " << std::fixed << setprecision(3) << sec.count() << L" seconds\n\n";
}

// ============================================================
// Worker
//
// 1)  t³ - y³ - z³ = o
//     z³ = t³ - y³ - o
//
// 2) -t³ + y³ + z³ = o
//     z³ = t³ + o - y³
// ============================================================

void forLoop(u64 a, u64 b, u64 t)
{
    u64 t3;

    if (!cube_u64(t, t3))
        return;

    for (u64 y = a; y <= b && state.load(std::memory_order_relaxed) == 1; ++y)
    {
        u64 y3;

        if (!cube_u64(y, y3))
            break;

        // ====================================================
        // Form 1:
        //
        // t³ - y³ - z³ = o
        //
        // z³ = t³ - y³ - o
        // ====================================================

        __int128 target = (__int128)t3 - (__int128)y3 - (__int128)o;

        if (target >= 0)
        {
            u64 z;

            if (cube_root((u64)target, z) && z < t)
            {
                int expected = 1;

                if (state.compare_exchange_strong(expected, -3, std::memory_order_acq_rel))
                {
                    fk = t;
                    fl = y;
                    fm = z;
                }

                if (state.load(std::memory_order_relaxed) != 1) return;
            }
        }

        // ====================================================
        // Form 2:
        //
        // -t³ + y³ + z³ = o
        //
        // z³ = t³ + o - y³
        // ====================================================

        target = (__int128)t3 + (__int128)o - (__int128)y3;

        if (target >= 0 && target <= (__int128)t3)
        {
            u64 z;

            if (cube_root((u64)target, z) && z < t)
            {
                int expected = 1;

                if (state.compare_exchange_strong(expected, -2, std::memory_order_acq_rel))
                {
                    fk = t;
                    fl = y;
                    fm = z;
                }

                if (state.load(std::memory_order_relaxed) != 1) return;
            }
        }
    }
}

void multiThread(u64 t)
{
    if (state.load(std::memory_order_relaxed) != 1)
        return;

    std::vector<std::thread> thr;
    thr.reserve(THREADS);

    u64 block = (t + THREADS - 1) / THREADS;

    if (block == 0)
        block = 1;


    for (unsigned i = 0; i + 1 < THREADS; ++i)
    {
        if (state.load(std::memory_order_relaxed) != 1)
            break;

        u64 a = (u64)i * block;
        u64 b = a + block - 1;

        if (a > t)
            break;

        if (b > t)
            b = t;

        thr.emplace_back(forLoop, a, b, t);
    }


    if (state.load(std::memory_order_relaxed) == 1)
    {
        u64 a = (u64)(THREADS - 1) * block;

        if (a <= t)
        {
            thr.emplace_back(forLoop, a, t, t);
        }
    }


    for (auto& th : thr)
    {
        th.join();
    }
}

int main()
{
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_U16TEXT);
#endif


INPUT:

    state.store(1, std::memory_order_relaxed);

    wcout << L"Input number: ";
    cin >> o;

    if ((o - 5) % 9 == 0 || (o - 4) % 9 == 0)
    {
		wcout << o << L" has no sum of 3 cubes solution." << std::endl;
        goto INPUT;
    }

    start_time = std::chrono::system_clock::now();

    for (u64 x = 0; ; ++x)
    {
        u64 x3;

        if (!cube_u64(x, x3))
            break;

        if (x3 > (u64)o)
            break;


        for (u64 y = 0; y <= x; ++y)
        {
            u64 y3;

            if (!cube_u64(y, y3))
                break;


            for (u64 z = 0; z <= x; ++z)
            {
                u64 z3;

                if (!cube_u64(z, z3))
                    break;

                __int128 s = (__int128)x3 + (__int128)y3 + (__int128)z3;

                if (s == (__int128)o)
                {
                    result(x, y, z, -1);
                    goto INPUT;
                }
            }
        }
    }

    for (u64 x = 1; x > 0; ++x)
    {
        multiThread(x);

        int mode =
            state.load(std::memory_order_relaxed);

        if (mode == -2 || mode == -3)
        {
            result(fk, fl, fm, mode);
            goto INPUT;
        }
    }
}
