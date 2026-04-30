#pragma once

#if __cplusplus < 202302L
#error out of date c++ version, compile with -stdc++=2c
#elif defined(__clang__) && __clang_major__ < 22
#error out of date clang, compile with latest version
#elif !defined(__clang__) && defined(__GNUC__) && __GNUC__ < 14
#error out of date g++, compile with latest version
#elif defined(_MSC_VER) && _MSC_VER < 19
#error out of date msvc, compile with latest version
#elif !defined(__clang__) && !defined(__GNUC__) && !defined(_MSC_VER)
#error compiler unknown, could not detect gcc, clang, or msvc
#else

#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace bit {
    namespace detail {
        template<
            typename tp_first_t,
            class... tp_rest_ts
        >
        struct first_type_impl : std::type_identity<tp_first_t> {};

        // remove when pack indexing is universally supported, *arm-gcc*
        template<class... tp_types_ts>
        using first_type_t = typename first_type_impl<tp_types_ts...>::type;
        
        template<
            typename tp_type1_t,
            typename tp_type2_t
        >
        concept different_to =
            !std::same_as<
                tp_type1_t,
                tp_type2_t
            >;

        template<
            typename tp_type1_t,
            typename tp_type2_t
        >
        auto constexpr equal_size_of = sizeof(tp_type1_t) == sizeof(tp_type2_t);

        template<typename tp_type_t>
        concept no_const = !std::is_const_v<std::remove_reference_t<tp_type_t>>;
        
        template<int tp_value>
        auto constexpr must_only_be_called_by_closure = tp_value != 0;

        struct integer_adapter_closure_base {};

        template<
            typename tp_adapter_t,
            class... tp_types_ts
        >
        struct partial : integer_adapter_closure_base {
        private:
            std::tuple<tp_types_ts...> m_elements;
            
            template<
                typename       tp_self_t,
                typename       tp_integral_t,
                std::size_t... tp_is
            >
            auto constexpr impl(
                this tp_self_t&& p_self,
                std::index_sequence<tp_is...>,
                tp_integral_t&&  p_value
            )
            noexcept(noexcept(
                tp_adapter_t{}.template operator()<1>(
                    std::declval<tp_integral_t>(),
                    std::get<tp_is>(std::declval<tp_self_t>().m_elements)...
                )
            ))
            -> decltype(
                tp_adapter_t{}.template operator()<1>(
                    std::forward<tp_integral_t>(p_value),
                    std::get<tp_is>(std::forward<tp_self_t>(p_self).m_elements)...
                )
            ) {
                return tp_adapter_t{}.template operator()<1>(
                    std::forward<tp_integral_t>(p_value),
                    std::get<tp_is>(std::forward<tp_self_t>(p_self).m_elements)...
                );
            }
        public:
            template<
                typename tp_self_t,
                typename tp_integral_t
            >
            auto constexpr operator()[[nodiscard]](
                this tp_self_t&& p_self,
                tp_integral_t&&  p_value
            )
            noexcept(noexcept(
                std::declval<tp_self_t>().impl(
                    std::index_sequence_for<tp_types_ts...>{},
                    std::declval<tp_integral_t>()
                )
            ))
            -> decltype(
                std::forward<tp_self_t>(p_self).impl(
                    std::index_sequence_for<tp_types_ts...>{},
                    std::forward<tp_integral_t>(p_value)
                )
            ) {
                return std::forward<tp_self_t>(p_self).impl(
                    std::index_sequence_for<tp_types_ts...>{},
                    std::forward<tp_integral_t>(p_value)
                );
            }

            template<class... tp_arguments_ts>
            requires(
                std::constructible_from<
                    std::tuple<tp_arguments_ts...>,
                    tp_arguments_ts...
                >
            )
            constexpr explicit partial(
                decltype(std::ignore),
                tp_arguments_ts&&... p_arguments
            )
            noexcept(
                std::is_nothrow_constructible_v<
                    std::tuple<tp_arguments_ts...>,
                    tp_arguments_ts...
                >
            )
            : m_elements{std::forward<tp_arguments_ts>(p_arguments)...}
            {}
        };

        struct integer_adapter_base {
            template<
                int      tp_overload = 2,
                typename tp_self_t,
                class... tp_arguments_ts
            >
            requires(
                tp_overload == 2 &&
                different_to<
                    std::remove_cvref_t<tp_self_t>,
                    integer_adapter_base
                > &&
                requires(
                    tp_self_t&&                       p_self,
                    first_type_t<tp_arguments_ts...>& p_first_value,
                    tp_arguments_ts&&...              p_arguments
                ) {
                    std::forward<tp_self_t>(p_self).template operator()<1>(
                        p_first_value,
                        std::forward<tp_arguments_ts>(p_arguments)...
                    );
                }
            )
            auto constexpr operator()(
                this tp_self_t,
                tp_arguments_ts&&... p_arguments
            )
            noexcept(noexcept(
                partial<
                    std::remove_cvref_t<tp_self_t>,
                    tp_arguments_ts...
                >{
                    std::ignore,
                    std::declval<tp_arguments_ts>()...
                }
            ))
            -> decltype(
                partial<
                    std::remove_cvref_t<tp_self_t>,
                    tp_arguments_ts...
                >{
                    std::ignore,
                    std::forward<tp_arguments_ts>(p_arguments)...
                }
            ) {
                return partial<
                    std::remove_cvref_t<tp_self_t>,
                    tp_arguments_ts...
                >{
                    std::ignore,
                    std::forward<tp_arguments_ts>(p_arguments)...
                };
            }
        };

        template<typename tp_type_t>
        concept integer_adapter_closure = std::derived_from<
            std::remove_cvref_t<tp_type_t>,
            integer_adapter_closure_base
        >;

        template<
            integer_adapter_closure tp_integer_adapter_closure_left_t,
            integer_adapter_closure tp_integer_adapter_closure_right_t
        >
        struct pipe : integer_adapter_closure_base {
        private:
            [[no_unique_address]] tp_integer_adapter_closure_left_t  m_left;
            [[no_unique_address]] tp_integer_adapter_closure_right_t m_right;
        public:
            template<
                typename tp_integral_t,
                typename tp_self_t
            >
            auto constexpr operator()[[nodiscard]](
                this tp_self_t&& p_self,
                tp_integral_t&&  p_value
            )
            noexcept(noexcept(
                std::invoke(
                    std::declval<tp_self_t>().m_right,
                    std::invoke(
                        std::declval<tp_self_t>().m_left,
                        std::declval<tp_integral_t>()
                    )
                )
            ))
            -> decltype(
                std::invoke(
                    std::forward<tp_self_t>(p_self).m_right,
                    std::invoke(
                        std::forward<tp_self_t>(p_self).m_left,
                        std::forward<tp_integral_t>(p_value)
                    )
                )
            ) {
                return std::invoke(
                    std::forward<tp_self_t>(p_self).m_right,
                    std::invoke(
                        std::forward<tp_self_t>(p_self).m_left,
                        std::forward<tp_integral_t>(p_value)
                    )
                );
            }
            
            template<
                integer_adapter_closure tp_integer_adapter_closure_left_argument_t,
                integer_adapter_closure tp_integer_adapter_closure_right_argument_t
            >
            requires(
                std::constructible_from<
                    tp_integer_adapter_closure_left_t,
                    tp_integer_adapter_closure_left_argument_t
                > &&
                std::constructible_from<
                    tp_integer_adapter_closure_right_t,
                    tp_integer_adapter_closure_right_argument_t
                >
            )
            constexpr explicit pipe(
                tp_integer_adapter_closure_left_argument_t&&  p_left,
                tp_integer_adapter_closure_right_argument_t&& p_right
            )
            noexcept(
                std::is_nothrow_constructible_v<
                    tp_integer_adapter_closure_left_t,
                    tp_integer_adapter_closure_left_argument_t
                > &&
                std::is_nothrow_constructible_v<
                    tp_integer_adapter_closure_right_t,
                    tp_integer_adapter_closure_right_argument_t
                >
            ) :
                m_left{std::forward<tp_integer_adapter_closure_left_argument_t>(p_left)},
                m_right{std::forward<tp_integer_adapter_closure_right_argument_t>(p_right)}
            {}
        };
    }
}

template<
    typename                             tp_integral_t,
    bit::detail::integer_adapter_closure tp_integer_adapter_closure_t
>
[[nodiscard]]
auto constexpr operator|(
    const tp_integral_t            p_value,
    tp_integer_adapter_closure_t&& p_integer_adapter_closure
)
noexcept(
    std::is_nothrow_invocable_v<
        tp_integer_adapter_closure_t,
        tp_integral_t
    >
)
-> std::invoke_result_t<
    tp_integer_adapter_closure_t,
    tp_integral_t
> {
    return std::invoke(
        std::forward<tp_integer_adapter_closure_t>(p_integer_adapter_closure),
        p_value
    );
}

template<
    bit::detail::no_const                tp_integral_t,
    bit::detail::integer_adapter_closure tp_integer_adapter_closure_t
>
requires(
    std::invocable<
        tp_integer_adapter_closure_t,
        tp_integral_t
    >
)
[[maybe_unused]]
auto constexpr operator|=(
    tp_integral_t&                 p_value,
    tp_integer_adapter_closure_t&& p_integer_adapter_closure
)
noexcept(
    std::is_nothrow_invocable_v<
        tp_integer_adapter_closure_t,
        tp_integral_t
    >
)
-> std::conditional_t<
    std::is_volatile_v<tp_integral_t>,
    void,
    tp_integral_t&
> {
    p_value = std::invoke(
        std::forward<tp_integer_adapter_closure_t>(p_integer_adapter_closure),
        p_value
    );
    if constexpr (!std::is_volatile_v<tp_integral_t>)
        return p_value;
}

template<
    bit::detail::integer_adapter_closure tp_integer_adapter_closure1_t,
    bit::detail::integer_adapter_closure tp_integer_adapter_closure2_t
>
requires(
    std::constructible_from<
        bit::detail::pipe<
            std::remove_cvref_t<tp_integer_adapter_closure1_t>,
            std::remove_cvref_t<tp_integer_adapter_closure2_t>
        >,
        tp_integer_adapter_closure1_t,
        tp_integer_adapter_closure2_t
    >
)
[[nodiscard]]
auto constexpr operator|(
    tp_integer_adapter_closure1_t&& p_integer_adapter_closure1,
    tp_integer_adapter_closure2_t&& p_integer_adapter_closure2
)
noexcept(
    std::is_nothrow_constructible_v<
        bit::detail::pipe<
            std::remove_cvref_t<tp_integer_adapter_closure1_t>,
            std::remove_cvref_t<tp_integer_adapter_closure2_t>
        >,
        tp_integer_adapter_closure1_t,
        tp_integer_adapter_closure2_t
    >
)
-> bit::detail::pipe<
    std::remove_cvref_t<tp_integer_adapter_closure1_t>,
    std::remove_cvref_t<tp_integer_adapter_closure2_t>
> {
    return bit::detail::pipe<
        std::remove_cvref_t<tp_integer_adapter_closure1_t>,
        std::remove_cvref_t<tp_integer_adapter_closure2_t>
    >{
        std::forward<tp_integer_adapter_closure1_t>(p_integer_adapter_closure1),
        std::forward<tp_integer_adapter_closure2_t>(p_integer_adapter_closure2)
    };
}

namespace bit {
    template<typename tp_type_t>
    auto constexpr bit_size_of = sizeof(tp_type_t) * 8;

    template<typename tp_type_t>
    auto constexpr half_word_bit_size_of = bit_size_of<tp_type_t> / 2;

    namespace detail {
        template<
            typename tp_first_t,
            class... tp_rest_ts
        >
        auto constexpr integrals_of_matching_signedness_impl =
            std::integral<tp_first_t> &&
            (... && std::integral<tp_rest_ts>) &&
            (... && (std::signed_integral<tp_first_t> == std::signed_integral<tp_rest_ts>));    
            
        template<class... tp_types_ts>
        concept integrals_of_matching_signedness = detail::integrals_of_matching_signedness_impl<tp_types_ts...>;
    }

    namespace detail {
        struct shift_left_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value1 << p_value2);
            }
        };
    }
    auto constexpr shift_left = detail::shift_left_fn{};

    namespace detail {
        struct shift_right_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value1 >> p_value2);
            }
        };
    }
    auto constexpr shift_right = detail::shift_right_fn{};

    namespace detail {
        struct bit_and_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value1 & p_value2);
            }
        };
    }
    auto constexpr bit_and = detail::bit_and_fn{};

    namespace detail {
        struct bit_xor_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value1 ^ p_value2);
            }
        };
    }
    auto constexpr bit_xor = detail::bit_and_fn{};

    namespace detail {
        struct bit_or_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value1 | p_value2);
            }
        };
    }
    auto constexpr bit_or = detail::bit_and_fn{};

    namespace detail {
        struct align_by_weak_shift_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_old_from_index,
                const tp_integral3_t p_new_from_index
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(
                    std::cmp_greater(p_new_from_index, p_old_from_index) ?
                    p_value << (p_new_from_index - p_old_from_index) :
                    p_value >> (p_old_from_index - p_new_from_index)
                );
            }
        };
    }
    auto constexpr align_by_weak_shift = detail::align_by_weak_shift_fn{};

    namespace detail {
        struct align_by_strong_shift_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_old_from_index,
                const tp_integral3_t p_new_from_index
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(
                    p_new_from_index >> p_old_from_index << p_new_from_index
                );
            }
        };
    }
    auto constexpr align_by_strong_shift = detail::align_by_strong_shift_fn{};

    namespace detail {
        struct enable_in_range_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index,
                const tp_integral3_t p_to_index
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value | (((1 << (p_to_index - p_from_index + 1)) - 1) << p_from_index));
            }
        };
    }
    auto constexpr enable_in_range = detail::enable_in_range_fn{};

    namespace detail {
        struct disable_in_range_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index,
                const tp_integral3_t p_to_index
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value & ~(((1 << (p_to_index - p_from_index + 1)) - 1) << p_from_index));
            }
        };
    }
    auto constexpr disable_in_range = detail::disable_in_range_fn{};

    namespace detail {
        struct set_in_range_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index,
                const tp_integral3_t p_to_index,
                const bool           p_state
            )
            const noexcept
            -> tp_integral1_t {
                return p_state ?
                    enable_in_range.template operator()<1>(
                        p_value,
                        p_from_index,
                        p_to_index
                    ) :
                    disable_in_range.template operator()<1>(
                        p_value,
                        p_from_index,
                        p_to_index
                    );
            }
        };
    }
    auto constexpr set_in_range = detail::set_in_range_fn{};

    namespace detail {
        struct enable_from_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index
            )
            const noexcept
            -> tp_integral1_t {
                return enable_in_range.template operator()<1>(
                    p_value,
                    p_from_index,
                    bit_size_of<tp_integral1_t> - 1
                );
            }

            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index,
                const tp_integral3_t p_count
            )
            const noexcept
            -> tp_integral1_t {
                return enable_in_range.template operator()<1>(
                    p_value,
                    p_from_index,
                    p_from_index + p_count
                );
            }
        };
    }
    auto constexpr enable_from = detail::enable_from_fn{};

    namespace detail {
        struct disable_from_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index
            )
            const noexcept
            -> tp_integral1_t {
                return disable_in_range.template operator()<1>(
                    p_value,
                    p_from_index,
                    bit_size_of<tp_integral1_t> - 1
                );
            }

            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index,
                const tp_integral3_t p_count
            )
            const noexcept
            -> tp_integral1_t {
                return disable_in_range.template operator()<1>(
                    p_value,
                    p_from_index,
                    p_from_index + p_count
                );
            }
        };
    }
    auto constexpr disable_from = detail::disable_from_fn{};

    namespace detail {
        struct set_from_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index,
                const bool           p_state
            )
            const noexcept
            -> tp_integral1_t {
                return p_state ?
                    enable_from.template operator()<1>(
                        p_value,
                        p_from_index
                    ) :
                    disable_from.template operator()<1>(
                        p_value,
                        p_from_index
                    );
            }

            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index,
                const tp_integral3_t p_count,
                const bool           p_state
            )
            const noexcept
            -> tp_integral1_t {
                return p_state ?
                    enable_from.template operator()<1>(
                        p_value,
                        p_from_index,
                        p_count
                    ) :
                    disable_from.template operator()<1>(
                        p_value,
                        p_from_index,
                        p_count
                    );
            }
        };
    }
    auto constexpr set_from = detail::set_from_fn{};

    namespace detail {
        struct enable_from_right_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_count
            )
            const noexcept
            -> tp_integral1_t {
                return enable_from.template operator()<1>(
                    p_value,
                    tp_integral1_t{0},
                    p_count
                );
            }
        };
    }
    auto constexpr enable_from_right = detail::enable_from_right_fn{};
    
    namespace detail {
        struct enable_from_left_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_count
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(
                    enable_from_right.template operator()<1>(
                        p_value,
                        p_count
                    ) <<
                    (bit_size_of<tp_integral1_t> - p_count));
            }
        };
    }
    auto constexpr enable_from_left = detail::enable_from_left_fn{};

    namespace detail {
        struct disable_from_right_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_count
            )
            const noexcept
            -> tp_integral1_t {
                return disable_from.template operator()<1>(
                    p_value,
                    tp_integral1_t{0},
                    p_count
                );
            }
        };
    }
    auto constexpr disable_from_right = detail::disable_from_right_fn{};
    
    namespace detail {
        struct disable_from_left_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_count
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(
                    p_value &
                    enable_from_right.template operator()<1>(
                        tp_integral1_t{},
                        p_count)
                    );
            }
        };
    }
    auto constexpr disable_from_left = detail::disable_from_left_fn{};

    namespace detail {
        struct set_from_right_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_count,
                const bool           p_state
            )
            const noexcept
            -> tp_integral1_t {
                return p_state ?
                    enable_from_right.template operator()<1>(
                        p_value,
                        p_count
                    ) :
                    disable_from_right.template operator()<1>(
                        p_value,
                        p_count
                    );
            }
        };
    }
    auto constexpr set_from_right = detail::set_from_right_fn{};
    
    namespace detail {
        struct set_from_left_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_count,
                const bool           p_state
            )
            const noexcept
            -> tp_integral1_t {
                return p_state ?
                    enable_from_left.template operator()<1>(
                        p_value,
                        p_count
                    ) :
                    disable_from_left.template operator()<1>(
                        p_value,
                        p_count
                    );
            }
        };
    }
    auto constexpr set_from_left = detail::set_from_left_fn{};

    namespace detail {
        template<std::integral tp_result_t>
        struct integers_to_mask_fn {
            template<std::integral... tp_integral_ts>
            requires(
                integrals_of_matching_signedness<tp_integral_ts...> &&
                sizeof...(tp_integral_ts) <= bit_size_of<tp_result_t>
            )
            [[nodiscard]]
            auto constexpr operator()(const tp_integral_ts... p_indices)
            const noexcept
            -> tp_result_t {
                auto l_result = tp_result_t{};
                auto l_index = std::size_t{};
                (... , (l_result |= (p_indices << l_index), ++l_index));
                return l_result;
            }
        };
    }
    template<std::integral tp_result_t>
    auto constexpr integers_to_mask = detail::integers_to_mask_fn<tp_result_t>{};

    namespace detail {
        struct decimal_to_mask_fn {
            template<
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_count
            )
            const noexcept
            -> tp_integral1_t {
                auto l_result = tp_integral1_t{};
                for (auto i = std::size_t{}; i < p_count; ++i) {
                    auto l_digit = p_value % 10;
                    p_value /= 10;
                    if (l_digit)
                        l_result |= 1 << i;
                }
                return l_result;
            }
        };
    }
    auto constexpr decimal_to_mask = detail::decimal_to_mask_fn{};

    namespace detail {
        struct enable_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral_t,
                class... tp_integral_ts
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral_t,
                    tp_integral_ts...
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral_t     p_value,
                const tp_integral_ts... p_indices
            )
            const noexcept
            -> tp_integral_t {
                return static_cast<tp_integral_t>(p_value | integers_to_mask<tp_integral_t>(p_indices...));
            }
        };
    }
    auto constexpr enable = detail::enable_fn{};

    namespace detail {
        struct disable_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral_t,
                class... tp_integral_ts
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral_t,
                    tp_integral_ts...
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral_t     p_value,
                const tp_integral_ts... p_indices
            )
            const noexcept
            -> tp_integral_t {
                return static_cast<tp_integral_t>(p_value & ~integers_to_mask<tp_integral_t>(p_indices...));
            }
        };
    }
    auto constexpr disable = detail::disable_fn{};
    
    namespace detail {
        struct set_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral_t,
                class... tp_integral_ts
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral_t,
                    tp_integral_ts...
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral_t     p_value,
                const bool              p_state,
                const tp_integral_ts... p_indices
            )
            const noexcept
            -> tp_integral_t {
                return p_state ?
                    enable.template operator()<1>(
                        p_value,
                        p_indices...
                    ) :
                    disable.template operator()<1>(
                        p_value,
                        p_indices...
                    );
            }
        };
    }
    auto constexpr set = detail::set_fn{};
    
    namespace detail {
        struct extract_in_range_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index,
                const tp_integral3_t p_to_index
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(
                    p_value &
                    enable_in_range.template operator()<1>(
                        tp_integral1_t{},
                        p_from_index,
                        p_to_index
                    )
                );
            }
        };
    }
    auto constexpr extract_in_range = detail::extract_in_range_fn{};

    namespace detail {
        struct extract_from_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index
            )
            const noexcept
            -> tp_integral1_t {
                return extract_in_range.template operator()<1>(
                    p_value,
                    p_from_index,
                    bit_size_of<tp_integral1_t> - 1
                );
            }

            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index,
                const tp_integral3_t p_count
            )
            const noexcept
            -> tp_integral1_t {
                return extract_in_range.template operator()<1>(
                    p_value,
                    p_from_index,
                    p_from_index + p_count
                );
            }
        };
    }
    auto constexpr extract_from = detail::extract_from_fn{};

    namespace detail {
        struct extract_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral_t,
                class... tp_integral_ts
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral_t,
                    tp_integral_ts...
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral_t     p_value,
                const tp_integral_ts... p_indices
            )
            const noexcept
            -> tp_integral_t {
                return static_cast<tp_integral_t>(p_value & integers_to_mask<tp_integral_t>(p_indices...));
            }
        };
    }
    auto constexpr extract = detail::extract_fn{};

    namespace detail {
        struct extract_low_word_fn : integer_adapter_closure_base {
            template<std::integral tp_integral_t>
            [[nodiscard]]
            auto constexpr operator()(const tp_integral_t p_value)
            const noexcept
            -> tp_integral_t {
                return extract_from.template operator()<1>(
                    p_value,
                    tp_integral_t{0},
                    bit_size_of<tp_integral_t>
                );
            }
        };
    }
    auto constexpr extract_low_word = detail::extract_low_word_fn{};

    namespace detail {
        struct extract_high_word_fn : integer_adapter_closure_base {
            template<std::integral tp_integral_t>
            [[nodiscard]]
            auto constexpr operator()(const tp_integral_t p_value)
            const noexcept
            -> tp_integral_t {
                return extract_from.template operator()<1>(
                    p_value,
                    bit_size_of<tp_integral_t>
                );
            }
        };
    }
    auto constexpr extract_high_word = detail::extract_high_word_fn{};

    namespace detail {
        struct get_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral_t,
                class... tp_integral_ts
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral_t,
                    tp_integral_ts...
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral_t     p_value,
                const tp_integral_ts... p_indices
            )
            const noexcept
            -> bool {
                return static_cast<bool>(extract.template operator()<1>(
                    p_value,
                    p_indices...
                ));
            }
        };
    }
    auto constexpr get = detail::get_fn{};

    namespace detail {
        struct enable_low_word_fn : integer_adapter_closure_base  {
            template<std::integral tp_integral_t>
            [[nodiscard]]
            auto constexpr operator()(const tp_integral_t p_value)
            const noexcept
            -> tp_integral_t {
                return enable_from_right.template operator()<1>(
                    p_value,
                    bit_size_of<tp_integral_t> / 2
                );
            }
        };
    }
    auto constexpr enable_low_word = detail::enable_low_word_fn{};

    namespace detail {
        struct disable_low_word_fn : integer_adapter_closure_base  {
            template<std::integral tp_integral_t>
            [[nodiscard]]
            auto constexpr operator()(const tp_integral_t p_value)
            const noexcept
            -> tp_integral_t {
                return disable_from_right.template operator()<1>(
                    p_value,
                    bit_size_of<tp_integral_t> / 2
                );
            }
        };
    }
    auto constexpr disable_low_word = detail::disable_low_word_fn{};

    namespace detail {
        struct enable_high_word_fn : integer_adapter_closure_base {
            template<std::integral tp_integral_t>
            [[nodiscard]]
            auto constexpr operator()(const tp_integral_t p_value)
            const noexcept
            -> tp_integral_t {
                return enable_from_left.template operator()<1>(
                    p_value,
                    bit_size_of<tp_integral_t> / 2
                );
            }
        };
    }
    auto constexpr enable_high_word = detail::enable_high_word_fn{};

    namespace detail {
        struct disable_high_word_fn : integer_adapter_closure_base {
            template<std::integral tp_integral_t>
            [[nodiscard]]
            auto constexpr operator()(const tp_integral_t p_value)
            const noexcept
            -> tp_integral_t {
                return disable_from_left.template operator()<1>(
                    p_value,
                    bit_size_of<tp_integral_t> / 2
                );
            }
        };
    }
    auto constexpr disable_high_word = detail::disable_high_word_fn{};
    
    template<std::integral tp_integral_t>
    auto constexpr low_word_mask = enable_from_right(
        tp_integral_t{},
        bit_size_of<tp_integral_t> / 2
    );

    template<std::integral tp_integral_t>
    auto constexpr high_word_mask = static_cast<tp_integral_t>(~low_word_mask<tp_integral_t>);

   namespace detail {
        struct copy_in_range_to_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t,
                typename tp_integral5_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t,
                    tp_integral4_t,
                    tp_integral5_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                const tp_integral2_t p_source,
                const tp_integral3_t p_from_index,
                const tp_integral4_t p_to_index,
                const tp_integral5_t p_output_index
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(
                    disable_in_range.template operator()<1>(
                        p_destination,
                        p_output_index,
                        p_output_index + p_to_index - p_from_index
                    ) |
                    align_by_strong_shift.template operator()<1>(
                        extract_in_range.template operator()<1>(
                            p_source,
                            p_from_index,
                            p_to_index
                        ),
                        p_from_index,
                        p_output_index
                    )
                );
            }
        };
    }
    auto constexpr copy_in_range_to = detail::copy_in_range_to_fn{};

    namespace detail {
        struct copy_in_range_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t,
                    tp_integral4_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                const tp_integral2_t p_source,
                const tp_integral3_t p_from_index,
                const tp_integral4_t p_to_index
            )
            const noexcept
            -> tp_integral1_t {
                return copy_in_range_to.template operator()<1>(
                    p_destination,
                    p_source,
                    p_from_index,
                    p_to_index,
                    p_from_index
                );
            }
        };
    }
    auto constexpr copy_in_range = detail::copy_in_range_fn{};

    namespace detail {
        struct copy_from_to_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t,
                    tp_integral4_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                const tp_integral2_t p_source,
                const tp_integral3_t p_from_index,
                const tp_integral4_t p_output_index
            )
            const noexcept
            -> tp_integral1_t {
                return copy_in_range_to.template operator()<1>(
                    p_destination,
                    p_source,
                    p_from_index,
                    bit_size_of<tp_integral1_t>,
                    p_output_index
                );
            }

            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t,
                typename tp_integral5_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t,
                    tp_integral4_t,
                    tp_integral5_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                const tp_integral2_t p_source,
                const tp_integral3_t p_from_index,
                const tp_integral4_t p_count,
                const tp_integral5_t p_output_index
            )
            const noexcept
            -> tp_integral1_t {
                return copy_in_range_to.template operator()<1>(
                    p_destination,
                    p_source,
                    p_from_index,
                    p_from_index + p_count,
                    p_output_index
                );
            }
        };
    }
    auto constexpr copy_from_to = detail::copy_from_to_fn{};

    namespace detail {
        struct copy_from_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                const tp_integral2_t p_source,
                const tp_integral3_t p_from_index
            )
            const noexcept
            -> tp_integral1_t {
                return copy_in_range.template operator()<1>(
                    p_destination,
                    p_source,
                    p_from_index,
                    bit_size_of<tp_integral1_t> - 1
                );
            }
            
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t,
                    tp_integral4_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                const tp_integral2_t p_source,
                const tp_integral3_t p_from_index,
                const tp_integral4_t p_count
            )
            const noexcept
            -> tp_integral1_t {
                return copy_in_range.template operator()<1>(
                    p_destination,
                    p_source,
                    p_from_index,
                    p_from_index + p_count
                );
            }
        };
    }
    auto constexpr copy_from = detail::copy_from_fn{};

    namespace detail {
        struct copy_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                class... tp_integral_ts
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral_ts...
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t    p_destination,
                const tp_integral2_t    p_source,
                const tp_integral_ts... p_indices
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(
                    disable.template operator()<1>(
                        p_destination,
                        p_indices...
                    ) |
                    extract.template operator()<1>(
                        p_source,
                        p_indices...
                    )
                );
            }
        };
    }
    auto constexpr copy = detail::copy_fn{};

    namespace detail {
        struct move_in_range_to_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t,
                typename tp_integral5_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>,
                    tp_integral3_t,
                    tp_integral4_t,
                    tp_integral5_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                tp_integral2_t&      p_source,
                const tp_integral3_t p_from_index,
                const tp_integral4_t p_to_index,
                const tp_integral5_t p_output_index
            )
            const noexcept
            -> tp_integral1_t {
                auto l_result = copy_in_range_to.template operator()<1>(
                    p_destination,
                    p_source,
                    p_from_index,
                    p_to_index,
                    p_output_index
                );
                p_source = disable_in_range.template operator()<1>(
                    p_source,
                    p_from_index,
                    p_to_index
                );
                return l_result;
            }
        };
    }
    auto constexpr move_in_range_to = detail::move_in_range_to_fn{};

    namespace detail {
        struct move_in_range_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>,
                    tp_integral3_t,
                    tp_integral4_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                tp_integral2_t&      p_source,
                const tp_integral3_t p_from_index,
                const tp_integral4_t p_to_index
            )
            const noexcept
            -> tp_integral1_t {
                return move_in_range_to.template operator()<1>(
                    p_destination,
                    p_source,
                    p_from_index,
                    p_to_index,
                    p_from_index
                );
            }
        };
    }
    auto constexpr move_in_range = detail::move_in_range_fn{};

    namespace detail {
        struct move_from_to_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>,
                    tp_integral3_t,
                    tp_integral4_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                tp_integral2_t&      p_source,
                const tp_integral3_t p_from_index,
                const tp_integral4_t p_output_index
            )
            const noexcept
            -> tp_integral1_t {
                return move_in_range_to.template operator()<1>(
                    p_destination,
                    p_source,
                    p_from_index,
                    bit_size_of<tp_integral1_t> - 1,
                    p_output_index
                );
            }

            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t,
                typename tp_integral5_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>,
                    tp_integral3_t,
                    tp_integral4_t,
                    tp_integral5_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                tp_integral2_t&      p_source,
                const tp_integral3_t p_from_index,
                const tp_integral4_t p_count,
                const tp_integral5_t p_output_index
            )
            const noexcept
            -> tp_integral1_t {
                return move_in_range_to.template operator()<1>(
                    p_destination,
                    p_source,
                    p_from_index,
                    p_from_index + p_count,
                    p_output_index
                );
            }
        };
    }
    auto constexpr move_from_to = detail::move_from_to_fn{};

    namespace detail {
        struct move_from_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>,
                    tp_integral3_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                tp_integral2_t&      p_source,
                const tp_integral3_t p_from_index
            )
            const noexcept
            -> tp_integral1_t {
                return move_from_to.template operator()<1>(
                    p_destination,
                    p_source,
                    p_from_index,
                    bit_size_of<tp_integral1_t> - 1,
                    p_from_index
                );
            }

            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>,
                    tp_integral3_t,
                    tp_integral4_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                tp_integral2_t&      p_source,
                const tp_integral3_t p_from_index,
                const tp_integral4_t p_count
            )
            const noexcept
            -> tp_integral1_t {
                return move_from_to.template operator()<1>(
                    p_destination,
                    p_source,
                    p_from_index,
                    p_from_index + p_count,
                    p_from_index
                );
            }
        };
    }
    auto constexpr move_from = detail::move_from_fn{};

    namespace detail {
        struct move_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t,
                class... tp_integral_ts
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>,
                    tp_integral_ts...
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t    p_destination,
                tp_integral2_t&         p_source,
                const tp_integral_ts... p_indices
            )
            const noexcept
            -> tp_integral1_t {
                auto l_result = copy(
                    p_destination,
                    p_source,
                    p_indices...
                );
                p_source = disable(
                    p_source,
                    p_indices...
                );
                return l_result;
            }
        };
    }
    auto constexpr move = detail::move_fn{};

    namespace detail {
        struct swap_in_range_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>,
                    tp_integral3_t,
                    tp_integral4_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                tp_integral2_t&      p_value2,
                const tp_integral3_t p_from_index,
                const tp_integral4_t p_to_index
            )
            const noexcept
            -> tp_integral1_t {
                auto l_value1 = static_cast<tp_integral1_t>(
                    disable_in_range.template operator()<1>(
                        p_value1,
                        p_from_index,
                        p_to_index
                    ) | extract_in_range.template operator()<1>(
                        p_value2,
                        p_from_index,
                        p_to_index
                    )
                );
                p_value2 = static_cast<std::remove_reference_t<tp_integral2_t>>(
                    disable.template operator()<1>(
                        p_value2,
                        p_from_index,
                        p_to_index   
                    ) | extract_in_range.template operator()<1>(
                        p_value1,
                        p_from_index,
                        p_to_index
                    )
                );
                return l_value1;
            }
        };
    }
    auto constexpr swap_in_range = detail::swap_in_range_fn{};

    namespace detail {
        struct swap_from_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>,
                    tp_integral3_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                tp_integral2_t&      p_value2,
                const tp_integral3_t p_from_index
            )
            const noexcept
            -> tp_integral1_t {
                return swap_in_range.template operator()<1>(
                    p_value1,
                    p_value2,
                    p_from_index,
                    bit_size_of<tp_integral1_t> - 1
                );
            }

            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>,
                    tp_integral3_t,
                    tp_integral4_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                tp_integral2_t&      p_value2,
                const tp_integral3_t p_from_index,
                const tp_integral4_t p_count
            )
            const noexcept
            -> tp_integral1_t {
                return swap_in_range.template operator()<1>(
                    p_value1,
                    p_value2,
                    p_from_index,
                    p_from_index + p_count
                );
            }
        };
    }
    auto constexpr swap_from = detail::swap_from_fn{};

    namespace detail {
        struct swap_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t,
                class... tp_integral_ts
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>,
                    tp_integral_ts...
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t    p_value1,
                tp_integral2_t&         p_value2,
                const tp_integral_ts... p_indices
            )
            const noexcept
            -> tp_integral1_t {
                auto l_value1 = static_cast<tp_integral1_t>(
                    disable.template operator()<1>(
                        p_value1,
                        p_indices...
                    ) | extract.template operator()<1>(
                        p_value2,
                        p_indices...
                    )
                );
                auto l_value2 = static_cast<std::remove_reference_t<tp_integral2_t>>(
                    disable.template operator()<1>(
                        p_value2,
                        p_indices...
                    ) | extract.template operator()<1>(
                        p_value1,
                        p_indices...
                    )
                );
                p_value2 = l_value2;
                return l_value1;
            }
        };
    }
    auto constexpr swap = detail::swap_fn{};

    namespace detail {
        struct reverse_in_range_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index,
                const tp_integral3_t p_to_index
            )
            const noexcept
            -> tp_integral1_t {
                auto l_result = p_value;
                for (auto i = 0; i < p_to_index - p_from_index + 1; ++i) {
                    l_result = set.template operator()<1>(
                        l_result,
                        p_to_index - i,
                        get.template operator()<1>(
                            p_value,
                            p_from_index + i
                        )
                    );
                    if (i == p_to_index)
                        break;
                }
                return l_result;
            }
        };
    }
    auto constexpr reverse_in_range = detail::reverse_in_range_fn{};

    namespace detail {
        struct reverse_from_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index
            )
            const noexcept
            -> tp_integral1_t {
                return reverse_in_range.template operator()<1>(
                    p_value,
                    p_from_index,
                    bit_size_of<tp_integral1_t> - 1
                );
            }

            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value,
                const tp_integral2_t p_from_index,
                const tp_integral3_t p_count
            )
            const noexcept
            -> tp_integral1_t {
                return reverse_in_range.template operator()<1>(
                    p_value,
                    p_from_index,
                    p_from_index + p_count
                );
            }
        };
    }
    auto constexpr reverse_from = detail::reverse_from_fn{};

    namespace detail {
        struct reverse_fn : integer_adapter_closure_base {
            template<std::integral tp_integral_t>
            [[nodiscard]]
            auto constexpr operator()(const tp_integral_t p_value)
            const noexcept
            -> tp_integral_t {
                return reverse_from.template operator()<1>(
                    p_value,
                    tp_integral_t{0}
                );
            }
        };
    }
    auto constexpr reverse = detail::reverse_fn{};

    namespace detail {
        struct reverse_low_word_fn : integer_adapter_closure_base {
            template<std::integral tp_integral_t>
            [[nodiscard]]
            auto constexpr operator()(const tp_integral_t p_value)
            const noexcept
            -> tp_integral_t {
                return reverse_from.template operator()<1>(
                    p_value,
                    tp_integral_t{0},
                    half_word_bit_size_of<tp_integral_t> - 1
                );
            }
        };
    }
    auto constexpr reverse_low_word = detail::reverse_low_word_fn{};

    namespace detail {
        struct reverse_high_word_fn : integer_adapter_closure_base {
            template<std::integral tp_integral_t>
            [[nodiscard]]
            auto constexpr operator()(const tp_integral_t p_value)
            const noexcept
            -> tp_integral_t {
                return reverse_from.template operator()<1>(
                    p_value,
                    half_word_bit_size_of<tp_integral_t>
                );
            }
        };
    }
    auto constexpr reverse_high_word = detail::reverse_high_word_fn{};
    
    namespace detail {
        struct copy_low_word_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                const tp_integral2_t p_source
            )
            const noexcept
            -> tp_integral1_t {
                return copy_from.template operator()<1>(
                    p_destination,
                    p_source,
                    tp_integral1_t{0},
                    half_word_bit_size_of<tp_integral1_t> - 1
                );
            }
        };
    }
    auto constexpr copy_low_word = detail::copy_low_word_fn{};

    namespace detail {
        struct copy_high_word_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                const tp_integral2_t p_source
            )
            const noexcept
            -> tp_integral1_t {
                return copy_from.template operator()<1>(
                    p_destination,
                    p_source,
                    half_word_bit_size_of<tp_integral1_t>
                );
            }
        };
    }
    auto constexpr copy_high_word = detail::copy_high_word_fn{};

    namespace detail {
        struct move_low_word_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                tp_integral2_t&      p_source
            )
            const noexcept
            -> tp_integral1_t {
                return move_from.template operator()<1>(
                    p_source,
                    p_destination,
                    tp_integral1_t{0},
                    half_word_bit_size_of<tp_integral1_t> - 1
                );
            }
        };
    }
    auto constexpr move_low_word = detail::move_low_word_fn{};

    namespace detail {
        struct move_high_word_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_destination,
                tp_integral2_t&      p_source
            )
            const noexcept
            -> tp_integral1_t {
                return move_from.template operator()<1>(
                    p_source,
                    p_destination,
                    half_word_bit_size_of<tp_integral1_t>
                );
            }
        };
    }
    auto constexpr move_high_word = detail::move_high_word_fn{};

    namespace detail {
        struct swap_low_word_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                tp_integral2_t&      p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return swap_from.template operator()<1>(
                    p_value1,
                    p_value2,
                    tp_integral1_t{0},
                    half_word_bit_size_of<tp_integral1_t> - 1
                );
            }
        };
    }
    auto constexpr swap_low_word = detail::swap_low_word_fn{};

    namespace detail {
        struct swap_high_word_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                no_const tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                > &&
                equal_size_of<
                    tp_integral1_t,
                    std::remove_reference_t<tp_integral2_t>
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                tp_integral2_t&      p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return swap_from.template operator()<1>(
                    p_value1,
                    p_value2,
                    half_word_bit_size_of<tp_integral1_t>
                );
            }
        };
    }
    auto constexpr swap_high_word = detail::swap_high_word_fn{};

    namespace detail {
        struct swap_half_word_fn : integer_adapter_closure_base {
            template<std::integral tp_integral_t>
            [[nodiscard]]
            auto constexpr operator()(const tp_integral_t p_value)
            const noexcept
            -> tp_integral_t {
                return static_cast<tp_integral_t>(
                    extract_in_range.template operator()<1>(
                        p_value,
                        tp_integral_t{0},
                        half_word_bit_size_of<tp_integral_t> - 1
                    ) |
                    extract_in_range.template operator()<1>(
                        p_value,
                        half_word_bit_size_of<tp_integral_t>
                    )
                );
            }
        };
    }
    auto constexpr swap_half_word = detail::swap_half_word_fn{};

    namespace detail {
        struct or_low_word_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value1 | extract_low_word(p_value2));
            }
        };
    }
    auto constexpr or_low_word = detail::or_low_word_fn{};

    namespace detail {
        struct or_high_word_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value1 | extract_high_word(p_value2));
            }
        };
    }
    auto constexpr or_high_word = detail::or_high_word_fn{};

    namespace detail {
        struct and_low_word_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value1 & extract_low_word(p_value2));
            }
        };
    }
    auto constexpr and_low_word = detail::and_low_word_fn{};

    namespace detail {
        struct and_high_word_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value1 & extract_high_word(p_value2));
            }
        };
    }
    auto constexpr and_high_word = detail::and_high_word_fn{};

    namespace detail {
        struct xor_low_word_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value1 ^ extract_low_word(p_value2));
            }
        };
    }
    auto constexpr xor_low_word = detail::xor_low_word_fn{};

    namespace detail {
        struct xor_high_word_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(p_value1 ^ extract_high_word(p_value2));
            }
        };
    }
    auto constexpr xor_high_word = detail::xor_high_word_fn{};

    namespace detail {
        struct compare_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t,
                typename tp_integral4_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t,
                    tp_integral4_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2,
                const tp_integral3_t p_index1,
                const tp_integral4_t p_index2
            )
            const noexcept
            -> bool {
                return
                    get.template operator()<1>(
                        p_value1,
                        p_index1
                    ) ==
                    get.template operator()<1>(
                        p_value2,
                        p_index2
                    );
            }
        };
    }
    auto constexpr compare = detail::compare_fn{};

    namespace detail {
        struct symetric_compare_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                typename tp_integral3_t
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t,
                    tp_integral3_t
                > &&
                equal_size_of<
                    tp_integral1_t,
                    tp_integral2_t
                >
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t p_value1,
                const tp_integral2_t p_value2,
                const tp_integral3_t p_index
            )
            const noexcept
            -> bool {
                return compare.template operator()<1>(
                    p_value1,
                    p_value2,
                    p_index,
                    p_index
                );
            }
        };
    }
    auto constexpr symetric_compare = detail::symetric_compare_fn{};

    namespace detail {
        struct assign_by_sequence_fn : integer_adapter_base {
            using integer_adapter_base::operator();
            template<int tp_overload = 0, 
                typename tp_integral1_t,
                typename tp_integral2_t,
                class... tp_integral_ts
            >
            requires(
                must_only_be_called_by_closure<tp_overload> &&
                integrals_of_matching_signedness<
                    tp_integral1_t,
                    tp_integral2_t
                > &&
                sizeof...(tp_integral_ts) <= bit_size_of<tp_integral1_t>
            )
            [[nodiscard]]
            auto constexpr operator()(
                const tp_integral1_t    p_value,
                const tp_integral2_t    p_from_index,
                const tp_integral_ts... p_indices
            )
            const noexcept
            -> tp_integral1_t {
                return static_cast<tp_integral1_t>(
                    disable_from.template operator()<1>(p_value, p_from_index, sizeof...(p_indices)) |
                    (integers_to_mask<tp_integral1_t>(p_indices...) << p_from_index)
                );
            }
        };
    }
    auto constexpr assign_by_sequence = detail::assign_by_sequence_fn{};
}

#endif
