// nanorange/iterator/common_iterator.hpp
//
// Copyright (c) 2018 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef NANORANGE_ITERATOR_COMMON_ITERATOR_HPP_INCLUDED
#define NANORANGE_ITERATOR_COMMON_ITERATOR_HPP_INCLUDED

#include <nanorange/iterator/concepts.hpp>

NANO_BEGIN_NAMESPACE

// [range.iterators.common]

namespace common_iterator_ {

template <typename I, typename S>
class common_iterator {
    static_assert(Iterator<I>, "");
    static_assert(Sentinel<S, I>, "");
    static_assert(!Same<I, S>, "");

    template <typename II, typename SS>
    friend class common_iterator;

    class op_arrow_proxy {
        value_type_t<I> keep_;

        op_arrow_proxy(reference_t<I>&& x) : keep_(std::move(x)) {}

    public:
        const value_type_t<I>* operator->() const
        {
            return std::addressof(keep_);
        }
    };

    template <typename II>
    using op_arrow_t = decltype(std::declval<const II&>().operator->());

    template <typename II>
    static constexpr auto do_op_arrow(const II& i, detail::priority_tag<2>)
        -> std::enable_if_t<
            std::is_pointer<II>::value || detail::exists_v<op_arrow_t, II>, I>
    {
        return i;
    }

    template <typename II>
    static constexpr auto do_op_arrow(const II& i, detail::priority_tag<1>)
        -> std::enable_if_t<std::is_reference<reference_t<const II>>::value,
                            std::add_pointer_t<reference_t<const II>>>
    {
        auto&& tmp = *i;
        return std::addressof(tmp);
    }

    template <typename II>
    static constexpr auto do_op_arrow(const II& i, detail::priority_tag<0>)
        -> op_arrow_proxy
    {
        return {i};
    }

public:
    using difference_type = difference_type_t<I>;

    constexpr common_iterator() : is_sentinel_{false}, iter_{} {}

    constexpr common_iterator(I i) : is_sentinel_{false}, iter_(i) {}

    constexpr common_iterator(S s) : is_sentinel_{true}, sentinel_{s} {}

    template <
        typename II, typename SS,
        std::enable_if_t<ConvertibleTo<II, I> && ConvertibleTo<SS, S>, int> = 0>
    constexpr common_iterator(const common_iterator<II, SS>& other)
        : is_sentinel_{other.is_sentinel_},
          iter_(other.iter_),
          sentinel_(other.sentinel_)
    {}

    template <typename II, typename SS>
    constexpr std::enable_if_t<ConvertibleTo<II, I> && ConvertibleTo<SS, S>,
                               common_iterator&>
    operator=(const common_iterator<II, SS>& other)
    {
        is_sentinel_ = other.is_sentinel_;
        iter_ = other.iter_;
        sentinel_ = other.sentinel_;
        return *this;
    };

    decltype(auto) operator*() { return *iter_; }

    template <typename II = I,
              std::enable_if_t<detail::Dereferenceable<const I>, int> = 0>
    decltype(auto) operator*() const
    {
        return *iter_;
    };

    template <typename II = I>
    auto operator-> () const
        -> decltype(common_iterator::do_op_arrow(std::declval<const II&>(),
                                                 detail::priority_tag<2>{}))
    {
        return do_op_arrow(iter_, detail::priority_tag<2>{});
    }

    common_iterator& operator++()
    {
        ++iter_;
        return *this;
    }

    template <typename II = I, std::enable_if_t<!ForwardIterator<II>, int> = 0>
    decltype(auto) operator++(int)
    {
        return iter_++;
    }

    template <typename II = I, std::enable_if_t<ForwardIterator<II>, int> = 0>
    common_iterator operator++(int)
    {
        common_iterator tmp = *this;
        ++iter_;
        return tmp;
    }

    friend rvalue_reference_t<I> iter_move(const common_iterator& i)
    {
        return ranges::iter_move(i.iter_);
    }

    template <typename I2, typename S2>
    friend std::enable_if_t<IndirectlySwappable<I2, I>>
    iter_swap(const common_iterator& x, const common_iterator<I2, S2>& y)
    {
        return ranges::iter_swap(x.iter_, y.iter_);
    }

    // private:
    // TODO: Some sort of variant-like union
    bool is_sentinel_;
    I iter_;
    S sentinel_;
};

template <typename I1, typename I2, typename S1, typename S2,
          std::enable_if_t<!EqualityComparableWith<I1, I2>, int> = 0>
bool operator==(const common_iterator<I1, S1>& x,
                const common_iterator<I2, S2>& y)
{
    return x.is_sentinel_ ? (y.is_sentinel_ || y.iter_ == x.sentinel_)
                          : (!y.is_sentinel_ || x.iter_ == y.sentinel_);
}

template <typename I1, typename I2, typename S1, typename S2,
          std::enable_if_t<EqualityComparableWith<I1, I2>, int> = 0>
bool operator==(const common_iterator<I1, S1>& x,
                const common_iterator<I2, S2>& y)
{
    return x.is_sentinel_
               ? (y.is_sentinel_ || y.iter_ == x.sentinel_)
               : (y.is_sentinel_ ? x.iter_ == y.sentinel_ : x.iter_ == y.iter_);
}

template <typename I1, typename I2, typename S1, typename S2>
bool operator!=(const common_iterator<I1, S1>& x,
                const common_iterator<I2, S2>& y)
{
    return !(x == y);
}

template <typename I2, typename I1, typename S1, typename S2>
std::enable_if_t<SizedSentinel<I1, I2> && SizedSentinel<S1, I2> &&
                     SizedSentinel<S2, I2>,
                 difference_type_t<I2>>
operator-(const common_iterator<I1, S1>& x, const common_iterator<I2, S2>& y)
{
    return x.is_sentinel_
               ? (y.is_sentinel_ ? 0 : x.sentinel_ - y.iter_)
               : (y.is_sentinel_ ? x.iter_ - y.sentinel_ : x.iter_ - y.iter_);
}

} // namespace common_iterator_

using common_iterator_::common_iterator;

template <typename I, typename S>
struct value_type<common_iterator<I, S>> {
    using type = value_type_t<I>;
};

template <typename I, typename S>
struct iterator_category<common_iterator<I, S>>
    : std::conditional<ForwardIterator<I>, forward_iterator_tag,
                       input_iterator_tag> {
};

NANO_END_NAMESPACE

namespace std {

template <typename I, typename S>
struct iterator_traits<nano::common_iterator<I, S>> {
    using difference_type =
        nano::difference_type_t<nano::common_iterator<I, S>>;
    using value_type = nano::value_type_t<nano::common_iterator<I, S>>;
    using pointer =
        std::add_pointer_t<nano::reference_t<nano::common_iterator<I, S>>>;
    using reference = nano::reference_t<nano::common_iterator<I, S>>;
    using iterator_category =
        std::conditional_t<nano::ForwardIterator<I>, std::forward_iterator_tag,
                           std::input_iterator_tag>;
};

} // namespace std

#endif