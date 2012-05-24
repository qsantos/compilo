	.file	1 "malloc.c"
	.text
	.align	2
	.globl	malloc
	.set	nomips16
	.ent	malloc
malloc:
	.frame	$fp,48,$31		# vars= 16, regs= 2/0, args= 16, gp= 8
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	reorder
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$fp,40($sp)
	move	$fp,$sp
	sw	$4,48($fp)
#APP
 # 21 "malloc.c" 1
	# nunits = ...
 # 0 "" 2
#NO_APP
	lw	$2,48($fp)
	#nop
	addiu	$2,$2,7
	srl	$2,$2,3
	addiu	$2,$2,1
	sw	$2,32($fp)
#APP
 # 24 "malloc.c" 1
	# prevp = freep
 # 0 "" 2
#NO_APP
	lw	$2,freep
	#nop
	sw	$2,24($fp)
#APP
 # 26 "malloc.c" 1
	# if (!prevp)
 # 0 "" 2
#NO_APP
	lw	$2,24($fp)
	#nop
	bne	$2,$0,malloc_l2
#APP
 # 29 "malloc.c" 1
	# if inside
 # 0 "" 2
#NO_APP
	la	$2,base
	sw	$2,base
	la	$2,base
	sw	$2,freep
	la	$2,base
	sw	$2,24($fp)
	sw	$0,base+4
malloc_l2:
#APP
 # 36 "malloc.c" 1
	# p = prevp->ptr
 # 0 "" 2
#NO_APP
	lw	$2,24($fp)
	#nop
	lw	$2,0($2)
	#nop
	sw	$2,28($fp)
#APP
 # 38 "malloc.c" 1
	# while (42)
 # 0 "" 2
#NO_APP
malloc_l8:
#APP
 # 41 "malloc.c" 1
	# if (p->size >= nunits)
 # 0 "" 2
#NO_APP
	lw	$2,28($fp)
	#nop
	lw	$3,4($2)
	lw	$2,32($fp)
	#nop
	sltu	$2,$3,$2
	bne	$2,$0,malloc_l3
#APP
 # 44 "malloc.c" 1
	# if (p->size == nunits)
 # 0 "" 2
#NO_APP
	lw	$2,28($fp)
	#nop
	lw	$3,4($2)
	lw	$2,32($fp)
	#nop
	bne	$3,$2,malloc_l4
#APP
 # 47 "malloc.c" 1
	# prevp->ptr = p->ptr
 # 0 "" 2
#NO_APP
	lw	$2,28($fp)
	#nop
	lw	$3,0($2)
	lw	$2,24($fp)
	#nop
	sw	$3,0($2)
	b	malloc_l5
malloc_l4:
#APP
 # 52 "malloc.c" 1
	# p->size -= nunits, ...
 # 0 "" 2
#NO_APP
	lw	$2,28($fp)
	#nop
	lw	$3,4($2)
	lw	$2,32($fp)
	#nop
	subu	$3,$3,$2
	lw	$2,28($fp)
	#nop
	sw	$3,4($2)
	lw	$2,28($fp)
	#nop
	lw	$2,4($2)
	#nop
	sll	$2,$2,3
	lw	$3,28($fp)
	#nop
	addu	$2,$3,$2
	sw	$2,28($fp)
	lw	$2,28($fp)
	lw	$3,32($fp)
	#nop
	sw	$3,4($2)
malloc_l5:
#APP
 # 58 "malloc.c" 1
	# freep = prevp & return p+1
 # 0 "" 2
#NO_APP
	lw	$2,24($fp)
	#nop
	sw	$2,freep
	lw	$2,28($fp)
	#nop
	addiu	$2,$2,8
	b	malloc_l6
malloc_l3:
#APP
 # 63 "malloc.c" 1
	# if (p == freep)
 # 0 "" 2
#NO_APP
	lw	$2,freep
	lw	$3,28($fp)
	#nop
	bne	$3,$2,malloc_l7
#APP
 # 66 "malloc.c" 1
	# morecore call
 # 0 "" 2
#NO_APP
	lw	$4,32($fp)
	jal	morecore
	sw	$2,28($fp)
#APP
 # 68 "malloc.c" 1
	# if (!p) return 0
 # 0 "" 2
#NO_APP
	lw	$2,28($fp)
	#nop
	bne	$2,$0,malloc_l7
	move	$2,$0
	b	malloc_l6
malloc_l7:
#APP
 # 73 "malloc.c" 1
	# prevp = p, p = p->ptr
 # 0 "" 2
#NO_APP
	lw	$2,28($fp)
	#nop
	sw	$2,24($fp)
	lw	$2,28($fp)
	#nop
	lw	$2,0($2)
	#nop
	sw	$2,28($fp)
	b	malloc_l8
malloc_l6:
	move	$sp,$fp
	lw	$31,44($sp)
	lw	$fp,40($sp)
	addiu	$sp,$sp,48
	j	$31
	.end	malloc
	.align	2
	.globl	morecore
	.set	nomips16
	.ent	morecore
morecore:
	.frame	$fp,40,$31		# vars= 0, regs= 3/0, args= 16, gp= 8
	.mask	0xc0010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	reorder
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$fp,32($sp)
	sw	$16,28($sp)
	move	$fp,$sp
	sw	$4,40($fp)
#APP
 # 81 "malloc.c" 1
	# if (nu < NALLOC)
 # 0 "" 2
#NO_APP
	lw	$2,40($fp)
	#nop
	sltu	$2,$2,1024
	beq	$2,$0,malloc_l10
#APP
 # 84 "malloc.c" 1
	# nu = NALLOC
 # 0 "" 2
#NO_APP
	li	$2,1024			# 0x400
	sw	$2,40($fp)
malloc_l10:
#APP
 # 89 "malloc.c" 1
	# n = nu * 8
 # 0 "" 2
#NO_APP
	lw	$2,40($fp)
	#nop
	sll	$2,$2,3
	move	$16,$2
#APP
 # 91 "malloc.c" 1
	move    $4, $16
	li      $v0, 9
	syscall
	move    $16, $2

 # 0 "" 2
 # 99 "malloc.c" 1
	# if (!up) return 0
 # 0 "" 2
#NO_APP
	bne	$16,$0,malloc_l11
	move	$2,$0
	b	malloc_l12
malloc_l11:
#APP
 # 103 "malloc.c" 1
	# up->size = nu
 # 0 "" 2
#NO_APP
	lw	$2,40($fp)
	#nop
	sw	$2,4($16)
#APP
 # 105 "malloc.c" 1
	# free(up+1)
 # 0 "" 2
#NO_APP
	addiu	$2,$16,8
	move	$4,$2
	jal	free
#APP
 # 107 "malloc.c" 1
	# return freep
 # 0 "" 2
#NO_APP
	lw	$2,freep
malloc_l12:
	move	$sp,$fp
	lw	$31,36($sp)
	lw	$fp,32($sp)
	lw	$16,28($sp)
	addiu	$sp,$sp,40
	j	$31
	.end	morecore
