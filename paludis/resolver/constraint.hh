/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_CONSTRAINT_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_CONSTRAINT_HH 1

#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/use_existing-fwd.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/destination_types-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/options.hh>
#include <paludis/dep_spec.hh>
#include <paludis/serialise-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        struct destination_type;
        struct nothing_is_fine_too;
        struct reason;
        struct spec;
        struct untaken;
        struct use_existing;
    }

    namespace resolver
    {
        struct Constraint
        {
            NamedValue<n::destination_type, DestinationType> destination_type;
            NamedValue<n::nothing_is_fine_too, bool> nothing_is_fine_too;
            NamedValue<n::reason, std::tr1::shared_ptr<const Reason> > reason;
            NamedValue<n::spec, PackageOrBlockDepSpec> spec;
            NamedValue<n::untaken, bool> untaken;
            NamedValue<n::use_existing, UseExisting> use_existing;

            void serialise(Serialiser &) const;

            static const std::tr1::shared_ptr<Constraint> deserialise(
                    Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE Constraints :
            private PrivateImplementationPattern<Constraints>
        {
            public:
                Constraints();
                ~Constraints();

                bool nothing_is_fine_too() const PALUDIS_ATTRIBUTE((warn_unused_result));
                bool all_untaken() const PALUDIS_ATTRIBUTE((warn_unused_result));
                UseExisting strictest_use_existing() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void add(const std::tr1::shared_ptr<const Constraint> &);

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::tr1::shared_ptr<const Constraint> > ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                bool empty() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void serialise(Serialiser &) const;

                static const std::tr1::shared_ptr<Constraints> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<resolver::Constraints>;
    extern template class WrappedForwardIterator<resolver::Constraints::ConstIteratorTag,
           const std::tr1::shared_ptr<const resolver::Constraint> >;
#endif
}

#endif
