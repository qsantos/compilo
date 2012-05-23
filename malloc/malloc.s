	.text
	.align	2
	.globl	malloc
malloc:
	lw	$2,freep
	addiu	$4,$4,7
	srl	$8,$4,3
	beq	$2,$0,malloc_l2
	addiu	$8,$8,1

	lw	$5,0($2)
	#nop
	lw	$7,4($5)
	#nop
	sltu	$3,$7,$8
	beq	$3,$0,malloc_l3
	move	$4,$2

malloc_l4:
	sll	$12,$8,3
	move	$10,$2
	sltu	$13,$8,1024
malloc_l18:
	beq	$5,$4,malloc_l8
	move	$3,$5

	lw	$2,0($5)
	#nop
	lw	$7,4($2)
	move	$5,$2
malloc_l9:
	sltu	$2,$7,$8
	bne	$2,$0,malloc_l18
	sw	$10,freep
	move	$4,$3
malloc_l3:
	beq	$8,$7,malloc_l30
	subu	$7,$7,$8

	sll	$2,$7,3
	sw	$7,4($5)
	addu	$5,$5,$2
	sw	$8,4($5)
	sw	$4,freep
	j	$31
	addiu	$2,$5,8

malloc_l8:
	bne	$13,$0,malloc_l21
	li	$6,8192			# 0x2000

	move	$6,$12
	move	$11,$12
	move	$7,$8
malloc_l10:
	move    $4, $6
	lw      $v0, 9
	syscall
	move    $6, $2

	beq	$6,$0,malloc_l11
	move	$9,$6

	move	$3,$5
	sltu	$2,$3,$6
	bne	$2,$0,malloc_l31
	sw	$7,4($6)

malloc_l14:
	lw	$2,0($3)
	#nop
	sltu	$5,$3,$2
	bne	$5,$0,malloc_l28
	sltu	$4,$6,$2

	bne	$4,$0,malloc_l13
malloc_l28:
	move	$3,$2
	sltu	$2,$3,$6
	beq	$2,$0,malloc_l14
malloc_l31:
	lw	$2,0($3)
	#nop
	sltu	$4,$6,$2
	bne	$4,$0,malloc_l13
	sltu	$5,$3,$2

	bne	$5,$0,malloc_l28
malloc_l13:
	addu	$11,$6,$11
	beq	$11,$2,malloc_l32
	lw	$4,4($3)
	#nop
	sll	$5,$4,3
	addu	$5,$3,$5
	beq	$6,$5,malloc_l33
	sw	$2,0($6)

malloc_l17:
	sw	$9,0($3)
	move	$10,$3
	move	$4,$3
	b	malloc_l9
	move	$5,$9

malloc_l21:
	li	$11,8192			# 0x2000
	b	malloc_l10
	li	$7,1024			# 0x400

malloc_l30:
	lw	$2,0($5)
	sw	$4,freep
	sw	$2,0($4)
	j	$31
	addiu	$2,$5,8

malloc_l32:
	lw	$4,4($2)
	lw	$2,0($2)
	addu	$7,$7,$4
	sw	$7,4($6)
	lw	$4,4($3)
	#nop
	sll	$5,$4,3
	addu	$5,$3,$5
	bne	$6,$5,malloc_l17
	sw	$2,0($6)

malloc_l33:
	addu	$4,$7,$4
	sw	$4,4($3)
	lw	$7,4($2)
	b	malloc_l17
	move	$9,$2

malloc_l2:
	la	$2,base
	move	$5,$2
	sw	$2,base
	sw	$2,freep
	sw	$0,base+4
	b	malloc_l4
	move	$4,$2

malloc_l11:
	sw	$10,freep
	j	$31
	move	$2,$0
