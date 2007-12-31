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

#include <paludis/use_requirements.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/visitor-impl.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct UseRequirementsWrapper
{
    static PyObject *
    find(const UseRequirements & self, const UseFlagName & u)
    {
        UseRequirements::ConstIterator i(self.find(u));
        if (i != self.end())
            return bp::incref(bp::object(*i).ptr());
        else
            return Py_None;
    }
};

void expose_use_requirements()
{
    /**
     * UseRequirements
     */
    register_shared_ptrs_to_python<UseRequirements>();
    bp::class_<UseRequirements>
        (
         "UseRequirements",
         "A selection of USE flag requirements.",
         bp::no_init
        )

        .def("empty", &UseRequirements::empty,
                "empty() -> bool\n"
                "Are we empty?"
            )

        .def("find", UseRequirementsWrapper::find,
                "find(UseFlagName) -> UseRequirement\n"
                "Find the requirement for a particular USE flag."
            )

        .def("insert", &UseRequirements::insert,
                "insert(UseRequirement)\n"
                "Insert a new requirement."
            )

        .def("__iter__", bp::range(&UseRequirements::begin, &UseRequirements::end))
        ;

    /**
     * UseRequirement
     */
    register_shared_ptrs_to_python<UseRequirement>();
    bp::class_<UseRequirement, boost::noncopyable>
        (
         "UseRequirement",
         "A requirement for a use flag.",
         bp::no_init
        )

        .def("flag", &UseRequirement::flag,
                "flag() -> UseFlagName\n"
                "Our use flag."
            )

        .def("satisfied_by", &UseRequirement::satisfied_by,
                "satisfied_by(Environment, PackageID) -> bool\n"
                "Does the package meet the requirement?"
            )
        ;

    /**
     * EnabledUseRequirement
     */
    bp::class_<EnabledUseRequirement, bp::bases<UseRequirement>, tr1::shared_ptr<EnabledUseRequirement> >
        (
         "EnabledUseRequirement",
         "An enabled requirement for a use flag.",
         bp::init<const UseFlagName &>("__init__(UseFlagName)")
        );

    /**
     * DisabledUseRequirement
     */
    bp::class_<DisabledUseRequirement, bp::bases<UseRequirement> >
        (
         "DisabledUseRequirement",
         "A disabled requirement for a use flag.",
         bp::init<const UseFlagName &>("__init__(UseFlagName)")
        );

    /**
     * ConditionalUseRequirement
     */
    bp::class_<ConditionalUseRequirement, bp::bases<UseRequirement>, boost::noncopyable>
        (
         "ConditionalUseRequirement",
         "A use requirement that depends on the use flags of the package it appears in.",
         bp::no_init
        )

        .def("package_id", &ConditionalUseRequirement::package_id,
                "package_id() -> PackageID\n"
                "Out package."
            )
        ;

    /**
     * IfMineThenUseRequirement
     */
    bp::class_<IfMineThenUseRequirement, bp::bases<ConditionalUseRequirement> >
        (
         "IfMineThenUseRequirement",
         "An if-then requirement for a use flag.",
         bp::init<const UseFlagName &, const tr1::shared_ptr<const PackageID> &>(
             "__init__(UseFlagName, PackageID)"
             )
        )
        ;

    /**
     * IfNotMineThenUseRequirement
     */
    bp::class_<IfNotMineThenUseRequirement, bp::bases<ConditionalUseRequirement> >
        (
         "IfNotMineThenUseRequirement",
         "An if-not-then requirement for a use flag.",
         bp::init<const UseFlagName &, const tr1::shared_ptr<const PackageID> &>(
             "__init__(UseFlagName, PackageID)"
             )
        )
        ;

    /**
     * IfMineThenNotUseRequirement
     */
    bp::class_<IfMineThenNotUseRequirement, bp::bases<ConditionalUseRequirement> >
        (
         "IfMineThenNotUseRequirement",
         "An if-then-not requirement for a use flag.",
         bp::init<const UseFlagName &, const tr1::shared_ptr<const PackageID> &>(
             "__init__(UseFlagName, PackageID)"
             )
        )
        ;

    /**
     * IfNotMineThenNotUseRequirement
     */
    bp::class_<IfNotMineThenNotUseRequirement, bp::bases<ConditionalUseRequirement> >
        (
         "IfNotMineThenNotUseRequirement",
         "An if-not-then-not requirement for a use flag.",
         bp::init<const UseFlagName &, const tr1::shared_ptr<const PackageID> &>(
             "__init__(UseFlagName, PackageID)"
             )
        )
        ;

    /**
     * EqualUseRequirement
     */
    bp::class_<EqualUseRequirement, bp::bases<ConditionalUseRequirement> >
        (
         "EqualUseRequirement",
         "An equal requirement for a use flag.",
         bp::init<const UseFlagName &, const tr1::shared_ptr<const PackageID> &>(
             "__init__(UseFlagName, PackageID)"
             )
        )
        ;

    /**
     * NotEqualUseRequirement
     */
    bp::class_<NotEqualUseRequirement, bp::bases<ConditionalUseRequirement> >
        (
         "NotEqualUseRequirement",
         "A not equal requirement for a use flag.",
         bp::init<const UseFlagName &, const tr1::shared_ptr<const PackageID> &>(
             "__init__(UseFlagName, PackageID)"
             )
        )
        ;

}