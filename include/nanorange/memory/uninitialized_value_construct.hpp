// nanorange/memory/uninitialized_value_construct.hpp
//
// Copyright (c) 2018 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef NANORANGE_MEMORY_UNINITIALIZED_VALUE_CONSTRUCT_HPP_INCLUDED
#define NANORANGE_MEMORY_UNINITIALIZED_VALUE_CONSTRUCT_HPP_INCLUDED

#include <nanorange/memory/destroy.hpp>

NANO_BEGIN_NAMESPACE

namespace detail {

struct uninitialized_value_construct_fn {
private:
    template <typename I, typename S>
    static I impl(I first, S last)
    {
        I it = first;
        try {
            for (; it != last; ++it) {
                ::new(const_cast<void*>(static_cast<const volatile void*>(std::addressof(
                        *it))))
                        std::remove_reference_t<iter_reference_t<I>>();
            }
            return it;
        } catch (...) {
            nano::destroy(first, ++it);
            throw;
        }
    }

public:
    template <typename I, typename S>
    std::enable_if_t<
        NoThrowForwardIterator<I> &&
        NoThrowSentinel<S, I> &&
        DefaultConstructible<iter_value_t<I>>, I>
    operator()(I first, S last) const
    {
        return uninitialized_value_construct_fn::impl(
                std::move(first), std::move(last));
    }

    template <typename Rng>
    std::enable_if_t<
        NoThrowForwardRange<Rng> &&
        DefaultConstructible<iter_value_t<iterator_t<Rng>>>,
        safe_iterator_t<Rng>>
    operator()(Rng&& rng) const
    {
        return uninitialized_value_construct_fn::impl(
                nano::begin(rng), nano::end(rng));
    }
};

}

NANO_INLINE_VAR(detail::uninitialized_value_construct_fn,
                uninitialized_value_construct)

namespace detail {

struct uninitialized_value_construct_n_fn {
    template <typename I>
    std::enable_if_t<
        NoThrowForwardIterator<I> &&
        DefaultConstructible<iter_value_t<I>>, I>
    operator()(I first, iter_difference_t<I> n) const
    {
        return nano::uninitialized_value_construct(
                    make_counted_iterator(std::move(first), n),
                    default_sentinel{}).base();
    }

};

}

NANO_INLINE_VAR(detail::uninitialized_value_construct_n_fn,
                uninitialized_value_construct_n)

NANO_END_NAMESPACE

#endif
