#ifndef SRC_ENGINE_TYPE_TRATS_H_
#define SRC_ENGINE_TYPE_TRATS_H_

#include <type_traits>
#include <initializer_list>
#include <algorithm>
#include "defaults.h"

namespace Engine {

    template <typename TYPE, TYPE...>
    struct static_pack;

    template <typename TYPE, TYPE Head, TYPE... Tail>
    struct static_and {
        static constexpr TYPE value = Head;
    };

    template <typename TYPE, TYPE Head, TYPE Next, TYPE... Tail>
    struct static_and<TYPE, Head, Next, Tail...> {
        static constexpr TYPE value = Head & static_and<TYPE, Next, Tail...>::value;
    };

    template <typename TYPE, TYPE Head, TYPE... Tail>
    struct static_add {
        static constexpr TYPE value = Head;
    };

    template <typename TYPE, TYPE Head, TYPE Next, TYPE... Tail>
    struct static_add<TYPE, Head, Next, Tail...> {
        static constexpr TYPE value = Head + static_add<TYPE, Next, Tail...>::value;
    };

    template <typename TYPE, TYPE Head, TYPE... Tail>
    struct static_max {
        static constexpr TYPE value = Head;
    };

    template <typename TYPE, TYPE Head, TYPE Next, TYPE... Tail>
    struct static_max<TYPE, Head, Next, Tail...> {
        static constexpr TYPE value = std::max(Head, static_max<TYPE, Next, Tail...>::value);
    };

    template <int From, int To, int Diff = To - From, int... Seq>
    struct static_range : std::conditional<
        Diff < 0,
        static_range<From, To, Diff + 1, Seq..., To - Diff>,
        static_range<From, To, Diff - 1, Seq..., To - Diff>
    >::type {};

    template <int From, int To, int... Seq>
    struct static_range<From, To, 0, Seq...> {
        typedef static_pack<int, Seq..., To> type;
        static constexpr unsigned size = sizeof...(Seq) + 1;
        static constexpr std::initializer_list<int> value = { Seq..., To };
    };

    template <int From, int To, int... Seq>
    constexpr std::initializer_list<int> static_range<From, To, 0, Seq...>::value;
}

#endif
