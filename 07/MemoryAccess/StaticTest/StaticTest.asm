// push constant 111
	@111
	D = A
	@SP
	M = M + 1
	A = M - 1
	M = D
// push constant 333
	@333
	D = A
	@SP
	M = M + 1
	A = M - 1
	M = D
// push constant 888
	@888
	D = A
	@SP
	M = M + 1
	A = M - 1
	M = D
// pop static 8
	@24
	D = A
	@R13
	M = D
	@SP
	M = M - 1
	A = M
	D = M
	@R13
	A = M
	M = D
// pop static 3
	@19
	D = A
	@R13
	M = D
	@SP
	M = M - 1
	A = M
	D = M
	@R13
	A = M
	M = D
// pop static 1
	@17
	D = A
	@R13
	M = D
	@SP
	M = M - 1
	A = M
	D = M
	@R13
	A = M
	M = D
// push static 3
	@19
	D = M
	@SP
	M = M + 1
	A = M - 1
	M = D
// push static 1
	@17
	D = M
	@SP
	M = M + 1
	A = M - 1
	M = D
// sub
	@SP
	M = M - 1
	A = M
	D = M
	A = A - 1
	M = M - D
// push static 8
	@24
	D = M
	@SP
	M = M + 1
	A = M - 1
	M = D
// add
	@SP
	M = M - 1
	A = M
	D = M
	A = A - 1
	M = M + D
// Final infinite loop.
(END_INF)
	@END_INF
	0;JEQ
