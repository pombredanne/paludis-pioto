/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/stringify.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>

#include <list>
#include <algorithm>

#include <libwrapiter/libwrapiter_forward_iterator.hh>

using namespace paludis;

#include <paludis/package_id-se.cc>

namespace paludis
{
    template <>
    struct Implementation<PackageID>
    {
        mutable std::list<tr1::shared_ptr<const MetadataKey> > keys;
    };
}

PackageID::PackageID() :
    PrivateImplementationPattern<PackageID>(new Implementation<PackageID>)
{
}

PackageID::~PackageID()
{
}

void
PackageID::add_key(const tr1::shared_ptr<const MetadataKey> & k) const
{
    using namespace tr1::placeholders;
    if (_imp->keys.end() != std::find_if(_imp->keys.begin(), _imp->keys.end(),
                tr1::bind(std::equal_to<std::string>(), k->raw_name(), tr1::bind(tr1::mem_fn(&MetadataKey::raw_name), _1))))
        throw ConfigurationError("Tried to add duplicate key '" + k->raw_name() + "' to ID '" + stringify(*this) + "'");

    _imp->keys.push_back(k);
}

PackageID::Iterator
PackageID::begin() const
{
    need_keys_added();
    return Iterator(indirect_iterator(_imp->keys.begin()));
}

PackageID::Iterator
PackageID::end() const
{
    need_keys_added();
    return Iterator(indirect_iterator(_imp->keys.end()));
}

PackageID::Iterator
PackageID::find(const std::string & s) const
{
    using namespace tr1::placeholders;

    need_keys_added();
    return std::find_if(begin(), end(),
            tr1::bind(std::equal_to<std::string>(), s, tr1::bind(tr1::mem_fn(&MetadataKey::raw_name), _1)));
}

std::ostream &
paludis::operator<< (std::ostream & s, const PackageID & i)
{
    s << i.canonical_form(idcf_full);
    return s;
}

bool
PackageIDSetComparator::operator() (const tr1::shared_ptr<const PackageID> & a,
        const tr1::shared_ptr<const PackageID> & b) const
{
    if (a->name() < b->name())
        return true;

    if (a->name() > b->name())
        return false;

    if (a->version() < b->version())
        return true;

    if (a->version() > b->version())
        return false;

    if (a->repository()->name().data() < b->repository()->name().data())
        return true;

    if (a->repository()->name().data() > b->repository()->name().data())
        return false;

    return a->arbitrary_less_than_comparison(*b);
}

bool
paludis::operator== (const PackageID & a, const PackageID & b)
{
    return (a.name() == b.name())
        && (a.version() == b.version())
        && (a.repository()->name() == b.repository()->name())
        && (! a.arbitrary_less_than_comparison(b))
        && (! b.arbitrary_less_than_comparison(a));
}
