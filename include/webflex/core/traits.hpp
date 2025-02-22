#pragma once

#include <tuple>
#include <format>
#include <functional>

namespace webflex::utils
{
    template <typename... Args>
    struct args_pack
    {
        std::tuple<Args...> args;

        explicit constexpr args_pack(Args &&...args_)
            : args(std::forward<Args>(args_)...) {}

        operator std::tuple<Args...>() const
        {
            return args;
        }
    };

    template <typename Tuple, std::size_t... Index>
    inline std::string format_with_tuple(const std::string &script, const Tuple &tuple, std::index_sequence<Index...>)
    {
        return std::vformat(script, std::make_format_args(std::get<Index>(tuple)...));
    }

    template <typename... _Args>
    inline std::string format_script(const std::string &script, const std::tuple<_Args...> &arguments)
    {
        return format_with_tuple(script, arguments, std::index_sequence_for<_Args...>{});
    }

    template <typename T>
    struct function_traits : function_traits<decltype(&T::operator())> {};

    template <typename R, typename... Args>
    struct function_traits<R (*)(Args...)>
    {
        using return_type = R;
        using arg_types = std::tuple<typename std::decay_t<Args>...>;
        using signature_t = R(Args...);
        static constexpr size_t arity = sizeof...(Args);

        template <size_t i>
        using arg = typename std::tuple_element<i, arg_types>::type;
    };

    template <typename R, typename... Args>
    struct function_traits<std::function<R(Args...)>>
    {
        using return_type = R;
        using arg_types = std::tuple<typename std::decay_t<Args>...>;
        using signature_t = R(Args...);
        static constexpr size_t arity = sizeof...(Args);

        template <size_t i>
        using arg = typename std::tuple_element<i, arg_types>::type;
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType (ClassType::*)(Args...)>
    {
        using return_type = ReturnType;
        using arg_types = std::tuple<typename std::decay_t<Args>...>;
        using signature_t = ReturnType(Args...);
        static constexpr size_t arity = sizeof...(Args);

        template <size_t i>
        using arg = typename std::tuple_element<i, arg_types>::type;
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType (ClassType::*)(Args...) const>
    {
        using return_type = ReturnType;
        using arg_types = std::tuple<typename std::decay_t<Args>...>;
        using signature_t = ReturnType(Args...);
        static constexpr size_t arity = sizeof...(Args);

        template <size_t i>
        using arg = typename std::tuple_element<i, arg_types>::type;
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType (ClassType::*)(Args...) noexcept>
    {
        using return_type = ReturnType;
        using arg_types = std::tuple<typename std::decay_t<Args>...>;
        using signature_t = ReturnType(Args...);
        static constexpr size_t arity = sizeof...(Args);

        template <size_t i>
        using arg = typename std::tuple_element<i, arg_types>::type;
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType (ClassType::*)(Args...) const noexcept>
    {
        using return_type = ReturnType;
        using arg_types = std::tuple<typename std::decay_t<Args>...>;
        using signature_t = ReturnType(Args...);
        static constexpr size_t arity = sizeof...(Args);

        template <size_t i>
        using arg = typename std::tuple_element<i, arg_types>::type;
    };

    template <typename ReturnType, typename Callable, typename... BoundArgs>
    struct function_traits<std::_Bind<ReturnType(Callable, BoundArgs...)>>
    {
        using return_type = ReturnType;
        using arg_types = std::tuple<std::decay_t<BoundArgs>...>;
        using signature_t = ReturnType(BoundArgs...);
        static constexpr size_t arity = sizeof...(BoundArgs);

        template <size_t i>
        using arg = typename std::tuple_element<i, arg_types>::type;
    };

    template <typename F>
    using function_signature_t = typename function_traits<F>::signature_t;

    template <typename Function, typename Tuple, std::size_t... I>
    auto call_with_tuple(Function &&function, Tuple &&tuple, std::index_sequence<I...>)
        -> decltype(function(std::get<I>(std::forward<Tuple>(tuple))...))
    {
        return function(std::get<I>(std::forward<Tuple>(tuple))...);
    }

    template <typename Function, typename ArgsTuple>
    auto unpack_and_call(Function &&function, ArgsTuple &&unpacked_args)
        -> decltype(call_with_tuple(
            std::forward<Function>(function),
            std::forward<ArgsTuple>(unpacked_args),
            std::make_index_sequence<std::tuple_size_v<std::decay_t<ArgsTuple>>>{}))
    {
        constexpr size_t args_count = std::tuple_size_v<std::decay_t<ArgsTuple>>;
        return call_with_tuple(
            std::forward<Function>(function),
            std::forward<ArgsTuple>(unpacked_args),
            std::make_index_sequence<args_count>{});
    }
}

namespace webflex
{
    template <typename... Args>
    inline constexpr utils::args_pack<Args...> make_args(Args &&...args)
    {
        return utils::args_pack<Args...>(std::forward<Args>(args)...);
    }
}