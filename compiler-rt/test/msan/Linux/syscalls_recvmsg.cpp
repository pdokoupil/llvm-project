// RUN: %clangxx_msan -DPRE1 -O0 %s -o %t && not %run %t 2>&1
// RUN: %clangxx_msan -O0 %s -o %t && %run %t

// PRE_SYSCALL(recvmsg) blanket-read the whole msghdr including the output-only
// msg_flags field, reporting a false positive on it. After the fix it reads
// only the input fields (up to, but not including, msg_flags), while still
// checking the genuine inputs.

#include <string.h>
#include <sys/socket.h>

#include <sanitizer/linux_syscall_hooks.h>
#include <sanitizer/msan_interface.h>

int main() {
  struct msghdr m;
  memset(&m, 0, sizeof(m));
#if defined(PRE1)
  // An INPUT field: the pre-hook must still report it uninitialized.
  __msan_poison(&m.msg_iovlen, sizeof(m.msg_iovlen));
#else
  // Output-only field: the pre-hook must NOT read it, so no report.
  __msan_poison(&m.msg_flags, sizeof(m.msg_flags));
#endif
  __sanitizer_syscall_pre_recvmsg(3, &m, 0);
  return 0;
}
