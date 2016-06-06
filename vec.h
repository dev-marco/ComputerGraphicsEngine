#ifndef SRC_ENGINE_VEC_H_
#define SRC_ENGINE_VEC_H_

#include <stdexcept>
#include <initializer_list>
#include <array>
#include <vector>
#include <iterator>
#include <type_traits>
#include <iostream>
#include <cmath>
#include "defaults.h"
#include "type_traits.h"

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

    protected:

        std::array<TYPE, SIZE> store;

    public:

        typedef typename std::array<TYPE, SIZE>::iterator iterator;
        typedef typename std::array<TYPE, SIZE>::const_iterator const_iterator;

        inline static constexpr Vec<SIZE, TYPE> axis (unsigned position) {
            std::array<Vec<SIZE, TYPE>, SIZE> axes;
            Vec<SIZE, TYPE> zero;
            for (unsigned i = 0; i < SIZE; ++i) {
                zero.store[i] = static_cast<TYPE>(0);
                for (unsigned j = 0; j < SIZE; ++j) {
                    axes[i].store[j] = static_cast<TYPE>(i == j);
                }
            }
            if (position < SIZE) {
                return axes[position];
            }
            return zero;
        }

        static const Vec<SIZE, TYPE> axisX, axisY, axisZ, axisW, zero;

        inline constexpr Vec (void) {}

        template <
            typename TYPE_C,
            typename = typename std::enable_if<std::is_trivially_constructible<TYPE, TYPE_C>::value, TYPE_C>::type
        >
        inline constexpr Vec (const TYPE_C &fill) { std::fill(std::begin(this->store), std::end(this->store), static_cast<TYPE>(fill)); }

        template <
            typename INIT,
            typename = typename INIT::iterator
        >
        inline constexpr Vec (const INIT &_copy, TYPE fill = static_cast<TYPE>(0)) {
            auto it_data = std::begin(this->store), end_data = std::end(this->store);
            auto it_copy = std::begin(_copy), end_copy = std::end(_copy);
            while (it_data != end_data && it_copy != end_copy) {
                *it_data = *it_copy;
                ++it_data, ++it_copy;
            }
            std::fill(it_data, end_data, fill);
        }

        inline constexpr Vec (const std::initializer_list<TYPE> &_copy) : Vec<SIZE, TYPE>(std::vector<TYPE>(_copy)) {}

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
        inline const std::array<TYPE, SIZE> &data (void) const { return this->store; }

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

        inline constexpr Vec<SIZE, TYPE> &operator - (void) {
            for (TYPE &value : this->store) {
                value = -value;
            }
            return *this;
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
                *this /= to_size / this->length();
            }
            return *this;
        }

// -------------------------------------

        inline Vec<SIZE, TYPE> normalized (void) const {
            TYPE one = static_cast<TYPE>(1);
            if (this->length2() != one) {
                return this->resized(one);
            }
            return *this;
        }

        inline Vec<SIZE, TYPE> &normalize (void) {
            TYPE one = static_cast<TYPE>(1);
            if (this->length2() != one) {
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

        inline constexpr Vec<SIZE, TYPE> translated (const Vec<SIZE, TYPE> &other) const {
            return (*this) + other;
        }

        inline constexpr Vec<SIZE, TYPE> &translate (const Vec<SIZE, TYPE> &other) {
            this += other;
            return *this;
        }

// -----------------------------------------------------------------------------

        constexpr Vec<SIZE, TYPE> operator + (const Vec<SIZE, TYPE> &other) const {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                result.store[i] = this->store[i] + other[i];
            }
            return result;
        }

        constexpr Vec<SIZE, TYPE> operator + (const TYPE &other) const {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                result.store[i] = this->store[i] + other;
            }
            return result;
        }

        constexpr Vec<SIZE, TYPE> &operator += (const Vec<SIZE, TYPE> &other) {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                this->store[i] += other[i];
            }
            return *this;
        }

        constexpr Vec<SIZE, TYPE> &operator += (const TYPE &other) {
            for (unsigned i = 0; i < SIZE; ++i) {
                this->store[i] += other;
            }
            return *this;
        }

// -------------------------------------

        constexpr Vec<SIZE, TYPE> operator - (const Vec<SIZE, TYPE> &other) const {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                result.store[i] = this->store[i] - other[i];
            }
            return result;
        }

        constexpr Vec<SIZE, TYPE> operator - (const TYPE &other) const {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                result.store[i] = this->store[i] - other;
            }
            return result;
        }

        constexpr Vec<SIZE, TYPE> &operator -= (const Vec<SIZE, TYPE> &other) {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                this->store[i] -= other[i];
            }
            return *this;
        }

        constexpr Vec<SIZE, TYPE> &operator -= (const TYPE &other) {
            for (unsigned i = 0; i < SIZE; ++i) {
                this->store[i] -= other;
            }
            return *this;
        }

// -------------------------------------

        constexpr Vec<SIZE, TYPE> operator * (const Vec<SIZE, TYPE> &other) const {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                result.store[i] = this->store[i] * other[i];
            }
            return result;
        }

        constexpr Vec<SIZE, TYPE> operator * (const TYPE &other) const {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                result.store[i] = this->store[i] * other;
            }
            return result;
        }

        constexpr Vec<SIZE, TYPE> &operator *= (const Vec<SIZE, TYPE> &other) {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                this->store[i] -= other[i];
            }
            return *this;
        }

        constexpr Vec<SIZE, TYPE> &operator *= (const TYPE &other) {
            for (unsigned i = 0; i < SIZE; ++i) {
                this->store[i] -= other;
            }
            return *this;
        }

// -------------------------------------

        constexpr Vec<SIZE, TYPE> operator / (const TYPE &other) const {
            Vec<SIZE, TYPE> result;
            for (unsigned i = 0; i < SIZE; ++i) {
                result.store[i] = this->store[i] / other;
            }
            return result;
        }

        constexpr Vec<SIZE, TYPE> &operator /= (const TYPE &other) {
            for (unsigned i = 0; i < SIZE; ++i) {
                this->store[i] /= other;
            }
            return *this;
        }

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
};

#endif
