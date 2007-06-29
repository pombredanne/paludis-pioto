/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_SETS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_SETS_HH 1

#include <paludis/dep_spec-fwd.hh>
#include <paludis/repository-fwd.hh>

/** \file
 * Declaration for the ERepositorySets class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    class Environment;
    class ERepository;

    /**
     * Holds the information about sets, except system, for a ERepository.
     *
     * \ingroup grperepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ERepositorySets :
        private PrivateImplementationPattern<ERepositorySets>,
        private InstantiationPolicy<ERepositorySets, instantiation_method::NonCopyableTag>
    {
        public:
            ///\name Basic operations
            ///\{

            ERepositorySets(const Environment * const env, const ERepository * const,
                    const ERepositoryParams &);
            ~ERepositorySets();

            ///\}

            /**
             * Fetch a package set other than system.
             */
            tr1::shared_ptr<SetSpecTree::ConstItem> package_set(const SetName & s) const;

            /**
             * Fetch the security or insecurity set.
             */
            tr1::shared_ptr<SetSpecTree::ConstItem> security_set(bool insecure) const;

            /**
             * Give a list of all the sets in this repo.
             */
            tr1::shared_ptr<const SetNameCollection> sets_list() const;
    };
}


#endif