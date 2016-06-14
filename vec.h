#ifndef SRC_ENGINE_VEC_H_
#define SRC_ENGINE_VEC_H_

#include <stdexcept>
#include <initializer_list>
#include <array>
#include <vector>
#include <list>
#include <iterator>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <iostream>
#include <chrono>
#include <random>
#include <cmath>
#include "defaults.h"
#include "type_traits.h"

#define ENGINE_VEC_TEMPLATE_IS_ITERATOR(TYPE) \
template < \
    typename ITERATOR, \
    typename = typename std::enable_if< \
        std::is_trivially_constructible< \
            TYPE, \
            typename std::iterator_traits<ITERATOR>::value_type \
        >::value, \
        ITERATOR \
    >::type \
>

#define ENGINE_VEC_TEMPLATE_IS_ITERATIVE(TYPE) \
template < \
    typename ITERATIVE, \
    typename = typename std::enable_if< \
        std::is_trivially_constructible< \
            TYPE, \
            typename std::iterator_traits<typename ITERATIVE::iterator>::value_type \
        >::value, \
        typename ITERATIVE::iterator \
    >::type \
>

#define ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE) \
template < \
    typename CONSTRUCTIBLE, \
    typename = typename std::enable_if< \
        std::is_trivially_constructible< \
            TYPE, \
            CONSTRUCTIBLE \
        >::value, \
        CONSTRUCTIBLE \
    >::type \
>

namespace Engine {

    class Filter {

        class _Range {};
        class _Ranges {};

    public:

        template <typename... TYPES>
        struct is_range : static_and<bool, std::is_base_of<_Range, TYPES>::value...> {};

        template <typename... TYPES>
        struct is_ranges : static_and<bool, std::is_base_of<_Ranges, TYPES>::value...> {};

        template <int START, int END>
        class Range : _Range {

            static_assert(END > START, "End value should be bigger than start.");

            inline constexpr Range (void) {}

        public:

            static constexpr int start = START;
            static constexpr int end = END;
            static constexpr unsigned size = END - START;
        };

        template <typename ...RANGES>
        class Ranges : _Ranges {

            static_assert(is_range<RANGES...>::value, "Ranges only accept Range as template.");

            inline constexpr Ranges (void) {}
        public:

            static constexpr unsigned size = sizeof...(RANGES);
            static constexpr std::array<int, size> starts = { RANGES::start... };
            static constexpr std::array<int, size> ends = { RANGES::end... };
            static constexpr unsigned full_size = static_add<unsigned, (RANGES::size + 1)...>::value;

        };

    };

    template <unsigned SIZE, typename TYPE = float_max_t>
    class Vec {

        static_assert(SIZE > 0, "Vec size should be bigger than zero.");

        inline static const TYPE _add_out (const TYPE &a, const TYPE &b) { return a + b; }
        inline static TYPE &_add_in (TYPE &a, const TYPE &b) { return a += b; }

        inline static const TYPE _sub_out (const TYPE &a, const TYPE &b) { return a - b; }
        inline static TYPE &_sub_in (TYPE &a, const TYPE &b) { return a -= b; }

        inline static const TYPE _mul_out (const TYPE &a, const TYPE &b) { return a * b; }
        inline static TYPE &_mul_in (TYPE &a, const TYPE &b) { return a *= b; }

        inline static const TYPE _div_out (const TYPE &a, const TYPE &b) { return a / b; }
        inline static TYPE &_div_in (TYPE &a, const TYPE &b) { return a /= b; }


    protected:

        std::array<TYPE, SIZE> store;

    public:

        typedef typename std::array<TYPE, SIZE>::iterator iterator;
        typedef typename std::array<TYPE, SIZE>::const_iterator const_iterator;

        static Vec<SIZE, TYPE> average (std::list<Vec<SIZE, TYPE>> vecs) {
            if (!vecs.empty()) {
                const Vec<SIZE, TYPE> first = std::move(vecs.front());
                Vec<SIZE, TYPE> result = first;
                vecs.pop_front();
                for (auto &vec : vecs) {
                    result += vec;
                }
                return result / vecs.size();
            }
            return Vec<SIZE, TYPE>::zero;
        }

        inline static const Vec<SIZE, TYPE> &axis (unsigned position) {
            static std::array<Vec<SIZE, TYPE>, SIZE> axes;
            static bool initialized = false;
            static Vec<SIZE, TYPE> zero;
            if (!initialized) {
                for (unsigned i = 0; i < SIZE; ++i) {
                    zero.store[i] = static_cast<TYPE>(0);
                    for (unsigned j = 0; j < SIZE; ++j) {
                        axes[i].store[j] = static_cast<TYPE>(i == j);
                    }
                }
                initialized = true;
            }
            if (position < SIZE) {
                return axes[position];
            }
            return zero;
        }

        inline static Vec<SIZE, TYPE> random (TYPE min_val = static_cast<TYPE>(0), TYPE max_val = static_cast<TYPE>(1)) {

            static std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
            std::uniform_real_distribution<TYPE> value(min_val, max_val);
            Vec<SIZE, TYPE> result;

            for (unsigned i = 0; i < SIZE; ++i) {
                result.store[i] = value(gen);
            }

            return result;
        }

        static const Vec<SIZE, TYPE> axisX, axisY, axisZ, axisW, zero, origin;

        inline Vec (void) {}

        ENGINE_VEC_TEMPLATE_IS_ITERATOR(TYPE)
        inline Vec (ITERATOR it_copy, ITERATOR end_copy, TYPE fill = static_cast<TYPE>(0)) {
            auto it_store = std::begin(this->store), end_store = std::end(this->store);
            while (it_store != end_store && it_copy != end_copy) {
                *it_store = *it_copy;
                ++it_store, ++it_copy;
            }
            std::fill(it_store, end_store, fill);
        }

        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        inline Vec (const CONSTRUCTIBLE &fill) { std::fill(std::begin(this->store), std::end(this->store), static_cast<TYPE>(fill)); }

        ENGINE_VEC_TEMPLATE_IS_ITERATIVE(TYPE)
        inline Vec (const ITERATIVE &_copy, TYPE fill = static_cast<TYPE>(0)) :
            Vec<SIZE, TYPE>(std::begin(_copy), std::end(_copy), fill) {}

        inline Vec (const Vec<SIZE, TYPE> &_copy, TYPE fill = static_cast<TYPE>(0)) :
            Vec<SIZE, TYPE>(std::begin(_copy), std::end(_copy), fill) {}

        inline Vec (const std::initializer_list<TYPE> &_copy, TYPE fill = static_cast<TYPE>(0)) :
            Vec<SIZE, TYPE>(std::begin(_copy), std::end(_copy), fill) {}

// -------------------------------------

        inline constexpr Vec<SIZE, TYPE> &operator = (Vec<SIZE, TYPE> &&other) { return this->swap(other); }

        inline constexpr Vec<SIZE, TYPE> &operator = (const Vec<SIZE, TYPE> &other) {
            this->store = other.store;
            return *this;
        }

// -----------------------------------------------------------------------------

        inline constexpr Vec<SIZE, TYPE> &swap (Vec<SIZE, TYPE> &other) {
            this->store.swap(other.store);
            return *this;
        }

// -----------------------------------------------------------------------------

        inline iterator begin (void) { return this->store.begin(); }
        inline iterator end (void) { return this->store.end(); }

// -----------------

        inline const_iterator begin (void) const { return this->store.cbegin(); }
        inline const_iterator end (void) const { return this->store.cend(); }

// -----------------

        inline const_iterator cbegin (void) const { return this->store.cbegin(); }
        inline const_iterator cend (void) const { return this->store.cend(); }

// -------------------------------------

        inline constexpr unsigned size (void) const { return SIZE; }
        inline const TYPE *data (void) const { return this->store.data(); }

// -------------------------------------

        inline constexpr operator bool(void) const { return (*this) != zero; }

// -----------------------------------------------------------------------------

        inline constexpr bool operator == (const Vec<SIZE, TYPE> &other) const { return this->store == other.store; }
        inline constexpr bool operator != (const Vec<SIZE, TYPE> &other) const { return this->store != other.store; }
        inline constexpr bool operator ! (void) const { return !static_cast<bool>(*this); }

// -----------------------------------------------------------------------------

        inline constexpr TYPE &operator [] (int position) {
            if (position < 0) {
                position += SIZE;
            }
            if (position < 0 || static_cast<unsigned>(position) >= SIZE) {
                throw std::out_of_range(std::to_string(position) + " is out of range in Vec of size " + std::to_string(SIZE));
            }
            return this->store[position];
        }

        inline constexpr const TYPE &operator [] (int position) const {
            if (position < 0) {
                position += SIZE;
            }
            if (position < 0 || static_cast<unsigned>(position) >= SIZE) {
                throw std::out_of_range(std::to_string(position) + " is out of range in Vec of size " + std::to_string(SIZE));
            }
            return this->store[position];
        }

// -------------------------------------

        friend std::ostream &operator << (std::ostream &out, const Vec<SIZE, TYPE> &vec) {
            out << "{ ";
            for (TYPE value : vec.store) {
                out << value << ' ';
            }
            return out << '}';
        }

// -------------------------------------

        inline constexpr Vec<SIZE, TYPE> operator - (void) const {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                result.store[i] = -this->store[i];
            }
            return result;
        }

// -----------------------------------------------------------------------------

        template <typename RANGES>
        inline constexpr typename std::enable_if<Filter::is_ranges<RANGES>::value, Vec<RANGES::full_size, TYPE>>::type range (void) const {
            Vec<RANGES::full_size, TYPE> result;
            for (unsigned i = 0, k = 0; i < RANGES::size; ++i) {
                for (int j = RANGES::starts[i]; j <= RANGES::ends[i]; ++j, ++k) {
                    result[k] = (*this)[j];
                }
            }
            return result;
        }

        template <typename RANGE>
        inline constexpr typename std::enable_if<Filter::is_range<RANGE>::value, Vec<RANGE::size, TYPE>>::type range (void) const {
            return this->range<Filter::Ranges<RANGE>>();
        }

// -----------------------------------------------------------------------------

        Vec<SIZE, TYPE> mapped (const std::function<TYPE(const TYPE &)> &func) const {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                result.store[i] = func(this->store[i]);
            }
            return result;
        }

        ENGINE_VEC_TEMPLATE_IS_ITERATOR(TYPE)
        Vec<SIZE, TYPE> mapped (const std::function<TYPE(const TYPE &, typename std::iterator_traits<ITERATOR>::reference)> &func, ITERATOR it_apply, const ITERATOR &end_apply) const {
            Vec<SIZE, TYPE> result;
            auto it_store = std::begin(this->store), end_store = std::end(this->store);
            iterator it_result = std::begin(result);
            while (it_store != end_store && it_apply != end_apply) {
                *it_result = func(*it_store, *it_apply);
                ++it_store, ++it_apply, ++it_result;
            }
            std::copy(it_store, end_store, it_result);
            return result;
        }

        Vec<SIZE, TYPE> &map (const std::function<void(TYPE &)> &func) {
            for (unsigned i = 0; i < SIZE; ++i) {
                func(this->store[i]);
            }
            return *this;
        }

        ENGINE_VEC_TEMPLATE_IS_ITERATOR(TYPE)
        Vec<SIZE, TYPE> &map (const std::function<void(TYPE &, typename std::iterator_traits<ITERATOR>::reference)> &func, ITERATOR it_apply, const ITERATOR &end_apply) {
            auto it_store = std::begin(this->store), end_store = std::end(this->store);
            while (it_store != end_store && it_apply != end_apply) {
                func(*it_store, *it_apply);
                ++it_store, ++it_apply;
            }
            return *this;
        }

// -----------------------------------------------------------------------------

        constexpr TYPE sum (void) const {
            TYPE result = static_cast<TYPE>(0);
            for (TYPE value : this->store) {
                result += value;
            }
            return result;
        }

        constexpr TYPE prod (void) const {
            TYPE result = static_cast<TYPE>(1);
            for (TYPE value : this->store) {
                result *= value;
            }
            return result;
        }

// -------------------------------------

        constexpr TYPE dot (const Vec<SIZE, TYPE> &other) const {
            TYPE result = static_cast<TYPE>(0);
            for (unsigned i = 0; i < SIZE; ++i) {
                result += other[i] * this->store[i];
            }
            return result;
        }

// -------------------------------------

        template <unsigned SZ = SIZE>
        inline constexpr typename std::enable_if<(SZ == 3), Vec<SZ, TYPE>>::type cross (const Vec<SZ, TYPE> &other) const {
            return {
                this->store[1] * other[2] - other[1] * this->store[2],
                this->store[2] * other[0] - other[2] * this->store[0],
                this->store[0] * other[1] - other[0] * this->store[1]
            };
        }

        template <unsigned SZ = SIZE>
        inline constexpr typename std::enable_if<(SZ != 3), Vec<SZ, TYPE>>::type cross (const Vec<SZ, TYPE> &other) const {
            throw new std::string("Cross product is only defined to 3D");
        }

// -----------------------------------------------------------------------------

        inline constexpr TYPE distance2 (const Vec<SIZE, TYPE> &other) const {
            TYPE result = static_cast<TYPE>(0);
            for (unsigned i = 0; i < SIZE; ++i) {
                TYPE diff = this->store[i] - other.store[i];
                result += diff * diff;
            }
            return result;
        }

        inline TYPE distance (const Vec<SIZE, TYPE> &other) const {
            return std::sqrt(this->distance2(other));
        }

// -------------------------------------

        inline constexpr TYPE length2 (void) const {
            return this->dot(*this);
        }

        inline TYPE length (void) const {
            return std::sqrt(this->length2());
        }

// -----------------------------------------------------------------------------

        inline constexpr Vec<SIZE, TYPE> resized (const TYPE from_size, const TYPE to_size) const {
            if (to_size != from_size) {
                return (*this) * (to_size / from_size);
            }
            return *this;
        }

        inline Vec<SIZE, TYPE> resized (const TYPE to_size) const {
            if (this->length2() != (to_size * to_size)) {
                return this->resized(this->length(), to_size);
            }
            return *this;
        }

        inline Vec<SIZE, TYPE> &resize (const TYPE to_size) {
            if (this->length2() != (to_size * to_size)) {
                *this *= to_size / this->length();
            }
            return *this;
        }

// -------------------------------------

        inline Vec<SIZE, TYPE> normalized (void) const {
            TYPE
                zero = static_cast<TYPE>(0),
                one = static_cast<TYPE>(1),
                length2 = this->length2();
            if (length2 != zero && length2 != one) {
                return this->resized(one);
            }
            return *this;
        }

        inline Vec<SIZE, TYPE> &normalize (void) {
            TYPE
                zero = static_cast<TYPE>(0),
                one = static_cast<TYPE>(1),
                length2 = this->length2();
            if (length2 != zero && length2 != one) {
                this->resize(one);
            }
            return *this;
        }

// -------------------------------------

        inline constexpr Vec<SIZE, TYPE> lerped (const Vec<SIZE, TYPE> &other, const float_max_t position) const {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                result.store[i] = (1.0 - position) * this->store[i] + position * other.store[i];
            }
            return result;
        }

        inline constexpr Vec<SIZE, TYPE> &lerp (const Vec<SIZE, TYPE> &other, const float_max_t position) {
            for (unsigned i = 0; i < SIZE; ++i) {
                this->store[i] = (1.0 - position) * this->store[i] + position * other.store[i];
            }
            return *this;
        }

// -------------------------------------

        inline Vec<SIZE, TYPE> clamped (const TYPE &min, const TYPE &max) const {
            const TYPE length2 = this->length2();
            if (length2 < (min * min)) {
                return this->resized(min);
            } else if (length2 > (max * max)) {
                return this->resized(max);
            }
            return *this;
        }

        inline Vec<SIZE, TYPE> &clamp (const TYPE &min, const TYPE &max) {
            const TYPE length2 = this->length2();
            if (length2 < (min * min)) {
                return this->resize(min);
            } else if (length2 > (max * max)) {
                return this->resize(max);
            }
            return *this;
        }

// -----------------------------------------------------------------------------

        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> operator + (const CONSTRUCTIBLE &other, const Vec<SIZE, TYPE> &vec) { return vec.mapped([ other ] (const TYPE &a) { return _add_out(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> operator + (const Vec<SIZE, TYPE> &vec, const CONSTRUCTIBLE &other) { return vec.mapped([ other ] (const TYPE &a) { return _add_out(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_ITERATIVE(TYPE)
        Vec<SIZE, TYPE> operator + (const ITERATIVE &other) const { return this->mapped(_add_out, std::begin(other), std::end(other)); }
        // ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        // friend Vec<SIZE, TYPE> operator + (const Vec<SIZE, TYPE> &vec, const std::initializer_list<CONSTRUCTIBLE> &other) { return vec.mapped(_add_out, std::begin(other), std::end(other)); }
        // ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        // friend Vec<SIZE, TYPE> operator + (const std::initializer_list<CONSTRUCTIBLE> &other, const Vec<SIZE, TYPE> &vec) { return vec.mapped(_add_out, std::begin(other), std::end(other)); }


        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> &operator += (const CONSTRUCTIBLE &other, Vec<SIZE, TYPE> &vec) { return vec.map([ other ] (TYPE &a) { _add_in(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> &operator += (Vec<SIZE, TYPE> &vec, const CONSTRUCTIBLE &other) { return vec.map([ other ] (TYPE &a) { _add_in(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_ITERATIVE(TYPE)
        Vec<SIZE, TYPE> &operator += (const ITERATIVE &other) { return this->map(_add_in, std::begin(other), std::end(other)); }
        Vec<SIZE, TYPE> &operator += (const std::initializer_list<TYPE> &other) { return this->map(_add_in, std::begin(other), std::end(other)); }

// -------------------------------------

        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> operator - (const CONSTRUCTIBLE &other, const Vec<SIZE, TYPE> &vec) { return vec.mapped([ other ] (const TYPE &a) { return _sub_out(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> operator - (const Vec<SIZE, TYPE> &vec, const CONSTRUCTIBLE &other) { return vec.mapped([ other ] (const TYPE &a) { return _sub_out(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_ITERATIVE(TYPE)
        Vec<SIZE, TYPE> operator - (const ITERATIVE &other) const { return this->mapped(_sub_out, std::begin(other), std::end(other)); }
        Vec<SIZE, TYPE> operator - (const std::initializer_list<TYPE> &other) const { return this->mapped(_sub_out, std::begin(other), std::end(other)); }

        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> &operator -= (const CONSTRUCTIBLE &other, Vec<SIZE, TYPE> &vec) { return vec.map([ other ] (TYPE &a) { _sub_in(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> &operator -= (Vec<SIZE, TYPE> &vec, const CONSTRUCTIBLE &other) { return vec.map([ other ] (TYPE &a) { _sub_in(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_ITERATIVE(TYPE)
        Vec<SIZE, TYPE> &operator -= (const ITERATIVE &other) { return this->map(_sub_in, std::begin(other), std::end(other)); }
        Vec<SIZE, TYPE> &operator -= (const std::initializer_list<TYPE> &other) { return this->map(_sub_in, std::begin(other), std::end(other)); }

// -------------------------------------

        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> operator * (const CONSTRUCTIBLE &other, const Vec<SIZE, TYPE> &vec) { return vec.mapped([ other ] (const TYPE &a) { return _mul_out(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> operator * (const Vec<SIZE, TYPE> &vec, const CONSTRUCTIBLE &other) { return vec.mapped([ other ] (const TYPE &a) { return _mul_out(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_ITERATIVE(TYPE)
        Vec<SIZE, TYPE> operator * (const ITERATIVE &other) const { return this->mapped(_mul_out, std::begin(other), std::end(other)); }
        Vec<SIZE, TYPE> operator * (const std::initializer_list<TYPE> &other) const { return this->mapped(_mul_out, std::begin(other), std::end(other)); }

        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> &operator *= (const CONSTRUCTIBLE &other, Vec<SIZE, TYPE> &vec) { return vec.map([ other ] (TYPE &a) { _mul_in(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> &operator *= (Vec<SIZE, TYPE> &vec, const CONSTRUCTIBLE &other) { return vec.map([ other ] (TYPE &a) { _mul_in(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_ITERATIVE(TYPE)
        Vec<SIZE, TYPE> &operator *= (const ITERATIVE &other) { return this->map(_mul_in, std::begin(other), std::end(other)); }
        Vec<SIZE, TYPE> &operator *= (const std::initializer_list<TYPE> &other) { return this->map(_mul_in, std::begin(other), std::end(other)); }

// -------------------------------------
//
        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> operator / (const CONSTRUCTIBLE &other, const Vec<SIZE, TYPE> &vec) { return vec.mapped([ other ] (const TYPE &a) { return _div_out(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> operator / (const Vec<SIZE, TYPE> &vec, const CONSTRUCTIBLE &other) { return vec.mapped([ other ] (const TYPE &a) { return _div_out(a, other); }); }

        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> &operator /= (const CONSTRUCTIBLE &other, Vec<SIZE, TYPE> &vec) { return vec.map([ other ] (TYPE &a) { _div_in(a, other); }); }
        ENGINE_VEC_TEMPLATE_IS_CONSTRUCTIBLE(TYPE)
        friend Vec<SIZE, TYPE> &operator /= (Vec<SIZE, TYPE> &vec, const CONSTRUCTIBLE &other) { return vec.map([ other ] (TYPE &a) { _div_in(a, other); }); }

// -----------------------------------------------------------------------------

    };

    template <typename ...RANGES>
    constexpr std::array<int, Filter::Ranges<RANGES...>::size> Filter::Ranges<RANGES...>::starts;

    template <typename ...RANGES>
    constexpr std::array<int, Filter::Ranges<RANGES...>::size> Filter::Ranges<RANGES...>::ends;

    template <unsigned SIZE, typename TYPE>
    const Vec<SIZE, TYPE> Vec<SIZE, TYPE>::axisX = Vec<SIZE, TYPE>::axis(0);

    template <unsigned SIZE, typename TYPE>
    const Vec<SIZE, TYPE> Vec<SIZE, TYPE>::axisY = Vec<SIZE, TYPE>::axis(1);

    template <unsigned SIZE, typename TYPE>
    const Vec<SIZE, TYPE> Vec<SIZE, TYPE>::axisZ = Vec<SIZE, TYPE>::axis(2);

    template <unsigned SIZE, typename TYPE>
    const Vec<SIZE, TYPE> Vec<SIZE, TYPE>::axisW = Vec<SIZE, TYPE>::axis(3);

    template <unsigned SIZE, typename TYPE>
    const Vec<SIZE, TYPE> Vec<SIZE, TYPE>::zero = Vec<SIZE, TYPE>::axis(SIZE);

    template <unsigned SIZE, typename TYPE>
    const Vec<SIZE, TYPE> Vec<SIZE, TYPE>::origin = Vec<SIZE, TYPE>::zero;
};

#endif
