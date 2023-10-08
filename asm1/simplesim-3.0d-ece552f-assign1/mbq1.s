	.file	1 "mbq1.c"

 # GNU C 2.7.2.3 [AL 1.1, MM 40, tma 0.1] SimpleScalar running sstrix compiled by GNU C

 # Cc1 defaults:
 # -mgas -mgpOPT

 # Cc1 arguments (-G value = 8, Cpu = default, ISA = 1):
 # -quiet -dumpbase -O2 -o

gcc2_compiled.:
__gnu_compiled_c:
	.sdata
	.align	2
$LC0:
	.ascii	"%d\000"
	.text
	.align	2
	.globl	main

	.extern	stdin, 4
	.extern	stdout, 4

	.text

	.loc	1 3
	.ent	main
main:
	.frame	$sp,88,$31		# vars= 32, regs= 10/0, args= 16, extra= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,88
	sw	$31,84($sp)
	sw	$fp,80($sp)
	sw	$23,76($sp)
	sw	$22,72($sp)
	sw	$21,68($sp)
	sw	$20,64($sp)
	sw	$19,60($sp)
	sw	$18,56($sp)
	sw	$17,52($sp)
	sw	$16,48($sp)
	jal	__main
	li	$19,0
	li	$18,2
	move	$20,$0
	move	$21,$0
	li	$22,0x0000a0b5		# 41141
	move	$23,$0
$L17:
	addu	$16,$18,$20
	sltu	$2,$16,$20
	addu	$17,$19,$21
	addu	$17,$17,$2
	addu	$18,$16,$20
	sltu	$2,$18,$20
	addu	$19,$17,$21
	addu	$19,$19,$2
	addu	$20,$16,$18
	sltu	$2,$20,$18
	addu	$21,$17,$19
	addu	$21,$21,$2
	sltu	$2,$18,$20
	subu	$4,$18,$20
	subu	$5,$19,$21
	subu	$5,$5,$2
	move	$6,$16
	move	$7,$17
	jal	__moddi3
	bgtz	$3,$L18
	bne	$3,$0,$L19
	sltu	$2,$2,10
	beq	$2,$0,$L18
$L19:
	addu	$22,$22,1
$L18:
	bgtz	$17,$L20
	bne	$17,$0,$L20
	li	$2,0x051c0000		# 85721088
$L20:
	bgtz	$19,$L23
	bne	$19,$0,$L22
	li	$2,0x01f20000		# 32636928
	ori	$2,$2,0x32a5
	sltu	$2,$2,$18
	beq	$2,$0,$L22
$L23:
	move	$6,$22
	sra	$7,$22,31
	move	$4,$18
	move	$5,$19
	jal	__moddi3
	move	$18,$2
	move	$19,$3
$L22:
	bgtz	$21,$L25
	bne	$21,$0,$L16
	li	$2,0x04580000		# 72876032
	ori	$2,$2,0x53d3
	sltu	$2,$2,$20
	beq	$2,$0,$L16
$L25:
	move	$6,$22
	sra	$7,$22,31
	move	$4,$20
	move	$5,$21
	jal	__moddi3
	move	$20,$2
	move	$21,$3
$L16:
	addu	$23,$23,1
	li	$2,0x00060000		# 393216
	ori	$2,$2,0x0f63
	slt	$2,$2,$23
	beq	$2,$0,$L17
	li	$19,-1
	li	$18,-5
	li	$21,0
	li	$20,1
	move	$23,$0
	li	$8,0x01e70000		# 31916032
	ori	$8,$8,0x48eb
	sw	$8,24($sp)
	li	$8,0x03690000		# 57212928
	ori	$8,$8,0x229d
	sw	$8,32($sp)
	li	$8,0x015d0000		# 22872064
	ori	$8,$8,0x87f3
	li	$fp,0x00090000		# 589824
	ori	$fp,$fp,0x7517
	sw	$8,40($sp)
$L30:
	addu	$16,$18,$20
	sltu	$2,$16,$20
	addu	$17,$19,$21
	addu	$17,$17,$2
	sltu	$2,$20,$16
	subu	$4,$20,$16
	subu	$5,$21,$17
	subu	$5,$5,$2
	move	$6,$18
	move	$7,$19
	jal	__moddi3
	bgtz	$3,$L31
	bne	$3,$0,$L32
	sltu	$2,$2,12
	beq	$2,$0,$L31
$L32:
	addu	$22,$22,1
$L31:
	addu	$18,$16,$20
	sltu	$2,$18,$20
	addu	$19,$17,$21
	addu	$19,$19,$2
	addu	$20,$16,$18
	sltu	$2,$20,$18
	addu	$21,$17,$19
	addu	$21,$21,$2
	bgtz	$17,$L33
	bne	$17,$0,$L33
	lw	$8,24($sp)
$L33:
	bgtz	$19,$L36
	bne	$19,$0,$L35
	.set	noreorder
	lw	$8,32($sp)
	#nop
	.set	reorder
	sltu	$2,$8,$18
	beq	$2,$0,$L35
$L36:
	move	$4,$18
	move	$5,$19
	li	$7,0
	li	$6,17
	jal	__moddi3
	move	$18,$2
	move	$19,$3
$L35:
	bgtz	$21,$L38
	bne	$21,$0,$L29
	.set	noreorder
	lw	$8,40($sp)
	#nop
	.set	reorder
	sltu	$2,$8,$20
	beq	$2,$0,$L29
$L38:
	move	$4,$20
	move	$5,$21
	li	$7,0
	li	$6,41
	jal	__moddi3
	move	$20,$2
	move	$21,$3
$L29:
	addu	$23,$23,1
	slt	$2,$fp,$23
	beq	$2,$0,$L30
	la	$4,$LC0
	move	$5,$22
	jal	printf
	move	$2,$0
	lw	$31,84($sp)
	lw	$fp,80($sp)
	lw	$23,76($sp)
	lw	$22,72($sp)
	lw	$21,68($sp)
	lw	$20,64($sp)
	lw	$19,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	addu	$sp,$sp,88
	j	$31
	.end	main
