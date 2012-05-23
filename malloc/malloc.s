	.data
base:
	.word 0,0
freep:
	.word 0
	
	.text
	.globl	malloc
malloc:
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$fp,40($sp)
	move	$fp,$sp
	sw	$4,48($fp)
	lw	$2,48($fp)
	addiu	$2,$2,7
	srl	$2,$2,3
	addiu	$2,$2,1
	sw	$2,32($fp)
	lui	$2,%hi(freep)
	lw	$2,%lo(freep)($2)
	sw	$2,24($fp)
	lw	$2,24($fp)
	bne	$2,$0,malloc_l2

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
malloc_l2:
	lw	$2,24($fp)
	lw	$2,0($2)
	sw	$2,28($fp)
malloc_l8:
	lw	$2,28($fp)
	lw	$3,4($2)
	lw	$2,32($fp)
	sltu	$2,$3,$2
	bne	$2,$0,malloc_l3

	lw	$2,28($fp)
	lw	$3,4($2)
	lw	$2,32($fp)
	bne	$3,$2,malloc_l4

	lw	$2,28($fp)
	lw	$3,0($2)
	lw	$2,24($fp)
	sw	$3,0($2)
	j	malloc_l5

malloc_l4:
	lw	$2,28($fp)
	lw	$3,4($2)
	lw	$2,32($fp)
	subu	$3,$3,$2
	lw	$2,28($fp)
	sw	$3,4($2)
	lw	$2,28($fp)
	lw	$2,4($2)
	sll	$2,$2,3
	lw	$3,28($fp)
	addu	$2,$3,$2
	sw	$2,28($fp)
	lw	$2,28($fp)
	lw	$3,32($fp)
	sw	$3,4($2)
malloc_l5:
	lui	$2,%hi(freep)
	lw	$3,24($fp)
	sw	$3,%lo(freep)($2)
	lw	$2,28($fp)
	addiu	$2,$2,8
	j	malloc_l6

malloc_l3:
	lui	$2,%hi(freep)
	lw	$2,%lo(freep)($2)
	lw	$3,28($fp)
	bne	$3,$2,malloc_l7

	lw	$4,32($fp)
	jal	morecore

	sw	$2,28($fp)
	lw	$2,28($fp)
	bne	$2,$0,malloc_l7

	move	$2,$0
	j	malloc_l6

malloc_l7:
	lw	$2,28($fp)
	sw	$2,24($fp)
	lw	$2,28($fp)
	lw	$2,0($2)
	sw	$2,28($fp)
	j	malloc_l8

malloc_l6:
	move	$sp,$fp
	lw	$31,44($sp)
	lw	$fp,40($sp)
	addiu	$sp,$sp,48
	j	$31

	.globl	morecore
morecore:
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$fp,32($sp)
	sw	$16,28($sp)
	move	$fp,$sp
	sw	$4,40($fp)
	lw	$2,40($fp)

	sltu	$2,$2,1024
	beq	$2,$0,malloc_l10
	li	$2,1024			# 0x400
	sw	$2,40($fp)
malloc_l10:
	lw	$2,40($fp)
	#nop
	sll	$2,$2,3
	move	$16,$2

# syscall: brk
	move    $4, $16
	syscall 9
	move    $16, $2
	
	bne	$16,$0,malloc_l11
	move	$2,$0
	j	malloc_l12
malloc_l11:
	lw	$2,40($fp)
	sw	$2,4($16)
	addiu	$2,$16,8
	move	$4,$2
	jal	free
	lui	$2,%hi(freep)
	lw	$2,%lo(freep)($2)
malloc_l12:
	move	$sp,$fp
	lw	$31,36($sp)
	lw	$fp,32($sp)
	lw	$16,28($sp)
	addiu	$sp,$sp,40
	j	$31

	.globl	free
free:
	addiu	$sp,$sp,-24
	sw	$fp,20($sp)
	move	$fp,$sp
	sw	$4,24($fp)
	lw	$2,24($fp)
	addiu	$2,$2,-8
	sw	$2,12($fp)
	lui	$2,%hi(freep)
	lw	$2,%lo(freep)($2)
	sw	$2,8($fp)
	j	malloc_l14

malloc_l17:
	lw	$2,8($fp)
	lw	$3,0($2)
	lw	$2,8($fp)
	sltu	$2,$2,$3
	bne	$2,$0,malloc_l15

	lw	$3,12($fp)
	lw	$2,8($fp)
	sltu	$2,$2,$3
	bne	$2,$0,malloc_l16

	lw	$2,8($fp)
	lw	$3,0($2)
	lw	$2,12($fp)
	sltu	$2,$2,$3
	bne	$2,$0,malloc_l16

malloc_l15:
	lw	$2,8($fp)
	lw	$2,0($2)
	sw	$2,8($fp)
malloc_l14:
	lw	$3,12($fp)
	lw	$2,8($fp)
	sltu	$2,$2,$3
	beq	$2,$0,malloc_l17

	lw	$2,8($fp)
	lw	$3,0($2)
	lw	$2,12($fp)
	sltu	$2,$2,$3
	beq	$2,$0,malloc_l17

malloc_l16:
	lw	$2,12($fp)
	lw	$2,4($2)
	sll	$2,$2,3
	lw	$3,12($fp)
	addu	$3,$3,$2
	lw	$2,8($fp)
	lw	$2,0($2)
	bne	$3,$2,malloc_l18

	lw	$2,12($fp)
	lw	$3,4($2)
	lw	$2,8($fp)
	lw	$2,0($2)
	lw	$2,4($2)
	addu	$3,$3,$2
	lw	$2,12($fp)
	sw	$3,4($2)
	lw	$2,8($fp)
	lw	$2,0($2)
	lw	$3,0($2)
	lw	$2,12($fp)
	sw	$3,0($2)
	j	malloc_l19

malloc_l18:
	lw	$2,8($fp)
	lw	$3,0($2)
	lw	$2,12($fp)
	sw	$3,0($2)
malloc_l19:
	lw	$2,8($fp)
	lw	$2,4($2)
	sll	$2,$2,3
	lw	$3,8($fp)
	addu	$3,$3,$2
	lw	$2,12($fp)
	bne	$3,$2,malloc_l20

	lw	$2,8($fp)
	lw	$3,4($2)
	lw	$2,12($fp)
	lw	$2,4($2)
	addu	$3,$3,$2
	lw	$2,8($fp)
	sw	$3,4($2)
	lw	$2,12($fp)
	lw	$3,0($2)
	lw	$2,8($fp)
	sw	$3,0($2)
	j	malloc_l21

malloc_l20:
	lw	$2,8($fp)
	lw	$3,12($fp)
	sw	$3,0($2)
malloc_l21:
	lui	$2,%hi(freep)
	lw	$3,8($fp)
	sw	$3,%lo(freep)($2)
	move	$sp,$fp
	lw	$fp,20($sp)
	addiu	$sp,$sp,24
	j	$31
