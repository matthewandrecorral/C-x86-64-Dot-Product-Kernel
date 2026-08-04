.code

PUBLIC dot_product_asm

dot_product_asm PROC

    xorpd xmm0, xmm0

    test r8, r8
    jz dot_product_done

dot_product_loop:

    movsd xmm1, QWORD PTR [rcx]

    mulsd xmm1, QWORD PTR [rdx]

    addsd xmm0, xmm1

    add rcx, 8

    add rdx, 8

    dec r8

    jnz dot_product_loop

dot_product_done:

    movsd QWORD PTR [r9], xmm0

    ret

dot_product_asm ENDP

END
