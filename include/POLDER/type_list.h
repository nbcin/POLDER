/*
 * Copyright (C) 2011-2014 Morwenn
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
#ifndef POLDER_TYPE_LIST_H_
#define POLDER_TYPE_LIST_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <type_traits>
#include <POLDER/details/config.h>

namespace polder
{
    // Forward declarations
    template<typename...>
    struct type_list;
    template<typename...>
    struct type_list_cat {};

    template<typename Head, typename... Tail>
    struct type_list<Head, Tail...>:
        type_list<Tail...>
    {
        using size_type = std::size_t;
        static constexpr size_type size = sizeof...(Tail) + 1;
        static constexpr bool is_empty = false;

        template<size_type N>
        using at = std::conditional_t<
            N == 0,
            Head,
            typename type_list<Tail...>::template at<N-1>
        >;

        using front = Head;
        using back  = at<size-1>;

        template<typename... Types>
        using push_front = type_list<Types..., Head, Tail...>;
        template<typename... Types>
        using push_back = type_list<Head, Tail..., Types...>;

        using pop_front = type_list<Tail...>;
        using pop_back = typename type_list_cat<type_list<Head>,
                                                typename type_list<Tail...>::pop_back
                                            >::type;

        template<typename List>
        using cat = typename type_list_cat<type_list<Head, Tail...>, List>::type;
    };

    template<typename Head>
    struct type_list<Head>
    {
        using size_type = std::size_t;
        static constexpr size_type size = 1;
        static constexpr bool is_empty = false;

        template<size_type N>
        using at = std::conditional_t<
            N == 0,
            Head,
            void
        >;

        using front = Head;
        using back  = Head;

        template<typename... Types>
        using push_front = type_list<Types..., Head>;
        template<typename... Types>
        using push_back = type_list<Head, Types...>;

        using pop_front = type_list<>;
        using pop_back = type_list<>;

        template<typename List>
        using cat = typename type_list_cat<type_list<Head>, List>::type;
    };

    /**
     * @brief Empty type list
     *
     * type_list specialization when the list does not
     * contain any element.
     */
    template<>
    struct type_list<>
    {
        using size_type = std::size_t;
        static constexpr size_type size = 0;
        static constexpr bool is_empty = true;

        template<size_type N>
        using at    = void;

        using front = void;
        using back  = void;

        template<typename... Types>
        using push_front = type_list<Types...>;
        template<typename... Types>
        using push_back = type_list<Types...>;

        using pop_front = type_list<>;
        using pop_back = type_list<>;

        template<typename List>
        using cat = List;
    };

    ////////////////////////////////////////////////////////////
    // type_list_cat
    ////////////////////////////////////////////////////////////

    template<>
    struct type_list_cat<type_list<>, type_list<>>
    {
        using type = type_list<>;
    };

    template<typename... Types>
    struct type_list_cat<type_list<>, type_list<Types...>>
    {
        using type = type_list<Types...>;
    };

    template<typename... Types>
    struct type_list_cat<type_list<Types...>, type_list<>>
    {
        using type = type_list<Types...>;
    };

    template<typename... A, typename... B>
    struct type_list_cat<type_list<A...>, type_list<B...>>
    {
        using type = type_list<A..., B...>;
    };
}

#endif // POLDER_TYPE_LIST_H_
