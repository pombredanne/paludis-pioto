/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include <python/paludis_python.hh>
#include <python/exception.hh>
#include <python/iterable.hh>

#include <paludis/util/fs_entry.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void expose_fs_entry()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<FSError>
        ("FSError", "BaseException",
         "Generic filesystem error class.");

    /**
     * FSEntry
     */
    bp::implicitly_convertible<std::string, FSEntry>();
    bp::to_python_converter<FSEntry, to_string<FSEntry> >();

    /**
     * FSEntryIterable
     */
    class_iterable<FSEntrySequence>
        (
         "FSEntryIterable",
         "Iterable of FSEntry",
         true
        );
}
