include misc.inc
.code

read_tsc proc

    rdtsc
    shl rdx, 20h
    or rax, rdx
    
    ret
read_tsc endp

END