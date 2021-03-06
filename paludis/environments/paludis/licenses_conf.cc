/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#include "licenses_conf.hh"
#include <paludis/environment.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/util/config_file.hh>
#include <paludis/package_id.hh>
#include <paludis/util/options.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <tr1/unordered_map>
#include <list>
#include <vector>
#include <map>

using namespace paludis;
using namespace paludis::paludis_environment;

typedef std::list<std::string> LicensesList;
typedef std::map<std::tr1::shared_ptr<const PackageDepSpec>, LicensesList> PDSToLicensesList;
typedef std::pair<std::tr1::shared_ptr<const SetSpecTree>, LicensesList> SetNameEntry;

typedef std::tr1::unordered_map<QualifiedPackageName, PDSToLicensesList, Hash<QualifiedPackageName> > SpecificMap;
typedef PDSToLicensesList UnspecificMap;
typedef std::tr1::unordered_map<SetName, SetNameEntry, Hash<SetName> > NamedSetMap;

namespace paludis
{
    template<>
    struct Implementation<LicensesConf>
    {
        const PaludisEnvironment * const env;

        SpecificMap qualified;
        UnspecificMap unqualified;
        mutable NamedSetMap set;
        mutable Mutex set_mutex;

        Implementation(const PaludisEnvironment * const e) :
            env(e)
        {
        }
    };
}

LicensesConf::LicensesConf(const PaludisEnvironment * const e) :
    PrivateImplementationPattern<LicensesConf>(new Implementation<LicensesConf>(e))
{
}

LicensesConf::~LicensesConf()
{
}

void
LicensesConf::add(const FSEntry & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as a licenses file:");

    std::tr1::shared_ptr<LineConfigFile> f(make_bashable_conf(filename, LineConfigFileOptions()));
    if (! f)
        return;

    for (LineConfigFile::ConstIterator line(f->begin()), line_end(f->end()) ;
            line != line_end ; ++line)
    {
        std::vector<std::string> tokens;
        tokenise_whitespace_quoted(*line, std::back_inserter(tokens));

        if (tokens.size() < 2)
            continue;

        try
        {
            std::tr1::shared_ptr<PackageDepSpec> d(new PackageDepSpec(parse_user_package_dep_spec(
                            tokens.at(0), _imp->env,
                            UserPackageDepSpecOptions() + updso_allow_wildcards + updso_no_disambiguation + updso_throw_if_set)));
            if (d->package_ptr())
            {
                LicensesList & k(_imp->qualified[*d->package_ptr()][d]);
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    k.push_back(*t);
            }
            else
            {
                LicensesList & k(_imp->unqualified[d]);
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    k.push_back(*t);
            }
        }
        catch (const GotASetNotAPackageDepSpec &)
        {
            NamedSetMap::iterator i(_imp->set.insert(std::make_pair(SetName(tokens.at(0)), std::make_pair(
                                make_null_shared_ptr(), LicensesList()))).first);

            for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                    t != t_end ; ++t)
                i->second.second.push_back(*t);
        }
    }
}

bool
LicensesConf::query(const std::string & t, const PackageID & e) const
{
    /* highest priority: specific */
    bool break_when_done(false);
    {
        SpecificMap::const_iterator i(_imp->qualified.find(e.name()));
        if (i != _imp->qualified.end())
        {
            for (PDSToLicensesList::const_iterator j(i->second.begin()), j_end(i->second.end()) ;
                    j != j_end ; ++j)
            {
                if (! match_package(*_imp->env, *j->first, e, MatchPackageOptions()))
                    continue;

                for (LicensesList::const_iterator l(j->second.begin()), l_end(j->second.end()) ;
                        l != l_end ; ++l)
                {
                    if (*l == t)
                        return true;

                    if (*l == "*")
                        return true;

                    if (*l == "-*")
                        break_when_done = true;
                }
            }
        }
    }

    if (break_when_done)
        return false;

    /* next: named sets */
    {
        Lock lock(_imp->set_mutex);
        for (NamedSetMap::iterator i(_imp->set.begin()), i_end(_imp->set.end()) ;
                 i != i_end ; ++i)
        {
            if (! i->second.first)
            {
                i->second.first = _imp->env->set(i->first);
                if (! i->second.first)
                {
                    Log::get_instance()->message("paludis_environment.licenses_conf.unknown_set", ll_warning, lc_no_context) << "Set name '"
                        << i->first << "' does not exist";
                    i->second.first.reset(new SetSpecTree(make_shared_ptr(new AllDepSpec)));
                }
            }

            if (! match_package_in_set(*_imp->env, *i->second.first, e, MatchPackageOptions()))
                continue;

            for (LicensesList::const_iterator l(i->second.second.begin()), l_end(i->second.second.end()) ;
                    l != l_end ; ++l)
            {
                if (*l == t)
                    return true;

                if (*l == "*")
                    return true;

                if (*l == "-*")
                    break_when_done = true;
            }
        }
    }


    if (break_when_done)
        return false;

    /* last: unspecific */
    for (PDSToLicensesList::const_iterator j(_imp->unqualified.begin()), j_end(_imp->unqualified.end()) ;
            j != j_end ; ++j)
    {
        if (! match_package(*_imp->env, *j->first, e, MatchPackageOptions()))
            continue;

        for (LicensesList::const_iterator l(j->second.begin()), l_end(j->second.end()) ;
                l != l_end ; ++l)
        {
            if (*l == t)
                return true;

            if (*l == "*")
                return true;
        }
    }

    return false;
}

