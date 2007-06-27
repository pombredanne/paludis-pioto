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

#include <paludis/repositories/gems/gems_repository.hh>
#include <paludis/repositories/gems/params.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/repositories/gems/gem_specification.hh>
#include <paludis/repositories/gems/gem_specifications.hh>
#include <paludis/repository_info.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/system.hh>
#include <paludis/eapi.hh>
#include <paludis/hashed_containers.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <fstream>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<GemsRepository>
    {
        const gems::RepositoryParams params;

        mutable tr1::shared_ptr<const CategoryNamePartCollection> category_names;
        mutable MakeHashedMap<CategoryNamePart, tr1::shared_ptr<const QualifiedPackageNameCollection> >::Type package_names;
        mutable MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type ids;

        mutable bool has_category_names;
        mutable bool has_ids;

        Implementation(const gems::RepositoryParams p) :
            params(p),
            has_category_names(false),
            has_ids(false)
        {
        }
    };
}

GemsRepository::GemsRepository(const gems::RepositoryParams & params) :
    Repository(RepositoryName("gems"),
            RepositoryCapabilities::create()
            .mask_interface(0)
            .installable_interface(this)
            .installed_interface(0)
            .sets_interface(0)
            .syncable_interface(0)
            .uninstallable_interface(0)
            .use_interface(0)
            .world_interface(0)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            .virtuals_interface(0)
            .provides_interface(0)
            .contents_interface(0)
            .config_interface(0)
            .destination_interface(0)
            .licenses_interface(0)
            .portage_interface(0)
            .pretend_interface(0)
            .make_virtuals_interface(0)
            .hook_interface(0),
            "gems"),
    PrivateImplementationPattern<GemsRepository>(new Implementation<GemsRepository>(params))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("location", stringify(_imp->params.location));
    config_info->add_kv("install_dir", stringify(_imp->params.install_dir));
    config_info->add_kv("buildroot", stringify(_imp->params.buildroot));
    config_info->add_kv("sync", _imp->params.sync);
    config_info->add_kv("sync_options", _imp->params.sync_options);

    _info->add_section(config_info);
}

GemsRepository::~GemsRepository()
{
}

void
GemsRepository::invalidate()
{
    _imp.reset(new Implementation<GemsRepository>(_imp->params));
}

bool
GemsRepository::do_has_category_named(const CategoryNamePart & c) const
{
    need_category_names();
    return _imp->category_names->end() != _imp->category_names->find(c);
}

bool
GemsRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    if (! do_has_category_named(q.category))
        return false;

    need_ids();
    return _imp->package_names.find(q.category)->second->end() != _imp->package_names.find(q.category)->second->find(q);
}

tr1::shared_ptr<const CategoryNamePartCollection>
GemsRepository::do_category_names() const
{
    need_category_names();
    return _imp->category_names;
}

tr1::shared_ptr<const QualifiedPackageNameCollection>
GemsRepository::do_package_names(const CategoryNamePart & c) const
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
GemsRepository::do_package_ids(const QualifiedPackageName & q) const
{
    if (! has_package_named(q))
        return make_shared_ptr(new PackageIDSequence::Concrete);

    need_ids();

    MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type::const_iterator i(
            _imp->ids.find(q));
    if (i == _imp->ids.end())
        return make_shared_ptr(new PackageIDSequence::Concrete);

    return i->second;
}

void
GemsRepository::need_category_names() const
{
    if (_imp->has_category_names)
        return;

    tr1::shared_ptr<CategoryNamePartCollection::Concrete> cat(new CategoryNamePartCollection::Concrete);
    _imp->category_names = cat;

    cat->insert(CategoryNamePart("gems"));
    _imp->has_category_names = true;
}

void
GemsRepository::need_ids() const
{
    if (_imp->has_ids)
        return;

    need_category_names();

    tr1::shared_ptr<QualifiedPackageNameCollection::Concrete> pkgs(new QualifiedPackageNameCollection::Concrete);
    _imp->package_names.insert(std::make_pair(CategoryNamePart("gems"), pkgs));

    Context context("When loading gems yaml file:");

    std::ifstream yaml_file(stringify(_imp->params.location / "yaml").c_str());
    if (! yaml_file)
        throw ConfigurationError("Gems yaml file '" + stringify(_imp->params.location / "yaml") + "' not readable");

    std::string output((std::istreambuf_iterator<char>(yaml_file)), std::istreambuf_iterator<char>());
    yaml::Document master_doc(output);
    gems::GemSpecifications specs(shared_from_this(), *master_doc.top());

    for (gems::GemSpecifications::Iterator i(specs.begin()), i_end(specs.end()) ;
            i != i_end ; ++i)
    {
        pkgs->insert(i->first.first);

        MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type::iterator v(_imp->ids.find(i->first.first));
        if (_imp->ids.end() == v)
            v = _imp->ids.insert(std::make_pair(i->first.first, make_shared_ptr(new PackageIDSequence::Concrete))).first;

        v->second->push_back(i->second);
    }

    _imp->has_ids = true;
}

void
GemsRepository::do_install(const tr1::shared_ptr<const PackageID> & id, const InstallOptions & o) const
{
    if (o.fetch_only)
        return;

    Command cmd(getenv_with_default("PALUDIS_GEMS_DIR", LIBEXECDIR "/paludis") +
            "/gems/gems.bash install '" + stringify(id->name().package) + "' '" + stringify(id->version()) + "'");
    cmd.with_stderr_prefix(stringify(*id) + "> ");
    cmd.with_setenv("GEMCACHE", stringify(_imp->params.location / "yaml"));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Install of '" + stringify(*id) + "' failed");
}
