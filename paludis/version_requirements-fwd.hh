/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_VERSION_REQUIREMENTS_FWD_HH
#define PALUDIS_GUARD_PALUDIS_VERSION_REQUIREMENTS_FWD_HH 1

#include <paludis/util/collection-fwd.hh>
#include <paludis/util/attributes.hh>
#include <iosfwd>

namespace paludis
{
    class VersionRequirement;

    /**
     * A collection of VersionRequirement instances, usually for a
     * PackageDepSpec.
     *
     * \see PackageDepSpec
     * \ingroup grpversions
     */
    typedef SequentialCollection<VersionRequirement> VersionRequirements;

    /**
     * Whether our version requirements are an 'and' or an 'or' set.
     *
     * \see PackageDepSpec
     * \ingroup grpversions
     */
    enum VersionRequirementsMode
    {
        vr_or,     ///\< Must match one
        vr_and,    ///\< Must match all
        last_vr
    };

    /**
     * Write a VersionRequirementsMode to a stream.
     *
     * \ingroup grpversions
     */
    std::ostream &
    operator<< (std::ostream &, const VersionRequirementsMode &) PALUDIS_VISIBLE;

}

#endif