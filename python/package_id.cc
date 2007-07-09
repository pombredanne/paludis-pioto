/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#include <paludis_python.hh>

#include <paludis/package_id.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/util/sequence.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct PackageIDWrapper
{
    static PyObject *
    find(const PackageID & self, const std::string & key)
    {
        PackageID::Iterator i(self.find(key));
        if (i != self.end())
            return bp::incref(bp::object(*i).ptr());
        else
            return Py_None;
    }
};

void PALUDIS_VISIBLE expose_package_id()
{
    /**
     * Enums
     */
    enum_auto("PackageIDCanonicalForm", last_idcf,
            "How to generate paludis::PackageID::canonical_form().");

    /**
     * PackageID
     */
    register_shared_ptrs_to_python<PackageID>();
    bp::class_<PackageID, boost::noncopyable>
        (
         "PackageID",
         bp::no_init
        )
        .def("canonical_form", &PackageID::canonical_form,
                "canonical_form(PackageIDCanonicalForm) -> string\n"
                )

        .add_property("name", &PackageID::name,
                "[ro] QualifiedPackageName\n"
                )

        .add_property("version", &PackageID::version,
                "[ro] VersionSpec\n"
                )

        .add_property("slot", &PackageID::slot,
                "[ro] SlotName\n"
                )

        .add_property("repository", &PackageID::repository,
                "[ro] Repository\n"
                )

        .add_property("eapi", &PackageID::eapi,
                "[ro] EAPI\n"
                )

        .def("__iter__", bp::range(&PackageID::begin, &PackageID::end))

        .def("find", &PackageIDWrapper::find,
                "find(string) -> MetadataKey\n"
            )

        .def("__eq__", &py_eq<PackageID>)

        .def("__ne__", &py_ne<PackageID>)
        ;

    /**
     * PackageIDIterable
     */
    class_iterable<PackageIDSequence>
        (
         "PackageIDIterable",
         "Iterable of PackageID"
        );
}