/* Test-only raw ECALL entry for unsupported numbers and malicious arguments.
 * RV64 C already supplies num/a1..a5 in the matching registers. This is not
 * a replacement for production SAVE_ALL/RESTORE_ALL or the user wrappers. */
.text
.align 2
.globl raw_ecall
raw_ecall:
    ecall
    ret
