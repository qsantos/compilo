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
