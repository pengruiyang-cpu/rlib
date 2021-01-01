	.file	"bitmap.c"
	.text
	.globl	bitmap_init
	.type	bitmap_init, @function
bitmap_init:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	-20(%rbp), %eax
	movq	%rax, %rdi
	call	malloc
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	bitmap_init, .-bitmap_init
	.globl	bitmap_set
	.type	bitmap_set, @function
bitmap_set:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movl	%esi, -28(%rbp)
	movl	%edx, %eax
	movb	%al, -32(%rbp)
	movl	-28(%rbp), %eax
	andl	$7, %eax
	movl	$128, %edx
	movl	%eax, %ecx
	sarl	%cl, %edx
	movl	%edx, %eax
	movb	%al, -1(%rbp)
	cmpb	$0, -32(%rbp)
	je	.L4
	movl	-28(%rbp), %eax
	shrl	$3, %eax
	movl	%eax, %edx
	movq	-24(%rbp), %rax
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movl	-28(%rbp), %edx
	shrl	$3, %edx
	movl	%edx, %ecx
	movq	-24(%rbp), %rdx
	addq	%rcx, %rdx
	orb	-1(%rbp), %al
	movb	%al, (%rdx)
	jmp	.L6
.L4:
	movl	-28(%rbp), %eax
	shrl	$3, %eax
	movl	%eax, %edx
	movq	-24(%rbp), %rax
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movl	%eax, %edx
	movzbl	-1(%rbp), %eax
	notl	%eax
	andl	%eax, %edx
	movl	-28(%rbp), %eax
	shrl	$3, %eax
	movl	%eax, %ecx
	movq	-24(%rbp), %rax
	addq	%rcx, %rax
	movb	%dl, (%rax)
.L6:
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	bitmap_set, .-bitmap_set
	.globl	bitmap_read
	.type	bitmap_read, @function
bitmap_read:
.LFB8:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movl	%esi, -28(%rbp)
	movl	-28(%rbp), %eax
	andl	$7, %eax
	movl	$128, %edx
	movl	%eax, %ecx
	sarl	%cl, %edx
	movl	%edx, %eax
	movb	%al, -1(%rbp)
	movl	-28(%rbp), %eax
	shrl	$3, %eax
	movl	%eax, %edx
	movq	-24(%rbp), %rax
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	andb	-1(%rbp), %al
	cmpb	%al, -1(%rbp)
	sete	%al
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	bitmap_read, .-bitmap_read
	.ident	"GCC: (GNU) 10.2.1 20201125 (Red Hat 10.2.1-9)"
	.section	.note.GNU-stack,"",@progbits
