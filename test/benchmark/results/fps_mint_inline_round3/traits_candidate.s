	.file	"traits_codegen.cpp"
	.text
	.section	.text._ZN10fps_detail4Mint4wordEj,"axG",@progbits,_ZN10fps_detail4Mint4wordEj,comdat
	.p2align 4
	.weak	_ZN10fps_detail4Mint4wordEj
	.type	_ZN10fps_detail4Mint4wordEj, @function
_ZN10fps_detail4Mint4wordEj:
.LFB5362:
	.cfi_startproc
	movl	%edi, %eax
	ret
	.cfi_endproc
.LFE5362:
	.size	_ZN10fps_detail4Mint4wordEj, .-_ZN10fps_detail4Mint4wordEj
	.text
	.p2align 4
	.globl	_Z12make_defaultv
	.type	_Z12make_defaultv, @function
_Z12make_defaultv:
.LFB5390:
	.cfi_startproc
	xorl	%eax, %eax
	ret
	.cfi_endproc
.LFE5390:
	.size	_Z12make_defaultv, .-_Z12make_defaultv
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC0:
	.string	"cannot create std::vector larger than max_size()"
	.section	.text.unlikely,"ax",@progbits
.LCOLDB1:
	.text
.LHOTB1:
	.p2align 4
	.globl	_Z11make_vectorm
	.type	_Z11make_vectorm, @function
_Z11make_vectorm:
.LFB5391:
	.cfi_startproc
	movq	%rsi, %rax
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	shrq	$61, %rax
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	jne	.L8
	vpxor	%xmm0, %xmm0, %xmm0
	movq	$0, 16(%rdi)
	movq	%rdi, %rbx
	vmovdqu	%xmm0, (%rdi)
	testq	%rsi, %rsi
	je	.L10
	leaq	0(,%rsi,4), %rbp
	movq	%rbp, %rdi
	call	_Znwm@PLT
	movq	%rbp, %rdx
	xorl	%esi, %esi
	leaq	(%rax,%rbp), %r12
	movq	%rax, (%rbx)
	movq	%rax, %rdi
	movq	%r12, 16(%rbx)
	call	memset@PLT
	movq	%r12, 8(%rbx)
	movq	%rbx, %rax
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L10:
	.cfi_restore_state
	xorl	%r12d, %r12d
	movq	$0, (%rdi)
	movq	%rbx, %rax
	movq	$0, 16(%rdi)
	movq	%r12, 8(%rbx)
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
	.section	.text.unlikely
	.cfi_startproc
	.type	_Z11make_vectorm.cold, @function
_Z11make_vectorm.cold:
.LFSB5391:
.L8:
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	.cfi_offset 6, -24
	.cfi_offset 12, -16
	leaq	.LC0(%rip), %rdi
	call	_ZSt20__throw_length_errorPKc@PLT
	.cfi_endproc
.LFE5391:
	.text
	.size	_Z11make_vectorm, .-_Z11make_vectorm
	.section	.text.unlikely
	.size	_Z11make_vectorm.cold, .-_Z11make_vectorm.cold
.LCOLDE1:
	.text
.LHOTE1:
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC2:
	.string	"vector::_M_default_append"
	.section	.text.unlikely
.LCOLDB3:
	.text
.LHOTB3:
	.p2align 4
	.globl	_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm
	.type	_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm, @function
_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm:
.LFB5392:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	movq	%rsi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rdi, %rbx
	subq	$24, %rsp
	.cfi_def_cfa_offset 80
	movq	8(%rdi), %r12
	movq	(%rdi), %r13
	movq	%r12, %r15
	subq	%r13, %r15
	movq	%r15, %rax
	sarq	$2, %rax
	cmpq	%rsi, %rax
	jb	.L26
	cmpq	%rax, %rsi
	jnb	.L24
	leaq	0(%r13,%rsi,4), %rax
	cmpq	%rax, %r12
	je	.L24
	movq	%rax, 8(%rdi)
.L24:
	addq	$24, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L26:
	.cfi_restore_state
	movq	16(%rdi), %rdx
	movq	%rsi, %rcx
	subq	%rax, %rcx
	subq	%r12, %rdx
	sarq	$2, %rdx
	cmpq	%rcx, %rdx
	jb	.L13
	leaq	0(,%rcx,4), %rdx
	movq	%r12, %rdi
	xorl	%esi, %esi
	movq	%rcx, (%rsp)
	call	memset@PLT
	movq	(%rsp), %rcx
	leaq	(%r12,%rcx,4), %rax
	movq	%rax, 8(%rbx)
	addq	$24, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L13:
	.cfi_restore_state
	movabsq	$2305843009213693951, %rdx
	subq	%rax, %rdx
	cmpq	%rcx, %rdx
	jb	.L23
	cmpq	%rcx, %rax
	movq	%rcx, %r14
	movq	%rcx, 8(%rsp)
	cmovnb	%rax, %r14
	addq	%rax, %r14
	movabsq	$2305843009213693951, %rax
	cmpq	%rax, %r14
	cmova	%rax, %r14
	leaq	0(,%r14,4), %rax
	movq	%rax, %rdi
	movq	%rax, (%rsp)
	call	_Znwm@PLT
	movq	8(%rsp), %rcx
	xorl	%esi, %esi
	leaq	(%rax,%r15), %rdi
	movq	%rax, %r14
	leaq	0(,%rcx,4), %rdx
	call	memset@PLT
	cmpq	%r13, %r12
	je	.L16
	movq	%r14, %rdx
	movq	%r13, %rax
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L17:
	movl	(%rax), %ecx
	addq	$4, %rax
	addq	$4, %rdx
	movl	%ecx, -4(%rdx)
	cmpq	%rax, %r12
	jne	.L17
.L16:
	testq	%r13, %r13
	je	.L18
	movq	16(%rbx), %rsi
	movq	%r13, %rdi
	subq	%r13, %rsi
	call	_ZdlPvm@PLT
.L18:
	movq	(%rsp), %r8
	leaq	(%r14,%rbp,4), %rax
	vmovq	%r14, %xmm1
	vpinsrq	$1, %rax, %xmm1, %xmm0
	addq	%r14, %r8
	vmovdqu	%xmm0, (%rbx)
	movq	%r8, 16(%rbx)
	addq	$24, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
	.section	.text.unlikely
	.cfi_startproc
	.type	_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm.cold, @function
_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm.cold:
.LFSB5392:
.L23:
	.cfi_def_cfa_offset 80
	.cfi_offset 3, -56
	.cfi_offset 6, -48
	.cfi_offset 12, -40
	.cfi_offset 13, -32
	.cfi_offset 14, -24
	.cfi_offset 15, -16
	leaq	.LC2(%rip), %rdi
	call	_ZSt20__throw_length_errorPKc@PLT
	.cfi_endproc
.LFE5392:
	.text
	.size	_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm, .-_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm
	.section	.text.unlikely
	.size	_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm.cold, .-_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm.cold
.LCOLDE3:
	.text
.LHOTE3:
	.section	.text._ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev,"axG",@progbits,_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED5Ev,comdat
	.align 2
	.p2align 4
	.weak	_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev
	.type	_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev, @function
_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev:
.LFB6240:
	.cfi_startproc
	movq	(%rdi), %rax
	testq	%rax, %rax
	je	.L29
	movq	16(%rdi), %rsi
	movq	%rax, %rdi
	subq	%rax, %rsi
	jmp	_ZdlPvm@PLT
	.p2align 4,,10
	.p2align 3
.L29:
	ret
	.cfi_endproc
.LFE6240:
	.size	_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev, .-_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev
	.weak	_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED1Ev
	.set	_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED1Ev,_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev
	.section	.rodata.str1.1
.LC4:
	.string	"int main()"
	.section	.rodata.str1.8
	.align 8
.LC5:
	.string	"/workspace/cp-fps-scratch-round3/mint-inline/traits_codegen.cpp"
	.align 8
.LC6:
	.string	"make_default().v==0 && global_zero.v==0"
	.section	.rodata.str1.1
.LC7:
	.string	"x.v==0"
.LC8:
	.string	"a[0].v==2*T::mod()-1"
.LC9:
	.string	"a[i].v==0"
.LC10:
	.string	"b[i].v==0"
.LC11:
	.string	"p[i].v==0"
	.section	.rodata.str1.8
	.align 8
.LC12:
	.ascii	"{\"sizeof\":4,\"alignof\":4,\"standard_layou"
	.string	"t\":true,\"trivially_copyable\":true,\"aggregate\":false,\"trivial\":false,\"trivially_default_constructible\":false,\"default_constructible\":true,\"nothrow_default_constructible\":true,\"constexpr_zero\":true,\"default_initialization\":\"zero\",\"resize_prefix\":\"preserved\"}"
	.section	.text.unlikely
.LCOLDB13:
	.section	.text.startup,"ax",@progbits
.LHOTB13:
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB5393:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA5393
	pushq	%r14
	.cfi_def_cfa_offset 16
	.cfi_offset 14, -16
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	subq	$32, %rsp
	.cfi_def_cfa_offset 64
	call	_Z12make_defaultv
	orl	global_zero(%rip), %eax
	jne	.L53
	movl	$513, %esi
	movq	%rsp, %rdi
	movq	%rsp, %rbx
.LEHB0:
	call	_Z11make_vectorm
.LEHE0:
	movq	(%rsp), %rbp
	movq	8(%rsp), %rdx
	cmpq	%rdx, %rbp
	je	.L32
	movq	%rbp, %rax
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L34:
	movl	(%rax), %r8d
	testl	%r8d, %r8d
	jne	.L54
	addq	$4, %rax
	cmpq	%rax, %rdx
	jne	.L34
.L32:
	movl	$1996488705, %edi
	call	_ZN10fps_detail4Mint4wordEj
	movl	$1025, %esi
	movq	%rbx, %rdi
	movl	%eax, 0(%rbp)
.LEHB1:
	call	_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm
	movq	(%rsp), %rdx
	movq	8(%rsp), %rsi
	cmpl	$1996488705, (%rdx)
	jne	.L35
	subq	%rdx, %rsi
	movl	$1, %eax
	movq	%rsi, %rcx
	sarq	$2, %rcx
	cmpq	$4, %rsi
	jbe	.L37
	.p2align 4
	.p2align 4,,10
	.p2align 3
.L36:
	movl	(%rdx,%rax,4), %edi
	testl	%edi, %edi
	jne	.L55
	incq	%rax
	cmpq	%rcx, %rax
	jb	.L36
.L37:
	movl	$516, %edi
	call	_Znam@PLT
.LEHE1:
	movq	%rax, %rbp
	movl	$516, %edx
	xorl	%esi, %esi
	movq	%rax, %rdi
	call	memset@PLT
	leaq	516(%rbp), %rdx
	.p2align 4
	.p2align 4,,10
	.p2align 3
.L40:
	movl	(%rax), %esi
	testl	%esi, %esi
	jne	.L56
	addq	$4, %rax
	cmpq	%rax, %rdx
	jne	.L40
	movl	$1028, %edi
.LEHB2:
	call	_Znwm@PLT
	movq	%rax, %rdi
	movl	$1028, %edx
	xorl	%esi, %esi
	call	memset@PLT
	movq	%rax, %rdi
	leaq	1028(%rax), %rdx
	.p2align 4
	.p2align 4,,10
	.p2align 3
.L42:
	movl	(%rax), %ecx
	testl	%ecx, %ecx
	jne	.L57
	addq	$4, %rax
	cmpq	%rdx, %rax
	jne	.L42
	call	_ZdlPv@PLT
	leaq	.LC12(%rip), %rdi
	call	puts@PLT
.LEHE2:
	movq	%rbp, %rdi
	call	_ZdaPv@PLT
	movq	%rbx, %rdi
	call	_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev
	addq	$32, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 32
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r14
	.cfi_def_cfa_offset 8
	ret
.L47:
	.cfi_restore_state
	movq	%rax, %r14
	jmp	.L43
.L46:
	movq	%rax, %rbp
	vzeroupper
	jmp	.L44
	.section	.gcc_except_table,"a",@progbits
.LLSDA5393:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE5393-.LLSDACSB5393
.LLSDACSB5393:
	.uleb128 .LEHB0-.LFB5393
	.uleb128 .LEHE0-.LEHB0
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB1-.LFB5393
	.uleb128 .LEHE1-.LEHB1
	.uleb128 .L46-.LFB5393
	.uleb128 0
	.uleb128 .LEHB2-.LFB5393
	.uleb128 .LEHE2-.LEHB2
	.uleb128 .L47-.LFB5393
	.uleb128 0
.LLSDACSE5393:
	.section	.text.startup
	.cfi_endproc
	.section	.text.unlikely
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDAC5393
	.type	main.cold, @function
main.cold:
.LFSB5393:
.L53:
	.cfi_def_cfa_offset 64
	.cfi_offset 3, -32
	.cfi_offset 6, -24
	.cfi_offset 14, -16
	leaq	.LC4(%rip), %rcx
	movl	$22, %edx
	leaq	.LC5(%rip), %rsi
	leaq	.LC6(%rip), %rdi
	call	__assert_fail@PLT
.L57:
	leaq	.LC4(%rip), %rcx
	movl	$26, %edx
	leaq	.LC5(%rip), %rsi
	leaq	.LC11(%rip), %rdi
	call	__assert_fail@PLT
.L56:
	leaq	.LC4(%rip), %rcx
	movl	$25, %edx
	leaq	.LC5(%rip), %rsi
	leaq	.LC10(%rip), %rdi
	call	__assert_fail@PLT
.L43:
	movq	%rbp, %rdi
	vzeroupper
	movq	%r14, %rbp
	call	_ZdaPv@PLT
.L44:
	movq	%rbx, %rdi
	call	_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev
	movq	%rbp, %rdi
.LEHB3:
	call	_Unwind_Resume@PLT
.LEHE3:
.L55:
	leaq	.LC4(%rip), %rcx
	movl	$24, %edx
	leaq	.LC5(%rip), %rsi
	leaq	.LC9(%rip), %rdi
	call	__assert_fail@PLT
.L54:
	leaq	.LC4(%rip), %rcx
	movl	$23, %edx
	leaq	.LC5(%rip), %rsi
	leaq	.LC7(%rip), %rdi
	call	__assert_fail@PLT
.L35:
	leaq	.LC4(%rip), %rcx
	movl	$24, %edx
	leaq	.LC5(%rip), %rsi
	leaq	.LC8(%rip), %rdi
	call	__assert_fail@PLT
	.cfi_endproc
.LFE5393:
	.section	.gcc_except_table
.LLSDAC5393:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSEC5393-.LLSDACSBC5393
.LLSDACSBC5393:
	.uleb128 .LEHB3-.LCOLDB13
	.uleb128 .LEHE3-.LEHB3
	.uleb128 0
	.uleb128 0
.LLSDACSEC5393:
	.section	.text.unlikely
	.section	.text.startup
	.size	main, .-main
	.section	.text.unlikely
	.size	main.cold, .-main.cold
.LCOLDE13:
	.section	.text.startup
.LHOTE13:
	.globl	global_zero
	.bss
	.align 4
	.type	global_zero, @object
	.size	global_zero, 4
global_zero:
	.zero	4
	.hidden	DW.ref.__gxx_personality_v0
	.weak	DW.ref.__gxx_personality_v0
	.section	.data.rel.local.DW.ref.__gxx_personality_v0,"awG",@progbits,DW.ref.__gxx_personality_v0,comdat
	.align 8
	.type	DW.ref.__gxx_personality_v0, @object
	.size	DW.ref.__gxx_personality_v0, 8
DW.ref.__gxx_personality_v0:
	.quad	__gxx_personality_v0
	.globl	__gxx_personality_v0
	.ident	"GCC: (Debian 14.2.0-19) 14.2.0"
	.section	.note.GNU-stack,"",@progbits
