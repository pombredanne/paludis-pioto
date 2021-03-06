/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/e/eapi.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/system.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/map.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <tr1/unordered_map>
#include <map>
#include <vector>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

template class InstantiationPolicy<EAPIData, instantiation_method::SingletonTag>;

namespace
{
    std::string predefined(const std::string & d, const KeyValueConfigFile &, const std::string & v)
    {
        if (v == "PALUDIS_EAPIS_DIR")
            return d;
        else
            return "";
    }

    std::string check_get(const KeyValueConfigFile & k, const std::string & key)
    {
        return k.get(key);
    }

    template <typename T_>
    T_ destringify_key(const KeyValueConfigFile & k, const std::string & key)
    {
        Context context("When getting key '" + key + "':");
        return destringify<T_>(check_get(k, key));
    }

    std::tr1::shared_ptr<const EAPIEbuildEnvironmentVariables> make_ebuild_environment_variables(const KeyValueConfigFile & k)
    {
        return make_shared_ptr(new EAPIEbuildEnvironmentVariables(make_named_values<EAPIEbuildEnvironmentVariables>(
                        value_for<n::description_choices>(check_get(k, "description_choices")),
                        value_for<n::description_use>(check_get(k, "description_use")),
                        value_for<n::env_a>(check_get(k, "env_a")),
                        value_for<n::env_aa>(check_get(k, "env_aa")),
                        value_for<n::env_accept_keywords>(check_get(k, "env_accept_keywords")),
                        value_for<n::env_arch>(check_get(k, "env_arch")),
                        value_for<n::env_d>(check_get(k, "env_d")),
                        value_for<n::env_distdir>(check_get(k, "env_distdir")),
                        value_for<n::env_ebuild_phase>(check_get(k, "env_ebuild_phase")),
                        value_for<n::env_ed>(check_get(k, "env_ed")),
                        value_for<n::env_eprefix>(check_get(k, "env_eprefix")),
                        value_for<n::env_eroot>(check_get(k, "env_eroot")),
                        value_for<n::env_filesdir>(check_get(k, "env_filesdir")),
                        value_for<n::env_iuse_implicit>(check_get(k, "env_iuse_implicit")),
                        value_for<n::env_jobs>(check_get(k, "env_jobs")),
                        value_for<n::env_kv>(check_get(k, "env_kv")),
                        value_for<n::env_p>(check_get(k, "env_p")),
                        value_for<n::env_pf>(check_get(k, "env_pf")),
                        value_for<n::env_portdir>(check_get(k, "env_portdir")),
                        value_for<n::env_replaced_by_id>(check_get(k, "env_replaced_by_id")),
                        value_for<n::env_replaced_by_version>(check_get(k, "env_replaced_by_version")),
                        value_for<n::env_replacing_ids>(check_get(k, "env_replacing_ids")),
                        value_for<n::env_replacing_versions>(check_get(k, "env_replacing_versions")),
                        value_for<n::env_t>(check_get(k, "env_t")),
                        value_for<n::env_use>(check_get(k, "env_use")),
                        value_for<n::env_use_expand>(check_get(k, "env_use_expand")),
                        value_for<n::env_use_expand_hidden>(check_get(k, "env_use_expand_hidden")),
                        value_for<n::env_use_expand_implicit>(check_get(k, "env_use_expand_implicit")),
                        value_for<n::env_use_expand_unprefixed>(check_get(k, "env_use_expand_unprefixed")),
                        value_for<n::env_use_expand_values_part>(check_get(k, "env_use_expand_values_part"))
            )));
    }

    std::tr1::shared_ptr<EAPIMetadataVariable> make_metadata_variable(const KeyValueConfigFile & k, const std::string & s)
    {
        return make_shared_ptr(new EAPIMetadataVariable(make_named_values<EAPIMetadataVariable>(
                        value_for<n::description>(check_get(k, "description_" + s)),
                        value_for<n::flat_list_index>(destringify_key<int>(k, "flat_list_" + s)),
                        value_for<n::name>(check_get(k, "metadata_" + s))
                        )));
    }

    std::tr1::shared_ptr<const EAPIEbuildMetadataVariables> make_ebuild_metadata_variables(const KeyValueConfigFile & k)
    {
        return make_shared_ptr(new EAPIEbuildMetadataVariables(make_named_values<EAPIEbuildMetadataVariables>(
            value_for<n::bugs_to>(make_metadata_variable(k, "bugs_to")),
            value_for<n::build_depend>(make_metadata_variable(k, "build_depend")),
            value_for<n::defined_phases>(make_metadata_variable(k, "defined_phases")),
            value_for<n::dependencies>(make_metadata_variable(k, "dependencies")),
            value_for<n::eapi>(make_metadata_variable(k, "eapi")),
            value_for<n::homepage>(make_metadata_variable(k, "homepage")),
            value_for<n::inherited>(make_metadata_variable(k, "inherited")),
            value_for<n::iuse>(make_metadata_variable(k, "iuse")),
            value_for<n::iuse_effective>(make_metadata_variable(k, "iuse_effective")),
            value_for<n::keywords>(make_metadata_variable(k, "keywords")),
            value_for<n::license>(make_metadata_variable(k, "license")),
            value_for<n::long_description>(make_metadata_variable(k, "long_description")),
            value_for<n::minimum_flat_list_size>(destringify_key<int>(k, "flat_list_minimum_size")),
            value_for<n::myoptions>(make_metadata_variable(k, "myoptions")),
            value_for<n::pdepend>(make_metadata_variable(k, "pdepend")),
            value_for<n::properties>(make_metadata_variable(k, "properties")),
            value_for<n::provide>(make_metadata_variable(k, "provide")),
            value_for<n::remote_ids>(make_metadata_variable(k, "remote_ids")),
            value_for<n::restrictions>(make_metadata_variable(k, "restrict")),
            value_for<n::run_depend>(make_metadata_variable(k, "run_depend")),
            value_for<n::short_description>(make_metadata_variable(k, "short_description")),
            value_for<n::slot>(make_metadata_variable(k, "slot")),
            value_for<n::src_uri>(make_metadata_variable(k, "src_uri")),
            value_for<n::upstream_changelog>(make_metadata_variable(k, "upstream_changelog")),
            value_for<n::upstream_documentation>(make_metadata_variable(k, "upstream_documentation")),
            value_for<n::upstream_release_notes>(make_metadata_variable(k, "upstream_release_notes")),
            value_for<n::use>(make_metadata_variable(k, "use")),
            value_for<n::use_expand>(make_metadata_variable(k, "use_expand")),
            value_for<n::use_expand_hidden>(make_metadata_variable(k, "use_expand_hidden"))
            )));
    }

    std::tr1::shared_ptr<Set<std::string> > make_set(const std::string & s)
    {
        std::tr1::shared_ptr<Set<std::string> > result(new Set<std::string>);
        tokenise_whitespace(s, result->inserter());
        return result;
    }

    std::tr1::shared_ptr<const EAPIEbuildOptions> make_ebuild_options(const KeyValueConfigFile & k)
    {
        return make_shared_ptr(new EAPIEbuildOptions(make_named_values<EAPIEbuildOptions>(
                        value_for<n::binary_from_env_variables>(check_get(k, "binary_from_env_variables")),
                        value_for<n::bracket_merged_variables>(check_get(k, "bracket_merged_variables")),
                        value_for<n::bracket_merged_variables_annotatable>(check_get(k, "bracket_merged_variables_annotatable")),
                        value_for<n::bracket_merged_variables_annotation>(check_get(k, "bracket_merged_variables_annotation")),
                        value_for<n::directory_if_exists_variables>(check_get(k, "directory_if_exists_variables")),
                        value_for<n::directory_variables>(check_get(k, "directory_variables")),
                        value_for<n::ebuild_functions>(check_get(k, "ebuild_functions")),
                        value_for<n::ebuild_module_suffixes>(check_get(k, "ebuild_module_suffixes")),
                        value_for<n::ebuild_must_not_set_variables>(check_get(k, "ebuild_must_not_set_variables")),
                        value_for<n::eclass_must_not_set_variables>(check_get(k, "eclass_must_not_set_variables")),
                        value_for<n::f_function_prefix>(check_get(k, "f_function_prefix")),
                        value_for<n::fix_mtimes>(destringify_key<bool>(k, "fix_mtimes")),
                        value_for<n::ignore_pivot_env_functions>(check_get(k, "ignore_pivot_env_functions")),
                        value_for<n::ignore_pivot_env_variables>(check_get(k, "ignore_pivot_env_variables")),
                        value_for<n::load_modules>(check_get(k, "load_modules")),
                        value_for<n::must_not_change_after_source_variables>(check_get(k, "must_not_change_after_source_variables")),
                        value_for<n::must_not_change_variables>(check_get(k, "must_not_change_variables")),
                        value_for<n::must_not_set_vars_starting_with>(check_get(k, "must_not_set_vars_starting_with")),
                        value_for<n::no_s_workdir_fallback>(destringify_key<bool>(k, "no_s_workdir_fallback")),
                        value_for<n::non_empty_variables>(check_get(k, "non_empty_variables")),
                        value_for<n::rdepend_defaults_to_depend>(destringify_key<bool>(k, "rdepend_defaults_to_depend")),
                        value_for<n::require_use_expand_in_iuse>(destringify_key<bool>(k, "require_use_expand_in_iuse")),
                        value_for<n::restrict_fetch>(make_set(check_get(k, "restrict_fetch"))),
                        value_for<n::restrict_mirror>(make_set(check_get(k, "restrict_mirror"))),
                        value_for<n::restrict_primaryuri>(make_set(check_get(k, "restrict_primaryuri"))),
                        value_for<n::save_base_variables>(check_get(k, "save_base_variables")),
                        value_for<n::save_unmodifiable_variables>(check_get(k, "save_unmodifiable_variables")),
                        value_for<n::save_variables>(check_get(k, "save_variables")),
                        value_for<n::shell_options>(check_get(k, "shell_options")),
                        value_for<n::source_merged_variables>(check_get(k, "source_merged_variables")),
                        value_for<n::support_eclasses>(destringify_key<bool>(k, "support_eclasses")),
                        value_for<n::support_exlibs>(destringify_key<bool>(k, "support_exlibs")),
                        value_for<n::utility_path_suffixes>(check_get(k, "utility_path_suffixes")),
                        value_for<n::vdb_from_env_unless_empty_variables>(check_get(k, "vdb_from_env_unless_empty_variables")),
                        value_for<n::vdb_from_env_variables>(check_get(k, "vdb_from_env_variables")),
                        value_for<n::want_portage_emulation_vars>(destringify_key<bool>(k, "want_portage_emulation_vars"))
                )));
    }

    std::tr1::shared_ptr<const EAPIEbuildPhases> make_ebuild_phases(const KeyValueConfigFile & k)
    {
        return make_shared_ptr(new EAPIEbuildPhases(make_named_values<EAPIEbuildPhases>(
                        value_for<n::ebuild_bad_options>(check_get(k, "ebuild_bad_options")),
                        value_for<n::ebuild_config>(check_get(k, "ebuild_config")),
                        value_for<n::ebuild_fetch_extra>(check_get(k, "ebuild_fetch_extra")),
                        value_for<n::ebuild_info>(check_get(k, "ebuild_info")),
                        value_for<n::ebuild_install>(check_get(k, "ebuild_install")),
                        value_for<n::ebuild_metadata>(check_get(k, "ebuild_metadata")),
                        value_for<n::ebuild_new_upgrade_phase_order>(destringify_key<bool>(k, "ebuild_new_upgrade_phase_order")),
                        value_for<n::ebuild_nofetch>(check_get(k, "ebuild_nofetch")),
                        value_for<n::ebuild_pretend>(check_get(k, "ebuild_pretend")),
                        value_for<n::ebuild_uninstall>(check_get(k, "ebuild_uninstall")),
                        value_for<n::ebuild_variable>(check_get(k, "ebuild_variable"))
            )));
    }

    std::tr1::shared_ptr<const EAPIPipeCommands> make_pipe_commands(const KeyValueConfigFile & k)
    {
        return make_shared_ptr(new EAPIPipeCommands(make_named_values<EAPIPipeCommands>(
                        value_for<n::no_slot_or_repo>(destringify_key<bool>(k, "pipe_commands_no_slot_or_repo")),
                        value_for<n::rewrite_virtuals>(destringify_key<bool>(k, "pipe_commands_rewrite_virtuals"))
                        )));
    }

    std::tr1::shared_ptr<const EAPIToolsOptions> make_tool_options(const KeyValueConfigFile & k)
    {
        return make_shared_ptr(new EAPIToolsOptions(make_named_values<EAPIToolsOptions>(
                        value_for<n::dodoc_r>(destringify_key<bool>(k, "dodoc_r")),
                        value_for<n::doins_symlink>(destringify_key<bool>(k, "doins_symlink")),
                        value_for<n::doman_lang_filenames>(destringify_key<bool>(k, "doman_lang_filenames")),
                        value_for<n::dosym_mkdir>(destringify_key<bool>(k, "dosym_mkdir")),
                        value_for<n::econf_extra_options>(k.get("econf_extra_options")),
                        value_for<n::failure_is_fatal>(destringify_key<bool>(k, "failure_is_fatal")),
                        value_for<n::unpack_fix_permissions>(destringify_key<bool>(k, "unpack_fix_permissions")),
                        value_for<n::unpack_suffixes>(k.get("unpack_suffixes")),
                        value_for<n::unpack_unrecognised_is_fatal>(destringify_key<bool>(k, "unpack_unrecognised_is_fatal"))
                        )));
    }

    std::tr1::shared_ptr<const EAPIAnnotations> make_annotations(const KeyValueConfigFile & k)
    {
        return make_shared_ptr(new EAPIAnnotations(make_named_values<EAPIAnnotations>(
                        value_for<n::myoptions_description>(k.get("annotations_myoptions_description")),
                        value_for<n::myoptions_number_selected>(k.get("annotations_myoptions_number_selected")),
                        value_for<n::myoptions_number_selected_at_least_one>(k.get("annotations_myoptions_number_selected_at_least_one")),
                        value_for<n::myoptions_number_selected_at_most_one>(k.get("annotations_myoptions_number_selected_at_most_one")),
                        value_for<n::myoptions_number_selected_exactly_one>(k.get("annotations_myoptions_number_selected_exactly_one")),
                        value_for<n::myoptions_requires>(k.get("annotations_myoptions_requires"))
                        )));
    }

    std::tr1::shared_ptr<const EAPIChoicesOptions> make_choices_options(const KeyValueConfigFile & k)
    {
        return make_shared_ptr(new EAPIChoicesOptions(make_named_values<EAPIChoicesOptions>(
                        value_for<n::fancy_test_flag>(check_get(k, "fancy_test_flag")),
                        value_for<n::has_expensive_tests>(destringify_key<bool>(k, "has_expensive_tests")),
                        value_for<n::has_optional_tests>(destringify_key<bool>(k, "has_optional_tests")),
                        value_for<n::has_recommended_tests>(destringify_key<bool>(k, "has_recommended_tests")),
                        value_for<n::profile_iuse_injection>(destringify_key<bool>(k, "profile_iuse_injection")),
                        value_for<n::use_expand_separator>(destringify_key<char>(k, "use_expand_separator"))
                        )));
    }

    std::tr1::shared_ptr<const SupportedEAPI> make_supported_eapi(const KeyValueConfigFile & k)
    {
        ELikePackageDepSpecOptions package_dep_spec_parse_options;
        {
            std::list<std::string> package_dep_spec_parse_options_tokens;
            tokenise_whitespace(check_get(k, "package_dep_spec_parse_options"), std::back_inserter(package_dep_spec_parse_options_tokens));
            for (std::list<std::string>::const_iterator t(package_dep_spec_parse_options_tokens.begin()),
                    t_end(package_dep_spec_parse_options_tokens.end()) ;
                    t != t_end ; ++t)
                package_dep_spec_parse_options += destringify<ELikePackageDepSpecOption>(*t);
        }

        DependencySpecTreeParseOptions dependency_spec_tree_parse_options;
        {
            std::list<std::string> dependency_spec_tree_parse_options_tokens;
            tokenise_whitespace(check_get(k, "dependency_spec_tree_parse_options"), std::back_inserter(dependency_spec_tree_parse_options_tokens));
            for (std::list<std::string>::const_iterator t(dependency_spec_tree_parse_options_tokens.begin()),
                    t_end(dependency_spec_tree_parse_options_tokens.end()) ;
                    t != t_end ; ++t)
                dependency_spec_tree_parse_options += destringify<DependencySpecTreeParseOption>(*t);
        }

        IUseFlagParseOptions iuse_flag_parse_options;
        {
            std::list<std::string> iuse_flag_parse_options_tokens;
            tokenise_whitespace(check_get(k, "iuse_flag_parse_options"), std::back_inserter(iuse_flag_parse_options_tokens));
            for (std::list<std::string>::const_iterator t(iuse_flag_parse_options_tokens.begin()),
                    t_end(iuse_flag_parse_options_tokens.end()) ;
                    t != t_end ; ++t)
                iuse_flag_parse_options += destringify<IUseFlagParseOption>(*t);
        }

        VersionSpecOptions version_spec_options;
        {
            std::list<std::string> version_spec_options_tokens;
            tokenise_whitespace(check_get(k, "version_spec_options"), std::back_inserter(version_spec_options_tokens));
            for (std::list<std::string>::const_iterator t(version_spec_options_tokens.begin()),
                    t_end(version_spec_options_tokens.end()) ;
                    t != t_end ; ++t)
                version_spec_options += destringify<VersionSpecOption>(*t);
        }

        MergerOptions merger_options;
        {
            std::list<std::string> merger_options_tokens;
            tokenise_whitespace(check_get(k, "merger_options"), std::back_inserter(merger_options_tokens));
            for (std::list<std::string>::const_iterator t(merger_options_tokens.begin()),
                    t_end(merger_options_tokens.end()) ;
                    t != t_end ; ++t)
                merger_options += destringify<MergerOption>(*t);
        }

        return make_shared_ptr(new SupportedEAPI(make_named_values<SupportedEAPI>(
                        value_for<n::annotations>(make_annotations(k)),
                        value_for<n::breaks_portage>(destringify_key<bool>(k, "breaks_portage")),
                        value_for<n::can_be_pbin>(destringify_key<bool>(k, "can_be_pbin")),
                        value_for<n::choices_options>(make_choices_options(k)),
                        value_for<n::dependency_labels>(make_shared_ptr(new const EAPILabels(check_get(k, "dependency_labels")))),
                        value_for<n::dependency_spec_tree_parse_options>(dependency_spec_tree_parse_options),
                        value_for<n::ebuild_environment_variables>(make_ebuild_environment_variables(k)),
                        value_for<n::ebuild_metadata_variables>(make_ebuild_metadata_variables(k)),
                        value_for<n::ebuild_options>(make_ebuild_options(k)),
                        value_for<n::ebuild_phases>(make_ebuild_phases(k)),
                        value_for<n::iuse_flag_parse_options>(iuse_flag_parse_options),
                        value_for<n::merger_options>(merger_options),
                        value_for<n::package_dep_spec_parse_options>(package_dep_spec_parse_options),
                        value_for<n::pipe_commands>(make_pipe_commands(k)),
                        value_for<n::tools_options>(make_tool_options(k)),
                        value_for<n::uri_labels>(make_shared_ptr(new const EAPILabels(check_get(k, "uri_labels")))),
                        value_for<n::userpriv_cannot_use_root>(destringify_key<bool>(k, "userpriv_cannot_use_root")),
                        value_for<n::version_spec_options>(version_spec_options)
                        )));
    }
}

namespace paludis
{
    template<>
    struct Implementation<EAPIData>
    {
        std::tr1::unordered_map<std::string, std::tr1::shared_ptr<const EAPI>, Hash<std::string> > values;

        Implementation()
        {
            Context c("When loading EAPI data:");

            for (DirIterator d(getenv_with_default("PALUDIS_EAPIS_DIR", DATADIR "/paludis/eapis")), d_end ;
                    d != d_end ; ++d)
            {
                if (! is_file_with_extension(*d, ".conf", IsFileWithOptions()))
                    continue;

                Context cc("When loading EAPI file '" + stringify(*d) + "':");
                KeyValueConfigFile k(*d, KeyValueConfigFileOptions(),
                        std::tr1::bind(&predefined, stringify(d->dirname()), std::tr1::placeholders::_1, std::tr1::placeholders::_2),
                        &KeyValueConfigFile::no_transformation);

                std::tr1::shared_ptr<EAPI> eapi(new EAPI(make_named_values<EAPI>(
                                value_for<n::exported_name>(check_get(k, "exported_name")),
                                value_for<n::name>(strip_trailing_string(d->basename(), ".conf")),
                                value_for<n::supported>(make_supported_eapi(k))
                                )));

                values.insert(std::make_pair(strip_trailing_string(d->basename(), ".conf"), eapi));
            }

            std::tr1::unordered_map<std::string, std::tr1::shared_ptr<const EAPI>, Hash<std::string> >::const_iterator i(values.find("0"));
            if (i == values.end())
                throw EAPIConfigurationError("No EAPI configuration found for EAPI 0");
            else
                values.insert(std::make_pair("", i->second));
        }
    };
}

EAPIConfigurationError::EAPIConfigurationError(const std::string & s) throw () :
    ConfigurationError("EAPI configuration error: " + s)
{
}

EAPIData::EAPIData() :
    PrivateImplementationPattern<EAPIData>(new Implementation<EAPIData>)
{
}

EAPIData::~EAPIData()
{
}

std::tr1::shared_ptr<const EAPI>
EAPIData::eapi_from_string(const std::string & s) const
{
    std::tr1::unordered_map<std::string, std::tr1::shared_ptr<const EAPI>, Hash<std::string> >::const_iterator i(_imp->values.find(s));
    if (i != _imp->values.end())
        return i->second;

    return make_shared_ptr(new EAPI(make_named_values<EAPI>(
                    value_for<n::exported_name>(s),
                    value_for<n::name>(s),
                    value_for<n::supported>(std::tr1::shared_ptr<const SupportedEAPI>()))
                ));
}

std::tr1::shared_ptr<const EAPI>
EAPIData::unknown_eapi() const
{
    return make_shared_ptr(new EAPI(make_named_values<EAPI>(
                    value_for<n::exported_name>("UNKNOWN"),
                    value_for<n::name>("UNKNOWN"),
                    value_for<n::supported>(std::tr1::shared_ptr<const SupportedEAPI>()))
                ));
}

namespace paludis
{
    template <>
    struct Implementation<EAPILabels>
    {
        std::map<std::string, std::string> v;
    };
}

EAPILabels::EAPILabels(const std::string & s) :
    PrivateImplementationPattern<EAPILabels>(new Implementation<EAPILabels>)
{
    std::vector<std::string> tokens;

    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(s, ";", "", std::back_inserter(tokens));

    for (std::vector<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
            t != t_end ; ++t)
    {
        std::vector<std::string> values;
        tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(*t, "=", "", std::back_inserter(values));

        if (values.size() != 2)
            throw EAPIConfigurationError("EAPI labels value '" + s + "' has bad values size '" + stringify(values.size()) + "'");

        _imp->v.insert(std::make_pair(strip_leading(strip_trailing(values[0], " \t\r\n"), " \t\r\n"),
                    strip_leading(strip_trailing(values[1], " \t\r\n"), " \t\r\n")));
    }
}

EAPILabels::EAPILabels(const EAPILabels & other) :
    PrivateImplementationPattern<EAPILabels>(new Implementation<EAPILabels>(*other._imp.operator-> ()))
{
}

EAPILabels::~EAPILabels()
{
}

const std::string
EAPILabels::class_for_label(const std::string & l) const
{
    std::map<std::string, std::string>::const_iterator i(_imp->v.find(l));
    if (_imp->v.end() == i)
        return "";
    return i->second;
}

