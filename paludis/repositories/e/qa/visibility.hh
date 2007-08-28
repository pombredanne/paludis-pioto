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

#include <paludis/qa-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/repositories/e/e_repository.hh>

#ifndef PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_E_QA_VISIBILITY_HH
#define PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_E_QA_VISIBILITY_HH 1

namespace paludis
{
    namespace erepository
    {
        bool visibility_check(
                const FSEntry &,
                QAReporter &,
                const Environment * const env,
                const tr1::shared_ptr<const ERepository> &,
                const tr1::shared_ptr<const PackageID> &,
                const std::string &) PALUDIS_VISIBLE;
    }
}

#endif