/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
 * Copyright (c) 2007 David Leverton
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_ITERATOR_FUNCS_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_ITERATOR_FUNCS_HH 1

#include <iterator>

namespace paludis
{
#ifdef PALUDIS_HAVE_STD_NEXT
    using std::next;
#else
    /**
     * Return a new iterator pointing to the item after i.
     *
     * \ingroup g_iterator
     */
    template <typename T_>
    T_ next(const T_ & i)
    {
        T_ result(i);
        return ++result;
    }

     /**
     * Return a new iterator advanced by a distance of n.
     *
     * \ingroup g_iterator
     */
    template <typename T_>
    T_ next(const T_ & i, const int & n)
    {
        T_ result(i);
        std::advance(result, n);
        return result;
    }
#endif

    /**
     * Return a new iterator pointing to the item before i.
     *
     * \ingroup g_iterator
     */
    template <typename T_>
    T_ previous(const T_ & i)
    {
        T_ result(i);
        return --result;
    }

    /**
     * Return the distance from a to b, except if it is greater than n,
     * in which case return n instead.
     *
     * \ingroup g_iterator
     */
    template <typename T_>
    std::size_t capped_distance(T_ a, const T_ & b, unsigned n)
    {
        std::size_t x(0);
        while ((x < n) && (a != b))
        {
            ++x;
            ++a;
        }
        return x;
    }
}

#endif
