#include "safetyhook/Factory.hpp"
#include "safetyhook/InlineHook.hpp"
#include "safetyhook/MidHook.hpp"
#include "safetyhook/ThreadFreezer.hpp"

#include "safetyhook/Builder.hpp"

namespace safetyhook {
Builder::~Builder() {
    m_factory->m_builder = nullptr;
}

std::unique_ptr<InlineHook> Builder::create_inline(void* target, void* destination) {
    auto hook = std::unique_ptr<InlineHook>{new InlineHook{m_factory, (uintptr_t)target, (uintptr_t)destination}};

    if (hook->m_trampoline == 0) {
        return nullptr;
    }

    return hook;
}

std::unique_ptr<MidHook> Builder::create_mid(void* target, MidHookFn destination) {
    auto hook = std::unique_ptr<MidHook>{new MidHook{m_factory, (uintptr_t)target, destination}};

    if (hook->m_stub == 0) {
        return nullptr;
    }

    return hook;
}

Builder::Builder(std::shared_ptr<Factory> factory) : m_factory{std::move(factory)} {
    std::scoped_lock _{m_factory->m_mux};

    if (m_factory->m_builder == nullptr) {
        m_factory->m_builder = this;
        m_threads = std::make_shared<ThreadFreezer>();
    } else {
        m_threads = m_factory->m_builder->m_threads;
    }
}

void Builder::fix_ip(uintptr_t old_ip, uintptr_t new_ip) {
    m_threads->fix_ip(old_ip, new_ip);
}

uintptr_t Builder::allocate(size_t size) {
    std::scoped_lock _{m_factory->m_mux};
    return m_factory->allocate(size);
}

uintptr_t Builder::allocate_near(const std::vector<uintptr_t>& desired_addresses, size_t size, size_t max_distance) {
    std::scoped_lock _{m_factory->m_mux};
    return m_factory->allocate_near(desired_addresses, size, max_distance);
}

void Builder::free(uintptr_t address, size_t size) {
    std::scoped_lock _{m_factory->m_mux};
    m_factory->free(address, size);
}
} // namespace safetyhook