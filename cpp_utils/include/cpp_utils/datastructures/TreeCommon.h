#ifndef TREECOMMON_H_HPODLZ4K
#define TREECOMMON_H_HPODLZ4K

#include "cpp_utils/types/NamedType.h"
#include <cstdint>

namespace details {

struct CountTag { };

struct SourcePosTag { };

struct DestinationPosTag { };

} // namespace details

namespace ds {

using Count = types::ImplicitNamedType<int64_t, details::CountTag>;

using SourcePosition = types::ImplicitNamedType<int64_t, details::SourcePosTag>;

using DestinationPosition =
    types::ImplicitNamedType<int64_t, details::DestinationPosTag>;

} // namespace ds

#endif /* end of include guard: TREECOMMON_H_HPODLZ4K */
