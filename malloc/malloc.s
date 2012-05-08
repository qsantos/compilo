	.file	1 "malloc.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 1
	.abicalls
	.local	base
	.comm	base,8,4
	.local	freep
	.comm	freep,4,4
	.text
	.align	2
	.globl	malloc
	.set	nomips16
	.ent	malloc
	.type	malloc, @function
malloc:
	.frame	$fp,48,$31		# vars= 16, regs= 2/0, args= 16, gp= 8
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$fp,40($sp)
	move	$fp,$sp
	sw	$4,48($fp)
	lw	$2,48($fp)
	nop
	addiu	$2,$2,7
	srl	$2,$2,3
	addiu	$2,$2,1
	sw	$2,32($fp)
	lui	$2,%hi(freep)
	lw	$2,%lo(freep)($2)
	nop
	sw	$2,24($fp)
	lw	$2,24($fp)
	nop
	bne	$2,$0,$L2
	nop

	lui	$2,%hi(base)
	lui	$3,%hi(base)
	addiu	$3,$3,%lo(base)
	sw	$3,%lo(base)($2)
	lui	$2,%hi(freep)
	lui	$3,%hi(base)
	addiu	$3,$3,%lo(base)
	sw	$3,%lo(freep)($2)
	lui	$2,%hi(base)
	addiu	$2,$2,%lo(base)
	sw	$2,24($fp)
	lui	$2,%hi(base)
	addiu	$2,$2,%lo(base)
	sw	$0,4($2)
$L2:
	lw	$2,24($fp)
	nop
	lw	$2,0($2)
	nop
	sw	$2,28($fp)
$L8:
	lw	$2,28($fp)
	nop
	lw	$3,4($2)
	lw	$2,32($fp)
	nop
	sltu	$2,$3,$2
	bne	$2,$0,$L3
	nop

	lw	$2,28($fp)
	nop
	lw	$3,4($2)
	lw	$2,32($fp)
	nop
	bne	$3,$2,$L4
	nop

	lw	$2,28($fp)
	nop
	lw	$3,0($2)
	lw	$2,24($fp)
	nop
	sw	$3,0($2)
	.option	pic0
	j	$L5
	nop

	.option	pic2
$L4:
	lw	$2,28($fp)
	nop
	lw	$3,4($2)
	lw	$2,32($fp)
	nop
	subu	$3,$3,$2
	lw	$2,28($fp)
	nop
	sw	$3,4($2)
	lw	$2,28($fp)
	nop
	lw	$2,4($2)
	nop
	sll	$2,$2,3
	lw	$3,28($fp)
	nop
	addu	$2,$3,$2
	sw	$2,28($fp)
	lw	$2,28($fp)
	lw	$3,32($fp)
	nop
	sw	$3,4($2)
$L5:
	lui	$2,%hi(freep)
	lw	$3,24($fp)
	nop
	sw	$3,%lo(freep)($2)
	lw	$2,28($fp)
	nop
	addiu	$2,$2,8
	.option	pic0
	j	$L6
	nop

	.option	pic2
$L3:
	lui	$2,%hi(freep)
	lw	$2,%lo(freep)($2)
	lw	$3,28($fp)
	nop
	bne	$3,$2,$L7
	nop

	lw	$4,32($fp)
	.option	pic0
	jal	morecore
	nop

	.option	pic2
	sw	$2,28($fp)
	lw	$2,28($fp)
	nop
	bne	$2,$0,$L7
	nop

	move	$2,$0
	.option	pic0
	j	$L6
	nop

	.option	pic2
$L7:
	lw	$2,28($fp)
	nop
	sw	$2,24($fp)
	lw	$2,28($fp)
	nop
	lw	$2,0($2)
	nop
	sw	$2,28($fp)
	.option	pic0
	j	$L8
	nop

	.option	pic2
$L6:
	move	$sp,$fp
	lw	$31,44($sp)
	lw	$fp,40($sp)
	addiu	$sp,$sp,48
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	malloc
	.size	malloc, .-malloc
	.align	2
	.globl	morecore
	.set	nomips16
	.ent	morecore
	.type	morecore, @function
morecore:
	.frame	$fp,40,$31		# vars= 0, regs= 3/0, args= 16, gp= 8
	.mask	0xc0010000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$fp,32($sp)
	sw	$16,28($sp)
	move	$fp,$sp
	sw	$4,40($fp)
	lw	$2,40($fp)
	#nop
	sltu	$2,$2,1024
	beq	$2,$0,$L10
	li	$2,1024			# 0x400
	sw	$2,40($fp)
$L10:
	lw	$2,40($fp)
	#nop
	sll	$2,$2,3
	move	$16,$2
#APP
 # 69 "malloc.c" 1
		move    $4, $16
	syscall 9
	move    $16, $2

 # 0 "" 2
#NO_APP
	bne	$16,$0,$L11
	move	$2,$0
	.option	pic0
	j	$L12
	.option	pic2
$L11:
	lw	$2,40($fp)
	#nop
	sw	$2,4($16)
	addiu	$2,$16,8
	move	$4,$2
	.option	pic0
	jal	free
	.option	pic2
	lui	$2,%hi(freep)
	lw	$2,%lo(freep)($2)
$L12:
	move	$sp,$fp
	lw	$31,36($sp)
	lw	$fp,32($sp)
	lw	$16,28($sp)
	addiu	$sp,$sp,40
	j	$31
	.end	morecore
	.size	morecore, .-morecore
	.align	2
	.globl	free
	.set	nomips16
	.ent	free
	.type	free, @function
free:
	.frame	$fp,24,$31		# vars= 8, regs= 1/0, args= 0, gp= 8
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$fp,20($sp)
	move	$fp,$sp
	sw	$4,24($fp)
	lw	$2,24($fp)
	nop
	addiu	$2,$2,-8
	sw	$2,12($fp)
	lui	$2,%hi(freep)
	lw	$2,%lo(freep)($2)
	nop
	sw	$2,8($fp)
	.option	pic0
	j	$L14
	nop

	.option	pic2
$L17:
	lw	$2,8($fp)
	nop
	lw	$3,0($2)
	lw	$2,8($fp)
	nop
	sltu	$2,$2,$3
	bne	$2,$0,$L15
	nop

	lw	$3,12($fp)
	lw	$2,8($fp)
	nop
	sltu	$2,$2,$3
	bne	$2,$0,$L16
	nop

	lw	$2,8($fp)
	nop
	lw	$3,0($2)
	lw	$2,12($fp)
	nop
	sltu	$2,$2,$3
	bne	$2,$0,$L16
	nop

$L15:
	lw	$2,8($fp)
	nop
	lw	$2,0($2)
	nop
	sw	$2,8($fp)
$L14:
	lw	$3,12($fp)
	lw	$2,8($fp)
	nop
	sltu	$2,$2,$3
	beq	$2,$0,$L17
	nop

	lw	$2,8($fp)
	nop
	lw	$3,0($2)
	lw	$2,12($fp)
	nop
	sltu	$2,$2,$3
	beq	$2,$0,$L17
	nop

$L16:
	lw	$2,12($fp)
	nop
	lw	$2,4($2)
	nop
	sll	$2,$2,3
	lw	$3,12($fp)
	nop
	addu	$3,$3,$2
	lw	$2,8($fp)
	nop
	lw	$2,0($2)
	nop
	bne	$3,$2,$L18
	nop

	lw	$2,12($fp)
	nop
	lw	$3,4($2)
	lw	$2,8($fp)
	nop
	lw	$2,0($2)
	nop
	lw	$2,4($2)
	nop
	addu	$3,$3,$2
	lw	$2,12($fp)
	nop
	sw	$3,4($2)
	lw	$2,8($fp)
	nop
	lw	$2,0($2)
	nop
	lw	$3,0($2)
	lw	$2,12($fp)
	nop
	sw	$3,0($2)
	.option	pic0
	j	$L19
	nop

	.option	pic2
$L18:
	lw	$2,8($fp)
	nop
	lw	$3,0($2)
	lw	$2,12($fp)
	nop
	sw	$3,0($2)
$L19:
	lw	$2,8($fp)
	nop
	lw	$2,4($2)
	nop
	sll	$2,$2,3
	lw	$3,8($fp)
	nop
	addu	$3,$3,$2
	lw	$2,12($fp)
	nop
	bne	$3,$2,$L20
	nop

	lw	$2,8($fp)
	nop
	lw	$3,4($2)
	lw	$2,12($fp)
	nop
	lw	$2,4($2)
	nop
	addu	$3,$3,$2
	lw	$2,8($fp)
	nop
	sw	$3,4($2)
	lw	$2,12($fp)
	nop
	lw	$3,0($2)
	lw	$2,8($fp)
	nop
	sw	$3,0($2)
	.option	pic0
	j	$L21
	nop

	.option	pic2
$L20:
	lw	$2,8($fp)
	lw	$3,12($fp)
	nop
	sw	$3,0($2)
$L21:
	lui	$2,%hi(freep)
	lw	$3,8($fp)
	nop
	sw	$3,%lo(freep)($2)
	move	$sp,$fp
	lw	$fp,20($sp)
	addiu	$sp,$sp,24
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	free
	.size	free, .-free
	.ident	"GCC: (GNU) 4.7.0"
