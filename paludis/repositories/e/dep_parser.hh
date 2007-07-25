/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_E_DEP_PARSER_HH
#define PALUDIS_GUARD_PALUDIS_E_DEP_PARSER_HH 1

#include <paludis/repositories/e/dep_parser-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/repositories/e/dep_lexer.hh>
#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <string>

/** \file
 * Declarations for the DepParser routines.
 *
 * \ingroup grpdepparser
 */

namespace paludis
{
    namespace erepository
    {
        /**
         * A DepStringParseError is thrown if an error is encountered when parsing
         * a dependency string.
         *
         * \ingroup grpexceptions
         * \ingroup grpdepparser
         */
        class PALUDIS_VISIBLE DepStringParseError : public DepStringError
        {
            public:
                /**
                 * Constructor.
                 */
                DepStringParseError(const std::string & dep_string,
                        const std::string & message) throw ();
        };

        /**
         * A DepStringNestingError is thrown if a dependency string does not have
         * properly balanced parentheses.
         *
         * \ingroup grpexceptions
         * \ingroup grpdepparser
         */
        class PALUDIS_VISIBLE DepStringNestingError : public DepStringParseError
        {
            public:
                /**
                 * Constructor.
                 */
                DepStringNestingError(const std::string & dep_string) throw ();
        };

        /**
         * Parse a dependency heirarchy.
         */
        tr1::shared_ptr<DependencySpecTree::ConstItem> parse_depend(const std::string & s,
                const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a provide heirarchy.
         */
        tr1::shared_ptr<ProvideSpecTree::ConstItem> parse_provide(const std::string & s,
                const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a restrict.
         */
        tr1::shared_ptr<RestrictSpecTree::ConstItem> parse_restrict(const std::string & s,
                const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a uri heirarchy.
         */
        tr1::shared_ptr<URISpecTree::ConstItem> parse_uri(const std::string & s,
                const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a license heirarchy.
         */
        tr1::shared_ptr<LicenseSpecTree::ConstItem> parse_license(const std::string & s,
                const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a URI label.
         */
        tr1::shared_ptr<LabelsDepSpec<URILabelVisitorTypes> > parse_uri_label(const std::string & s,
                const EAPI &) PALUDIS_VISIBLE;
    }
}

#endif