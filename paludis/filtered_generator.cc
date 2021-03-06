/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <ostream>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<FilteredGenerator>
    {
        Generator generator;
        Filter filter;

        Implementation(const Generator & g, const Filter & f) :
            generator(g),
            filter(f)
        {
        }
    };
}

FilteredGenerator::FilteredGenerator(const FilteredGenerator & other) :
    PrivateImplementationPattern<FilteredGenerator>(new Implementation<FilteredGenerator>(other._imp->generator, other._imp->filter))
{
}

FilteredGenerator::FilteredGenerator(const Generator & g, const Filter & f) :
    PrivateImplementationPattern<FilteredGenerator>(new Implementation<FilteredGenerator>(g, f))
{
}

FilteredGenerator::FilteredGenerator(const FilteredGenerator & g, const Filter & f) :
    PrivateImplementationPattern<FilteredGenerator>(new Implementation<FilteredGenerator>(g.generator(), filter::And(g.filter(), f)))
{
}

FilteredGenerator::~FilteredGenerator()
{
}

FilteredGenerator &
FilteredGenerator::operator= (const FilteredGenerator & other)
{
    if (this != &other)
    {
        _imp->generator = other._imp->generator;
        _imp->filter = other._imp->filter;
    }
    return *this;
}

const Generator &
FilteredGenerator::generator() const
{
    return _imp->generator;
}

const Filter &
FilteredGenerator::filter() const
{
    return _imp->filter;
}

FilteredGenerator
paludis::operator| (const FilteredGenerator & g, const Filter & f)
{
    return FilteredGenerator(g, f);
}

std::ostream &
paludis::operator<< (std::ostream & s, const FilteredGenerator & fg)
{
    s << fg.generator() << " with filter " << fg.filter();
    return s;
}

template class PrivateImplementationPattern<FilteredGenerator>;

