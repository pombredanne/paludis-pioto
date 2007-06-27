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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_GEMS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_GEMS_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/repositories/gems/params-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/tr1_memory.hh>

namespace paludis
{
    /**
     * Repository for Gem packages.
     *
     * \ingroup grpgemsrepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE GemsRepository :
        public Repository,
        public RepositoryInstallableInterface,
        private PrivateImplementationPattern<GemsRepository>,
        public tr1::enable_shared_from_this<GemsRepository>
    {
        private:
            void need_category_names() const;
            void need_ids() const;

        protected:
            /* RepositoryInstallableInterface */

            virtual void do_install(const tr1::shared_ptr<const PackageID> &, const InstallOptions &) const;

            /* Repository */

            virtual tr1::shared_ptr<const PackageIDSequence> do_package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const QualifiedPackageNameCollection> do_package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartCollection> do_category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            /**
             * Constructor.
             */
            GemsRepository(const gems::RepositoryParams &);

            /**
             * Destructor.
             */
            ~GemsRepository();

            virtual void invalidate();
    };
}

#endif