/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_NAMED_VALUE_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_NAMED_VALUE_HH 1

#include <paludis/util/named_value-fwd.hh>
#include <paludis/util/tribool.hh>

namespace paludis
{
    /**
     * A NamedValue is used to hold a member of type V_ for a class.
     *
     * NamedValue is used to simplify 'plain old data' style classes, and to
     * provide compiler-time-checked named parameters for functions. Use
     * thestruct.themember() and thestruct.themember() = to access the real
     * underlying values.
     *
     * Usually a struct containing NamedValue objects will be constructed using
     * the make_named_values() function. For each NamedValue object,
     * make_named_values() takes a parameter in the form
     * value_for<n::whatever_K_is>(the_value).
     *
     * In all cases, NamedValue members are listed in name-sorted order, and
     * the same name is used for K_ and the member name.
     *
     * \ingroup g_oo
     */
    template <typename K_, typename V_>
    class NamedValue
    {
        private:
            V_ _value;

        public:
            typedef K_ KeyType;
            typedef V_ ValueType;

            template <typename T_>
            NamedValue(const NamedValue<K_, T_> & v) :
                _value(v())
            {
            }

            explicit NamedValue(const V_ & v) :
                _value(v)
            {
            }

            NamedValue(const NamedValue & v) :
                _value(v._value)
            {
            }

            V_ & operator() ()
            {
                return _value;
            }

            const V_ & operator() () const
            {
                return _value;
            }
    };

    template <typename K_, typename V_>
    NamedValue<K_, V_>
    value_for(const V_ & v)
    {
        return NamedValue<K_, V_>(v);
    }

    template <typename K_>
    NamedValue<K_, std::string>
    value_for(const char * const v)
    {
        return NamedValue<K_, std::string>(v);
    }

    template <typename K_>
    NamedValue<K_, Tribool>
    value_for(TriboolIndeterminateValueType v)
    {
        return NamedValue<K_, Tribool>(v);
    }
}

#endif
