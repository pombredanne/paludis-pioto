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

#include <paludis/repositories/gems/installed_gems_repository.hh>
#include <paludis/repositories/gems/params.hh>
#include <paludis/repositories/gems/gem_specification.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/repository_info.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/eapi.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

using namespace paludis;

typedef MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type IDMap;

namespace paludis
{
    template <>
    struct Implementation<InstalledGemsRepository>
    {
        const gems::InstalledRepositoryParams params;

        mutable tr1::shared_ptr<const CategoryNamePartCollection> category_names;
        mutable MakeHashedMap<CategoryNamePart, tr1::shared_ptr<const QualifiedPackageNameCollection> >::Type package_names;
        mutable IDMap ids;

        mutable bool has_category_names;
        mutable bool has_ids;

        Implementation(const gems::InstalledRepositoryParams p) :
            params(p),
            has_category_names(false),
            has_ids(false)
        {
        }
    };
}

InstalledGemsRepository::InstalledGemsRepository(const gems::InstalledRepositoryParams & params) :
    Repository(RepositoryName("installed-gems"),
            RepositoryCapabilities::create()
            .mask_interface(0)
            .installable_interface(0)
            .installed_interface(this)
            .sets_interface(0)
            .syncable_interface(0)
            .uninstallable_interface(this)
            .use_interface(0)
            .world_interface(0)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            .virtuals_interface(0)
            .provides_interface(0)
            .contents_interface(0)
            .config_interface(0)
            .destination_interface(this)
            .licenses_interface(0)
            .portage_interface(0)
            .make_virtuals_interface(0)
            .pretend_interface(0)
            .hook_interface(0),
            "installed_gems"),
    PrivateImplementationPattern<InstalledGemsRepository>(new Implementation<InstalledGemsRepository>(params))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("install_dir", stringify(_imp->params.install_dir));
    config_info->add_kv("buildroot", stringify(_imp->params.buildroot));

    _info->add_section(config_info);
}

InstalledGemsRepository::~InstalledGemsRepository()
{
}

void
InstalledGemsRepository::invalidate()
{
    _imp.reset(new Implementation<InstalledGemsRepository>(_imp->params));
}

bool
InstalledGemsRepository::do_has_category_named(const CategoryNamePart & c) const
{
    need_category_names();
    return _imp->category_names->end() != _imp->category_names->find(c);
}

bool
InstalledGemsRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    if (! do_has_category_named(q.category))
        return false;

    need_ids();
    return _imp->package_names.find(q.category)->second->end() != _imp->package_names.find(q.category)->second->find(q);
}

tr1::shared_ptr<const CategoryNamePartCollection>
InstalledGemsRepository::do_category_names() const
{
    need_category_names();
    return _imp->category_names;
}

tr1::shared_ptr<const QualifiedPackageNameCollection>
InstalledGemsRepository::do_package_names(const CategoryNamePart & c) const
{
    if (! has_category_named(c))
        return make_shared_ptr(new QualifiedPackageNameCollection::Concrete);

    need_ids();

    MakeHashedMap<CategoryNamePart, tr1::shared_ptr<const QualifiedPackageNameCollection> >::Type::const_iterator i(
            _imp->package_names.find(c));
    if (i == _imp->package_names.end())
        return make_shared_ptr(new QualifiedPackageNameCollection::Concrete);
    return i->second;
}

tr1::shared_ptr<const PackageIDSequence>
InstalledGemsRepository::do_package_ids(const QualifiedPackageName & q) const
{
    if (! has_package_named(q))
        return make_shared_ptr(new PackageIDSequence::Concrete);

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(q));
    if (i == _imp->ids.end())
        return make_shared_ptr(new PackageIDSequence::Concrete);

    return i->second;
}

void
InstalledGemsRepository::need_category_names() const
{
    if (_imp->has_category_names)
        return;

    tr1::shared_ptr<CategoryNamePartCollection::Concrete> cat(new CategoryNamePartCollection::Concrete);
    _imp->category_names = cat;

    cat->insert(CategoryNamePart("gems"));
    _imp->has_category_names = true;
}

void
InstalledGemsRepository::need_ids() const
{
    if (_imp->has_ids)
        return;

    static CategoryNamePart gems("gems");

    Context c("When loading entries for repository '" + stringify(name()) + "':");

    need_category_names();

    tr1::shared_ptr<QualifiedPackageNameCollection::Concrete> pkgs(new QualifiedPackageNameCollection::Concrete);
    _imp->package_names.insert(std::make_pair(gems, pkgs));

    for (DirIterator d(_imp->params.install_dir / "specifications"), d_end ; d != d_end ; ++d)
    {
        if (! is_file_with_extension(*d, ".gemspec", IsFileWithOptions()))
            continue;

        std::string s(strip_trailing_string(d->basename(), ".gemspec"));
        std::string::size_type h(s.rfind('-'));
        if (std::string::npos == h)
        {
            Log::get_instance()->message(ll_qa, lc_context) << "Unrecognised file name format '"
                << *d << "' (no hyphen)";
            continue;
        }

        VersionSpec v(s.substr(h + 1));
        PackageNamePart p(s.substr(0, h));
        pkgs->insert(gems + p);

        if (_imp->ids.end() == _imp->ids.find(gems + p))
            _imp->ids.insert(std::make_pair(gems + p, make_shared_ptr(new PackageIDSequence::Concrete)));
        _imp->ids.find(gems + p)->second->push_back(make_shared_ptr(new gems::GemSpecification(shared_from_this(), p, v, *d)));
    }
}

bool
InstalledGemsRepository::is_suitable_destination_for(const PackageID & e) const
{
    std::string f(e.repository()->format());
    return f == "gems";
}

bool
InstalledGemsRepository::is_default_destination() const
{
    return true;
}

bool
InstalledGemsRepository::want_pre_post_phases() const
{
    return true;
}

void
InstalledGemsRepository::merge(const MergeOptions &)
{
    throw InternalError(PALUDIS_HERE, "Invalid target for merge");
}

FSEntry
InstalledGemsRepository::root() const
{
    return FSEntry("/");
}

void
InstalledGemsRepository::do_uninstall(const tr1::shared_ptr<const PackageID> & id,
        const UninstallOptions &) const
{
    Command cmd(getenv_with_default("PALUDIS_GEMS_DIR", LIBEXECDIR "/paludis") +
            "/gems/gems.bash uninstall '" + stringify(id->name().package) + "' '" + stringify(id->version()) + "'");
    cmd.with_stderr_prefix(stringify(*id) + "> ");
    cmd.with_setenv("GEM_HOME", stringify(_imp->params.install_dir));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Uninstall of '" + stringify(*id) + "' failed");
}

time_t
InstalledGemsRepository::do_installed_time(const PackageID &) const
{
    return 0;
}
