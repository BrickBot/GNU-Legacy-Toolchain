.assembly extern libstd
{
  .ver 0:0:0:0
}
.assembly extern gcc4net
{
  .ver 0:0:0:0
}
.assembly extern mscorlib
{
  .ver 1:0:5000:0
  .publickeytoken = (B7 7A 5C 56 19 34 E0 89 ) // .z\V.4..
}
.assembly 'mp3dec'
{
  .hash algorithm 0x00008004
  .ver  0:0:0:0
}

.module mp3dec

    .method public static 
           default int32 mp3_filterbank_kern () cil managed 
    {
	.maxstack 10

	ldc.i4 4000
	conv.i

	//dup
	//conv.i4
	//call void p(int32)

	ldflda unsigned int8* SI_L3_CHGR::band_width_ptr
        conv.i

	//dup
	//conv.i4
	//call void p(int32)
	
	ret 
    }


    .method public static 
           default int32 'main' (string[] args)  cil managed 
    {
     	.entrypoint
	.maxstack 8
	call void class [gcc4net]gcc4net.StartupHelper::Startup()
	call int32 mp3_filterbank_kern()
	ret 
    }

  .class public explicit ansi sealed serializable SI_L3_CHGR
  	extends [mscorlib]System.ValueType
  {
    .pack 4
    .size 76

    .field [48] public  unsigned int8* band_width_ptr

  }

    .method public static  specialname 
           default void '.init' ()  cil managed 
    {
	.maxstack 3
	call void class [libstd]libstd::'.init'()
	ret 
    }

  .class public explicit ansi sealed serializable 'array?[6]I8'
  	extends [mscorlib]System.ValueType
  {
    .pack 1
    .size 6
    .field [0] public specialname  int8 elem__

  } 

    .field  private static valuetype 'array?[6]I8' fmtstr at D_X

    .method private static 
           default void 'p' (int32 n)  cil managed 
    {
	.maxstack 8
	ldsflda valuetype 'array?[6]I8' 'fmtstr'
	conv.i 
	ldarg.0 
	call vararg int32 class [libstd]libstd::printf(int8*, ..., int32)
	pop
	ret 
    }

.data D_X = bytearray (
	 25 64 0A 00 00 00) // size: 6
