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

#include <paludis/repositories/e/e_repository_profile.hh>
#include <paludis/repositories/e/e_repository_profile_file.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/save.hh>
#include <paludis/util/system.hh>
#include <paludis/util/join.hh>
#include <paludis/config_file.hh>
#include <paludis/dep_tag.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/distribution.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include <list>
#include <algorithm>
#include <set>
#include <vector>

#include <strings.h>
#include <ctype.h>

using namespace paludis;

namespace
{
    typedef MakeHashedSet<UseFlagName>::Type UseFlagSet;
    typedef MakeHashedMap<std::string, std::string>::Type EnvironmentVariablesMap;
    typedef MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> >::Type VirtualsMap;
    typedef MakeHashedMap<QualifiedPackageName, std::list<tr1::shared_ptr<const PackageDepSpec> > >::Type PackageMaskMap;

    typedef MakeHashedMap<UseFlagName, bool>::Type FlagStatusMap;
    typedef std::list<std::pair<tr1::shared_ptr<const PackageDepSpec>, FlagStatusMap> > PackageFlagStatusMapList;

    struct StackedValues
    {
        std::string origin;

        FlagStatusMap use_mask;
        FlagStatusMap use_force;
        PackageFlagStatusMapList package_use;
        PackageFlagStatusMapList package_use_mask;
        PackageFlagStatusMapList package_use_force;

        StackedValues(const std::string & o) :
            origin(o)
        {
        }
    };

    typedef std::list<StackedValues> StackedValuesList;
}

namespace paludis
{
    /**
     * Implementation for ERepositoryProfile.
     *
     * \ingroup grperepository
     * \see ERepositoryProfile
     */
    template<>
    class Implementation<ERepositoryProfile>
    {
        private:
            void load_environment();
            void load_profile_directory_recursively(const FSEntry & dir);
            void load_profile_parent(const FSEntry & dir);
            void load_profile_make_defaults(const FSEntry & dir);

            void load_basic_use_file(const FSEntry & file, FlagStatusMap & m);
            void load_spec_use_file(const FSEntry & file, PackageFlagStatusMapList & m);

            void add_use_expand_to_use();
            void make_vars_from_file_vars();
            void handle_profile_arch_var();
            void load_special_make_defaults_vars();

            ProfileFile packages_file;
            ProfileFile virtuals_file;
            ProfileFile package_mask_file;

            bool is_incremental(const std::string & s) const;


        public:
            ///\name General variables
            ///\{

            const Environment * const env;
            const ERepository * const repository;

            ///\}

            ///\name Environment variables
            ///\{

            EnvironmentVariablesMap environment_variables;

            ///\}

            ///\name System package set
            ///\{

            tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > system_packages;
            tr1::shared_ptr<GeneralSetDepTag> system_tag;

            ///\}

            ///\name Virtuals
            ///\{

            VirtualsMap virtuals;

            ///\}

            ///\name USE related values
            ///\{

            UseFlagSet use;
            UseFlagSet use_expand;
            UseFlagSet use_expand_hidden;
            StackedValuesList stacked_values_list;

            ///\}

            ///\name Masks
            ///\{

            PackageMaskMap package_mask;

            ///\}

            ///\name Basic operations
            ///\{

            Implementation(const Environment * const e, const ERepository * const p,
                    const RepositoryName & name, const FSEntryCollection & dirs,
                    bool arch_is_special) :
                env(e),
                repository(p),
                system_packages(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                            tr1::shared_ptr<AllDepSpec>(new AllDepSpec))),
                system_tag(new GeneralSetDepTag(SetName("system"), stringify(name)))
            {
                load_environment();

                for (FSEntryCollection::Iterator d(dirs.begin()), d_end(dirs.end()) ;
                        d != d_end ; ++d)
                    load_profile_directory_recursively(*d);

                make_vars_from_file_vars();
                load_special_make_defaults_vars();
                add_use_expand_to_use();
                if (arch_is_special)
                    handle_profile_arch_var();
            }

            ~Implementation()
            {
            }

            ///\}
    };
}

void
Implementation<ERepositoryProfile>::load_environment()
{
    environment_variables["CONFIG_PROTECT"] = getenv_with_default("CONFIG_PROTECT", "/etc");
    environment_variables["CONFIG_PROTECT_MASK"] = getenv_with_default("CONFIG_PROTECT_MASK", "");
}

void
Implementation<ERepositoryProfile>::load_profile_directory_recursively(const FSEntry & dir)
{
    Context context("When adding profile directory '" + stringify(dir) + ":");
    Log::get_instance()->message(ll_debug, lc_context, "Loading profile directory '" + stringify(dir) + "'");

    if (! dir.is_directory_or_symlink_to_directory())
    {
        Log::get_instance()->message(ll_warning, lc_context,
                "Profile component '" + stringify(dir) + "' is not a directory");
        return;
    }

    stacked_values_list.push_back(StackedValues(stringify(dir)));

    load_profile_parent(dir);
    load_profile_make_defaults(dir);

    load_basic_use_file(dir / "use.mask", stacked_values_list.back().use_mask);
    load_basic_use_file(dir / "use.force", stacked_values_list.back().use_force);
    load_spec_use_file(dir / "package.use", stacked_values_list.back().package_use);
    load_spec_use_file(dir / "package.use.mask", stacked_values_list.back().package_use_mask);
    load_spec_use_file(dir / "package.use.force", stacked_values_list.back().package_use_force);

    packages_file.add_file(dir / "packages");
    if (DistributionData::get_instance()->distribution_from_string(env->default_distribution())->support_old_style_virtuals)
        virtuals_file.add_file(dir / "virtuals");
    package_mask_file.add_file(dir / "package.mask");
}

void
Implementation<ERepositoryProfile>::load_profile_parent(const FSEntry & dir)
{
    Context context("When handling parent file for profile directory '" + stringify(dir) + ":");

    if (! (dir / "parent").exists())
        return;

    LineConfigFile file(dir / "parent", LineConfigFileOptions() + lcfo_disallow_continuations + lcfo_disallow_comments);

    LineConfigFile::Iterator i(file.begin()), i_end(file.end());
    bool once(false);
    if (i == i_end)
        Log::get_instance()->message(ll_warning, lc_context, "parent file is empty");
    else
        for ( ; i != i_end ; ++i)
        {
            if ('#' == i->at(0))
            {
                if (! once)
                    Log::get_instance()->message(ll_qa, lc_context, "Comments not allowed in '" + stringify(dir / "parent") + "'");
                once = true;
                continue;
            }

            FSEntry parent_dir(dir);
            do
            {
                try
                {
                    parent_dir = (parent_dir / *i).realpath();
                }
                catch (const FSError & e)
                {
                    Log::get_instance()->message(ll_warning, lc_context, "Skipping parent '"
                            + *i + "' due to exception: " + e.message() + " (" + e.what() + ")");
                    continue;
                }

                load_profile_directory_recursively(parent_dir);

            } while (false);
        }
}

void
Implementation<ERepositoryProfile>::load_profile_make_defaults(const FSEntry & dir)
{
    Context context("When handling make.defaults file for profile directory '" + stringify(dir) + ":");

    if (! (dir / "make.defaults").exists())
        return;

    KeyValueConfigFile file(dir / "make.defaults", KeyValueConfigFileOptions() +
            kvcfo_disallow_space_around_equals + kvcfo_disallow_space_inside_unquoted_values);

    for (KeyValueConfigFile::Iterator k(file.begin()), k_end(file.end()) ;
            k != k_end ; ++k)
    {
        if (is_incremental(k->first))
        {
            std::list<std::string> val, val_add;
            WhitespaceTokeniser::get_instance()->tokenise(environment_variables[k->first], std::back_inserter(val));
            WhitespaceTokeniser::get_instance()->tokenise(k->second, std::back_inserter(val_add));

            for (std::list<std::string>::const_iterator v(val_add.begin()), v_end(val_add.end()) ;
                    v != v_end ; ++v)
            {
                if (v->empty())
                    continue;
                if (*v == "-*")
                    val.clear();
                else if ('-' == v->at(0))
                    val.remove(v->substr(1));
                else
                    val.push_back(*v);
            }

            environment_variables[k->first] = join(val.begin(), val.end(), " ");
        }
        else
            environment_variables[k->first] = k->second;

        Log::get_instance()->message(ll_debug, lc_context, "Profile environment variable '" +
                stringify(k->first) + "' is now '" + stringify(environment_variables[k->first]) + "'");
    }

    try
    {
        use_expand.clear();
        WhitespaceTokeniser::get_instance()->tokenise(environment_variables["USE_EXPAND"],
                create_inserter<UseFlagName>(std::inserter(use_expand, use_expand.end())));
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Loading USE_EXPAND failed due to exception: "
                + e.message() + " (" + e.what() + ")");
    }
}

void
Implementation<ERepositoryProfile>::load_special_make_defaults_vars()
{
    try
    {
        use.clear();
        WhitespaceTokeniser::get_instance()->tokenise(environment_variables["USE"],
                create_inserter<UseFlagName>(std::inserter(use, use.end())));
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Loading USE failed due to exception: "
                + e.message() + " (" + e.what() + ")");
    }

    try
    {
        use_expand.clear();
        WhitespaceTokeniser::get_instance()->tokenise(environment_variables["USE_EXPAND"],
                create_inserter<UseFlagName>(std::inserter(use_expand, use_expand.end())));
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Loading USE_EXPAND failed due to exception: "
                + e.message() + " (" + e.what() + ")");
    }
    try
    {
        use_expand_hidden.clear();
        WhitespaceTokeniser::get_instance()->tokenise(environment_variables["USE_EXPAND_HIDDEN"],
                create_inserter<UseFlagName>(std::inserter(use_expand_hidden, use_expand_hidden.end())));
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Loading USE_EXPAND_HIDDEN failed due to exception: "
                + e.message() + " (" + e.what() + ")");
    }
}

bool
Implementation<ERepositoryProfile>::is_incremental(const std::string & s) const
{
    try
    {
        return (s == "USE") || (s == "USE_EXPAND") || (s == "USE_EXPAND_HIDDEN")
            || (s == "CONFIG_PROTECT") || (s == "CONFIG_PROTECT_MASK")
            || (use_expand.end() != use_expand.find(UseFlagName(s)));
    }
    catch (const Exception &)
    {
        return (s == "USE") || (s == "USE_EXPAND") || (s == "USE_EXPAND_HIDDEN")
            || (s == "CONFIG_PROTECT") || (s == "CONFIG_PROTECT_MASK");
    }
}

void
Implementation<ERepositoryProfile>::make_vars_from_file_vars()
{
    try
    {
        for (ProfileFile::Iterator i(packages_file.begin()), i_end(packages_file.end()) ; i != i_end ; ++i)
        {
            if (0 != i->compare(0, 1, "*", 0, 1))
                continue;

            Context context_spec("When parsing '" + *i + "':");
            tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(i->substr(1), pds_pm_eapi_0));
            spec->set_tag(system_tag);
            system_packages->add(tr1::shared_ptr<SetSpecTree::ConstItem>(new TreeLeaf<SetSpecTree, PackageDepSpec>(spec)));
        }
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Loading packages "
                " failed due to exception: " + e.message() + " (" + e.what() + ")");
    }

    if (DistributionData::get_instance()->distribution_from_string(
                env->default_distribution())->support_old_style_virtuals)
        try
        {
            for (ProfileFile::Iterator line(virtuals_file.begin()), line_end(virtuals_file.end()) ;
                    line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));
                if (tokens.size() < 2)
                    continue;

                QualifiedPackageName v(tokens[0]);
                virtuals.erase(v);
                virtuals.insert(std::make_pair(v, tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(tokens[1],
                                    pds_pm_eapi_0))));
            }
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Loading virtuals "
                    " failed due to exception: " + e.message() + " (" + e.what() + ")");
        }

    for (ProfileFile::Iterator line(package_mask_file.begin()), line_end(package_mask_file.end()) ;
            line != line_end ; ++line)
    {
        if (line->empty())
            continue;

        try
        {
            tr1::shared_ptr<const PackageDepSpec> a(new PackageDepSpec(*line, pds_pm_eapi_0));
            if (a->package_ptr())
                package_mask[*a->package_ptr()].push_back(a);
            else
                Log::get_instance()->message(ll_warning, lc_context, "Loading package.mask spec '"
                        + stringify(*line) + "' failed because specification does not restrict to a "
                        "unique package");
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Loading package.mask spec '"
                    + stringify(*line) + "' failed due to exception '" + e.message() + "' ("
                    + e.what() + ")");
        }
    }
}

void
Implementation<ERepositoryProfile>::load_basic_use_file(const FSEntry & file, FlagStatusMap & m)
{
    if (! file.exists())
        return;

    Context context("When loading basic use file '" + stringify(file) + ":");
    LineConfigFile f(file, LineConfigFileOptions());
    for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
            line != line_end ; ++line)
    {
        std::list<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));

        for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                t != t_end ; ++t)
        {
            try
            {
                if (t->empty())
                    continue;
                if ('-' == t->at(0))
                    m[UseFlagName(t->substr(1))] = false;
                else
                    m[UseFlagName(*t)] = true;
            }
            catch (const Exception & e)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Ignoring token '"
                        + *t + "' due to exception '" + e.message() + "' (" + e.what() + ")");
            }
        }
    }
}

void
Implementation<ERepositoryProfile>::load_spec_use_file(const FSEntry & file, PackageFlagStatusMapList & m)
{
    if (! file.exists())
        return;

    Context context("When loading specised use file '" + stringify(file) + ":");
    LineConfigFile f(file, LineConfigFileOptions());
    for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
            line != line_end ; ++line)
    {
        std::list<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));

        if (tokens.empty())
            continue;

        try
        {
            tr1::shared_ptr<const PackageDepSpec> spec(new PackageDepSpec(*tokens.begin(), pds_pm_eapi_0));
            PackageFlagStatusMapList::iterator n(m.insert(m.end(), std::make_pair(spec, FlagStatusMap())));

            for (std::list<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                    t != t_end ; ++t)
            {
                try
                {
                    if (t->empty())
                        continue;
                    if ('-' == t->at(0))
                        n->second[UseFlagName(t->substr(1))] = false;
                    else
                        n->second[UseFlagName(*t)] = true;
                }
                catch (const Exception & e)
                {
                    Log::get_instance()->message(ll_warning, lc_context, "Ignoring token '"
                            + *t + "' due to exception '" + e.message() + "' (" + e.what() + ")");
                }
            }
        }
        catch (const PackageDepSpecError & e)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Ignoring line '"
                    + *line + "' due to exception '" + e.message() + "' (" + e.what() + ")");
        }
    }
}

void
Implementation<ERepositoryProfile>::add_use_expand_to_use()
{
    Context context("When adding USE_EXPAND to USE:");

    stacked_values_list.push_back(StackedValues("use_expand special values"));

    for (UseFlagSet::const_iterator x(use_expand.begin()), x_end(use_expand.end()) ;
            x != x_end ; ++x)
    {
        std::string lower_x;
        std::transform(x->data().begin(), x->data().end(), std::back_inserter(lower_x),
                &::tolower);

        std::list<std::string> uses;
        WhitespaceTokeniser::get_instance()->tokenise(environment_variables[stringify(*x)],
                std::back_inserter(uses));
        for (std::list<std::string>::const_iterator u(uses.begin()), u_end(uses.end()) ;
                u != u_end ; ++u)
            use.insert(UseFlagName(lower_x + "_" + *u));
    }
}

void
Implementation<ERepositoryProfile>::handle_profile_arch_var()
{
    Context context("When handling profile ARCH variable:");

    std::string arch_s(environment_variables["ARCH"]);
    if (arch_s.empty())
        throw ERepositoryConfigurationError("ARCH variable is unset or empty");

    stacked_values_list.push_back(StackedValues("arch special values"));
    try
    {
        UseFlagName arch(arch_s);

        use.insert(arch);
        stacked_values_list.back().use_force[arch] = true;
    }
    catch (const Exception &)
    {
        throw ERepositoryConfigurationError("ARCH variable has invalid value '" + arch_s + "'");
    }
}

ERepositoryProfile::ERepositoryProfile(
        const Environment * const env, const ERepository * const p, const RepositoryName & name,
        const FSEntryCollection & location,
        bool arch_is_special) :
    PrivateImplementationPattern<ERepositoryProfile>(
            new Implementation<ERepositoryProfile>(env, p, name, location, arch_is_special))
{
}

ERepositoryProfile::~ERepositoryProfile()
{
}

bool
ERepositoryProfile::use_masked(const UseFlagName & u,
        const PackageID & e) const
{
    bool result(false);
    for (StackedValuesList::const_iterator i(_imp->stacked_values_list.begin()),
            i_end(_imp->stacked_values_list.end()) ; i != i_end ; ++i)
    {
        FlagStatusMap::const_iterator f(i->use_mask.find(u));
        if (i->use_mask.end() != f)
            result = f->second;

        for (PackageFlagStatusMapList::const_iterator g(i->package_use_mask.begin()),
                g_end(i->package_use_mask.end()) ; g != g_end ; ++g)
        {
            if (! match_package(*_imp->env, *g->first, e))
                continue;

            FlagStatusMap::const_iterator h(g->second.find(u));
            if (g->second.end() != h)
                result = h->second;
        }
    }

    return result;
}

bool
ERepositoryProfile::use_forced(const UseFlagName & u, const PackageID & e) const
{
    bool result(false);
    for (StackedValuesList::const_iterator i(_imp->stacked_values_list.begin()),
            i_end(_imp->stacked_values_list.end()) ; i != i_end ; ++i)
    {
        FlagStatusMap::const_iterator f(i->use_force.find(u));
        if (i->use_force.end() != f)
            result = f->second;

        for (PackageFlagStatusMapList::const_iterator g(i->package_use_force.begin()),
                g_end(i->package_use_force.end()) ; g != g_end ; ++g)
        {
            if (! match_package(*_imp->env, *g->first, e))
                continue;

            FlagStatusMap::const_iterator h(g->second.find(u));
            if (g->second.end() != h)
                result = h->second;
        }
    }

    return result;
}

UseFlagState
ERepositoryProfile::use_state_ignoring_masks(const UseFlagName & u,
        const PackageID & e) const
{
    UseFlagState result(use_unspecified);

    result = _imp->use.end() != _imp->use.find(u) ? use_enabled : use_unspecified;

    for (StackedValuesList::const_iterator i(_imp->stacked_values_list.begin()),
            i_end(_imp->stacked_values_list.end()) ; i != i_end ; ++i)
    {
        for (PackageFlagStatusMapList::const_iterator g(i->package_use.begin()),
                g_end(i->package_use.end()) ; g != g_end ; ++g)
        {
            if (! match_package(*_imp->env, *g->first, e))
                continue;

            FlagStatusMap::const_iterator h(g->second.find(u));
            if (g->second.end() != h)
                result = h->second ? use_enabled : use_disabled;
        }
    }

    if (use_unspecified == result)
    {
        if (e.iuse_key())
        {
            IUseFlagCollection::Iterator i(e.iuse_key()->value()->find(IUseFlag(u, use_unspecified)));
            if (i != e.iuse_key()->value()->end())
                result = i->state;
        }
    }

    return result;
}

std::string
ERepositoryProfile::environment_variable(const std::string & s) const
{
    EnvironmentVariablesMap::const_iterator i(_imp->environment_variables.find(s));
    if (_imp->environment_variables.end() == i)
        return "";
    else
        return i->second;
}

tr1::shared_ptr<SetSpecTree::ConstItem>
ERepositoryProfile::system_packages() const
{
    return _imp->system_packages;
}

ERepositoryProfile::UseExpandIterator
ERepositoryProfile::begin_use_expand() const
{
    return UseExpandIterator(_imp->use_expand.begin());
}

ERepositoryProfile::UseExpandIterator
ERepositoryProfile::end_use_expand() const
{
    return UseExpandIterator(_imp->use_expand.end());
}

ERepositoryProfile::UseExpandIterator
ERepositoryProfile::begin_use_expand_hidden() const
{
    return UseExpandIterator(_imp->use_expand_hidden.begin());
}

ERepositoryProfile::UseExpandIterator
ERepositoryProfile::end_use_expand_hidden() const
{
    return UseExpandIterator(_imp->use_expand_hidden.end());
}

ERepositoryProfile::VirtualsIterator
ERepositoryProfile::begin_virtuals() const
{
    return VirtualsIterator(_imp->virtuals.begin());
}

ERepositoryProfile::VirtualsIterator
ERepositoryProfile::end_virtuals() const
{
    return VirtualsIterator(_imp->virtuals.end());
}

bool
ERepositoryProfile::profile_masked(const PackageID & id) const
{
    PackageMaskMap::const_iterator rr(_imp->package_mask.find(id.name()));
    if (_imp->package_mask.end() == rr)
        return false;
    else
    {
        for (std::list<tr1::shared_ptr<const PackageDepSpec> >::const_iterator k(rr->second.begin()),
                k_end(rr->second.end()) ; k != k_end ; ++k)
            if (match_package(*_imp->env, **k, id))
                return true;
    }

    return false;
}
