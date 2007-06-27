/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_QUERY_NOTEBOOK_PAGE_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_QUERY_NOTEBOOK_PAGE_HH 1

namespace gtkpaludis
{
    class QueryNotebookPage
    {
        private:
            bool _populated;

        public:
            QueryNotebookPage();
            virtual ~QueryNotebookPage() = 0;

            void populate_once();
            virtual void populate() = 0;
    };
}


#endif