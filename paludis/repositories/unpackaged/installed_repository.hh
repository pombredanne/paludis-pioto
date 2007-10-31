/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_INSTALLED_UNPACKAGED_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_INSTALLED_UNPACKAGED_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/map.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    namespace unpackaged_repositories
    {
#include <paludis/repositories/unpackaged/installed_repository-sr.hh>
    }

    class PALUDIS_VISIBLE InstalledUnpackagedRepository :
        private PrivateImplementationPattern<InstalledUnpackagedRepository>,
        public Repository,
        public RepositoryDestinationInterface,
        public RepositoryInstalledInterface,
        public RepositorySetsInterface
    {
        public:
            InstalledUnpackagedRepository(
                    const RepositoryName &,
                    const unpackaged_repositories::InstalledUnpackagedRepositoryParams &);

            ~InstalledUnpackagedRepository();

            virtual void invalidate();
            virtual void invalidate_masks();

            virtual bool is_suitable_destination_for(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool is_default_destination() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool want_pre_post_phases() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void merge(const MergeOptions &);

            virtual FSEntry root() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            void deindex(const QualifiedPackageName &) const;

            virtual tr1::shared_ptr<const SetNameSet> sets_list() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartSet> category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                    const PackageNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

            virtual tr1::shared_ptr<SetSpecTree::ConstItem> package_set(const SetName & id) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif