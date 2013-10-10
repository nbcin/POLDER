/*
 * Copyright (C) 2011-2013 Morwenn
 *
 * POLDER is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * POLDER is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

template<typename InputIt, typename UnaryOperation>
auto range_map(InputIt first, InputIt last, UnaryOperation unary_op)
    -> void
{
    while (first != last)
    {
        unary_op(*first++);
    }
}

template<typename InputIt1, typename InputIt2, typename BinaryOperation>
auto range_map(InputIt1 first1, InputIt1 last1, InputIt2 first2, BinaryOperation binary_op)
    -> void
{
    while (first1 != last1)
    {
        binary_op(*first1++, *first2++);
    }
}