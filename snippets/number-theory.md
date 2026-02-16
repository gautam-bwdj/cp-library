# Number Theory Snippets

## MEX (Minimum Excludant)

```cpp
// Returns the smallest non-negative integer not present in the array
// Time: O(n)
[[nodiscard]]
constexpr int mex(const vector<int>& arr) {
    const int n = ssize(arr);
    vector<char> seen(n + 1);

    for (const auto& x : arr) {
        if (x >= 0 && x <= n) seen[x] = 1;
    }

    for (int i = 0; i <= n; ++i) {
        if (!seen[i]) return i;
    }

    return n + 1;
}
```

## Extended GCD

```cpp
struct extgcd {
    int gcd, x, y;

    // Computes gcd(a, b) and coefficients x, y such that ax + by = gcd(a, b)
    // Time: O(log(min(a, b)))
    [[nodiscard]]
    constexpr extgcd(int a, int b) noexcept {
        if (b == 0) {
            gcd = a;
            x = 1; y = 0;
        } else {
            extgcd res(b, a % b);
            gcd = res.gcd;
            x = res.y;
            y = res.x - (a / b) * res.y;
        }
    }

    // Modular inverse using Extended GCD
    // Returns -1 if inverse doesn't exist (when gcd(a, m) != 1)
    // Time: O(log m)
    [[nodiscard]]
    static constexpr int mod_inv(int a, int m) noexcept {
        extgcd res(a, m);
        return (res.gcd != 1) ? -1 : (res.x % m + m) % m;
    }
};
```

## Euler's Totient Function

```cpp
namespace euler {

    // Computes φ(n) - count of integers in [1, n] coprime to n
    // Time: O(√n)
    [[nodiscard]]
    constexpr int phi(int n) noexcept {
        int res = n;

        for (int p = 2; p * p <= n; ++p) {
            if (n % p == 0) {
                while (n % p == 0) n /= p;
                res -= res / p;
            }
        }

        if (n > 1) res -= res / n;
        return res;
    }

    // Compute φ for all numbers from 0 to n
    // Time: O(n log log n)
    [[nodiscard]]
    vector<int> sieve(int n) {
        vector<int> res(n + 1);
        iota(res.begin(), res.end(), 0);

        for (int i = 2; i <= n; ++i) {
            if (res[i] == i) {
                for (int j = i; j <= n; j += i) {
                    res[j] -= res[j] / i;
                }
            }
        }

        return res;
    }
}
```

## Divisors

```cpp
namespace divisors {

    // Returns all divisors of n in sorted order
    // Time: O(√n)
    [[nodiscard]]
    vector<int> get(int n) {
        vector<int> res;

        for (int i = 1; i * i <= n; ++i) {
            if (n % i == 0) {
                res.push_back(i);
                if (i != n / i) res.push_back(n / i);
            }
        }

        ranges::sort(res);
        return res;
    }

    // Returns the total number of divisors of n
    // Time: O(√n)
    [[nodiscard]]
    constexpr int count(int n) noexcept {
        int cnt = 0;
        for (int i = 1; i * i <= n; ++i) {
            if (n % i == 0) {
                cnt += (i * i == n) ? 1 : 2;
            }
        }
        return cnt;
    }
}
```

## Prime Factors

```cpp
namespace prime_factors {

    // Returns prime factorization as vector of {prime, exponent} pairs
    // Time: O(√n)
    [[nodiscard]]
    vector<pair<int, int>> get(int n) {
        vector<pair<int, int>> res;

        for (int p = 2; p * p <= n; ++p) {
            if (n % p == 0) {
                int exp = 0;
                while (n % p == 0) {
                    n /= p;
                    ++exp;
                }
                res.emplace_back(p, exp);
            }
        }

        if (n > 1) res.emplace_back(n, 1);
        return res;
    }

    // Returns distinct prime factors of n
    // Time: O(√n)
    [[nodiscard]]
    vector<int> unique(int n) {
        vector<int> res;

        for (int p = 2; p * p <= n; ++p) {
            if (n % p == 0) {
                res.push_back(p);
                while (n % p == 0) n /= p;
            }
        }

        if (n > 1) res.push_back(n);
        return res;
    }
}
```
