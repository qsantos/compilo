	.data
base:
	.word 0,0
freep:
	.word 0

	.text
	.globl free
	.align	2
	.globl	morecore
morecore:
	sltu	$2,$4,1024
	bne	$2,$0,malloc_l45
	move	$8,$4

	sll	$9,$4,3
	move	$5,$9
malloc_l35:
	move    $4, $5
	lw      $v0, 9
	syscall
	move    $5, $2
	beq	$5,$0,malloc_l46
	move	$4,$5

	lw	$2,freep
	#nop
	sltu	$3,$2,$5
	bne	$3,$0,malloc_l53
	sw	$8,4($5)

malloc_l39:
	lw	$3,0($2)
	#nop
	sltu	$7,$2,$3
	bne	$7,$0,malloc_l52
	sltu	$6,$5,$3

	bne	$6,$0,malloc_l38
malloc_l52:
	move	$2,$3
	sltu	$3,$2,$5
	beq	$3,$0,malloc_l39
malloc_l53:
	lw	$3,0($2)
	#nop
	sltu	$6,$5,$3
	bne	$6,$0,malloc_l38
	sltu	$7,$2,$3

	bne	$7,$0,malloc_l52
malloc_l38:
	addu	$9,$5,$9
	beq	$9,$3,malloc_l54
malloc_l41:
	lw	$6,4($2)
	#nop
	sll	$7,$6,3
	addu	$7,$2,$7
	beq	$5,$7,malloc_l55
	sw	$3,0($5)

malloc_l42:
	sw	$4,0($2)
	sw	$2,freep
	j	$31
malloc_l45:
	li	$5,8192			# 0x2000
	li	$9,8192			# 0x2000
	b	malloc_l35
	li	$8,1024			# 0x400

malloc_l54:
	lw	$6,4($3)
	lw	$3,0($3)
	addu	$8,$8,$6
	b	malloc_l41
	sw	$8,4($5)

malloc_l55:
	lw	$5,4($5)
	move	$4,$3
	addu	$6,$5,$6
	b	malloc_l42
	sw	$6,4($2)

malloc_l46:
	j	$31
	move	$2,$0
