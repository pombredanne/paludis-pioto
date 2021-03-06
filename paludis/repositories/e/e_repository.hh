/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_E_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_E_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/profile.hh>
#include <paludis/repositories/e/layout.hh>
#include <tr1/memory>
#include <string>

/** \file
 * Declaration for the ERepository class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    class ERepositoryNews;

    /**
     * A ERepository is a Repository that handles the layout used by
     * Portage for the main Gentoo tree.
     *
     * \ingroup grperepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ERepository :
        public Repository,
        public RepositoryEnvironmentVariableInterface,
        public RepositoryVirtualsInterface,
        public RepositoryDestinationInterface,
        public RepositoryManifestInterface,
        public std::tr1::enable_shared_from_this<ERepository>,
        private PrivateImplementationPattern<ERepository>
    {
        private:
            PrivateImplementationPattern<ERepository>::ImpPtr & _imp;
            void _add_metadata_keys() const;

            void need_mirrors() const;

            const std::string _guess_eapi(const QualifiedPackageName &, const FSEntry & e) const;

        protected:
            virtual void need_keys_added() const;

        public:
            /**
             * Constructor.
             */
            ERepository(const erepository::ERepositoryParams &);

            /**
             * Destructor.
             */
            ~ERepository();

            virtual void invalidate();

            virtual void invalidate_masks();

            virtual void purge_invalid_cache() const;

            /* RepositoryVirtualsInterface */

            virtual std::tr1::shared_ptr<const VirtualsSequence> virtual_packages() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositoryDestinationInterface */

            virtual bool is_suitable_destination_for(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool is_default_destination() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool want_pre_post_phases() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void merge(const MergeParams &);

            /* RepositoryManifestInterface */
            virtual void make_manifest(const QualifiedPackageName & qpn);

            /* RepositorySyncableInterface */

            virtual bool sync(const std::tr1::shared_ptr<OutputManager> &) const;

            /* RepositoryEnvironmentVariableInterface */

            virtual std::string get_environment_variable(
                    const std::tr1::shared_ptr<const PackageID> & for_package,
                    const std::string & var) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* Repository */

            virtual std::tr1::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const CategoryNamePartSet> category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                    const PackageNamePart &) const;

            virtual bool has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

            const erepository::ERepositoryParams & params() const;

            std::string profile_variable(const std::string &) const;
            std::string environment_updated_profile_variable(const std::string &) const;

            HookResult perform_hook(const Hook &)
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const CategoryNamePartSet> unimportant_category_names() const;
            virtual const bool is_unimportant() const;

            /**
             * Update GLEP 42 news files.
             */
            void update_news() const;

            const std::tr1::shared_ptr<const erepository::Layout> layout() const;
            const std::tr1::shared_ptr<const erepository::Profile> profile() const;

            std::tr1::shared_ptr<const RepositoryMaskInfo> repository_masked(const PackageID &) const;

            void regenerate_cache() const;

            /* Keys */

            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > installed_root_key() const;
            virtual const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > info_vars_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > accept_keywords_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > sync_host_key() const;

            ///\name RepositoryFactory functions
            ///\{

            static RepositoryName repository_factory_name(
                    const Environment * const env,
                    const std::tr1::function<std::string (const std::string &)> &);

            static std::tr1::shared_ptr<Repository> repository_factory_create(
                    Environment * const env,
                    const std::tr1::function<std::string (const std::string &)> &);

            static std::tr1::shared_ptr<const RepositoryNameSet> repository_factory_dependencies(
                    const Environment * const env,
                    const std::tr1::function<std::string (const std::string &)> &);

            ///\}

            const std::tr1::shared_ptr<const Set<UnprefixedChoiceName> > arch_flags() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::tr1::shared_ptr<const UseDesc> use_desc() const PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string eapi_for_file(const FSEntry &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\name Set methods
            ///\{

            virtual void populate_sets() const;

            ///\}

            ///\name Entries
            ///\{

            bool is_package_file(const QualifiedPackageName &, const FSEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            VersionSpec extract_package_file_version(const QualifiedPackageName &, const FSEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::tr1::shared_ptr<const erepository::ERepositoryID> make_id(
                    const QualifiedPackageName &, const FSEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            void fetch(const std::tr1::shared_ptr<const erepository::ERepositoryID> &,
                    const FetchAction &) const;

            void pretend_fetch(const std::tr1::shared_ptr<const erepository::ERepositoryID> &,
                    PretendFetchAction &) const;

            void install(const std::tr1::shared_ptr<const erepository::ERepositoryID> &,
                    const InstallAction &) const;

            bool pretend(const std::tr1::shared_ptr<const erepository::ERepositoryID> &,
                    const PretendAction &) const;

            void info(const std::tr1::shared_ptr<const erepository::ERepositoryID> &,
                    const InfoAction &) const;

            const std::string get_package_file_manifest_key(const FSEntry &, const QualifiedPackageName &) const;

            const std::string binary_ebuild_name(const QualifiedPackageName &, const VersionSpec &, const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };
}

#endif
