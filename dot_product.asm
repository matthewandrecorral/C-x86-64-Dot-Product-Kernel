; ============================================================
; dot_product.asm
;
; Calculates:
;     sdot = A[0] * B[0] + A[1] * B[1] + ... + A[n-1] * B[n-1]
;
; C declaration:
;     void dot_product_asm(
;         const double* vector_a,
;         const double* vector_b,
;         size_t n,
;         double* sdot
;     );
;
; Windows x64 parameter registers:
;     RCX = address of vector_a
;     RDX = address of vector_b
;     R8  = n
;     R9  = address of sdot
;
; XMM registers:
;     XMM0 = accumulated sum
;     XMM1 = current product
; ============================================================

.code

PUBLIC dot_product_asm

dot_product_asm PROC

    ; Set the accumulator to 0.0.
    xorpd xmm0, xmm0

    ; If n is zero, skip the loop and store 0.0.
    test r8, r8
    jz dot_product_done

dot_product_loop:

    ; Load one double from vector A.
    movsd xmm1, QWORD PTR [rcx]

    ; Multiply it by one double from vector B.
    mulsd xmm1, QWORD PTR [rdx]

    ; Add the product to the running total.
    addsd xmm0, xmm1

    ; Move to the next double in vector A.
    add rcx, 8

    ; Move to the next double in vector B.
    add rdx, 8

    ; Reduce the remaining element count.
    dec r8

    ; Repeat while there are elements remaining.
    jnz dot_product_loop

dot_product_done:

    ; Store the final result in the memory location sdot.
    movsd QWORD PTR [r9], xmm0

    ret

dot_product_asm ENDP

END