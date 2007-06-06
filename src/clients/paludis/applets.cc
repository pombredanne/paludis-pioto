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

#include "applets.hh"
#include <src/output/colour.hh>
#include <functional>
#include <iomanip>
#include <iostream>
#include <paludis/paludis.hh>
#include <paludis/util/log.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/virtual_constructor-impl.hh>
#include <paludis/repositories/repository_maker.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <string>
#include <set>
#include <map>

/** \file
 * Handle the --has-version, --best-version and various --list actions for the
 * main paludis program.
 */

using namespace paludis;

int do_has_version(tr1::shared_ptr<Environment> env)
{
    int return_code(0);

    Context context("When performing has-version action from command line:");

    std::string query(*CommandLine::get_instance()->begin_parameters());
    tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(query, pds_pm_permissive));
    tr1::shared_ptr<const PackageDatabaseEntryCollection> entries(env->package_database()->query(
                query::Matches(*spec) & query::InstalledAtRoot(env->root()), qo_whatever));

    if (entries->empty())
        return_code = 1;

    return return_code;
}

int do_best_version(tr1::shared_ptr<Environment> env)
{
    int return_code(0);

    Context context("When performing best-version action from command line:");

    std::string query(*CommandLine::get_instance()->begin_parameters());
    tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(query, pds_pm_permissive));
    tr1::shared_ptr<const PackageDatabaseEntryCollection> entries(env->package_database()->query(
                query::Matches(*spec) & query::InstalledAtRoot(env->root()), qo_order_by_version));

    /* make built_with_use work for virtuals... icky... */
    while (! entries->empty())
    {
        tr1::shared_ptr<const VersionMetadata> metadata(env->package_database()->fetch_repository(
                    entries->last()->repository)->version_metadata(entries->last()->name,
                    entries->last()->version));
        if (! metadata->virtual_interface)
            break;

        Log::get_instance()->message(ll_qa, lc_context, "best-version of '" + query +
                "' resolves to '" + stringify(*entries->last()) + "', which is a virtual for '"
                + stringify(*metadata->virtual_interface->virtual_for) + "'. This will break with "
                "new style virtuals.");
        tr1::shared_ptr<PackageDatabaseEntryCollection> new_entries(
                new PackageDatabaseEntryCollection::Concrete);
        new_entries->push_back(*metadata->virtual_interface->virtual_for);
        entries = new_entries;
    }

    if (entries->empty())
        return_code = 1;
    else
    {
        // don't include repo, it breaks built_with_use and the like.
        std::string entry(
                stringify(entries->last()->name) + "-" +
                stringify(entries->last()->version));
        std::cout << entry << std::endl;
    }

    return return_code;
}

int do_environment_variable(tr1::shared_ptr<Environment> env)
{
    int return_code(0);

    Context context("When performing environment-variable action from command line:");

    std::string spec_str(*CommandLine::get_instance()->begin_parameters());
    std::string var_str(* next(CommandLine::get_instance()->begin_parameters()));
    tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(spec_str, pds_pm_permissive));

    tr1::shared_ptr<const PackageDatabaseEntryCollection> entries(env->package_database()->query(
                query::Matches(*spec) & query::InstalledAtRoot(env->root()), qo_order_by_version));

    if (entries->empty())
        entries = env->package_database()->query(query::Matches(*spec), qo_order_by_version);

    if (entries->empty())
        throw NoSuchPackageError(spec_str);

    tr1::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(
                entries->begin()->repository));
    RepositoryEnvironmentVariableInterface * env_if(
            repo->environment_variable_interface);

    if (! env_if)
    {
        std::cerr << "Repository '" << repo->name() <<
            "' cannot be queried for environment variables" << std::endl;
        return_code |= 1;
    }
    else
        std::cout << env_if->get_environment_variable(*entries->begin(), var_str) << std::endl;

    return return_code;
}

int do_configuration_variable(tr1::shared_ptr<Environment> env)
{
    int return_code(0);

    Context context("When performing configuration-variable action from command line:");

    std::string repo_str(*CommandLine::get_instance()->begin_parameters());
    std::string var_str(* next(CommandLine::get_instance()->begin_parameters()));

    tr1::shared_ptr<const RepositoryInfo> info(env->package_database()->fetch_repository(
                RepositoryName(repo_str))->info(false));

    return_code = 1;
    for (RepositoryInfo::SectionIterator s(info->begin_sections()),
            s_end(info->end_sections()) ; s != s_end ; ++s)
        for (RepositoryInfoSection::KeyValueIterator k((*s)->begin_kvs()),
                k_end((*s)->end_kvs()) ; k != k_end ; ++k)
            if (var_str == k->first)
            {
                std::cout << k->second << std::endl;
                return_code = 0;
                break;
            }

    return return_code;
}

int do_list_repository_formats()
{
    int return_code(1);

    std::set<std::string> keys;
    RepositoryMaker::get_instance()->copy_keys(std::inserter(keys, keys.begin()));

    if (! keys.empty())
    {
        return_code = 0;
        for (std::set<std::string>::const_iterator k(keys.begin()), k_end(keys.end()) ;
                k != k_end ; ++k)
            std::cout << "* " << colour(cl_key_name, *k) << std::endl;
    }

    return return_code;
}

int do_list_sync_protocols(tr1::shared_ptr<Environment> env)
{
    std::map<std::string, std::string> syncers;

    tr1::shared_ptr<const FSEntryCollection> sd(env->syncers_dirs());
    for (FSEntryCollection::Iterator d(sd->begin()),
            d_end(sd->end()) ; d != d_end ; ++d)
    {
        FSEntry dir(*d);
        if (! dir.is_directory())
            continue;

        for (DirIterator f(dir), f_end; f != f_end; ++f)
        {
            std::string name(f->basename());
            if (f->has_permission(fs_ug_owner, fs_perm_execute) &&
                   0 == name.compare(0, 2, "do", 0, 2))
            {
                name.erase(0, 2);
                if (syncers.find(name) == syncers.end())
                    syncers[name] = stringify(*f);
            }
        }
    }

    int return_code(1);

    if (! syncers.empty())
    {
        return_code = 0;
        for (std::map<std::string, std::string>::const_iterator s(syncers.begin()), s_end(syncers.end()) ;
                s != s_end ; ++s)
        {
            std::cout << "* " << colour(cl_key_name, s->first) << std::endl;
            if (0 != run_command(Command(s->second + " --help")
                        .with_setenv("PALUDIS_FETCHERS_DIRS", join(sd->begin(), sd->end(), " "))
                        .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))))
                Log::get_instance()->message(ll_warning, lc_context, "Syncer help command '" +
                        s->second + " --help' failed");
        }
    }

    return return_code;
}

int do_list_dep_tag_categories()
{
    int return_code(1);

    std::set<std::string> keys;
    DepTagCategoryMaker::get_instance()->copy_keys(std::inserter(keys, keys.begin()));

    if (! keys.empty())
    {
        return_code = 0;
        for (std::set<std::string>::const_iterator k(keys.begin()), k_end(keys.end()) ;
                k != k_end ; ++k)
            std::cout << "* " << colour(cl_key_name, *k) << std::endl;
    }

    return return_code;
}

int do_regenerate_cache(tr1::shared_ptr<Environment> env, bool installed)
{
    Context context("When performing cache regeneration action from command line:");

    for (PackageDatabase::RepositoryIterator r(env->package_database()->begin_repositories()),
            r_end(env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (installed != (0 != (*r)->installed_interface))
            continue;

        std::cout << "Regenerating cache for " << (*r)->name() << "..." << std::endl;
        (*r)->regenerate_cache();
    }

    return 0;
}

