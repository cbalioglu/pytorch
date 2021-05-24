#pragma once

namespace at {
namespace detail {

void suspendAllocators();

void restoreAllocators();

} // namespace detail
} // namespace at
