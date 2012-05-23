	.data
base:
	.word 0,0
freep:
	.word 0

	.text
	.globl free
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
	sll	$2,$2,3
	move	$16,$2

	move    $4, $16
	li      $v0, 9
	syscall
	move    $16, $2

	bne	$16,$0,malloc_l11
	move	$2,$0
	b	malloc_l12
malloc_l11:
	lw	$2,40($fp)
	sw	$2,4($16)
	addiu	$2,$16,8
	move	$4,$2
	jal	free
	lw	$2,freep
malloc_l12:
	move	$sp,$fp
	lw	$31,36($sp)
	lw	$fp,32($sp)
	lw	$16,28($sp)
	addiu	$sp,$sp,40
	j	$31
