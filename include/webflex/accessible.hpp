#ifndef JSACCESSIBLE_HPP
#define JSACCESSIBLE_HPP

#include <memory>
#include "js_args.hpp"
#include "core/traits.hpp"
#include "defines.hpp"

#define JS_ACCESSIBLE_METHOD(METHOD, ...)                                                                                                                                   \
    Method method__##METHOD = exposeMethod(#METHOD, std::function<typename webflex::utils::function_signature_t<decltype(&ClassType::METHOD)>>([this](auto &&...args) {     \
        return std::invoke(&ClassType::METHOD, *this, std::forward<decltype(args)>(args)...);                                                                               \
    }), ##__VA_ARGS__);

#define JS_ACCESSIBLE_FIELD(FIELD)                                                                                         \
    Field field__##FIELD = [this]() -> Field {                                                                             \
        exposeMethod("get__" #FIELD, [this]() -> decltype(ClassType::FIELD) { return this->FIELD; });                      \
        exposeMethod("set__" #FIELD, [this](const decltype(ClassType::FIELD)& new_value) { this->FIELD = new_value; });    \
        return exposeField(#FIELD);                                                                                        \
    }();

#define JS_DECLARE_TYPE(CLASS)  \
    using ClassType = CLASS;

namespace webflex
{
    namespace impl { class JsAccessibleImpl; }

    class JsAccessible : public std::enable_shared_from_this<JsAccessible>
    {
    public:
        JsAccessible();
        ~JsAccessible();

    protected:
        struct Field {};
        struct Method {};

        template <typename Function>
        Method exposeMethod(const std::string_view &name, Function &&function, Launch police = Launch::Sync);

        Field exposeField(const std::string_view &name);

    private:
        const std::unordered_map<std::string_view, Launch> &methods() const { return m_members; }

        const std::vector<std::string_view>& fields() const { return m_fields; }

        void exposeMemberWithReturnImpl(const std::string_view &name, const std::function<core::js_any_t(const JsArguments &)> &callback, Launch police = Launch::Sync);

        void exposeMemberNoReturnImpl(const std::string_view &name, const std::function<void(const JsArguments &)> &callback, Launch police = Launch::Sync);

        const std::unique_ptr<impl::JsAccessibleImpl> &impl() const { return m_impl; }

        friend class Browser;

    private:
        std::unique_ptr<impl::JsAccessibleImpl> m_impl;
        std::unordered_map<std::string_view, Launch> m_members;
        std::vector<std::string_view> m_fields;
    };

    template <typename Function>
    JsAccessible::Method JsAccessible::exposeMethod(const std::string_view &name, Function &&function, Launch police)
    {
        using args_tuple = typename utils::function_traits<Function>::arg_types;
        using return_type = typename utils::function_traits<Function>::return_type;

        auto callback = [function = std::forward<Function>(function)](const webflex::JsArguments &js_args)
        {
            constexpr size_t args_count = std::tuple_size_v<args_tuple>;

            return utils::unpack_and_call(function,
                [&]<std::size_t... I>(std::index_sequence<I...>)
                {
                    return std::tuple<std::tuple_element_t<I, args_tuple>...>(
                        (utils::extract_value<std::tuple_element_t<I, args_tuple>>(js_args, I))...);
                }(std::make_index_sequence<args_count>{})
            );
        };
 
        if constexpr (!std::is_void_v<return_type>)
        {
            exposeMemberWithReturnImpl(name, std::move(callback));
        }
        else
        {
            exposeMemberNoReturnImpl(name, std::move(callback));
        }

        m_members.emplace(std::make_pair(name, police));

        return Method{};
    }
}

#endif // JSACCESSIBLE_HPP
