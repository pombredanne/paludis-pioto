/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis_ruby.hh>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_filter_module;
    static VALUE c_filter;
    static VALUE c_filter_all;
    static VALUE c_filter_supports_action;
    static VALUE c_filter_not_masked;
    static VALUE c_filter_installed_at_root;
    static VALUE c_filter_and;

    VALUE
    filter_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    VALUE
    filter_all_new(VALUE self)
    {
        Filter * ptr(0);
        try
        {
            ptr = new filter::All();
            VALUE data(Data_Wrap_Struct(self, 0, &Common<Filter>::free, ptr));
            rb_obj_call_init(data, 0, &self);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    filter_not_masked_new(VALUE self)
    {
        Filter * ptr(0);
        try
        {
            ptr = new filter::NotMasked();
            VALUE data(Data_Wrap_Struct(self, 0, &Common<Filter>::free, ptr));
            rb_obj_call_init(data, 0, &self);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    filter_installed_at_root_new(VALUE self, VALUE root_v)
    {
        Filter * ptr(0);
        try
        {
            FSEntry root(StringValuePtr(root_v));
            ptr = new filter::InstalledAtRoot(root);
            VALUE data(Data_Wrap_Struct(self, 0, &Common<Filter>::free, ptr));
            rb_obj_call_init(data, 1, &root_v);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    filter_and_new(VALUE self, VALUE f1_v, VALUE f2_v)
    {
        Filter * ptr(0);
        try
        {
            Filter f1(value_to_filter(f1_v));
            Filter f2(value_to_filter(f2_v));
            ptr = new filter::And(f1, f2);
            VALUE data(Data_Wrap_Struct(self, 0, &Common<Filter>::free, ptr));
            rb_obj_call_init(data, 2, &f1_v);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    filter_supports_action_new(VALUE self, VALUE action_class)
    {
        Filter * ptr(0);
        try
        {
            if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, install_action_value_ptr()))
                ptr = new filter::SupportsAction<InstallAction>();
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, installed_action_value_ptr()))
                ptr = new filter::SupportsAction<InstalledAction>();
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, uninstall_action_value_ptr()))
                ptr = new filter::SupportsAction<UninstallAction>();
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, pretend_action_value_ptr()))
                ptr = new filter::SupportsAction<PretendAction>();
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, config_action_value_ptr()))
                ptr = new filter::SupportsAction<ConfigAction>();
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, fetch_action_value_ptr()))
                ptr = new filter::SupportsAction<FetchAction>();
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, info_action_value_ptr()))
                ptr = new filter::SupportsAction<InfoAction>();
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, pretend_fetch_action_value_ptr()))
                ptr = new filter::SupportsAction<PretendFetchAction>();
            else
                rb_raise(rb_eTypeError, "Can't convert %s into an Action subclass", rb_obj_classname(action_class));

            VALUE data(Data_Wrap_Struct(self, 0, &Common<Filter>::free, ptr));
            rb_obj_call_init(data, 1, &action_class);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    void do_register_filter()
    {
        c_filter_module = rb_define_module_under(paludis_module(), "Filter");

        c_filter = rb_define_class_under(c_filter_module, "Filter", rb_cObject);
        rb_funcall(c_filter, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_filter, "initialize", RUBY_FUNC_CAST(&filter_init), -1);
        rb_define_method(c_filter, "to_s", RUBY_FUNC_CAST(&Common<Filter>::to_s), 0);

        c_filter_all = rb_define_class_under(c_filter_module, "All", c_filter);
        rb_define_singleton_method(c_filter_all, "new", RUBY_FUNC_CAST(&filter_all_new), 0);

        c_filter_not_masked = rb_define_class_under(c_filter_module, "NotMasked", c_filter);
        rb_define_singleton_method(c_filter_not_masked, "new", RUBY_FUNC_CAST(&filter_not_masked_new), 0);

        c_filter_installed_at_root = rb_define_class_under(c_filter_module, "InstalledAtRoot", c_filter);
        rb_define_singleton_method(c_filter_installed_at_root, "new", RUBY_FUNC_CAST(&filter_installed_at_root_new), 1);

        c_filter_and = rb_define_class_under(c_filter_module, "And", c_filter);
        rb_define_singleton_method(c_filter_and, "new", RUBY_FUNC_CAST(&filter_and_new), 2);

        c_filter_supports_action = rb_define_class_under(c_filter_module, "SupportsAction", c_filter);
        rb_define_singleton_method(c_filter_supports_action, "new", RUBY_FUNC_CAST(&filter_supports_action_new), 1);
    }
}

Filter
paludis::ruby::value_to_filter(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_filter))
    {
        Filter * f_ptr;
        Data_Get_Struct(v, Filter, f_ptr);
        return *f_ptr;
    }
    else
        rb_raise(rb_eTypeError, "Can't convert %s into Filter", rb_obj_classname(v));
}

VALUE
paludis::ruby::filter_to_value(const Filter & v)
{
    Filter * vv(new Filter(v));
    return Data_Wrap_Struct(c_filter, 0, &Common<Filter>::free, vv);
}

RegisterRubyClass::Register paludis_ruby_register_filter PALUDIS_ATTRIBUTE((used))
    (&do_register_filter);
