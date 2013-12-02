/*
 * Copyright (C) 2011-2013 Morwenn
 *
 * POLDER is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * POLDER is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

inline namespace standard
{
    template<typename Integer>
    auto is_even(Integer n)
        -> bool
    {
        return meta::is_even(n);
    }

    template<typename Integer>
    auto is_odd(Integer n)
        -> bool
    {
        return meta::is_odd(n);
    }

    template<typename Unsigned>
    auto is_prime(Unsigned n)
        -> bool
    {
        return meta::is_prime(n);
    }

    template<typename Float>
    auto degree(Float rad)
        -> Float
    {
        return meta::degree(rad);
    }

    template<typename Float>
    auto radian(Float deg)
        -> Float
    {
        return meta::radian(deg);
    }

    template<typename Float>
    auto sinc(Float x)
        -> Float
    {
        return std::sin(x) / x;
    }

    template<typename Float>
    auto normalized_sinc(Float x)
        -> Float
    {
        return std::sin(x * M_PI) / (x * M_PI);
    }

    template<typename Float>
    auto quadratic(Float A, Float B, Float C)
        -> std::pair<std::complex<Float>, std::complex<Float>>
    {
        A *= 2; // A is not used alone
        const Float delta = std::fma(B, B, -2*A*C);
        B = -B; // Only the opposite of B is used after this
        if (delta < 0)
        {
            const Float tmp_div = B / A;
            const Float delta_root = std::sqrt(delta);
            return std::make_pair(std::complex<Float>(tmp_div, delta_root/A),
                                  std::complex<Float>(tmp_div, -delta_root/A));
        }
        else if (delta == 0)
        {
            const Float res = B / A;
            return std::make_pair(res, res);
        }
        else
        {
            const Float delta_root = std::sqrt(delta);
            return std::make_pair((B+delta_root)/A, (B-delta_root)/A);
        }
    }

    template<typename Unsigned>
    auto fibonacci(Unsigned n)
        -> Unsigned
    {
        Unsigned a = 0;
        Unsigned b = 1;
        for (Unsigned i = 0 ; i < n ; ++i)
        {
            b += a;
            a = b - a;
        }
        return a;
    }

    template<typename Unsigned>
    auto prime(Unsigned n)
        -> Unsigned
    {
        static std::vector<Unsigned> primes = { 1, 2, 3 };

        // If the prime number is not already known,
        // compute and store it
        if (primes.size()-1 < n)
        {
            // We search numbers greater than
            // the greatest known prime member
            Unsigned tested_number = primes.back();

            // While we have less than n prime numbers
            while (primes.size()-1 < n)
            {
                // We iterate 2 by 2 (to avoid non-even numbers)
                tested_number += 2;
                auto root = std::sqrt(tested_number);

                // Assume the number is a prime
                // until the opposite is proven
                bool is_prime = true;

                // Try to divide the supposed prime by all the known primes
                // starting with 3
                for (auto it = primes.begin()+2 ; it != primes.end() ; ++it)
                {
                    const auto& pri = *it;
                    // We don't have to search above the square root
                    if (pri > root) break;

                    // If the number has an integer divider
                    if (tested_number % pri == 0)
                    {
                        // It's not a prime number
                        is_prime = false;
                        break;
                    }
                }

                if (is_prime)
                {
                    primes.push_back(tested_number);
                }
            }
        }
        return primes[n];
    }

    template<typename Unsigned>
    auto gcd(Unsigned a, Unsigned b)
        -> Unsigned
    {
        if (a == 0 || b == 0)
        {
            return 0;
        }
        if (b > a)
        {
            std::swap(a, b);
        }

        Unsigned r = a % b;
        while (r != 0)
        {
            a = b;
            b = r;
            r = a % b;
        }
        return b;
    }

    template<typename Unsigned>
    auto lcm(Unsigned a, Unsigned b)
        -> Unsigned
    {
        if (a == 0 || b == 0)
        {
            return 1;
        }
        return a*b / gcd(a, b);
    }
}

namespace meta
{
    ////////////////////////////////////////////////////////////
    // Static variables, Initialization & Helpers
    ////////////////////////////////////////////////////////////

    namespace
    {
        // Helper for the is_prime() function
        template<typename Unsigned>
        constexpr auto _is_prime_helper(Unsigned n, Unsigned div)
            -> bool
        {
            return (div*div > n) ? true :
                                 (n % div == 0) ? false :
                                                _is_prime_helper(n, div+2);
        }
    }

    ////////////////////////////////////////////////////////////
    // POLDER functions
    ////////////////////////////////////////////////////////////

    template<typename Integer>
    constexpr auto is_even(Integer n)
        -> bool
    {
        return not Integer(n & 1);
    }

    template<typename Integer>
    constexpr auto is_odd(Integer n)
        -> bool
    {
        return Integer(n & 1);
    }

    template<typename Unsigned>
    constexpr auto is_prime(Unsigned n)
        -> bool
    {
        return (n < 2) ? false :
                         (n == 2) ? true :
                                  _is_prime_helper(n, 3);
    }

    template<typename Float>
    constexpr auto degree(Float rad)
        -> Float
    {
        return rad * M_180_PI;
    }

    template<typename Float>
    constexpr auto radian(Float deg)
        -> Float
    {
        return deg * M_PI_180;
    }

    template<typename Unsigned>
    constexpr auto fibonacci(Unsigned n)
        -> Unsigned
    {
        return (n < 2) ? n : meta::fibonacci(n-2) + meta::fibonacci(n-1);
    }
}
