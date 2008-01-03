/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_NAME_FWD_HH
#define PALUDIS_GUARD_PALUDIS_NAME_FWD_HH 1

#include <paludis/util/validated-fwd.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/options-fwd.hh>
#include <string>

/** \file
 * Forward declarations for paludis/name.hh .
 *
 * \ingroup g_names
 */

namespace paludis
{
    class PackageNamePartError;
    class PackageNamePartValidator;

    /**
     * A PackageNamePart holds a std::string that is a valid name for the
     * package part of a QualifiedPackageName.
     *
     * \ingroup g_names
     */
    typedef Validated<std::string, PackageNamePartValidator> PackageNamePart;

    /**
     * Holds a set of PackageNamePart instances.
     *
     * \ingroup g_names
     */
    typedef Set<PackageNamePart> PackageNamePartSet;

    class CategoryNamePartError;
    class CategoryNamePartValidator;

    /**
     * A CategoryNamePart holds a std::string that is a valid name for the
     * category part of a QualifiedPackageName.
     *
     * \ingroup g_names
     */
    typedef Validated<std::string, CategoryNamePartValidator> CategoryNamePart;

    /**
     * Holds a set of CategoryNamePart instances.
     *
     * \ingroup g_names
     */
    typedef Set<CategoryNamePart> CategoryNamePartSet;

    class UseFlagNameError;
    class IUseFlagNameError;
    class UseFlagNameValidator;

    /**
     * A UseFlagName holds a std::string that is a valid name for a USE flag.
     *
     * \ingroup g_names
     */
    typedef Validated<std::string, UseFlagNameValidator> UseFlagName;

    /**
     * A collection of UseFlagName instances.
     *
     * \ingroup g_names
     */
    typedef Set<UseFlagName> UseFlagNameSet;

#include <paludis/name-se.hh>

    class QualifiedPackageName;
    class IUseFlag;

    /**
     * Options for IUseFlag.
     *
     * \ingroup g_names
     * \since 0.26
     */
    typedef Options<IUseFlagParseOption> IUseFlagParseOptions;

    /**
     * Output a QualifiedPackageName to a stream.
     *
     * \ingroup g_names
     */
    std::ostream & operator<< (std::ostream &, const QualifiedPackageName &) PALUDIS_VISIBLE;

    /**
     * Output an IUseFlag to a stream.
     *
     * \ingroup g_names
     */
    std::ostream & operator<< (std::ostream &, const IUseFlag &) PALUDIS_VISIBLE;

    /**
     * Holds a collection of QualifiedPackageName instances.
     *
     * \ingroup g_names
     */
    typedef Set<QualifiedPackageName> QualifiedPackageNameSet;

    class QualifiedPackageNameError;

    /**
     * A CategoryNamePart plus a PackageNamePart is a QualifiedPackageName.
     *
     * \ingroup g_names
     */
    inline const QualifiedPackageName
    operator+ (const CategoryNamePart & c, const PackageNamePart & p) PALUDIS_ATTRIBUTE((warn_unused_result));

    class SlotNameError;
    class SlotNameValidator;

    /**
     * A SlotName holds a std::string that is a valid name for a SLOT.
     *
     * \ingroup g_names
     */
    typedef Validated<std::string, SlotNameValidator> SlotName;

    class RepositoryNameError;
    class RepositoryNameValidator;

    /**
     * A RepositoryName holds a std::string that is a valid name for a
     * Repository.
     *
     * \ingroup g_names
     */
    typedef Validated<std::string, RepositoryNameValidator, false> RepositoryName;

    /**
     * Holds a collection of RepositoryName instances.
     *
     * \ingroup g_names
     */
    typedef Sequence<RepositoryName> RepositoryNameSequence;

    class RepositoryNameComparator;

    class KeywordNameValidator;
    class KeywordNameError;

    /**
     * A KeywordName holds a std::string that is a valid name for a KEYWORD.
     *
     * \ingroup g_names
     */
    typedef Validated<std::string, KeywordNameValidator> KeywordName;

    /**
     * Holds a collection of KeywordName instances.
     *
     * \ingroup g_names
     */
    typedef Set<KeywordName> KeywordNameSet;

    class SetNameValidator;
    class SetNameError;

    /**
     * A SetName holds a std::string that is a valid name for a set.
     *
     * \ingroup g_names
     */
    typedef Validated<std::string, SetNameValidator> SetName;

    /**
     * A collection of set names.
     *
     * \ingroup g_names
     */
    typedef Set<SetName> SetNameSet;

    /**
     * A collection of use flags.
     *
     * \ingroup g_names
     */
    typedef Set<IUseFlag> IUseFlagSet;
}

#endif
