#pragma once

#include <variant>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <string>

namespace webflex::core {
    using js_value_t = std::variant<std::monostate, int, double, bool, std::string>;
    using js_list_t = std::vector<js_value_t>;
    using js_dict_t = std::unordered_map<std::string, js_value_t>;
    using js_any_t = std::variant<std::monostate, js_value_t, js_list_t, js_dict_t>;
}
