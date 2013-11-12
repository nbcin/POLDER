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

namespace detail
{

}

template<typename Ret, typename... Args>
MemoizedFunction<Ret, Args...>::MemoizedFunction(const std::function<Ret(Args...)>& func):
    _func(func)
{}

template<typename Ret, typename... Args>
auto MemoizedFunction<Ret, Args...>::operator()(Args&&... args)
    -> Ret
{
    const auto t_args = std::make_tuple(args...);
    auto it = _cache.find(t_args);
    if (it == _cache.end())
    {
        it = _cache.emplace(std::piecewise_construct,
                            std::forward_as_tuple(t_args),
                            std::forward_as_tuple(_func(args...))
                        ).first;
    }
    return it->second;
}

template<typename Ret, typename... Args>
auto memoized(Ret (&func)(Args...))
    -> MemoizedFunction<Ret, Args...>
{
    return { std::function<Ret(Args...)>(func) };
}

template<typename Ret, typename... Args>
auto memoized(std::function<Ret(Args...)> func)
    -> MemoizedFunction<Ret, Args...>
{
    return { func };
}

template<typename Function, std::size_t... Ind>
auto memoized_impl(Function&& func, indices<Ind...>)
    -> MemoizedFunction<
        typename function_traits<typename std::remove_reference<Function>::type>::result_type,
        typename function_traits<typename std::remove_reference<Function>::type>::template arg<Ind>::type...>
{
    using Ret = typename function_traits<typename std::remove_reference<Function>::type>::result_type;
    return { std::function<Ret(typename function_traits<typename std::remove_reference<Function>::type>::template arg<Ind>::type...)>(func) };
}

template<typename Function, typename Indices=make_indices<function_traits<typename std::remove_reference<Function>::type>::arity>>
auto memoized(Function&& func)
    -> decltype(memoized_impl(std::forward<Function>(func), Indices()))
{
    return memoized_impl(std::forward<Function>(func), Indices());
}
