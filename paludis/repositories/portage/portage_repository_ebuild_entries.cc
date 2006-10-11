/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/portage/portage_repository_ebuild_entries.hh>
#include <paludis/repositories/portage/portage_repository.hh>

#include <paludis/dep_atom_flattener.hh>
#include <paludis/ebuild.hh>
#include <paludis/environment.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/version_metadata.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>

#include <fstream>
#include <list>
#include <set>

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for PortageRepositoryEbuildEntries.
     *
     * \ingroup grpportagerepository
     */
    template<>
    struct Implementation<PortageRepositoryEbuildEntries> :
        InternalCounted<Implementation<PortageRepositoryEbuildEntries> >
    {
        const Environment * const environment;
        PortageRepository * const portage_repository;
        const PortageRepositoryParams params;

        Implementation(const Environment * const e, PortageRepository * const p,
                const PortageRepositoryParams & k) :
            environment(e),
            portage_repository(p),
            params(k)
        {
        }
    };
}

PortageRepositoryEbuildEntries::PortageRepositoryEbuildEntries(
        const Environment * const e, PortageRepository * const p, const PortageRepositoryParams & k) :
    PortageRepositoryEntries(".ebuild"),
    PrivateImplementationPattern<PortageRepositoryEbuildEntries>(new
            Implementation<PortageRepositoryEbuildEntries>(e, p, k))
{
}

PortageRepositoryEbuildEntries::~PortageRepositoryEbuildEntries()
{
}

VersionMetadata::Pointer
PortageRepositoryEbuildEntries::generate_version_metadata(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    VersionMetadata::Pointer result(new VersionMetadata::Ebuild(PortageDepParser::parse_depend));

    FSEntry cache_file(_imp->params.cache);
    cache_file /= stringify(q.category);
    cache_file /= stringify(q.package) + "-" + stringify(v);

    bool ok(false);
    if (cache_file.is_regular_file())
    {
        std::ifstream cache(stringify(cache_file).c_str());
        std::string line;

        if (cache)
        {
            std::getline(cache, line); result->deps.build_depend_string = line;
            std::getline(cache, line); result->deps.run_depend_string = line;
            std::getline(cache, line); result->slot = SlotName(line);
            std::getline(cache, line); result->get_ebuild_interface()->src_uri = line;
            std::getline(cache, line); result->get_ebuild_interface()->restrict_string = line;
            std::getline(cache, line); result->homepage = line;
            std::getline(cache, line); result->license_string = line;
            std::getline(cache, line); result->description = line;
            std::getline(cache, line); result->get_ebuild_interface()->keywords = line;
            std::getline(cache, line); result->get_ebuild_interface()->inherited = line;
            std::getline(cache, line); result->get_ebuild_interface()->iuse = line;
            std::getline(cache, line);
            std::getline(cache, line); result->deps.post_depend_string = line;
            std::getline(cache, line); result->get_ebuild_interface()->provide_string = line;
            std::getline(cache, line); result->eapi = line;

            // check mtimes
            time_t cache_time(cache_file.mtime());
            ok = true;

            if ((_imp->params.location / stringify(q.category) /
                        stringify(q.package) /
                        (stringify(q.package) + "-" + stringify(v)
                            + ".ebuild")).mtime() > cache_time)
                ok = false;
            else
            {
                FSEntry timestamp(_imp->params.location / "metadata" / "timestamp");
                if (timestamp.exists())
                    cache_time = timestamp.mtime();

                std::list<std::string> inherits;
                WhitespaceTokeniser::get_instance()->tokenise(
                        stringify(result->get_ebuild_interface()->inherited),
                        std::back_inserter(inherits));
                for (FSEntryCollection::Iterator e(_imp->params.eclassdirs->begin()),
                        e_end(_imp->params.eclassdirs->end()) ; e != e_end ; ++e)
                    for (std::list<std::string>::const_iterator i(inherits.begin()),
                            i_end(inherits.end()) ; i != i_end ; ++i)
                    {
                        if ((*e / (*i + ".eclass")).exists())
                            if (((*e / (*i + ".eclass"))).mtime() > cache_time)
                                ok = false;
                    }
            }

            if (! ok)
                Log::get_instance()->message(ll_warning, lc_no_context, "Stale cache file at '"
                        + stringify(cache_file) + "'");
        }
        else
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "Couldn't read the cache file at '"
                    + stringify(cache_file) + "'");
    }

    if (! ok)
    {
        if (_imp->params.cache.basename() != "empty")
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "No usable cache entry for '" + stringify(q) +
                    "-" + stringify(v) + "' in '" + stringify(_imp->portage_repository->name()) + "'");

        PackageDatabaseEntry e(q, v, _imp->portage_repository->name());
        EbuildMetadataCommand cmd(EbuildCommandParams::create()
                .environment(_imp->environment)
                .db_entry(&e)
                .ebuild_dir(_imp->params.location / stringify(q.category) /
                            stringify(q.package))
                .files_dir(_imp->params.location / stringify(q.category) /
                            stringify(q.package) / "files")
                .eclassdirs(_imp->params.eclassdirs)
                .portdir(_imp->params.location)
                .distdir(_imp->params.distdir)
                .buildroot(_imp->params.buildroot));

        if (! cmd())
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "No usable metadata for '" + stringify(q)
                    + "-" + stringify(v) + "' in '" + stringify(_imp->portage_repository->name()) + "'");

        if (0 == ((result = cmd.metadata())))
            throw InternalError(PALUDIS_HERE, "cmd.metadata() is zero pointer???");
    }

    return result;

}

namespace
{
    class AAFinder :
        private InstantiationPolicy<AAFinder, instantiation_method::NonCopyableTag>,
        protected DepAtomVisitorTypes::ConstVisitor
    {
        private:
            mutable std::list<const StringDepAtom *> _atoms;

        protected:
            void visit(const AllDepAtom * a)
            {
                std::for_each(a->begin(), a->end(), accept_visitor(
                            static_cast<DepAtomVisitorTypes::ConstVisitor *>(this)));
            }

            void visit(const AnyDepAtom *) PALUDIS_ATTRIBUTE((noreturn))
            {
                throw InternalError(PALUDIS_HERE, "Found unexpected AnyDepAtom");
            }

            void visit(const UseDepAtom * a)
            {
                std::for_each(a->begin(), a->end(), accept_visitor(
                            static_cast<DepAtomVisitorTypes::ConstVisitor *>(this)));
            }

            void visit(const PlainTextDepAtom * a)
            {
                _atoms.push_back(a);
            }

            void visit(const PackageDepAtom * a)
            {
                _atoms.push_back(a);
            }

            void visit(const BlockDepAtom * a)
            {
                _atoms.push_back(a);
            }

        public:
            AAFinder(const DepAtom::ConstPointer a)
            {
                a->accept(static_cast<DepAtomVisitorTypes::ConstVisitor *>(this));
            }

            typedef std::list<const StringDepAtom *>::const_iterator Iterator;

            Iterator begin()
            {
                return _atoms.begin();
            }

            Iterator end() const
            {
                return _atoms.end();
            }
    };

}

void
PortageRepositoryEbuildEntries::install(const QualifiedPackageName & q, const VersionSpec & v,
        const InstallOptions & o, PortageRepositoryProfile::ConstPointer p) const
{
    if (! _imp->portage_repository->has_version(q, v))
    {
        throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                + stringify(v) + "' since has_version failed");
    }

    VersionMetadata::ConstPointer metadata(_imp->portage_repository->version_metadata(q, v));

    PackageDatabaseEntry e(q, v, _imp->portage_repository->name());

    bool fetch_restrict(false), no_mirror(false);
    {
        std::list<std::string> restricts;
        WhitespaceTokeniser::get_instance()->tokenise(
                metadata->get_ebuild_interface()->restrict_string, std::back_inserter(restricts));
        fetch_restrict = (restricts.end() != std::find(restricts.begin(), restricts.end(), "fetch")) ||
            (restricts.end() != std::find(restricts.begin(), restricts.end(), "nofetch"));
        no_mirror = (restricts.end() != std::find(restricts.begin(), restricts.end(), "mirror")) ||
            (restricts.end() != std::find(restricts.begin(), restricts.end(), "nomirror"));
    }

    std::string archives, all_archives, flat_src_uri;
    {
        std::set<std::string> already_in_archives;

        /* make A and FLAT_SRC_URI */
        DepAtom::ConstPointer f_atom(PortageDepParser::parse(metadata->get_ebuild_interface()->src_uri,
                    PortageDepParserPolicy<PlainTextDepAtom, false>::get_instance()));
        DepAtomFlattener f(_imp->params.environment, &e, f_atom);

        for (DepAtomFlattener::Iterator ff(f.begin()), ff_end(f.end()) ; ff != ff_end ; ++ff)
        {
            std::string::size_type pos((*ff)->text().rfind('/'));
            if (std::string::npos == pos)
            {
                if (already_in_archives.end() == already_in_archives.find((*ff)->text()))
                {
                    archives.append((*ff)->text());
                    already_in_archives.insert((*ff)->text());
                }
            }
            else
            {
                if (already_in_archives.end() == already_in_archives.find((*ff)->text().substr(pos + 1)))
                {
                    archives.append((*ff)->text().substr(pos + 1));
                    already_in_archives.insert((*ff)->text().substr(pos + 1));
                }
            }
            archives.append(" ");

            /* add * mirror entries */
            for (Environment::MirrorIterator
                    m(_imp->params.environment->begin_mirrors("*")),
                    m_end(_imp->params.environment->end_mirrors("*")) ;
                    m != m_end ; ++m)
                flat_src_uri.append(m->second + "/" + (*ff)->text().substr(pos + 1) + " ");

            if (0 == (*ff)->text().compare(0, 9, "mirror://"))
            {
                std::string mirror((*ff)->text().substr(9));
                std::string::size_type spos(mirror.find('/'));

                if (std::string::npos == spos)
                    throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                            + stringify(v) + "' since SRC_URI is broken");

                if (! _imp->portage_repository->is_mirror(mirror.substr(0, spos)) &&
                        _imp->params.environment->begin_mirrors(mirror.substr(0, spos)) == 
                        _imp->params.environment->end_mirrors(mirror.substr(0, spos)))
                    throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                            + stringify(v) + "' since SRC_URI references unknown mirror:// '" +
                            mirror.substr(0, spos) + "'");

                for (Environment::MirrorIterator
                        m(_imp->params.environment->begin_mirrors(mirror.substr(0, spos))),
                        m_end(_imp->params.environment->end_mirrors(mirror.substr(0, spos))) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(m->second + "/" + mirror.substr(spos + 1) + " ");

                for (RepositoryMirrorsInterface::MirrorsIterator
                        m(_imp->portage_repository->begin_mirrors(mirror.substr(0, spos))),
                        m_end(_imp->portage_repository->end_mirrors(mirror.substr(0, spos))) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(m->second + "/" + mirror.substr(spos + 1) + " ");
            }
            else
                flat_src_uri.append((*ff)->text());
            flat_src_uri.append(" ");

            /* add mirror://gentoo/ entries */
            std::string master_mirror(strip_trailing_string(stringify(_imp->portage_repository->name()), "x-"));
            if (! no_mirror && _imp->portage_repository->is_mirror(master_mirror))
            {
                for (Environment::MirrorIterator
                        m(_imp->params.environment->begin_mirrors(master_mirror)),
                        m_end(_imp->params.environment->end_mirrors(master_mirror)) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(m->second + "/" + (*ff)->text().substr(pos + 1) + " ");

                for (RepositoryMirrorsInterface::MirrorsIterator
                        m(_imp->portage_repository->begin_mirrors(master_mirror)),
                        m_end(_imp->portage_repository->end_mirrors(master_mirror)) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(m->second + "/" + (*ff)->text().substr(pos + 1) + " ");
            }
        }

        /* make AA */
        DepAtom::ConstPointer g_atom(PortageDepParser::parse(
                    metadata->get_ebuild_interface()->src_uri,
                    PortageDepParserPolicy<PlainTextDepAtom, false>::get_instance()));
        AAFinder g(g_atom);
        std::set<std::string> already_in_all_archives;

        for (AAFinder::Iterator gg(g.begin()), gg_end(g.end()) ; gg != gg_end ; ++gg)
        {
            std::string::size_type pos((*gg)->text().rfind('/'));
            if (std::string::npos == pos)
            {
                if (already_in_all_archives.end() == already_in_all_archives.find((*gg)->text()))
                {
                    all_archives.append((*gg)->text());
                    already_in_all_archives.insert((*gg)->text());
                }
            }
            else
            {
                if (already_in_all_archives.end() == already_in_all_archives.find((*gg)->text().substr(pos + 1)))
                {
                    all_archives.append((*gg)->text().substr(pos + 1));
                    already_in_all_archives.insert((*gg)->text().substr(pos + 1));
                }
            }
            all_archives.append(" ");
        }
    }

    /* Strip trailing space. Some ebuilds rely upon this. From kde-meta.eclass:
     *     [[ -n ${A/${TARBALL}/} ]] && unpack ${A/${TARBALL}/}
     * Rather annoying.
     */
    archives = strip_trailing(archives, " ");
    all_archives = strip_trailing(all_archives, " ");

    /* make use */
    std::string use;
    {
        std::set<UseFlagName> iuse;
        WhitespaceTokeniser::get_instance()->tokenise(metadata->get_ebuild_interface()->
                iuse, create_inserter<UseFlagName>(std::inserter(iuse, iuse.begin())));
        for (std::set<UseFlagName>::const_iterator iuse_it(iuse.begin()), iuse_end(iuse.end()) ;
                iuse_it != iuse_end; ++iuse_it)
            if (_imp->params.environment->query_use(*iuse_it, &e))
                use += (*iuse_it).data() + " ";
    }

    use += p->environment_variable("ARCH") + " ";

    /* add expand to use (iuse isn't reliable for use_expand things), and make the expand
     * environment variables */
    AssociativeCollection<std::string, std::string>::Pointer expand_vars(
            new AssociativeCollection<std::string, std::string>::Concrete);
    for (PortageRepositoryProfile::UseExpandIterator x(p->begin_use_expand()),
            x_end(p->end_use_expand()) ; x != x_end ; ++x)
    {
        std::string lower_x;
        std::transform(x->data().begin(), x->data().end(), std::back_inserter(lower_x), &::tolower);

        expand_vars->insert(stringify(*x), "");

        /* possible values from profile */
        std::set<UseFlagName> possible_values;
        WhitespaceTokeniser::get_instance()->tokenise(p->environment_variable(stringify(*x)),
                create_inserter<UseFlagName>(std::inserter(possible_values, possible_values.end())));

        /* possible values from environment */
        UseFlagNameCollection::ConstPointer possible_values_from_env(_imp->params.environment->
                known_use_expand_names(*x, &e));
        for (UseFlagNameCollection::Iterator i(possible_values_from_env->begin()),
                i_end(possible_values_from_env->end()) ; i != i_end ; ++i)
            possible_values.insert(UseFlagName(stringify(*i).substr(lower_x.length() + 1)));

        for (std::set<UseFlagName>::const_iterator u(possible_values.begin()), u_end(possible_values.end()) ;
                u != u_end ; ++u)
        {
            if (! _imp->params.environment->query_use(UseFlagName(lower_x + "_" + stringify(*u)), &e))
                continue;

            use.append(lower_x + "_" + stringify(*u) + " ");

            std::string value;
            AssociativeCollection<std::string, std::string>::Iterator i(expand_vars->find(stringify(*x)));
            if (expand_vars->end() != i)
            {
                value = i->second;
                if (! value.empty())
                    value.append(" ");
                expand_vars->erase(i);
            }
            value.append(stringify(*u));
            expand_vars->insert(stringify(*x), value);
        }
    }

    EbuildFetchCommand fetch_cmd(EbuildCommandParams::create()
            .environment(_imp->params.environment)
            .db_entry(&e)
            .ebuild_dir(_imp->params.location / stringify(q.category) /
                        stringify(q.package))
            .files_dir(_imp->params.location / stringify(q.category) /
                        stringify(q.package) / "files")
            .eclassdirs(_imp->params.eclassdirs)
            .portdir(_imp->params.location)
            .distdir(_imp->params.distdir)
            .buildroot(_imp->params.buildroot),

            EbuildFetchCommandParams::create()
            .a(archives)
            .aa(all_archives)
            .use(use)
            .use_expand(join(p->begin_use_expand(), p->end_use_expand(), " "))
            .expand_vars(expand_vars)
            .flat_src_uri(flat_src_uri)
            .root(stringify(_imp->params.root) + "/")
            .profiles(_imp->params.profiles)
            .no_fetch(fetch_restrict));

    fetch_cmd();

    if (o.fetch_only)
        return;

    EbuildInstallCommand install_cmd(EbuildCommandParams::create()
            .environment(_imp->params.environment)
            .db_entry(&e)
            .ebuild_dir(_imp->params.location / stringify(q.category) /
                        stringify(q.package))
            .files_dir(_imp->params.location / stringify(q.category) /
                        stringify(q.package) / "files")
            .eclassdirs(_imp->params.eclassdirs)
            .portdir(_imp->params.location)
            .distdir(_imp->params.distdir)
            .buildroot(_imp->params.buildroot),

            EbuildInstallCommandParams::create()
                    .use(use)
                    .a(archives)
                    .aa(all_archives)
                    .use_expand(join(p->begin_use_expand(), p->end_use_expand(), " "))
                    .expand_vars(expand_vars)
                    .root(stringify(_imp->params.root) + "/")
                    .profiles(_imp->params.profiles)
                    .disable_cfgpro(o.no_config_protect)
                    .slot(SlotName(metadata->slot)));

    install_cmd();
}

std::string
PortageRepositoryEbuildEntries::get_environment_variable(const QualifiedPackageName & q,
        const VersionSpec & v, const std::string & var,
        PortageRepositoryProfile::ConstPointer) const
{
    PackageDatabaseEntry for_package(q, v, _imp->portage_repository->name());

    EbuildVariableCommand cmd(EbuildCommandParams::create()
            .environment(_imp->params.environment)
            .db_entry(&for_package)
            .ebuild_dir(_imp->params.location / stringify(q.category) /
                        stringify(q.package))
            .files_dir(_imp->params.location / stringify(q.category) /
                        stringify(q.package) / "files")
            .eclassdirs(_imp->params.eclassdirs)
            .portdir(_imp->params.location)
            .distdir(_imp->params.distdir)
            .buildroot(_imp->params.buildroot),

            var);

    if (! cmd())
        throw EnvironmentVariableActionError("Couldn't get environment variable '" +
                stringify(var) + "' for package '" + stringify(for_package) + "'");

    return cmd.result();
}

PortageRepositoryEbuildEntries::Pointer
PortageRepositoryEbuildEntries::make_portage_repository_ebuild_entries(
        const Environment * const e, PortageRepository * const r, const PortageRepositoryParams & p)
{
    return Pointer(new PortageRepositoryEbuildEntries(e, r, p));
}

#ifdef PALUDIS_ENABLE_VISIBILITY
#  pragma GCC visibility push(default)
#endif
namespace
{
    const PortageRepositoryEntriesMaker::RegisterMaker register_portage_repository_ebuild_entries PALUDIS_ATTRIBUTE((used)) (
            "ebuild", &PortageRepositoryEbuildEntries::make_portage_repository_ebuild_entries);
}
#ifdef PALUDIS_ENABLE_VISIBILITY
#  pragma GCC visibility pop
#endif

