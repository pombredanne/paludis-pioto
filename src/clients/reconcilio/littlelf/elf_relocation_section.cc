
#include "elf_relocation_section.hh"
#include "elf_types.hh"

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>

#include <libwrapiter/libwrapiter_forward_iterator-impl.hh>

#include <istream>
#include <vector>

using namespace paludis;

namespace paludis
{
    template <typename ElfType_, typename Relocation_>
    struct Implementation<RelocationSection<ElfType_, Relocation_> >
    {
        std::vector<typename Relocation_::Entry> relocations;
    };
}

template <typename ElfType_>
RelocationEntry<ElfType_>::RelocationEntry(const typename ElfType_::Relocation & my_relocation) :
    _my_relocation(my_relocation)
{
}

template <typename ElfType_>
RelocationEntry<ElfType_>::~RelocationEntry()
{
}

template <typename ElfType_>
RelocationAEntry<ElfType_>::RelocationAEntry(const typename ElfType_::RelocationA & my_relocation) :
    _my_relocation(my_relocation)
{
}

template <typename ElfType_>
RelocationAEntry<ElfType_>::~RelocationAEntry()
{
}

template <typename ElfType_> const std::string Relocation<ElfType_>::type_name = "REL";
template <typename ElfType_> const std::string RelocationA<ElfType_>::type_name = "RELA";

template <typename ElfType_, typename Relocation_>
RelocationSection<ElfType_, Relocation_>::RelocationSection(const typename ElfType_::SectionHeader & shdr, std::istream & stream) :
    Section<ElfType_>(shdr),
    PrivateImplementationPattern<RelocationSection>(new Implementation<RelocationSection>)
{
    std::vector<typename Relocation_::Type> relocations(shdr.sh_size / shdr.sh_entsize);
    stream.seekg(shdr.sh_offset, std::ios::beg);
    stream.read(reinterpret_cast<char *>(&relocations.front()), shdr.sh_size);
    for (typename std::vector<typename Relocation_::Type>::iterator i = relocations.begin(); i != relocations.end(); ++i)
        _imp->relocations.push_back(typename Relocation_::Entry(*i));
}

template <typename ElfType_, typename Relocation_>
RelocationSection<ElfType_, Relocation_>::~RelocationSection()
{
}

template <typename ElfType_, typename Relocation_>
typename RelocationSection<ElfType_, Relocation_>::RelocationIterator
RelocationSection<ElfType_, Relocation_>::relocation_begin() const
{
    return RelocationIterator(_imp->relocations.begin());
}

template <typename ElfType_, typename Relocation_>
typename RelocationSection<ElfType_, Relocation_>::RelocationIterator
RelocationSection<ElfType_, Relocation_>::relocation_end() const
{
    return RelocationIterator(_imp->relocations.end());
}

template class RelocationSection<Elf32Type, Relocation<Elf32Type> >;
template class RelocationSection<Elf32Type, RelocationA<Elf32Type> >;
template class RelocationSection<Elf64Type, Relocation<Elf64Type> >;
template class RelocationSection<Elf64Type, RelocationA<Elf64Type> >;
