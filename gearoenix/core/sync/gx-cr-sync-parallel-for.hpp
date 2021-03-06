#ifndef GEAROENIX_CORE_SYNC_PARALLEL_FOR_HPP
#define GEAROENIX_CORE_SYNC_PARALLEL_FOR_HPP
#include "../gx-cr-build-configuration.hpp"

#ifndef GX_THREAD_NOT_SUPPORTED
#include <functional>
#include <map>
#include <thread>

namespace gearoenix::core::sync {
struct KernelWorkers;
}

#endif

namespace gearoenix::core::sync {
struct ParallelFor final {
#ifndef GX_THREAD_NOT_SUPPORTED
public:
    typedef std::map<std::thread::id, std::pair<std::unique_ptr<KernelWorkers>, std::function<void(unsigned int, unsigned int)>>> Map;

private:
    static Map threads;

    static void exec(std::function<void(unsigned int, unsigned int)> fun) noexcept;
#endif
public:
    ParallelFor() = delete;
    ~ParallelFor() = delete;

    template <typename Iter, typename Fun>
    static void exec(Iter first, Iter end, Fun f)
    {
#ifdef GX_THREAD_NOT_SUPPORTED
        for (; first != end; ++first)
            f(*first);
#else
        exec([f, first, end](const unsigned int kernels_count, const unsigned int kernel_index) noexcept {
            for (Iter iter = (first + kernel_index); iter != end; iter += kernels_count) {
                f(*iter, kernel_index);
            }
        });
#endif
    }
};
}
#endif
