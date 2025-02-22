#include "webflex/accessible.hpp"
#include "webflex/impl/accessible_impl.hpp"
#include "webflex/core/utils.hpp"

namespace webflex {
    JsAccessible::JsAccessible() : m_impl(std::make_unique<impl::JsAccessibleImpl>()) {}

    JsAccessible::~JsAccessible() {
        m_members.clear();
    }

    void JsAccessible::exposeMemberNoReturnImpl(
        const std::string_view& name, 
        const std::function<void(const JsArguments&)>& callback,
        Launch policy
    ) {
        m_impl->add(name.data(), std::make_pair(policy, std::move(callback)));
    }
    
    void JsAccessible::exposeMemberWithReturnImpl(
        const std::string_view& name, 
        const std::function<core::js_any_t(const JsArguments&)>& callback,
        Launch policy
    ) {
        m_impl->add(name.data(), std::make_pair(policy, std::move(callback)));
    }

    JsAccessible::Field JsAccessible::exposeField(const std::string_view& name) {
        m_fields.push_back(name);
        return {};
    }
}
