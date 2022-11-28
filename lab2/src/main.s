.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.data
.balign 4
// Your global variables go here
.global arr
arr: .word 11, 22, 24, 14, 20, 21, 15, 19, 8, 22, 15, 8, 8, 10, 9, 23
.balign 4
.global str
str: .asciz "Frperg 01234 Zrffntr 56789! Rapbqrq Urer..."
.balign 4
.global value
value: .word 0


.text
.global intsub
intsub:
    // Your code for intsub goes here
    PUSH {R4-R7, LR}
	movs r0, #0 //i
	ldr r1, =arr //arr
	movs r2, #0 //value
for:
	movs r3, #4
	ands r3, r2
	cmp r3, #4
	bne else
if:
	ldrb r4, [r1, r0] //r4 = arr[i]
	adds r0, #4 //i = i + 1
	ldrb r5, [r1, r0] //r5 = arr[i + 1]
	subs r0, #4 //i = i - 1 (return back to regular i)
	adds r2, r4 //value += arr[i]
	adds r2, r5 //value += arr[i + 1]
	b endif
else:
	ldrb r4, [r1, r0] //r4 = arr[i]
	adds r0, #4 //i = i + 1
	ldrb r5, [r1, r0] //r5 = arr[i + 1]
	subs r0, #4 //i = i - 1 (return back to regular i)
	lsls r3, r4, #2 //r3 = r4 << 2 = arr[i] << 2
	adds r2, r3 //value = value + r3
	b endif
endif:
	subs r3, r4, r5 //r3 = arr[i] - arr[i = 1]
	str r3, [r1, r0] //store the value in r3, @ arr[i]
	adds r0, #4
	cmp r0, #60 //15 * 4 = 60, to keep with counter
	blt for

    // You must terminate your subroutine with bx lr
    // unless you know how to use PUSH and POP.
    ldr r6, =value
    str r2, [r6]
    POP {R4-R7, PC}



.global charsub
charsub:
    // Your code for charsub goes here
    PUSH {R4-R7, LR}
    movs r0, #0 //x
    ldr r1, =str //r1 = str
for2:
	ldrb r2, [r1, r0] //r2 = str[x]
	cmp r2, #0
	beq endfor //for condition

	movs r3, r2 //r3 = str[x]
	movs r4, #32 //r4 = 32
	bics r3, r4 //r3 = str[x] & ~32

	cmp r3, 0x41 //str[x] & 32 - 'A'
	blt else2

	cmp r3, 0x4d //str[x] & 32 - 'M'
	bgt else2
if2:
	movs r5, r2 //r5 = str[x]
	adds r5, #13 //r5 = str[x] + 13
	strb r5, [r1, r0] //str r5 @ [r1, r0]
	b endif2
else2:
	cmp r3, 0x4e
	blt endif2

	cmp r3, 0x5a
	bgt endif2

	movs r5, r2
	subs r5, #13
	strb r5, [r1, r0]
	b endif2
endif2:
	adds r0, #1
	b for2
endfor:
    // You must terminate your subroutine with bx lr
    // unless you know how to use PUSH and POP.
    POP {R4-R7, PC}


.global login
login: .string "dpenebac" // Make sure you put your login here.
.balign 2
.global main
main:
    bl autotest // uncomment AFTER you debug your subroutines
    bl intsub
    bl charsub
    bkpt
