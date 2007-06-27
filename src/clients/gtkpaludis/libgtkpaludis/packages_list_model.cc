/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "main_window.hh"
#include "markup.hh"
#include "packages_list_model.hh"
#include "packages_page.hh"
#include <paludis/dep_spec_flattener.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/visitor-impl.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <list>
#include <algorithm>
#include <set>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesListModel>
    {
        MainWindow * const main_window;
        PackagesPage * const packages_page;
        PackagesListModel::Columns columns;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            main_window(m),
            packages_page(p)
        {
        }
    };
}

namespace
{
    struct PopulateItem
    {
        std::string title;
        std::string status_markup;
        std::string description;
        tr1::shared_ptr<const QualifiedPackageName> qpn;
        PackagesPackageFilterOption local_best_option;
        std::list<PopulateItem> children;
        bool merge_if_one_child;

        const PackagesPackageFilterOption children_best_option() const
        {
            PackagesPackageFilterOption result(local_best_option);
            for (std::list<PopulateItem>::const_iterator i(children.begin()), i_end(children.end()) ;
                    i != i_end ; ++i)
                result = std::max(result, i->children_best_option());

            return result;
        }

        PopulateItem(const std::string & t) :
            title(t),
            local_best_option(ppfo_all_packages),
            merge_if_one_child(true)
        {
        }
    };
}

namespace gtkpaludis
{
    struct PackagesListModel::PopulateData
    {
        std::list<PopulateItem> items;
    };

    struct PackagesListModel::PopulateDataIterator :
        libwrapiter::ForwardIterator<PackagesListModel::PopulateDataIterator, const PopulateItem>
    {
        PopulateDataIterator(const std::list<PopulateItem>::const_iterator & i) :
            libwrapiter::ForwardIterator<PackagesListModel::PopulateDataIterator, const PopulateItem>(i)
        {
        }
    };
}


PackagesListModel::PackagesListModel(MainWindow * const m, PackagesPage * const p) :
    PrivateImplementationPattern<PackagesListModel>(new Implementation<PackagesListModel>(m, p)),
    Gtk::TreeStore(_imp->columns)
{
}

PackagesListModel::~PackagesListModel()
{
}

PackagesListModel::Columns::Columns()
{
    add(col_package);
    add(col_status_markup);
    add(col_description);
    add(col_qpn);
    add(col_best_package_filter_option);
}

PackagesListModel::Columns::~Columns()
{
}

PackagesListModel::Columns &
PackagesListModel::columns()
{
    return _imp->columns;
}

void
PackagesListModel::populate()
{
    _imp->main_window->paludis_thread_action(
            sigc::mem_fun(this, &PackagesListModel::populate_in_paludis_thread), "Populating packages list model");
}

namespace
{
    PopulateItem make_item(const PackageDepSpec & pds,
            const QualifiedPackageName & qpn,
            paludis::tr1::shared_ptr<const VersionMetadata> metadata,
            const Environment * const environment)
    {
        PackagesPackageFilterOption best_option(ppfo_all_packages);
        std::string status;
        paludis::tr1::shared_ptr<const PackageDatabaseEntryCollection> ci(
                environment->package_database()->query(
                    query::InstalledAtRoot(environment->root()) &
                    query::Matches(pds) &
                    query::Matches(PackageDepSpec(
                            paludis::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(qpn)),
                            paludis::tr1::shared_ptr<CategoryNamePart>(),
                            paludis::tr1::shared_ptr<PackageNamePart>(),
                            paludis::tr1::shared_ptr<VersionRequirements>(),
                            vr_and,
                            paludis::tr1::shared_ptr<SlotName>(new SlotName(metadata->slot)))),
                    qo_order_by_version));

        paludis::tr1::shared_ptr<const PackageDatabaseEntryCollection> av(
                environment->package_database()->query(
                    query::RepositoryHasInstallableInterface() &
                    query::Matches(pds) &
                    query::Matches(PackageDepSpec(
                            paludis::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(qpn)),
                            paludis::tr1::shared_ptr<CategoryNamePart>(),
                            paludis::tr1::shared_ptr<PackageNamePart>(),
                            paludis::tr1::shared_ptr<VersionRequirements>(),
                            vr_and,
                            paludis::tr1::shared_ptr<SlotName>(new SlotName(metadata->slot)))) &
                    query::NotMasked(),
                    qo_order_by_version));

        if (! ci->empty())
        {
            status = markup_escape(stringify(ci->last()->version));
            best_option = ppfo_installed_packages;

            if (! av->empty())
            {
                if (av->last()->version < ci->last()->version)
                {
                    status.append(markup_bold(markup_escape(" > " + stringify(av->last()->version))));
                    best_option = ppfo_upgradable_packages;
                }
                else if (av->last()->version > ci->last()->version)
                {
                    status.append(markup_bold(markup_escape(" < " + stringify(av->last()->version))));
                    best_option = ppfo_upgradable_packages;
                }
            }
        }
        else
        {
            paludis::tr1::shared_ptr<const PackageDatabaseEntryCollection> av(
                    environment->package_database()->query(
                        query::Matches(pds) &
                        query::Matches(PackageDepSpec(
                                paludis::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(qpn)),
                                paludis::tr1::shared_ptr<CategoryNamePart>(),
                                paludis::tr1::shared_ptr<PackageNamePart>(),
                                paludis::tr1::shared_ptr<VersionRequirements>(),
                                vr_and,
                                paludis::tr1::shared_ptr<SlotName>(new SlotName(metadata->slot)))) &
                        query::RepositoryHasInstallableInterface() &
                        query::NotMasked(),
                        qo_order_by_version));
            if (av->empty())
            {
                status.append(markup_foreground("grey", markup_escape("masked")));
                best_option = ppfo_all_packages;
            }
            else
            {
                status.append(markup_foreground("grey", markup_escape(stringify(av->last()->version))));
                best_option = ppfo_visible_packages;
            }
        }

        PopulateItem result(stringify(metadata->slot));
        result.status_markup = status;
        result.description = metadata->description;
        result.qpn = make_shared_ptr(new QualifiedPackageName(qpn));
        result.local_best_option = best_option;
        return result;
    }
}

void
PackagesListModel::populate_in_paludis_thread()
{
    paludis::tr1::shared_ptr<PopulateData> data(new PopulateData);

    if (_imp->packages_page->get_category())
    {
        paludis::tr1::shared_ptr<const PackageDatabaseEntryCollection> c(
                _imp->main_window->environment()->package_database()->query(
                    *_imp->packages_page->get_repository_filter() &
                    query::Category(*_imp->packages_page->get_category()),
                    qo_best_version_in_slot_only));

        QualifiedPackageName old_qpn("OLD/OLD");

        for (PackageDatabaseEntryCollection::ReverseIterator p(c->rbegin()), p_end(c->rend()) ;
                p != p_end ; ++p)
        {
            paludis::tr1::shared_ptr<const VersionMetadata> metadata(
                    _imp->main_window->environment()->package_database()->fetch_repository(p->repository)->version_metadata(
                        p->name, p->version));

            if (old_qpn != p->name)
            {
                data->items.push_front(PopulateItem(stringify(p->name.package)));
                data->items.begin()->children.push_front(make_item(
                            PackageDepSpec(make_shared_ptr(new QualifiedPackageName(p->name))),
                            p->name, metadata, _imp->main_window->environment()));
                data->items.begin()->qpn = data->items.begin()->children.begin()->qpn;
                old_qpn = p->name;
            }
            else
                data->items.begin()->children.push_front(make_item(
                            PackageDepSpec(make_shared_ptr(new QualifiedPackageName(p->name))),
                            p->name, metadata, _imp->main_window->environment()));
        }
    }
    else if (_imp->packages_page->get_set())
    {
        DepSpecFlattener f(_imp->main_window->environment(), 0);
        _imp->main_window->environment()->set(*_imp->packages_page->get_set())->accept(f);
        std::set<std::string> a;
        std::transform(indirect_iterator(f.begin()), indirect_iterator(f.end()), std::inserter(a, a.begin()),
                tr1::mem_fn(&StringDepSpec::text));

        for (std::set<std::string>::const_iterator i(a.begin()), i_end(a.end()) ;
                i != i_end ; ++i)
        {
            std::list<PopulateItem>::iterator atom_iter(data->items.insert(data->items.end(), PopulateItem(*i)));
            atom_iter->merge_if_one_child = false;
            PackageDepSpec ds(*i, pds_pm_unspecific);
            if (ds.package_ptr())
            {
                paludis::tr1::shared_ptr<const PackageDatabaseEntryCollection> c(
                        _imp->main_window->environment()->package_database()->query(
                            *_imp->packages_page->get_repository_filter() &
                            query::Matches(ds),
                            qo_best_version_in_slot_only));

                for (PackageDatabaseEntryCollection::ReverseIterator p(c->rbegin()), p_end(c->rend()) ;
                        p != p_end ; ++p)
                {
                    paludis::tr1::shared_ptr<const VersionMetadata> metadata(
                            _imp->main_window->environment()->package_database()->fetch_repository(p->repository)->version_metadata(
                                p->name, p->version));

                    atom_iter->children.push_back(make_item(ds,
                                p->name, metadata, _imp->main_window->environment()));
                    atom_iter->qpn = atom_iter->children.back().qpn;
                }
            }
            else
            {
                paludis::tr1::shared_ptr<const PackageDatabaseEntryCollection> c(
                        _imp->main_window->environment()->package_database()->query(
                            *_imp->packages_page->get_repository_filter() &
                            query::Matches(ds),
                            qo_best_version_in_slot_only));

                QualifiedPackageName old_qpn("OLD/OLD");
                std::list<PopulateItem>::iterator pkg_iter;

                for (PackageDatabaseEntryCollection::ReverseIterator p(c->rbegin()), p_end(c->rend()) ;
                        p != p_end ; ++p)
                {
                    paludis::tr1::shared_ptr<const VersionMetadata> metadata(
                            _imp->main_window->environment()->package_database()->fetch_repository(p->repository)->version_metadata(
                                p->name, p->version));

                    if (old_qpn != p->name)
                    {
                        pkg_iter = atom_iter->children.insert(atom_iter->children.end(), PopulateItem(stringify(p->name)));
                        pkg_iter->children.push_back(make_item(ds,
                                    p->name, metadata, _imp->main_window->environment()));
                        pkg_iter->qpn = pkg_iter->children.back().qpn;
                        old_qpn = p->name;
                    }
                    else
                        pkg_iter->children.push_front(make_item(ds,
                                    p->name, metadata, _imp->main_window->environment()));
                }
            }
        }
    }

    _imp->main_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &PackagesListModel::populate_in_gui_thread), data));
}

void
PackagesListModel::populate_in_gui_thread(paludis::tr1::shared_ptr<const PackagesListModel::PopulateData> names)
{
    clear();
    Gtk::TreeNodeChildren c(children());
    _populate_in_gui_thread_recursive(c,
            PopulateDataIterator(names->items.begin()),
            PopulateDataIterator(names->items.end()));
}

void
PackagesListModel::_populate_in_gui_thread_recursive(
        Gtk::TreeNodeChildren & t,
        PopulateDataIterator i,
        PopulateDataIterator i_end)
{
    for ( ; i != i_end ; ++i)
    {
        iterator r(append(t));
        (*r)[_imp->columns.col_package] = i->title;
        (*r)[_imp->columns.col_best_package_filter_option] = i->children_best_option();
        (*r)[_imp->columns.col_qpn] = i->qpn;
        (*r)[_imp->columns.col_description] = i->description;
        (*r)[_imp->columns.col_status_markup] = i->status_markup;

        if (! i->children.empty())
        {
            if (i->merge_if_one_child && (next(i->children.begin()) == i->children.end()))
            {
                (*r)[_imp->columns.col_description] = i->children.begin()->description;
                (*r)[_imp->columns.col_status_markup] = i->children.begin()->status_markup;

            }
            else
            {
                Gtk::TreeNodeChildren c(r->children());
                _populate_in_gui_thread_recursive(c, PopulateDataIterator(i->children.begin()),
                        PopulateDataIterator(i->children.end()));
            }
        }
    }
}
