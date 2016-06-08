.fpu	softvfp
.thumb
.syntax unified
.section .text

	.equ	SCB_VTOR, 			0xE000ED08
	.equ	ICSR, 				0xE000ED04
	.equ	PENDSVSET_BIT, 		0x10000000
	.equ	SHPR3, 				0xE000ED20
	.equ	PENDSV_PRI_LOWEST, 	0x00FF0000


.global _fm_interrupt_enable_
.type _fm_interrupt_enable_, %function
_fm_interrupt_enable_:
	MSR 	PRIMASK, r0
	BX 		lr

.global _fm_interrupt_disable_
.type _fm_interrupt_disable_, %function
_fm_interrupt_disable_:
	MRS 	r0, PRIMASK
	CPSID 	I
	BX 		lr

.global _fm_thread_context_switch_
.type _fm_thread_context_switch_, %function
_fm_thread_context_switch_:
/*
** r0 from
** r1 to
*/
	LDR 	r2, =fm_thread_interrupt_flag
	LDR		r3, [r2]
	CMP		r3, #1       //中断嵌套中多次调用后，防止保护现场的线程被改变以及重复的操作
	BEQ		_reswitch
	MOV		r3, #1
	STR		r3, [r2]

	LDR		r2, =fm_thread_from_address
	STR		r0, [r2]
_reswitch:
	LDR		r2, =fm_thread_to_address
	STR		r1, [r2]

	LDR		r0, =ICSR
	LDR		r1, =PENDSVSET_BIT
	STR		r1, [r0]
	BX		lr
/* old version not support interrupt
	STMFD 	sp!, {r2-r12, lr}
	MRS 	r4, psr
	STMFD 	sp!, {r4}
	STR 	sp, [r0]
	LDR 	r1, [r1]
	CMP 	r1, #0
	BNE 	_get_into_new_thread
	CPSIE 	I
	BL 		routine
	B 		_finish
_get_into_new_thread:
	MOV 	sp, r1
	LDMFD 	sp!, {r4}
	MSR 	psr, r4
	LDMFD 	sp!, {r2-r12, lr}
_finish:
	MOV 	pc, lr
*/

.global PendSV_Handler
.type PendSV_Handler, %function
PendSV_Handler:
	MRS		r2, PRIMASK  //保存中断信息
	CPSID	I			//禁用中断
	LDR		r0, =fm_thread_interrupt_flag
	LDR		r1, [r0]
	CBZ		r1, PendSV_exit

	MOV		r1, #0
	STR		r1, [r0]

	LDR		r0, =fm_thread_from_address
	LDR		r1, [r0]
	CBZ		r1, switch_to_new_thread

	MRS		r1, psp
	STMFD	r1!, {r4-r11}
	LDR		r0, [r0]
	STR		r1,	[r0]
switch_to_new_thread:
	LDR		r1, =fm_thread_to_address
	LDR		r1, [r1]    //得到堆栈指针
	LDR		r1, [r1]
	LDMFD	r1!, {r4-r11}
	MSR		psp, r1
PendSV_exit:
	MSR		PRIMASK, r2
	ORR		lr, lr, #0x04
	BX		lr

.global _fm_thread_start_
.type _fm_thread_start_, %function
/*
* r0 开始的线程
*/
_fm_thread_start_:
	LDR		r1, =fm_thread_to_address
	STR		r0, [r1]

	LDR		r1, =fm_thread_from_address
	MOV		r0, #0
	STR		r0, [r1]

	LDR		r1, =fm_thread_interrupt_flag
	MOV		r0, #1
	STR		r0, [r1]


//_set_PandSV_Handler_:
	LDR		r0, =SHPR3
	LDR		r1, =PENDSV_PRI_LOWEST
	LDR.W	r2, [r0, #0]
	ORR 	r1, r1, r2
	STR		r1, [r0]

	LDR		r0, =ICSR
	LDR		r1, =PENDSVSET_BIT
	STR		r1, [r0]

	LDR 	r0, =SCB_VTOR
	LDR		r0, [r0]
	LDR		r0, [r0]
	NOP
	MSR		msp, r0
	CPSIE	I

.global _update_psp_
.type _update_psp_, %function
_update_psp_:
	LDR		r0, =psp
	MRS		r1, psp
	STR		r1, [r0]
	BX		lr
