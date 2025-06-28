.assembly 'testvolatile'
{
  .hash algorithm 0x00008004
  .ver  0:0:0:0
}

.module testvolatile

    .field  private static  int32 'w'

    .method public static
           default void system_wait ()  cil managed
    {
        .maxstack 8

        IL_0012:  volatile.
        IL_0014:  ldsfld int32 'w'
        IL_0019:  ldc.i4.1
        IL_001a:  sub
        IL_001d:  //volatile.
		  stsfld int32 'w'
        IL_0024:  //volatile. 
		  ldsfld int32 'w'
        IL_0029:  ldc.i4.0
        IL_002a:  bgt.s IL_0012

        IL_002c:  ret
    }

    .method public static 
           default int32 'main' (string[] args)  cil managed 
    {
     	.entrypoint
	.maxstack 8
	ldc.i4 2000
        stsfld int32 'w'
	call void system_wait ()
	ldc.i4.0
	ret 
    }
