	.file	"traits_codegen.cpp"
	.text
	.section	.text._ZN10fps_detail4MintC2Ev,"axG",@progbits,_ZN10fps_detail4MintC5Ev,comdat
	.align 2
	.p2align 4
	.weak	_ZN10fps_detail4MintC2Ev
	.type	_ZN10fps_detail4MintC2Ev, @function
_ZN10fps_detail4MintC2Ev:
.LFB5360:
	.cfi_startproc
	movl	$0, (%rdi)
	ret
	.cfi_endproc
.LFE5360:
	.size	_ZN10fps_detail4MintC2Ev, .-_ZN10fps_detail4MintC2Ev
	.weak	_ZN10fps_detail4MintC1Ev
	.set	_ZN10fps_detail4MintC1Ev,_ZN10fps_detail4MintC2Ev
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
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	movq	%rsi, %rax
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	subq	$8, %rsp
	.cfi_def_cfa_offset 48
	shrq	$61, %rax
	jne	.L11
	vpxor	%xmm0, %xmm0, %xmm0
	movq	$0, 16(%rdi)
	movq	%rdi, %r12
	movq	%rsi, %rbp
	vmovdqu	%xmm0, (%rdi)
	testq	%rsi, %rsi
	je	.L13
	leaq	0(,%rsi,4), %r13
	movq	%r13, %rdi
	call	_Znwm@PLT
	addq	%rax, %r13
	vmovq	%rax, %xmm1
	movq	%rax, %rbx
	movq	%r13, 16(%r12)
	vpunpcklqdq	%xmm1, %xmm1, %xmm0
	vmovdqu	%xmm0, (%r12)
	.p2align 4,,10
	.p2align 3
.L9:
	movl	$0, (%rbx)
	movq	%rbx, %rdi
	addq	$4, %rbx
	call	_ZN10fps_detail4MintC1Ev
	decq	%rbp
	jne	.L9
	movq	%r13, %rax
.L8:
	movq	%rax, 8(%r12)
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	movq	%r12, %rax
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L13:
	.cfi_restore_state
	movq	$0, (%rdi)
	xorl	%eax, %eax
	movq	$0, 16(%rdi)
	jmp	.L8
	.cfi_endproc
	.section	.text.unlikely
	.cfi_startproc
	.type	_Z11make_vectorm.cold, @function
_Z11make_vectorm.cold:
.LFSB5391:
.L11:
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -40
	.cfi_offset 6, -32
	.cfi_offset 12, -24
	.cfi_offset 13, -16
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
	je	.L16
	movq	16(%rdi), %rsi
	movq	%rax, %rdi
	subq	%rax, %rsi
	jmp	_ZdlPvm@PLT
	.p2align 4,,10
	.p2align 3
.L16:
	ret
	.cfi_endproc
.LFE6240:
	.size	_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev, .-_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev
	.weak	_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED1Ev
	.set	_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED1Ev,_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev
	.section	.rodata._ZNSt6vectorIN10fps_detail4MintESaIS1_EE17_M_default_appendEm.str1.1,"aMS",@progbits,1
.LC2:
	.string	"vector::_M_default_append"
	.section	.text._ZNSt6vectorIN10fps_detail4MintESaIS1_EE17_M_default_appendEm,"axG",@progbits,_ZNSt6vectorIN10fps_detail4MintESaIS1_EE17_M_default_appendEm,comdat
	.align 2
	.p2align 4
	.weak	_ZNSt6vectorIN10fps_detail4MintESaIS1_EE17_M_default_appendEm
	.type	_ZNSt6vectorIN10fps_detail4MintESaIS1_EE17_M_default_appendEm, @function
_ZNSt6vectorIN10fps_detail4MintESaIS1_EE17_M_default_appendEm:
.LFB6244:
	.cfi_startproc
	testq	%rsi, %rsi
	je	.L37
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
	movq	%rdi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rsi, %rbx
	subq	$40, %rsp
	.cfi_def_cfa_offset 96
	movq	8(%rdi), %r13
	movq	16(%rdi), %rax
	subq	%r13, %rax
	sarq	$2, %rax
	cmpq	%rsi, %rax
	jb	.L19
	movq	%rsi, %r14
	movq	%r13, %r12
	.p2align 4,,10
	.p2align 3
.L20:
	movl	$0, (%r12)
	movq	%r12, %rdi
	addq	$4, %r12
	call	_ZN10fps_detail4MintC1Ev
	decq	%r14
	jne	.L20
	leaq	0(%r13,%rbx,4), %rax
	movq	%rax, 8(%rbp)
	addq	$40, %rsp
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
.L37:
	.cfi_restore 3
	.cfi_restore 6
	.cfi_restore 12
	.cfi_restore 13
	.cfi_restore 14
	.cfi_restore 15
	ret
	.p2align 4,,10
	.p2align 3
.L19:
	.cfi_def_cfa_offset 96
	.cfi_offset 3, -56
	.cfi_offset 6, -48
	.cfi_offset 12, -40
	.cfi_offset 13, -32
	.cfi_offset 14, -24
	.cfi_offset 15, -16
	movq	(%rdi), %r14
	movq	%r13, %r15
	subq	%r14, %r15
	movq	%r15, %rax
	sarq	$2, %rax
	movq	%rax, %rsi
	movq	%rax, 16(%rsp)
	movabsq	$2305843009213693951, %rax
	subq	%rsi, %rax
	cmpq	%rbx, %rax
	jb	.L40
	movq	16(%rsp), %rcx
	movq	%rbx, %r12
	movabsq	$2305843009213693951, %rdx
	cmpq	%rcx, %rbx
	movq	%rcx, %rax
	cmovnb	%rbx, %rax
	addq	%rcx, %rax
	cmpq	%rdx, %rax
	cmova	%rdx, %rax
	salq	$2, %rax
	movq	%rax, %rdi
	movq	%rax, 24(%rsp)
	call	_Znwm@PLT
	movq	%rax, 8(%rsp)
	addq	%rax, %r15
	.p2align 4,,10
	.p2align 3
.L22:
	movl	$0, (%r15)
	movq	%r15, %rdi
	addq	$4, %r15
	call	_ZN10fps_detail4MintC1Ev
	decq	%r12
	jne	.L22
	cmpq	%r13, %r14
	je	.L23
	movq	8(%rsp), %rax
	subq	%r14, %r13
	movq	%r14, %rdx
	addq	%rax, %r13
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L24:
	movl	(%rdx), %ecx
	addq	$4, %rax
	addq	$4, %rdx
	movl	%ecx, -4(%rax)
	cmpq	%r13, %rax
	jne	.L24
.L23:
	testq	%r14, %r14
	je	.L25
	movq	16(%rbp), %rsi
	movq	%r14, %rdi
	subq	%r14, %rsi
	call	_ZdlPvm@PLT
.L25:
	movq	16(%rsp), %rax
	movq	8(%rsp), %rdx
	addq	%rax, %rbx
	vmovq	%rdx, %xmm1
	leaq	(%rdx,%rbx,4), %rax
	vpinsrq	$1, %rax, %xmm1, %xmm0
	movq	24(%rsp), %rax
	vmovdqu	%xmm0, 0(%rbp)
	addq	%rdx, %rax
	movq	%rax, 16(%rbp)
	addq	$40, %rsp
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
.L40:
	.cfi_restore_state
	leaq	.LC2(%rip), %rdi
	call	_ZSt20__throw_length_errorPKc@PLT
	.cfi_endproc
.LFE6244:
	.size	_ZNSt6vectorIN10fps_detail4MintESaIS1_EE17_M_default_appendEm, .-_ZNSt6vectorIN10fps_detail4MintESaIS1_EE17_M_default_appendEm
	.text
	.p2align 4
	.globl	_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm
	.type	_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm, @function
_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm:
.LFB5392:
	.cfi_startproc
	movq	8(%rdi), %rdx
	movq	(%rdi), %rcx
	movq	%rdx, %rax
	subq	%rcx, %rax
	sarq	$2, %rax
	cmpq	%rsi, %rax
	jb	.L45
	cmpq	%rax, %rsi
	jnb	.L44
	leaq	(%rcx,%rsi,4), %rax
	cmpq	%rax, %rdx
	je	.L44
	movq	%rax, 8(%rdi)
.L44:
	ret
	.p2align 4,,10
	.p2align 3
.L45:
	subq	%rax, %rsi
	jmp	_ZNSt6vectorIN10fps_detail4MintESaIS1_EE17_M_default_appendEm
	.cfi_endproc
.LFE5392:
	.size	_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm, .-_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC3:
	.string	"int main()"
	.section	.rodata.str1.8
	.align 8
.LC4:
	.string	"/workspace/cp-fps-scratch-round3/mint-inline/traits_codegen.cpp"
	.align 8
.LC5:
	.string	"make_default().v==0 && global_zero.v==0"
	.section	.rodata.str1.1
.LC6:
	.string	"x.v==0"
.LC7:
	.string	"a[0].v==2*T::mod()-1"
.LC8:
	.string	"a[i].v==0"
.LC9:
	.string	"b[i].v==0"
.LC10:
	.string	"p[i].v==0"
	.section	.rodata.str1.8
	.align 8
.LC11:
	.ascii	"{\"sizeof\":4,\"alignof\":4,\"standard_layou"
	.string	"t\":true,\"trivially_copyable\":true,\"aggregate\":false,\"trivial\":false,\"trivially_default_constructible\":false,\"default_constructible\":true,\"nothrow_default_constructible\":true,\"constexpr_zero\":true,\"default_initialization\":\"zero\",\"resize_prefix\":\"preserved\"}"
	.section	.text.unlikely
.LCOLDB12:
	.section	.text.startup,"ax",@progbits
.LHOTB12:
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
	pushq	%r13
	.cfi_def_cfa_offset 24
	.cfi_offset 13, -24
	pushq	%r12
	.cfi_def_cfa_offset 32
	.cfi_offset 12, -32
	pushq	%rbp
	.cfi_def_cfa_offset 40
	.cfi_offset 6, -40
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	subq	$32, %rsp
	.cfi_def_cfa_offset 80
	call	_Z12make_defaultv
	orl	global_zero(%rip), %eax
	jne	.L73
	movl	$513, %esi
	movq	%rsp, %rdi
	movq	%rsp, %r13
.LEHB0:
	call	_Z11make_vectorm
.LEHE0:
	movq	(%rsp), %rbx
	movq	8(%rsp), %rdx
	cmpq	%rdx, %rbx
	je	.L48
	movq	%rbx, %rax
	.p2align 4
	.p2align 4,,10
	.p2align 3
.L50:
	movl	(%rax), %edi
	testl	%edi, %edi
	jne	.L74
	addq	$4, %rax
	cmpq	%rax, %rdx
	jne	.L50
.L48:
	movl	$1996488705, %edi
	call	_ZN10fps_detail4Mint4wordEj
	movl	$1025, %esi
	movq	%r13, %rdi
	movl	%eax, (%rbx)
.LEHB1:
	call	_Z13resize_vectorRSt6vectorIN10fps_detail4MintESaIS1_EEm
	movq	(%rsp), %rdx
	movq	8(%rsp), %rsi
	cmpl	$1996488705, (%rdx)
	jne	.L51
	subq	%rdx, %rsi
	movl	$1, %eax
	movq	%rsi, %rcx
	sarq	$2, %rcx
	cmpq	$4, %rsi
	jbe	.L53
	.p2align 4
	.p2align 4,,10
	.p2align 3
.L52:
	movl	(%rdx,%rax,4), %esi
	testl	%esi, %esi
	jne	.L75
	incq	%rax
	cmpq	%rcx, %rax
	jb	.L52
.L53:
	movl	$516, %edi
	call	_Znam@PLT
.LEHE1:
	movq	%rax, %r12
	movq	%rax, %rbx
	leaq	516(%rax), %rbp
	movq	%rax, %r14
	.p2align 4,,10
	.p2align 3
.L55:
	movl	$0, (%r14)
	movq	%r14, %rdi
	addq	$4, %r14
	call	_ZN10fps_detail4MintC1Ev
	cmpq	%r14, %rbp
	jne	.L55
	.p2align 4
	.p2align 4,,10
	.p2align 3
.L57:
	movl	(%rbx), %ecx
	testl	%ecx, %ecx
	jne	.L76
	addq	$4, %rbx
	cmpq	%rbp, %rbx
	jne	.L57
	movl	$1028, %edi
.LEHB2:
	call	_Znwm@PLT
	movq	%rax, %r14
	leaq	1028(%rax), %rbx
	movq	%rax, %rbp
	.p2align 4,,10
	.p2align 3
.L58:
	movq	%rbp, %rdi
	addq	$4, %rbp
	call	_ZN10fps_detail4MintC1Ev
	cmpq	%rbx, %rbp
	jne	.L58
	movq	%r14, %rax
	.p2align 4
	.p2align 4,,10
	.p2align 3
.L60:
	movl	(%rax), %edx
	testl	%edx, %edx
	jne	.L77
	addq	$4, %rax
	cmpq	%rax, %rbx
	jne	.L60
	movq	%r14, %rdi
	call	_ZdlPv@PLT
	leaq	.LC11(%rip), %rdi
	call	puts@PLT
.LEHE2:
	movq	%r12, %rdi
	call	_ZdaPv@PLT
	movq	%r13, %rdi
	call	_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev
	addq	$32, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 48
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 40
	popq	%rbp
	.cfi_def_cfa_offset 32
	popq	%r12
	.cfi_def_cfa_offset 24
	popq	%r13
	.cfi_def_cfa_offset 16
	popq	%r14
	.cfi_def_cfa_offset 8
	ret
.L65:
	.cfi_restore_state
	movq	%rax, %rbx
	jmp	.L61
.L64:
	movq	%rax, %rbx
	vzeroupper
	jmp	.L62
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
	.uleb128 .L64-.LFB5393
	.uleb128 0
	.uleb128 .LEHB2-.LFB5393
	.uleb128 .LEHE2-.LEHB2
	.uleb128 .L65-.LFB5393
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
.L73:
	.cfi_def_cfa_offset 80
	.cfi_offset 3, -48
	.cfi_offset 6, -40
	.cfi_offset 12, -32
	.cfi_offset 13, -24
	.cfi_offset 14, -16
	leaq	.LC3(%rip), %rcx
	movl	$22, %edx
	leaq	.LC4(%rip), %rsi
	leaq	.LC5(%rip), %rdi
	call	__assert_fail@PLT
.L77:
	leaq	.LC3(%rip), %rcx
	movl	$26, %edx
	leaq	.LC4(%rip), %rsi
	leaq	.LC10(%rip), %rdi
	call	__assert_fail@PLT
.L76:
	leaq	.LC3(%rip), %rcx
	movl	$25, %edx
	leaq	.LC4(%rip), %rsi
	leaq	.LC9(%rip), %rdi
	call	__assert_fail@PLT
.L61:
	movq	%r12, %rdi
	vzeroupper
	call	_ZdaPv@PLT
.L62:
	movq	%r13, %rdi
	call	_ZNSt12_Vector_baseIN10fps_detail4MintESaIS1_EED2Ev
	movq	%rbx, %rdi
.LEHB3:
	call	_Unwind_Resume@PLT
.LEHE3:
.L75:
	leaq	.LC3(%rip), %rcx
	movl	$24, %edx
	leaq	.LC4(%rip), %rsi
	leaq	.LC8(%rip), %rdi
	call	__assert_fail@PLT
.L51:
	leaq	.LC3(%rip), %rcx
	movl	$24, %edx
	leaq	.LC4(%rip), %rsi
	leaq	.LC7(%rip), %rdi
	call	__assert_fail@PLT
.L74:
	leaq	.LC3(%rip), %rcx
	movl	$23, %edx
	leaq	.LC4(%rip), %rsi
	leaq	.LC6(%rip), %rdi
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
	.uleb128 .LEHB3-.LCOLDB12
	.uleb128 .LEHE3-.LEHB3
	.uleb128 0
	.uleb128 0
.LLSDACSEC5393:
	.section	.text.unlikely
	.section	.text.startup
	.size	main, .-main
	.section	.text.unlikely
	.size	main.cold, .-main.cold
.LCOLDE12:
	.section	.text.startup
.LHOTE12:
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
