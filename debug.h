#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <concepts>

/* ANSI color codes */
#define DBG_RESET  "\033[0m"
#define DBG_CYAN   "\033[36m"
#define DBG_YELLOW "\033[33m"
#define DBG_GREEN  "\033[32m"
#define DBG_GRAY   "\033[90m"
#define DBG_BLUE   "\033[34m"

namespace dbg_internal {

// Concepts for type checking
template<typename T>
concept Iterable = requires(T t) {
    { t.begin() } -> std::input_or_output_iterator;
    { t.end() } -> std::input_or_output_iterator;
};

template<typename T>
concept Pair = requires {
    typename T::first_type;
    typename T::second_type;
} && std::same_as<T, std::pair<typename T::first_type, typename T::second_type>>;

template<typename T>
concept MapLike = requires {
    typename T::key_type;
    typename T::mapped_type;
};

template<typename T>
concept Stack = requires(T t) {
    typename T::container_type;
    { t.top() };
    { t.pop() };
};

template<typename T>
concept Queue = requires(T t) {
    typename T::container_type;
    { t.front() };
    { t.pop() };
} && !Stack<T>;

template<typename T>
concept PriorityQueue = requires(T t) {
    typename T::container_type;
    typename T::value_compare;
    { t.top() };
    { t.pop() };
};

// Forward declaration
template<typename T>
std::string to_debug_string(const T& val);

// Print tuple helper
template<typename Tuple, size_t... Is>
std::string tuple_to_string_impl(const Tuple& t, std::index_sequence<Is...>) {
    std::ostringstream oss;
    oss << "(";
    ((oss << (Is == 0 ? "" : ", ") << to_debug_string(std::get<Is>(t))), ...);
    oss << ")";
    return oss.str();
}

template<typename... Types>
std::string tuple_to_string(const std::tuple<Types...>& t) {
    return tuple_to_string_impl(t, std::index_sequence_for<Types...>{});
}

// Stack printing helper
template<Stack T>
std::string stack_to_string(T stk) {
    std::ostringstream oss;
    oss << "[";

    bool first = true;
    while (!stk.empty()) {
        if (!first) oss << ", ";
        oss << to_debug_string(stk.top());
        stk.pop();
        first = false;
    }

    oss << "]";
    return oss.str();
}

// Queue printing helper
template<Queue T>
std::string queue_to_string(T q) {
    std::ostringstream oss;
    oss << "[";

    bool first = true;
    while (!q.empty()) {
        if (!first) oss << ", ";
        oss << to_debug_string(q.front());
        q.pop();
        first = false;
    }

    oss << "]";
    return oss.str();
}

// Priority queue printing helper
template<PriorityQueue T>
std::string priority_queue_to_string(T pq) {
    std::ostringstream oss;
    oss << "[";

    bool first = true;
    while (!pq.empty()) {
        if (!first) oss << ", ";
        oss << to_debug_string(pq.top());
        pq.pop();
        first = false;
    }

    oss << "]";
    return oss.str();
}

// Convert any value to debug string
template<typename T>
std::string to_debug_string(const T& val) {
    using Type = std::decay_t<T>;
    std::ostringstream oss;

    if constexpr (std::same_as<Type, bool>) {
        oss << (val ? "true" : "false");
    }
    else if constexpr (std::same_as<Type, char>) {
        if (std::isprint(static_cast<unsigned char>(val)))
            oss << "'" << val << "'";
        else
            oss << "'\\x" << std::hex << static_cast<int>(static_cast<unsigned char>(val)) << "'";
    }
    else if constexpr (std::same_as<Type, std::string> ||
                       std::same_as<Type, const char*> ||
                       std::same_as<Type, char*>) {
        oss << "\"" << val << "\"";
    }
    else if constexpr (std::is_pointer_v<Type>) {
        if (val == nullptr)
            oss << "nullptr";
        else
            oss << val;
    }
    else if constexpr (Pair<Type>) {
        oss << "(" << to_debug_string(val.first) << ", " << to_debug_string(val.second) << ")";
    }
    else if constexpr (requires { typename std::tuple_size<Type>::type; }) {
        // Tuple-like types
        oss << tuple_to_string(val);
    }
    else if constexpr (Stack<Type>) {
        oss << stack_to_string(val);
    }
    else if constexpr (PriorityQueue<Type>) {
        oss << priority_queue_to_string(val);
    }
    else if constexpr (Queue<Type>) {
        oss << queue_to_string(val);
    }
    else if constexpr (MapLike<Type>) {
        oss << "{";
        bool first = true;
        for (const auto& [key, value] : val) {
            if (!first) oss << ", ";
            oss << to_debug_string(key) << ": " << to_debug_string(value);
            first = false;
        }
        oss << "}";
    }
    else if constexpr (Iterable<Type>) {
        oss << "[";
        bool first = true;
        for (const auto& item : val) {
            if (!first) oss << ", ";
            oss << to_debug_string(item);
            first = false;
        }
        oss << "]";
    }
    else if constexpr (requires { oss << val; }) {
        // Fallback for types with << operator
        oss << val;
    }
    else {
        oss << "[unprintable type]";
    }

    return oss.str();
}

}

// Helper to print single variable
template<typename T>
void debug_single(const char* name, const T& val) {
    std::cerr << DBG_CYAN << "[debug]" << DBG_RESET << " "
              << DBG_YELLOW << name << DBG_RESET << " = "
              << DBG_GREEN << dbg_internal::to_debug_string(val) << DBG_RESET
              << "\n";
}

// Unified variadic debug implementation
template<typename... Args>
void debug_impl(const char* names, Args&&... args) {
    if constexpr (sizeof...(Args) == 1) {
        // Single argument - simple case
        (debug_single(names, args), ...);
    } else {
        // Multiple arguments - parse names using fixed-size arrays
        constexpr size_t MAX_NAMES = 32;
        std::string name_list[MAX_NAMES];
        size_t name_count = 0;

        std::string names_str(names);
        std::string current;
        int depth = 0;

        for (const char& c : names_str) {
            if (c == '<' || c == '(' || c == '[' || c == '{') depth++;
            else if (c == '>' || c == ')' || c == ']' || c == '}') depth--;
            else if (c == ',' && depth == 0) {
                while (!current.empty() && std::isspace(current.front())) current.erase(0, 1);
                while (!current.empty() && std::isspace(current.back())) current.pop_back();
                if (name_count < MAX_NAMES) {
                    name_list[name_count++] = current;
                }
                current.clear();
                continue;
            }
            current += c;
        }
        if (!current.empty()) {
            while (!current.empty() && std::isspace(current.front())) current.erase(0, 1);
            while (!current.empty() && std::isspace(current.back())) current.pop_back();
            if (name_count < MAX_NAMES) {
                name_list[name_count++] = current;
            }
        }

        size_t idx = 0;
        (void)((idx < name_count ? (debug_single(name_list[idx++].c_str(), args), 0) : 0), ...);
    }
}

// Unified debug macro - handles single or multiple arguments
#define debug(...) debug_impl(#__VA_ARGS__, __VA_ARGS__)

// Array debugging
template<typename T, size_t N>
void debug_array_impl(const char* name, const T (&arr)[N]) {
    std::cerr << DBG_CYAN << "[debug]" << DBG_RESET << " "
              << DBG_YELLOW << name << "[" << N << "]" << DBG_RESET << " = "
              << DBG_GREEN << "[";
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) std::cerr << ", ";
        std::cerr << dbg_internal::to_debug_string(arr[i]);
    }
    std::cerr << "]" << DBG_RESET << "\n";
}

#define debug_array(arr) debug_array_impl(#arr, arr)

// C-style array with length
template<typename T>
void debug_carray_impl(const char* name, const T* arr, const size_t len) {
    std::cerr << DBG_CYAN << "[debug]" << DBG_RESET << " "
              << DBG_YELLOW << name << "[" << len << "]" << DBG_RESET << " = "
              << DBG_GREEN << "[";
    for (size_t i = 0; i < len; ++i) {
        if (i > 0) std::cerr << ", ";
        std::cerr << dbg_internal::to_debug_string(arr[i]);
        if (i >= 19) { std::cerr << ", ..."; break; }
    }
    std::cerr << "]" << DBG_RESET << "\n";
}

#define debug_n(arr, n) debug_carray_impl(#arr, arr, n)

// Quick assertion
#define debug_assert(cond) do { \
    if (!(cond)) { \
        std::cerr << "\033[31m[ASSERT FAILED]\033[0m " \
                  << __FILE__ << ":" << __LINE__ << " - " << #cond << "\n"; \
        std::terminate(); \
    } \
} while(0)

// Function trace
#define DEBUG_TRACE() std::cerr << DBG_BLUE << "[trace]" << DBG_RESET \
                                << " " << __func__ << "() at " \
                                << __FILE__ << ":" << __LINE__ << "\n"

#endif
