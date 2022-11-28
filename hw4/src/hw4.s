.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

//const char login[] = "xyz";
//void hello(void) {
//    printf("Hello, %s!\n", login);  // Here, printf is given two arguments
//}

.global login
login: .string "dpenebac"
hello_str: .string "Hello, %s!\n"
.balign  2
.global hello
hello:
	push {lr}
	ldr r0, =hello_str
	ldr r1, =login
	bl printf
	pop  {pc}

//void showsub2(int a, int b)
//{
//    printf("%d - %d = %d\n", a, b, a-b); // Here, printf is given four args
//	  a - b = (a-b)
//}
showsub2_str: .string "%d - %d = %d\n"
.balign  2
.global showsub2
showsub2:
	push {lr}
	//r0 = 5, r1 = 3
	movs r2, r1
	movs r1, r0
	subs r3, r1, r2
	//r1 = 5, r2 = 3, r3 = 5 - 3

	ldr r0, =showsub2_str

	bl printf

	pop  {pc}

//void showsub3(int a, int b, int c)
//{
//    printf("%d - %d - %d = %d\n", a, b, c, a-b-c); // five args
//}
showsub3_str: .string "%d - %d - %d = %d\n"
.balign 2
.global showsub3
showsub3:
	push {lr}

	movs r3, r2 //c
	movs r2, r1 //b
	movs r1, r0 //a

	movs r0, #0
	subs r0, r1, r2 //a - b
	subs r0, r3 //a - b - c
	sub  sp, #4     // Allocate 4 bytes on stack. register is 4 bytes
	str r0, [sp,#0]  // Store on stack

	//r1 = a, r2 = b, r3 = c, r4 = 5

	ldr r0, =showsub3_str

	bl printf

	//unstore on stack
	add sp, #4

	pop {pc}

//void listing(const char *school, int course, const char *verb,
//             int enrollment, const char *season, int year)
//{
//    // You need to allocate space on the stack to hold some of the
//    // seven parameters to printf().  Then move the arguments around
//    // to call printf() with the proper arguments.
//    printf("%s %05d %s %d students in %s, %d\n",
//    		school, course, verb, enrollment, season, year);
//}
listing_str: .string "%s %s %s %d students in %s, %s\n"
.balign 2
.global listing
listing:
	push {r4, lr}

	movs r4, r3
	movs r3, r2
	movs r2, r1
	movs r1, r0

	ldr r0, =listing_str
	bl printf



	pop {r4, pc}



//string inputs
school: .string "ECE"
course: .string "2343"
verb: .string "has"
enrollment: .string "3000"
season: .string "Summer"
year: .string "2022"
.balign 2

//spacing issues
space: .string "\nSTART HERE\n\n"
.balign 2

//main
.global main
main:
	bl serial_init

	ldr r0, =space
	bl printf

	bl hello

	movs r0, #3
	movs r1, #10
	bl showsub2

	movs r0, #15
	movs r1, #4
	movs r2, #3
	bl showsub3

	ldr r0, =season
	ldr r1, =year
	sub sp, #8 //allocate 8 bytes (2 registers) on stack
	str r0, [sp, #0]
	str r1, [sp, #4]
	ldr r0, =school
	ldr r1, =course
	ldr r2, =verb
	ldr r3, =enrollment
	bl listing
	add sp, #8

	bkpt
