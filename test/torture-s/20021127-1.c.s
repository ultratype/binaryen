	.text
	.file	"/b/build/slave/linux/build/src/src/work/gcc/gcc/testsuite/gcc.c-torture/execute/20021127-1.c"
	.section	.text.main,"ax",@progbits
	.hidden	main
	.globl	main
	.type	main,@function
main:                                   # @main
	.result 	i32
# BB#0:                                 # %entry
	i32.const	$push0=, 0
	return  	$pop0
	.endfunc
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.section	.text.llabs,"ax",@progbits
	.hidden	llabs
	.globl	llabs
	.type	llabs,@function
llabs:                                  # @llabs
	.param  	i64
	.result 	i64
# BB#0:                                 # %entry
	call    	abort@FUNCTION
	unreachable
	.endfunc
.Lfunc_end1:
	.size	llabs, .Lfunc_end1-llabs

	.hidden	a                       # @a
	.type	a,@object
	.section	.data.a,"aw",@progbits
	.globl	a
	.align	3
a:
	.int64	-1                      # 0xffffffffffffffff
	.size	a, 8


	.ident	"clang version 3.9.0 "