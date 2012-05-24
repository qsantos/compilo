	.globl	free
	.set	nomips16
	.ent	free
free:
	.frame	$fp,24,$31		# vars= 8, regs= 1/0, args= 0, gp= 8
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	reorder
	addiu	$sp,$sp,-24
	sw	$fp,20($sp)
	move	$fp,$sp
	sw	$4,24($fp)
	lw	$2,24($fp)
	#nop
	addiu	$2,$2,-8
	sw	$2,12($fp)
	lw	$2,freep
	#nop
	sw	$2,8($fp)
	b	free_l14
free_l17:
	lw	$2,8($fp)
	#nop
	lw	$3,0($2)
	lw	$2,8($fp)
	#nop
	sltu	$2,$2,$3
	bne	$2,$0,free_l15
	lw	$3,12($fp)
	lw	$2,8($fp)
	#nop
	sltu	$2,$2,$3
	bne	$2,$0,free_l16
	lw	$2,8($fp)
	#nop
	lw	$3,0($2)
	lw	$2,12($fp)
	#nop
	sltu	$2,$2,$3
	bne	$2,$0,free_l16
free_l15:
	lw	$2,8($fp)
	#nop
	lw	$2,0($2)
	#nop
	sw	$2,8($fp)
free_l14:
	lw	$3,12($fp)
	lw	$2,8($fp)
	#nop
	sltu	$2,$2,$3
	beq	$2,$0,free_l17
	lw	$2,8($fp)
	#nop
	lw	$3,0($2)
	lw	$2,12($fp)
	#nop
	sltu	$2,$2,$3
	beq	$2,$0,free_l17
free_l16:
	lw	$2,12($fp)
	#nop
	lw	$2,4($2)
	#nop
	sll	$2,$2,3
	lw	$3,12($fp)
	#nop
	addu	$3,$3,$2
	lw	$2,8($fp)
	#nop
	lw	$2,0($2)
	#nop
	bne	$3,$2,free_l18
	lw	$2,12($fp)
	#nop
	lw	$3,4($2)
	lw	$2,8($fp)
	#nop
	lw	$2,0($2)
	#nop
	lw	$2,4($2)
	#nop
	addu	$3,$3,$2
	lw	$2,12($fp)
	#nop
	sw	$3,4($2)
	lw	$2,8($fp)
	#nop
	lw	$2,0($2)
	#nop
	lw	$3,0($2)
	lw	$2,12($fp)
	#nop
	sw	$3,0($2)
	b	free_l19
free_l18:
	lw	$2,8($fp)
	#nop
	lw	$3,0($2)
	lw	$2,12($fp)
	#nop
	sw	$3,0($2)
free_l19:
	lw	$2,8($fp)
	#nop
	lw	$2,4($2)
	#nop
	sll	$2,$2,3
	lw	$3,8($fp)
	#nop
	addu	$3,$3,$2
	lw	$2,12($fp)
	#nop
	bne	$3,$2,free_l20
	lw	$2,8($fp)
	#nop
	lw	$3,4($2)
	lw	$2,12($fp)
	#nop
	lw	$2,4($2)
	#nop
	addu	$3,$3,$2
	lw	$2,8($fp)
	#nop
	sw	$3,4($2)
	lw	$2,12($fp)
	#nop
	lw	$3,0($2)
	lw	$2,8($fp)
	#nop
	sw	$3,0($2)
	b	free_l21
free_l20:
	lw	$2,8($fp)
	lw	$3,12($fp)
	#nop
	sw	$3,0($2)
free_l21:
	lw	$2,8($fp)
	#nop
	sw	$2,freep
	move	$sp,$fp
	lw	$fp,20($sp)
	addiu	$sp,$sp,24
	j	$31
	.end	free
