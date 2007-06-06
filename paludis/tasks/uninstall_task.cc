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

#include "uninstall_task.hh"
#include <paludis/environment.hh>
#include <paludis/dep_list/uninstall_list.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/tasks/exceptions.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/query.hh>
#include <paludis/hook.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<UninstallTask>
    {
        Environment * const env;
        UninstallOptions uninstall_options;

        std::list<std::string> raw_targets;
        std::list<tr1::shared_ptr<PackageDepSpec> > targets;

        bool pretend;
        bool preserve_world;
        bool all_versions;
        bool with_unused_dependencies;
        bool with_dependencies;
        bool unused;

        bool had_set_targets;
        bool had_package_targets;

        Implementation<UninstallTask>(Environment * const e) :
            env(e),
            uninstall_options(false),
            pretend(false),
            preserve_world(false),
            all_versions(false),
            with_unused_dependencies(false),
            with_dependencies(false),
            unused(false),
            had_set_targets(false),
            had_package_targets(false)
        {
        }
    };
}

UninstallTask::UninstallTask(Environment * const e) :
    PrivateImplementationPattern<UninstallTask>(new Implementation<UninstallTask>(e))
{
}

UninstallTask::~UninstallTask()
{
}

void
UninstallTask::set_pretend(const bool v)
{
    _imp->pretend = v;
}

void
UninstallTask::set_no_config_protect(const bool v)
{
    _imp->uninstall_options.no_config_protect = v;
}

void
UninstallTask::set_preserve_world(const bool v)
{
    _imp->preserve_world = v;
}

void
UninstallTask::add_target(const std::string & target)
{
    Context context("When adding uninstall target '" + target + "':");

    /* we might have a dep spec, but we might just have a simple package name
     * without a category. either should work. */
    if (std::string::npos != target.find('/'))
    {
        if (_imp->had_set_targets)
            throw HadBothPackageAndSetTargets();

        _imp->had_package_targets = true;
        tr1::shared_ptr<PackageDepSpec> pds(new PackageDepSpec(target, pds_pm_permissive));
        pds->set_tag(tr1::shared_ptr<const DepTag>(new TargetDepTag));
        _imp->targets.push_back(pds);
    }
    else
        try
        {
            tr1::shared_ptr<SetSpecTree::ConstItem> spec(_imp->env->set(SetName(target)));
            if (spec)
            {
                if (_imp->had_package_targets)
                    throw HadBothPackageAndSetTargets();

                if (_imp->had_set_targets)
                    throw MultipleSetTargetsSpecified();

                _imp->had_set_targets = true;
                DepSpecFlattener f(_imp->env, 0);
                spec->accept(f);
                for (DepSpecFlattener::Iterator i(f.begin()), i_end(f.end()) ; i != i_end ; ++i)
                    _imp->targets.push_back(tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(
                                    stringify((*i)->text()), pds_pm_permissive)));
            }
            else
            {
                if (_imp->had_set_targets)
                    throw HadBothPackageAndSetTargets();

                _imp->had_package_targets = false;
                tr1::shared_ptr<PackageDepSpec> pds(new PackageDepSpec(
                                tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(
                                        _imp->env->package_database()->fetch_unique_qualified_package_name(
                                            PackageNamePart(target))))));
                pds->set_tag(tr1::shared_ptr<const DepTag>(new TargetDepTag));
                _imp->targets.push_back(pds);
            }
        }
        catch (const SetNameError &)
        {
            if (_imp->had_set_targets)
                throw HadBothPackageAndSetTargets();

            _imp->had_package_targets = false;
            tr1::shared_ptr<PackageDepSpec> pds(new PackageDepSpec(
                            tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(
                                    _imp->env->package_database()->fetch_unique_qualified_package_name(
                                        PackageNamePart(target))))));
            pds->set_tag(tr1::shared_ptr<const DepTag>(new TargetDepTag));
            _imp->targets.push_back(pds);
        }

    _imp->raw_targets.push_back(target);

    if (_imp->unused)
        throw InternalError(PALUDIS_HERE, "Trying to mix unused and normal targets?");
}

void
UninstallTask::add_unused()
{
    Context context("When adding unused packages:");
    _imp->unused = true;

    if (! _imp->raw_targets.empty())
        throw InternalError(PALUDIS_HERE, "Trying to mix unused and normal targets?");
}

void
UninstallTask::execute()
{
    Context context("When executing uninstall task:");

    on_build_unmergelist_pre();

    UninstallList list(_imp->env, UninstallListOptions::create()
            .with_dependencies(_imp->with_dependencies)
            .with_unused_dependencies(_imp->with_unused_dependencies));

    if (_imp->unused)
        list.add_unused();
    else
        for (std::list<tr1::shared_ptr<PackageDepSpec> >::const_iterator t(_imp->targets.begin()),
                t_end(_imp->targets.end()) ; t != t_end ; ++t)
        {
            Context local_context("When looking for target '" + stringify(**t) + "':");

            tr1::shared_ptr<const PackageDatabaseEntryCollection> r(_imp->env->package_database()->query(
                        query::Matches(**t) & query::RepositoryHasUninstallableInterface(), qo_order_by_version));
            if (r->empty())
            {
                if (! _imp->had_set_targets)
                    throw NoSuchPackageError(stringify(**t));
            }
            else if (next(r->begin()) != r->end())
            {
                if (_imp->all_versions)
                {
                    /* all_versions, not all_packages. */
                    for (PackageDatabaseEntryCollection::Iterator i_start(r->begin()), i(r->begin()),
                            i_end(r->end()) ; i != i_end ; ++i)
                        if (i->name != i_start->name)
                            throw AmbiguousUnmergeTargetError(stringify(**t), r);

                    for (PackageDatabaseEntryCollection::Iterator i(r->begin()), i_end(r->end()) ;
                            i != i_end ; ++i)
                        list.add(*i);
                }
                else
                    throw AmbiguousUnmergeTargetError(stringify(**t), r);
            }
            else
                list.add(*r->begin());
        }

    on_build_unmergelist_post();

    on_display_unmerge_list_pre();

    for (UninstallList::Iterator i(list.begin()), i_end(list.end()) ; i != i_end ; ++i)
        on_display_unmerge_list_entry(*i);

    on_display_unmerge_list_post();

    if (_imp->pretend)
        return;

    if (_imp->preserve_world)
        on_preserve_world();
    else
    {
        on_update_world_pre();

        tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > all(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                    tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));

        std::map<QualifiedPackageName, std::set<VersionSpec> > being_removed;
        for (UninstallList::Iterator i(list.begin()), i_end(list.end()) ; i != i_end ; ++i)
            if (! i->skip_uninstall)
                being_removed[i->package.name].insert(i->package.version);

        for (std::map<QualifiedPackageName, std::set<VersionSpec> >::const_iterator
                i(being_removed.begin()), i_end(being_removed.end()) ; i != i_end ; ++i)
        {
            bool remove(true);
            tr1::shared_ptr<PackageDatabaseEntryCollection> installed(
                    _imp->env->package_database()->query(query::Matches(PackageDepSpec(
                                tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(i->first)))) &
                        query::RepositoryHasInstalledInterface(), qo_whatever));
            for (PackageDatabaseEntryCollection::Iterator r(installed->begin()), r_end(installed->end()) ;
                    r != r_end && remove ; ++r)
                if (i->second.end() == i->second.find(r->version))
                    remove = false;

            if (remove)
                all->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                                tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(
                                        tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(i->first)))))));
        }

        world_remove_packages(all);

        if (_imp->had_set_targets)
            for (std::list<std::string>::const_iterator t(_imp->raw_targets.begin()),
                    t_end(_imp->raw_targets.end()) ; t != t_end ; ++t)
                world_remove_set(SetName(*t));

        on_update_world_post();
    }

    if (0 !=
        _imp->env->perform_hook(Hook("uninstall_all_pre")("TARGETS", join(_imp->raw_targets.begin(),
                         _imp->raw_targets.end(), " "))).max_exit_status)
        throw PackageUninstallActionError("Uninstall aborted by hook");
    on_uninstall_all_pre();

    int x(0), y(0);
    for (UninstallList::Iterator i(list.begin()), i_end(list.end()) ; i != i_end ; ++i)
        if (! i->skip_uninstall)
            ++y;

    for (UninstallList::Iterator i(list.begin()), i_end(list.end()) ; i != i_end ; ++i)
    {
        if (i->skip_uninstall)
            continue;
        ++x;

        std::string cpvr(stringify(i->package));

        if (0 !=
            _imp->env->perform_hook(Hook("uninstall_pre")("TARGET", cpvr)
                     ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status)
            throw PackageUninstallActionError("Uninstall of '" + cpvr + "' aborted by hook");
        on_uninstall_pre(*i);

        const RepositoryUninstallableInterface * const uninstall_interface(
                _imp->env->package_database()->fetch_repository(i->package.repository)->
                uninstallable_interface);
        if (! uninstall_interface)
            throw InternalError(PALUDIS_HERE, "Trying to uninstall from a non-uninstallable repo");

        try
        {
            uninstall_interface->uninstall(i->package.name, i->package.version, _imp->uninstall_options);
        }
        catch (const PackageUninstallActionError & e)
        {
            HookResult PALUDIS_ATTRIBUTE((unused)) dummy(_imp->env->perform_hook(Hook("uninstall_fail")("TARGET", cpvr)("MESSAGE", e.message())));
            throw;
        }

        on_uninstall_post(*i);
        if (0 !=
            _imp->env->perform_hook(Hook("uninstall_post")("TARGET", cpvr)
                     ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status)
            throw PackageUninstallActionError("Uninstall of '" + cpvr + "' aborted by hook");
    }

    on_uninstall_all_post();
    if (0 !=
        _imp->env->perform_hook(Hook("uninstall_all_post")("TARGETS", join(_imp->raw_targets.begin(),
                         _imp->raw_targets.end(), " "))).max_exit_status)
        throw PackageUninstallActionError("Uninstall aborted by hook");
}

AmbiguousUnmergeTargetError::~AmbiguousUnmergeTargetError() throw ()
{
}

void
UninstallTask::set_with_unused_dependencies(const bool value)
{
    _imp->with_unused_dependencies = value;
}

void
UninstallTask::set_with_dependencies(const bool value)
{
    _imp->with_dependencies = value;
}

void
UninstallTask::set_all_versions(const bool value)
{
    _imp->all_versions = value;
}

void
UninstallTask::world_remove_set(const SetName & s)
{
    for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->world_interface)
            (*r)->world_interface->remove_from_world(s);

    on_update_world(s);
}

namespace
{
    struct WorldTargetFinder :
        ConstVisitor<SetSpecTree>,
        ConstVisitor<SetSpecTree>::VisitConstSequence<WorldTargetFinder, AllDepSpec>
    {
        using ConstVisitor<SetSpecTree>::VisitConstSequence<WorldTargetFinder, AllDepSpec>::visit_sequence;

        UninstallTask * const task;
        std::list<const PackageDepSpec *> items;
        bool inside_any;
        bool inside_use;

        WorldTargetFinder(UninstallTask * const t) :
            task(t),
            inside_any(false),
            inside_use(false)
        {
        }

        void visit_leaf(const PackageDepSpec & a)
        {
            if (! (inside_any || inside_use || a.slot_ptr() ||
                        (a.version_requirements_ptr() && ! a.version_requirements_ptr()->empty())))
            {
                items.push_back(&a);
                task->on_update_world(a);
            }
        }
    };
}

void
UninstallTask::world_remove_packages(tr1::shared_ptr<const SetSpecTree::ConstItem> a)
{
    WorldTargetFinder w(this);
    a->accept(w);
    for (std::list<const PackageDepSpec *>::const_iterator i(w.items.begin()),
            i_end(w.items.end()) ; i != i_end ; ++i)
    {
        for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
                r_end(_imp->env->package_database()->end_repositories()) ;
                r != r_end ; ++r)
            if ((*r)->world_interface && (*i)->package_ptr())
                (*r)->world_interface->remove_from_world(*(*i)->package_ptr());
    }
}

