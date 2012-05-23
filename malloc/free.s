	.align	2
	.globl	free
free:
	lw	$2,freep
	addiu	$5,$4,-8
	sltu	$3,$2,$5
	bne	$3,$0,malloc_l69
malloc_l59:
	lw	$3,0($2)
	#nop
	sltu	$7,$2,$3
	bne	$7,$0,malloc_l68
	sltu	$6,$5,$3

	bne	$6,$0,malloc_l58
malloc_l68:
	move	$2,$3
	sltu	$3,$2,$5
	beq	$3,$0,malloc_l59
malloc_l69:
	lw	$3,0($2)
	#nop
	sltu	$6,$5,$3
	bne	$6,$0,malloc_l58
	sltu	$7,$2,$3

	bne	$7,$0,malloc_l68
malloc_l58:
	lw	$6,-4($4)
	#nop
	sll	$7,$6,3
	addu	$7,$5,$7
	beq	$7,$3,malloc_l70
	lw	$6,4($2)
	#nop
	sll	$7,$6,3
	addu	$7,$2,$7
	beq	$5,$7,malloc_l71
	sw	$3,-8($4)

malloc_l62:
	sw	$5,0($2)
	sw	$2,freep
	j	$31
malloc_l70:
	lw	$7,4($3)
	lw	$3,0($3)
	addu	$6,$7,$6
	sw	$6,-4($4)
	lw	$6,4($2)
	#nop
	sll	$7,$6,3
	addu	$7,$2,$7
	bne	$5,$7,malloc_l62
	sw	$3,-8($4)

malloc_l71:
	lw	$4,-4($4)
	move	$5,$3
	addu	$6,$4,$6
	sw	$6,4($2)
	sw	$5,0($2)
	sw	$2,freep
	j	$31
