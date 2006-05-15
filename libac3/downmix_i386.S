/* This is basicly gcc generated.
 * Only the floating point rounding mode loads and saves
 * are removed in the stream_sample_to_s16 functions.
 */
	.file	"downmix.c"
	.version	"01.01"
gcc2_compiled.:
.text
	.align 4
.globl downmix_3f_2r_to_2ch
	.type	 downmix_3f_2r_to_2ch,@function
downmix_3f_2r_to_2ch:
	pushl %ebp
	movl %esp,%ebp
	movl 8(%ebp),%eax
	movl 12(%ebp),%edx
	movl $255,%ecx
	addl $4096,%eax
	.p2align 4,,7
.L343:
	flds (%edx)
	flds 4(%edx)
	fld %st(1)
	fmuls -4096(%eax)
	fxch %st(1)
	fmuls -3072(%eax)
	fadd %st,%st(1)
	fxch %st(2)
	fmuls -2048(%eax)
	flds 8(%edx)
	fxch %st(1)
	faddp %st,%st(3)
	fld %st(0)
	fmuls -1024(%eax)
	faddp %st,%st(2)
	fmuls (%eax)
	faddp %st,%st(2)
	fstps -4096(%eax)
	addl $4,%eax
	fstps -3076(%eax)
	decl %ecx
	jns .L343
	leave
	ret
.Lfe1:
	.size	 downmix_3f_2r_to_2ch,.Lfe1-downmix_3f_2r_to_2ch
	.align 4
.globl downmix_2f_2r_to_2ch
	.type	 downmix_2f_2r_to_2ch,@function
downmix_2f_2r_to_2ch:
	pushl %ebp
	movl %esp,%ebp
	movl 8(%ebp),%eax
	movl 12(%ebp),%edx
	movl $255,%ecx
	addl $1024,%eax
	.p2align 4,,7
.L349:
	flds (%edx)
	flds 8(%edx)
	fld %st(1)
	fld %st(1)
	fxch %st(1)
	fmuls -1024(%eax)
	fxch %st(1)
	fmuls 1024(%eax)
	faddp %st,%st(1)
	fxch %st(2)
	fmuls (%eax)
	fxch %st(1)
	fmuls 2048(%eax)
	faddp %st,%st(1)
	fxch %st(1)
	fstps -1024(%eax)
	addl $4,%eax
	fstps -4(%eax)
	decl %ecx
	jns .L349
	leave
	ret
.Lfe2:
	.size	 downmix_2f_2r_to_2ch,.Lfe2-downmix_2f_2r_to_2ch
	.align 4
.globl downmix_3f_1r_to_2ch
	.type	 downmix_3f_1r_to_2ch,@function
downmix_3f_1r_to_2ch:
	pushl %ebp
	movl %esp,%ebp
	movl 8(%ebp),%eax
	movl 12(%ebp),%edx
	movl $255,%ecx
	addl $3072,%eax
	.p2align 4,,7
.L355:
	flds (%edx)
	flds 4(%edx)
	fld %st(1)
	fmuls -3072(%eax)
	fxch %st(1)
	fmuls -2048(%eax)
	fadd %st,%st(1)
	fxch %st(2)
	fmuls -1024(%eax)
	faddp %st,%st(2)
	flds 8(%edx)
	fmuls (%eax)
	fsubr %st,%st(1)
	faddp %st,%st(2)
	fstps -3072(%eax)
	addl $4,%eax
	fstps -2052(%eax)
	decl %ecx
	jns .L355
	leave
	ret
.Lfe3:
	.size	 downmix_3f_1r_to_2ch,.Lfe3-downmix_3f_1r_to_2ch
	.align 4
.globl downmix_2f_1r_to_2ch
	.type	 downmix_2f_1r_to_2ch,@function
downmix_2f_1r_to_2ch:
	pushl %ebp
	movl %esp,%ebp
	movl 8(%ebp),%eax
	movl 12(%ebp),%edx
	movl $255,%ecx
	addl $1024,%eax
	.p2align 4,,7
.L361:
	flds (%edx)
	flds 8(%edx)
	fld %st(1)
	fmuls -1024(%eax)
	fxch %st(1)
	fmuls 1024(%eax)
	fsubr %st,%st(1)
	fxch %st(2)
	fmuls (%eax)
	faddp %st,%st(2)
	fstps -1024(%eax)
	addl $4,%eax
	fstps -4(%eax)
	decl %ecx
	jns .L361
	leave
	ret
.Lfe4:
	.size	 downmix_2f_1r_to_2ch,.Lfe4-downmix_2f_1r_to_2ch
	.align 4
.globl downmix_3f_0r_to_2ch
	.type	 downmix_3f_0r_to_2ch,@function
downmix_3f_0r_to_2ch:
	pushl %ebp
	movl %esp,%ebp
	movl 8(%ebp),%eax
	movl 12(%ebp),%edx
	movl $255,%ecx
	addl $1024,%eax
	.p2align 4,,7
.L367:
	flds (%edx)
	flds 4(%edx)
	fld %st(1)
	fmuls -1024(%eax)
	fxch %st(1)
	fmuls (%eax)
	fadd %st,%st(1)
	fxch %st(2)
	fmuls 1024(%eax)
	faddp %st,%st(2)
	fstps -1024(%eax)
	addl $4,%eax
	fstps -4(%eax)
	decl %ecx
	jns .L367
	leave
	ret
.Lfe5:
	.size	 downmix_3f_0r_to_2ch,.Lfe5-downmix_3f_0r_to_2ch
	.align 4
.globl stream_sample_2ch_to_s16
	.type	 stream_sample_2ch_to_s16,@function
stream_sample_2ch_to_s16:
	pushl %ebp
	movl %esp,%ebp
	subl $28,%esp
	pushl %edi
	pushl %esi
	pushl %ebx
	movl 8(%ebp),%edx
	movl 12(%ebp),%ebx
	movl 16(%ebp),%ecx
	movl $255,%esi
	.p2align 4,,7
.L373:
	flds (%ebx)
	fistpl -8(%ebp)
	movl -8(%ebp),%eax
	movw %ax,(%edx)
	addl $2,%edx
	addl $4,%ebx
	flds (%ecx)
	fistpl -8(%ebp)
	movl -8(%ebp),%eax
	movw %ax,(%edx)
	addl $4,%ecx
	addl $2,%edx
	decl %esi
	jns .L373
	popl %ebx
	popl %esi
	popl %edi
	leave
	ret
.Lfe6:
	.size	 stream_sample_2ch_to_s16,.Lfe6-stream_sample_2ch_to_s16
.section	.rodata
	.align 4
.LC46:
	.long 0x3f350481
.text
	.align 4
.globl stream_sample_1ch_to_s16
	.type	 stream_sample_1ch_to_s16,@function
stream_sample_1ch_to_s16:
	pushl %ebp
	movl %esp,%ebp
	subl $16,%esp
	pushl %esi
	pushl %ebx
	movl 8(%ebp),%edx
	movl 12(%ebp),%ecx
	flds .LC46
	movl $255,%ebx
	.p2align 4,,7
.L379:
	flds (%ecx)
	fmul %st(1),%st
	fistpl -8(%ebp)
	movl -8(%ebp),%eax
	movw %ax,-2(%ebp)
	addl $4,%ecx
	filds -2(%ebp)
	fistpl -8(%ebp)
	movl -8(%ebp),%eax
	movw %ax,(%edx)
	addl $2,%edx
	movw %ax,(%edx)
	addl $2,%edx
	decl %ebx
	jns .L379
	fstp %st(0)
	popl %ebx
	popl %esi
	leave
	ret
.Lfe7:
	.size	 stream_sample_1ch_to_s16,.Lfe7-stream_sample_1ch_to_s16
	.ident	"GCC: (GNU) 2.95.3 19991030 (prerelease)"
