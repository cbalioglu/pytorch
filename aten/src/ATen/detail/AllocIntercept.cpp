#include <ATen/detail/AllocIntercept.h>

#include <cstdint>
#include <limits>

#include <iostream>
#include <cerrno>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <c10/core/Allocator.h>
#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>

namespace at {
namespace detail {
namespace {

void noopDeleter(void* ptr) {}

struct FakeAllocator : Allocator {
  DataPtr allocate(std::size_t num_bytes) const override;

  DeleterFnPtr raw_deleter() const override {
    return noopDeleter;
  }
};
static void* addr_ = nullptr;
DataPtr FakeAllocator::allocate(std::size_t num_bytes) const {
  if (addr_ == nullptr) {
//  std::string p{"/tmp/torch-noalloc-XXXXXX"};
//  auto fd = ::mkstemp(&p.front());
//  ::ftruncate(fd, 1024*1024*1024);
  auto fd = ::open("/dev/null", O_RDWR);
  TORCH_CHECK(fd != -1);
  addr_ = ::mmap(/*addr*/ nullptr, 1024 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, /*offset*/0);
  TORCH_CHECK(addr_ != MAP_FAILED);
  }
  std::cout << "ALLOCATING" << std::endl;
  return DataPtr{addr_, addr_, noopDeleter, Device{DeviceType::CPU}};
}

static FakeAllocator fake_allocator{};

Allocator* original_allocator = nullptr;

void replaceAllocator(DeviceType t, Allocator* alloc) {
  SetAllocator(t, alloc, /*priority*/ std::numeric_limits<std::uint8_t>::max());
}

void suspendCPUAllocator() {
  original_allocator = GetAllocator(DeviceType::CPU);

  replaceAllocator(DeviceType::CPU, &fake_allocator);
}

void restoreCPUAllocator() {
  replaceAllocator(DeviceType::CPU, original_allocator);

  original_allocator = nullptr;
}

} // namespace

void suspendAllocators() {
  suspendCPUAllocator();
}

void restoreAllocators() {
  restoreCPUAllocator();
}

} // namespace detail
} // namespace at
