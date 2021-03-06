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

#include <paludis/selection.hh>
#include <paludis/selection_handler.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <algorithm>
#include <functional>
#include <tr1/functional>
#include <map>

using namespace paludis;

DidNotGetExactlyOneError::DidNotGetExactlyOneError(const std::string & s, const std::tr1::shared_ptr<const PackageIDSet> & r) throw () :
    Exception("Did not get unique result for '" + stringify(s) + "' (got { " + join(indirect_iterator(r->begin()),
                    indirect_iterator(r->end()), ", ") + "})")
{
}

namespace paludis
{
    template <>
    struct Implementation<Selection>
    {
        std::tr1::shared_ptr<const SelectionHandler> handler;

        Implementation(const std::tr1::shared_ptr<const SelectionHandler> & h) :
            handler(h)
        {
        }
    };
}

Selection::Selection(const std::tr1::shared_ptr<const SelectionHandler> & h) :
    PrivateImplementationPattern<Selection>(new Implementation<Selection>(h))
{
}

Selection::Selection(const Selection & other) :
    PrivateImplementationPattern<Selection>(new Implementation<Selection>(other._imp->handler))
{
}

Selection::~Selection()
{
}

Selection &
Selection::operator= (const Selection & other)
{
    if (this != &other)
        _imp->handler = other._imp->handler;
    return *this;
}

std::tr1::shared_ptr<PackageIDSequence>
Selection::perform_select(const Environment * const env) const
{
    Context context("When finding " + _imp->handler->as_string() + ":");
    return _imp->handler->perform_select(env);
}

std::string
Selection::as_string() const
{
    return _imp->handler->as_string();
}

namespace
{
    std::string slot_as_string(const std::tr1::shared_ptr<const PackageID> & id)
    {
        if (id->slot_key())
            return stringify(id->slot_key()->value());
        else
            return "(none)";
    }

    class SomeArbitraryVersionSelectionHandler :
        public SelectionHandler
    {
        public:
            SomeArbitraryVersionSelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            virtual std::tr1::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const
            {
                std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

                std::tr1::shared_ptr<const RepositoryNameSet> r(_fg.filter().repositories(env, _fg.generator().repositories(env)));
                if (r->empty())
                    return result;

                std::tr1::shared_ptr<const CategoryNamePartSet> c(_fg.filter().categories(env, r, _fg.generator().categories(env, r)));
                if (c->empty())
                    return result;

                std::tr1::shared_ptr<const QualifiedPackageNameSet> p(_fg.filter().packages(env, r, _fg.generator().packages(env, r, c)));
                if (p->empty())
                    return result;

                for (QualifiedPackageNameSet::ConstIterator q(p->begin()), q_end(p->end()) ; q != q_end ; ++q)
                {
                    std::tr1::shared_ptr<QualifiedPackageNameSet> s(new QualifiedPackageNameSet);
                    s->insert(*q);
                    std::tr1::shared_ptr<const PackageIDSet> i(_fg.filter().ids(env, _fg.generator().ids(env, r, s)));
                    if (! i->empty())
                    {
                        result->push_back(*i->begin());
                        break;
                    }
                }

                return result;
            }

            virtual std::string as_string() const
            {
                return "some arbitrary version from " + stringify(_fg);
            }
    };

    class BestVersionOnlySelectionHandler :
        public SelectionHandler
    {
        public:
            BestVersionOnlySelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            virtual std::tr1::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const
            {
                using namespace std::tr1::placeholders;

                std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

                std::tr1::shared_ptr<const RepositoryNameSet> r(_fg.filter().repositories(env, _fg.generator().repositories(env)));
                if (r->empty())
                    return result;

                std::tr1::shared_ptr<const CategoryNamePartSet> c(_fg.filter().categories(env, r, _fg.generator().categories(env, r)));
                if (c->empty())
                    return result;

                std::tr1::shared_ptr<const QualifiedPackageNameSet> p(_fg.filter().packages(env, r, _fg.generator().packages(env, r, c)));
                if (p->empty())
                    return result;

                for (QualifiedPackageNameSet::ConstIterator q(p->begin()), q_end(p->end()) ; q != q_end ; ++q)
                {
                    std::tr1::shared_ptr<QualifiedPackageNameSet> s(new QualifiedPackageNameSet);
                    s->insert(*q);
                    std::tr1::shared_ptr<const PackageIDSet> i(_fg.filter().ids(env, _fg.generator().ids(env, r, s)));
                    if (! i->empty())
                        result->push_back(*std::max_element(i->begin(), i->end(), PackageIDComparator(env->package_database().get())));
                }

                return result;
            }

            virtual std::string as_string() const
            {
                return "best version of each package from " + stringify(_fg);
            }
    };

    class AllVersionsSortedSelectionHandler :
        public SelectionHandler
    {
        public:
            AllVersionsSortedSelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            virtual std::tr1::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const
            {
                using namespace std::tr1::placeholders;

                std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

                std::tr1::shared_ptr<const RepositoryNameSet> r(_fg.filter().repositories(env, _fg.generator().repositories(env)));
                if (r->empty())
                    return result;

                std::tr1::shared_ptr<const CategoryNamePartSet> c(_fg.filter().categories(env, r, _fg.generator().categories(env, r)));
                if (c->empty())
                    return result;

                std::tr1::shared_ptr<const QualifiedPackageNameSet> p(_fg.filter().packages(env, r, _fg.generator().packages(env, r, c)));
                if (p->empty())
                    return result;

                std::tr1::shared_ptr<const PackageIDSet> i(_fg.filter().ids(env, _fg.generator().ids(env, r, p)));
                std::copy(i->begin(), i->end(), result->back_inserter());
                result->sort(PackageIDComparator(env->package_database().get()));

                return result;
            }

            virtual std::string as_string() const
            {
                return "all versions sorted from " + stringify(_fg);
            }
    };

    class AllVersionsUnsortedSelectionHandler :
        public SelectionHandler
    {
        public:
            AllVersionsUnsortedSelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            virtual std::tr1::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const
            {
                using namespace std::tr1::placeholders;

                std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

                std::tr1::shared_ptr<const RepositoryNameSet> r(_fg.filter().repositories(env, _fg.generator().repositories(env)));
                if (r->empty())
                    return result;

                std::tr1::shared_ptr<const CategoryNamePartSet> c(_fg.filter().categories(env, r, _fg.generator().categories(env, r)));
                if (c->empty())
                    return result;

                std::tr1::shared_ptr<const QualifiedPackageNameSet> p(_fg.filter().packages(env, r, _fg.generator().packages(env, r, c)));
                if (p->empty())
                    return result;

                std::tr1::shared_ptr<const PackageIDSet> i(_fg.filter().ids(env, _fg.generator().ids(env, r, p)));
                std::copy(i->begin(), i->end(), result->back_inserter());

                return result;
            }

            virtual std::string as_string() const
            {
                return "all versions in some arbitrary order from " + stringify(_fg);
            }
    };

    class AllVersionsGroupedBySlotSelectioHandler :
        public SelectionHandler
    {
        public:
            AllVersionsGroupedBySlotSelectioHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            virtual std::tr1::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const
            {
                using namespace std::tr1::placeholders;

                std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

                std::tr1::shared_ptr<const RepositoryNameSet> r(_fg.filter().repositories(env, _fg.generator().repositories(env)));
                if (r->empty())
                    return result;

                std::tr1::shared_ptr<const CategoryNamePartSet> c(_fg.filter().categories(env, r, _fg.generator().categories(env, r)));
                if (c->empty())
                    return result;

                std::tr1::shared_ptr<const QualifiedPackageNameSet> p(_fg.filter().packages(env, r, _fg.generator().packages(env, r, c)));
                if (p->empty())
                    return result;

                std::tr1::shared_ptr<const PackageIDSet> id(_fg.filter().ids(env, _fg.generator().ids(env, r, p)));

                typedef std::map<std::pair<QualifiedPackageName, std::string>, std::tr1::shared_ptr<PackageIDSequence> > SlotMap;
                SlotMap by_slot;
                for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                        i != i_end ; ++i)
                {
                    SlotMap::iterator m(by_slot.find(std::make_pair((*i)->name(), slot_as_string(*i))));
                    if (m == by_slot.end())
                        m = by_slot.insert(std::make_pair(std::make_pair((*i)->name(), slot_as_string(*i)),
                                    make_shared_ptr(new PackageIDSequence))).first;
                    m->second->push_back(*i);
                }

                PackageIDComparator comparator(env->package_database().get());
                for (SlotMap::iterator i(by_slot.begin()), i_end(by_slot.end()) ;
                        i != i_end ; ++i)
                    i->second->sort(comparator);

                while (! by_slot.empty())
                {
                    SlotMap::iterator m(by_slot.begin());
                    for (SlotMap::iterator n(by_slot.begin()), n_end(by_slot.end()) ; n != n_end ; ++n)
                        if (! comparator(*m->second->last(), *n->second->last()))
                            m = n;

                    std::copy(m->second->begin(), m->second->end(), result->back_inserter());
                    by_slot.erase(m);
                }

                return result;
            }

            virtual std::string as_string() const
            {
                return "all versions grouped by slot from " + stringify(_fg);
            }
    };

    class BestVersionInEachSlotSelectionHandler :
        public SelectionHandler
    {
        public:
            BestVersionInEachSlotSelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            virtual std::tr1::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const
            {
                using namespace std::tr1::placeholders;

                std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

                std::tr1::shared_ptr<const RepositoryNameSet> r(_fg.filter().repositories(env, _fg.generator().repositories(env)));
                if (r->empty())
                    return result;

                std::tr1::shared_ptr<const CategoryNamePartSet> c(_fg.filter().categories(env, r, _fg.generator().categories(env, r)));
                if (c->empty())
                    return result;

                std::tr1::shared_ptr<const QualifiedPackageNameSet> p(_fg.filter().packages(env, r, _fg.generator().packages(env, r, c)));
                if (p->empty())
                    return result;

                std::tr1::shared_ptr<const PackageIDSet> id(_fg.filter().ids(env, _fg.generator().ids(env, r, p)));

                typedef std::map<std::pair<QualifiedPackageName, std::string>, std::tr1::shared_ptr<PackageIDSequence> > SlotMap;
                SlotMap by_slot;
                for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                        i != i_end ; ++i)
                {
                    SlotMap::iterator m(by_slot.find(std::make_pair((*i)->name(), slot_as_string(*i))));
                    if (m == by_slot.end())
                        m = by_slot.insert(std::make_pair(std::make_pair((*i)->name(), slot_as_string(*i)),
                                    make_shared_ptr(new PackageIDSequence))).first;
                    m->second->push_back(*i);
                }

                PackageIDComparator comparator(env->package_database().get());
                for (SlotMap::iterator i(by_slot.begin()), i_end(by_slot.end()) ;
                        i != i_end ; ++i)
                    i->second->sort(comparator);

                while (! by_slot.empty())
                {
                    SlotMap::iterator m(by_slot.begin());
                    for (SlotMap::iterator n(by_slot.begin()), n_end(by_slot.end()) ; n != n_end ; ++n)
                        if (! comparator(*m->second->last(), *n->second->last()))
                            m = n;

                    std::copy(m->second->last(), m->second->end(), result->back_inserter());
                    by_slot.erase(m);
                }

                return result;
            }

            virtual std::string as_string() const
            {
                return "best version in each slot from " + stringify(_fg);
            }
    };

    class RequireExactlyOneSelectionHandler :
        public SelectionHandler
    {
        public:
            RequireExactlyOneSelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            virtual std::tr1::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const
            {
                using namespace std::tr1::placeholders;

                std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

                std::tr1::shared_ptr<const RepositoryNameSet> r(_fg.filter().repositories(env, _fg.generator().repositories(env)));
                if (r->empty())
                    throw DidNotGetExactlyOneError(as_string(), make_shared_ptr(new PackageIDSet));

                std::tr1::shared_ptr<const CategoryNamePartSet> c(_fg.filter().categories(env, r, _fg.generator().categories(env, r)));
                if (c->empty())
                    throw DidNotGetExactlyOneError(as_string(), make_shared_ptr(new PackageIDSet));

                std::tr1::shared_ptr<const QualifiedPackageNameSet> p(_fg.filter().packages(env, r, _fg.generator().packages(env, r, c)));
                if (p->empty())
                    throw DidNotGetExactlyOneError(as_string(), make_shared_ptr(new PackageIDSet));

                std::tr1::shared_ptr<const PackageIDSet> i(_fg.filter().ids(env, _fg.generator().ids(env, r, p)));

                if (i->empty() || next(i->begin()) != i->end())
                    throw DidNotGetExactlyOneError(as_string(), i);

                result->push_back(*i->begin());

                return result;
            }

            virtual std::string as_string() const
            {
                return "the single version from " + stringify(_fg);
            }
    };
}

selection::SomeArbitraryVersion::SomeArbitraryVersion(const FilteredGenerator & f) :
    Selection(make_shared_ptr(new SomeArbitraryVersionSelectionHandler(f)))
{
}

selection::BestVersionOnly::BestVersionOnly(const FilteredGenerator & f) :
    Selection(make_shared_ptr(new BestVersionOnlySelectionHandler(f)))
{
}

selection::AllVersionsSorted::AllVersionsSorted(const FilteredGenerator & f) :
    Selection(make_shared_ptr(new AllVersionsSortedSelectionHandler(f)))
{
}

selection::AllVersionsUnsorted::AllVersionsUnsorted(const FilteredGenerator & f) :
    Selection(make_shared_ptr(new AllVersionsUnsortedSelectionHandler(f)))
{
}

selection::AllVersionsGroupedBySlot::AllVersionsGroupedBySlot(const FilteredGenerator & f) :
    Selection(make_shared_ptr(new AllVersionsGroupedBySlotSelectioHandler(f)))
{
}

selection::BestVersionInEachSlot::BestVersionInEachSlot(const FilteredGenerator & f) :
    Selection(make_shared_ptr(new BestVersionInEachSlotSelectionHandler(f)))
{
}

selection::RequireExactlyOne::RequireExactlyOne(const FilteredGenerator & f) :
    Selection(make_shared_ptr(new RequireExactlyOneSelectionHandler(f)))
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const Selection & g)
{
    s << g.as_string();
    return s;
}

template class PrivateImplementationPattern<Selection>;

