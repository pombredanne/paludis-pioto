/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "categories_list_model.hh"
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
    struct Implementation<CategoriesListModel>
    {
        MainWindow * const main_window;
        PackagesPage * const packages_page;
        CategoriesListModel::Columns columns;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            main_window(m),
            packages_page(p)
        {
        }
    };
}

CategoriesListModel::CategoriesListModel(MainWindow * const m, PackagesPage * const p) :
    PrivateImplementationPattern<CategoriesListModel>(new Implementation<CategoriesListModel>(m, p)),
    Gtk::ListStore(_imp->columns)
{
}

CategoriesListModel::~CategoriesListModel()
{
}

void
CategoriesListModel::populate()
{
    _imp->main_window->paludis_thread_action(
            sigc::mem_fun(this, &CategoriesListModel::populate_in_paludis_thread), "Populating categories list model");
}

void
CategoriesListModel::populate_in_paludis_thread()
{
    paludis::tr1::shared_ptr<CategoryNamePartCollection> columns(
            new CategoryNamePartCollection::Concrete);

    paludis::tr1::shared_ptr<RepositoryNameCollection> repos(
            _imp->packages_page->get_repository_filter()->repositories(*_imp->main_window->environment()));

    if (repos)
    {
        for (RepositoryNameCollection::Iterator r(repos->begin()), r_end(repos->end()) ;
                r != r_end ; ++r)
        {
            paludis::tr1::shared_ptr<const CategoryNamePartCollection> cats(
                    _imp->main_window->environment()->package_database()->fetch_repository(*r)->category_names());
            std::copy(cats->begin(), cats->end(), columns->inserter());
        }
    }
    else
    {
        for (IndirectIterator<PackageDatabase::RepositoryIterator>
                r(indirect_iterator(_imp->main_window->environment()->package_database()->begin_repositories())),
                r_end(indirect_iterator(_imp->main_window->environment()->package_database()->end_repositories())) ;
                r != r_end ; ++r)
        {
            paludis::tr1::shared_ptr<const CategoryNamePartCollection> cats(r->category_names());
            std::copy(cats->begin(), cats->end(), columns->inserter());
        }
    }

    _imp->main_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &CategoriesListModel::populate_in_gui_thread), columns));
}

void
CategoriesListModel::populate_in_gui_thread(paludis::tr1::shared_ptr<const CategoryNamePartCollection> names)
{
    clear();
    for (CategoryNamePartCollection::Iterator n(names->begin()), n_end(names->end()) ;
            n != n_end ; ++n)
        (*append())[_imp->columns.col_cat_name] = stringify(*n);
}


CategoriesListModel::Columns::Columns()
{
    add(col_cat_name);
}

CategoriesListModel::Columns::~Columns()
{
}

CategoriesListModel::Columns &
CategoriesListModel::columns()
{
    return _imp->columns;
}

