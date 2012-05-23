	.align	2
	.globl	free
free:
	lw	$2,freep
	addiu	$5,$4,-8
	sltu	$3,$2,$5
	bne	$3,$0,mem_l47
mem_l37:
	lw	$3,0($2)
	#nop
	sltu	$7,$2,$3
	bne	$7,$0,mem_l46
	sltu	$6,$5,$3

	bne	$6,$0,mem_l36
mem_l46:
	move	$2,$3
	sltu	$3,$2,$5
	beq	$3,$0,mem_l37
mem_l47:
	lw	$3,0($2)
	#nop
	sltu	$6,$5,$3
	bne	$6,$0,mem_l36
	sltu	$7,$2,$3

	bne	$7,$0,mem_l46
mem_l36:
	lw	$6,-4($4)
	#nop
	sll	$7,$6,3
	addu	$7,$5,$7
	beq	$7,$3,mem_l48
	lw	$6,4($2)
	#nop
	sll	$7,$6,3
	addu	$7,$2,$7
	beq	$5,$7,mem_l49
	sw	$3,-8($4)

mem_l40:
	sw	$5,0($2)
	sw	$2,freep
	j	$31
mem_l48:
	lw	$7,4($3)
	lw	$3,0($3)
	addu	$6,$7,$6
	sw	$6,-4($4)
	lw	$6,4($2)
	#nop
	sll	$7,$6,3
	addu	$7,$2,$7
	bne	$5,$7,mem_l40
	sw	$3,-8($4)

mem_l49:
	lw	$4,-4($4)
	move	$5,$3
	addu	$6,$4,$6
	sw	$6,4($2)
	sw	$5,0($2)
	sw	$2,freep
	j	$31
