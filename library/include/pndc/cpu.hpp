#pragma once

namespace pndc {
inline static void cpu_relax() {
    asm volatile ("pause");
}
}
