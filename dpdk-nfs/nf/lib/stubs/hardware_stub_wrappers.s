	.file	"hwstub_wrappers.c"
	.text
	.globl	stub_hardware_read_wrapper
	.align	16, 0x90
	.type	stub_hardware_read_wrapper,@function
stub_hardware_read_wrapper:             # @stub_hardware_read_wrapper
	.cfi_startproc
# BB#0:
	pushq	%rbp
.Ltmp2:
	.cfi_def_cfa_offset 16
.Ltmp3:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp4:
	.cfi_def_cfa_register %rbp
	subq	$272, %rsp              # imm = 0x110
        nop
        nop
        nop
        nop
        nop
        nop
	callq	stub_hardware_read
	addq	$272, %rsp              # imm = 0x110
	popq	%rbp
	ret
.Ltmp5:
	.size	stub_hardware_read_wrapper, .Ltmp5-stub_hardware_read_wrapper
	.cfi_endproc

	.globl	stub_hardware_write_wrapper
	.align	16, 0x90
	.type	stub_hardware_write_wrapper,@function
stub_hardware_write_wrapper:            # @stub_hardware_write_wrapper
	.cfi_startproc
# BB#0:
	pushq	%rbp
.Ltmp8:
	.cfi_def_cfa_offset 16
.Ltmp9:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp10:
	.cfi_def_cfa_register %rbp
	subq	$288, %rsp              # imm = 0x120
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
	callq	stub_hardware_write
	movabsq	$12235020, %rax         # imm = 0xBAB10C
	addq	$288, %rsp              # imm = 0x120
	popq	%rbp
	ret
.Ltmp11:
	.size	stub_hardware_write_wrapper, .Ltmp11-stub_hardware_write_wrapper
	.cfi_endproc

	.type	DEVICES,@object         # @DEVICES
	.comm	DEVICES,208,16

	.ident	"clang version 3.4.2 (tags/RELEASE_342/final 331981)"
	.section	".note.GNU-stack","",@progbits
