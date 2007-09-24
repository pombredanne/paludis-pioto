/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_MARKUP_FORMATTER_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_MARKUP_FORMATTER_HH 1

#include <paludis/formatter.hh>
#include <paludis/name-fwd.hh>
#include <paludis/dep_spec-fwd.hh>

namespace gtkpaludis
{
    class MarkupFormatter :
        public paludis::CanFormat<paludis::UseFlagName>,
        public paludis::CanFormat<paludis::IUseFlag>,
        public paludis::CanFormat<paludis::KeywordName>,
        public paludis::CanFormat<paludis::UseDepSpec>,
        public paludis::CanFormat<paludis::PackageDepSpec>,
        public paludis::CanFormat<paludis::BlockDepSpec>,
        public paludis::CanFormat<paludis::LabelsDepSpec<paludis::DependencyLabelVisitorTypes> >,
        public paludis::CanFormat<paludis::LabelsDepSpec<paludis::URILabelVisitorTypes> >,
        public paludis::CanFormat<paludis::PlainTextDepSpec>,
        public paludis::CanFormat<paludis::URIDepSpec>,
        public paludis::CanFormat<paludis::tr1::shared_ptr<const paludis::PackageID> >,
        public paludis::CanFormat<std::string>,
        public paludis::CanSpace
    {
        public:
            std::string format(const paludis::IUseFlag &, const paludis::format::Plain &) const;
            std::string format(const paludis::IUseFlag &, const paludis::format::Enabled &) const;
            std::string format(const paludis::IUseFlag &, const paludis::format::Disabled &) const;
            std::string format(const paludis::IUseFlag &, const paludis::format::Forced &) const;
            std::string format(const paludis::IUseFlag &, const paludis::format::Masked &) const;
            std::string decorate(const paludis::IUseFlag &, const std::string &, const paludis::format::Added &) const;
            std::string decorate(const paludis::IUseFlag &, const std::string &, const paludis::format::Changed &) const;

            std::string format(const paludis::UseFlagName &, const paludis::format::Plain &) const;
            std::string format(const paludis::UseFlagName &, const paludis::format::Enabled &) const;
            std::string format(const paludis::UseFlagName &, const paludis::format::Disabled &) const;
            std::string format(const paludis::UseFlagName &, const paludis::format::Forced &) const;
            std::string format(const paludis::UseFlagName &, const paludis::format::Masked &) const;

            std::string format(const paludis::UseDepSpec &, const paludis::format::Plain &) const;
            std::string format(const paludis::UseDepSpec &, const paludis::format::Enabled &) const;
            std::string format(const paludis::UseDepSpec &, const paludis::format::Disabled &) const;
            std::string format(const paludis::UseDepSpec &, const paludis::format::Forced &) const;
            std::string format(const paludis::UseDepSpec &, const paludis::format::Masked &) const;

            std::string format(const paludis::PackageDepSpec &, const paludis::format::Plain &) const;
            std::string format(const paludis::PackageDepSpec &, const paludis::format::Installed &) const;
            std::string format(const paludis::PackageDepSpec &, const paludis::format::Installable &) const;

            std::string format(const paludis::PlainTextDepSpec &, const paludis::format::Plain &) const;
            std::string format(const paludis::PlainTextDepSpec &, const paludis::format::Accepted &) const;
            std::string format(const paludis::PlainTextDepSpec &, const paludis::format::Unaccepted &) const;

            std::string format(const paludis::KeywordName &, const paludis::format::Plain &) const;
            std::string format(const paludis::KeywordName &, const paludis::format::Accepted &) const;
            std::string format(const paludis::KeywordName &, const paludis::format::Unaccepted &) const;

            std::string format(const std::string &, const paludis::format::Plain &) const;

            std::string format(const paludis::LabelsDepSpec<paludis::URILabelVisitorTypes> &, const paludis::format::Plain &) const;

            std::string format(const paludis::LabelsDepSpec<paludis::DependencyLabelVisitorTypes> &, const paludis::format::Plain &) const;

            std::string format(const paludis::URIDepSpec &, const paludis::format::Plain &) const;

            std::string format(const paludis::BlockDepSpec &, const paludis::format::Plain &) const;

            std::string format(const paludis::tr1::shared_ptr<const paludis::PackageID> &, const paludis::format::Plain &) const;
            std::string format(const paludis::tr1::shared_ptr<const paludis::PackageID> &, const paludis::format::Installed &) const;
            std::string format(const paludis::tr1::shared_ptr<const paludis::PackageID> &, const paludis::format::Installable &) const;

            std::string newline() const;
            std::string indent(const int) const;
    };
}

#endif