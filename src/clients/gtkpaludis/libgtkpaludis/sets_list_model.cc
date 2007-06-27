/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "sets_list_model.hh"
#include "main_window.hh"
#include "packages_page.hh"
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<SetsListModel>
    {
        MainWindow * const main_window;
        PackagesPage * const packages_page;
        SetsListModel::Columns columns;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            main_window(m),
            packages_page(p)
        {
        }
    };
}

SetsListModel::SetsListModel(MainWindow * const m, PackagesPage * const p) :
    PrivateImplementationPattern<SetsListModel>(new Implementation<SetsListModel>(m, p)),
    Gtk::ListStore(_imp->columns)
{
}

SetsListModel::~SetsListModel()
{
}

void
SetsListModel::populate()
{
    _imp->main_window->paludis_thread_action(
            sigc::mem_fun(this, &SetsListModel::populate_in_paludis_thread), "Populating sets list model");
}

void
SetsListModel::populate_in_paludis_thread()
{
    paludis::tr1::shared_ptr<SetNameCollection> columns(
            new SetNameCollection::Concrete);

    paludis::tr1::shared_ptr<RepositoryNameCollection> repos(
            _imp->packages_page->get_repository_filter()->repositories(*_imp->main_window->environment()));

    if (repos)
    {
        for (RepositoryNameCollection::Iterator r(repos->begin()), r_end(repos->end()) ;
                r != r_end ; ++r)
        {
            RepositorySetsInterface * const i(_imp->main_window->environment()->package_database()->fetch_repository(*r)->sets_interface);
            if (i)
            {
                paludis::tr1::shared_ptr<const SetNameCollection> sets(i->sets_list());
                std::copy(sets->begin(), sets->end(), columns->inserter());
            }
        }
    }
    else
    {
        for (IndirectIterator<PackageDatabase::RepositoryIterator>
                r(indirect_iterator(_imp->main_window->environment()->package_database()->begin_repositories())),
                r_end(indirect_iterator(_imp->main_window->environment()->package_database()->end_repositories())) ;
                r != r_end ; ++r)
        {
            RepositorySetsInterface * const i(r->sets_interface);
            if (i)
            {
                paludis::tr1::shared_ptr<const SetNameCollection> sets(i->sets_list());
                std::copy(sets->begin(), sets->end(), columns->inserter());
            }
        }
    }

    tr1::shared_ptr<const SetNameCollection> sets(_imp->main_window->environment()->set_names());
    std::copy(sets->begin(), sets->end(), columns->inserter());

    _imp->main_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &SetsListModel::populate_in_gui_thread), columns));
}

void
SetsListModel::populate_in_gui_thread(paludis::tr1::shared_ptr<const SetNameCollection> names)
{
    clear();
    for (SetNameCollection::Iterator n(names->begin()), n_end(names->end()) ;
            n != n_end ; ++n)
        (*append())[_imp->columns.col_set_name] = stringify(*n);
}


SetsListModel::Columns::Columns()
{
    add(col_set_name);
}

SetsListModel::Columns::~Columns()
{
}

SetsListModel::Columns &
SetsListModel::columns()
{
    return _imp->columns;
}
