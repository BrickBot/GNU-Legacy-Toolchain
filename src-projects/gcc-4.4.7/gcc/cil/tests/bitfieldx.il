.assembly extern gcc4net {}
.module '<Module>'
.custom instance void ['OpenSystem.C']'OpenSystem.C'.ModuleAttribute::.ctor() = (01 00 00 00)

.method public static void 'p' (int32 'i') cil managed
{
	.locals ()

	ldsflda	valuetype 'array?[4]I8' '?string3083477516'
	conv.i
	ldarg	'i'
	call	vararg int32 [libstd]libstd::printf(int8 *, ..., int32)
	pop
	ret
	.maxstack 2

} // p

.method public static void 'pu' (unsigned int32 'i') cil managed
{
	.locals ()

	ldsflda	valuetype 'array?[4]I8' '?string3083477544'
	conv.i
	ldarg	'i'
	call	vararg int32 [libstd]libstd::printf(int8 *, ..., unsigned int32)
	pop
	ret
	.maxstack 2

} // pu

.method public static void 'w' (valuetype 'un' * 'u') cil managed
{
	.locals (int32 'i?0')

	ldc.i4	0
	stloc	'i?0'
	br	?L1
?L0:
	ldarg	'u'
	ldflda	valuetype 'array?[8]UI32' 'un'::'ll'
	conv.i
	ldloc	'i?0'
	ldc.i4	4
	mul
	add
	ldind.u4
	call	void 'pu' (unsigned int32)
	ldloc	'i?0'
	ldc.i4	1
	add
	stloc	'i?0'
?L1:
	ldloc	'i?0'
	ldc.i4	7
	ble	?L0
?L2:
	ret
	.maxstack 3

} // w

.method public static void 'i' (valuetype 'un' * 'u') cil managed
{
	.locals (int32 'i?0')

	ldc.i4	0
	stloc	'i?0'
	br	?L1
?L0:
	ldarg	'u'
	ldflda	valuetype 'array?[8]UI32' 'un'::'ll'
	conv.i
	ldloc	'i?0'
	ldc.i4	4
	mul
	add
	ldc.i4	4294967295
	stind.i4
	ldloc	'i?0'
	ldc.i4	1
	add
	stloc	'i?0'
?L1:
	ldloc	'i?0'
	ldc.i4	7
	ble	?L0
?L2:
	ret
	.maxstack 3

} // i

.method public static int32 '.start'(class [mscorlib]System.String[] 'args') cil managed
{
	.entrypoint
	.maxstack 2
	.locals (int32 'argc', int8** 'argv')
	call	void [gcc4net]gcc4net.StartupHelper::Startup()
	call void class [libstd]libstd::'.init'()
	call	int32 main()
	ret
} // .start


.method public static int32 m (valuetype 'un' *) cil managed
{
	.locals (valuetype 'foo' * ufbptr)

	    ldarg.0
	   ldflda	valuetype 'foo' 'un'::'f'
	  conv.i
	  ldc.i4	2
	 add
	stloc	'ufbptr'

	 ldloc	'ufbptr'
	 ldc.i4 64523
	stind.i2
	
	    ldarg.0
	   ldflda	valuetype 'array?[8]UI32' 'un'::'ll'
	  conv.i
	 ldind.u4
	 ldc.i4	4228644863
	ceq
       ret
}

.method public static int32 m2 (int32 *) cil managed
{
	.locals (int32 * ufbptr)

	   ldarg.0
	  conv.i
	  ldc.i4	2
	 add
	stloc	'ufbptr'

	 ldloc	'ufbptr'
	 ldc.i4 64523
	stind.i2
	
	   ldarg.0
	  conv.i
	 ldind.u4
	 ldc.i4	4228644863
	ceq
       ret
}

.method public static int32 'main' () cil managed
{
	.locals (valuetype 'un' 'u')

	 ldloca	'u'
	call	void 'i' (valuetype 'un' *)
	 ldloca	'u'
	call	void 'w' (valuetype 'un' *)
	 ldloca	'u'
	call	int32 'm2' (int32 *)
	 ldloca	'u'
	call	void 'w' (valuetype 'un' *)

	ret
} // main
// string: "%d\n"
.data 'DataStr3083477516' = bytearray(25 64 0a 00 )
.field private static valuetype 'array?[4]I8' '?string3083477516' at 'DataStr3083477516'
// string: "%u\n"
.data 'DataStr3083477544' = bytearray(25 75 0a 00 )
.field private static valuetype 'array?[4]I8' '?string3083477544' at 'DataStr3083477544'

.class public explicit sealed serializable ansi 'foo' extends ['mscorlib']System.ValueType
{
	.size 16
	.pack 8
	.field [0] public int8 'z'
	.field [0] public int32 'a'
	.field [0] public unsigned int32 'v'
	.field [2] public int16 'b'
	.field [0] public int32 'c'
	.field [4] public int32 'd'
	.field [5] public int8 'x'
	.field [8] public float64 'y'
}

.class public explicit sealed serializable ansi 'array?[8]UI32' extends ['mscorlib']System.ValueType
{
	.size 32
	.pack 4
	.field [0] public specialname unsigned int32 'elem__'
}

.class public explicit sealed serializable ansi 'array?[4]I8' extends ['mscorlib']System.ValueType
{
	.size 4
	.pack 1
	.field [0] public specialname int8 'elem__'
}

.class public explicit sealed serializable ansi 'un' extends ['mscorlib']System.ValueType
{
	.size 32
	.pack 8
	.field [0] public valuetype 'array?[8]UI32' 'll'
	.field [0] public valuetype 'foo' 'f'
}
