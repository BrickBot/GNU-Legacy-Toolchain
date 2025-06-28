#include <stdio.h>
#include <stdlib.h>

struct layer_data
{
  /* bit input */
  FILE* Infile;
  unsigned char Rdbfr[2048];
  unsigned char *Rdptr;
  unsigned char Inbfr[16];
  /* from mpeg2play */
  unsigned int Bfr;
  unsigned char *Rdmax;
  int Incnt;
  int Bitcnt;
  /* sequence header and quant_matrix_extension() */
  int intra_quantizer_matrix[64];
  int non_intra_quantizer_matrix[64];
  int chroma_intra_quantizer_matrix[64];
  int chroma_non_intra_quantizer_matrix[64];
  
  int load_intra_quantizer_matrix;
  int load_non_intra_quantizer_matrix;
  int load_chroma_intra_quantizer_matrix;
  int load_chroma_non_intra_quantizer_matrix;

  int MPEG2_Flag;
  /* sequence scalable extension */
  int scalable_mode;
  /* picture coding extension */
  int q_scale_type;
  int alternate_scan;
  /* picture spatial scalable extension */
  int pict_scal;
  /* slice/macroblock */
  int priority_breakpoint;
  int quantizer_scale;
  int intra_slice;
  short block[12][64];
};

static struct layer_data *ld;
static int System_Stream_Flag = 0;
#define SEQUENCE_END_CODE       0x1B7
#define PACK_START_CODE         0x1BA
#define SYSTEM_START_CODE       0x1BB
#define VIDEO_ELEMENTARY_STREAM 0x1e0
#define ISO_END_CODE            0x1B9


#define INFILE "/home/ricardo/cvs/bench-dotnet-performance/mpeg2dec/run/test.m2v.in"

void Fill_Buffer()
{
  int Buffer_Level;

  Buffer_Level = fread(ld->Rdbfr,1,2048,ld->Infile);
  ld->Rdptr = ld->Rdbfr;

  if (System_Stream_Flag)
    ld->Rdmax -= 2048;

  
  /* end of the bitstream file */
  if (Buffer_Level < 2048)
  {
    /* just to be safe */
    if (Buffer_Level < 0)
      Buffer_Level = 0;

    /* pad until the next to the next 32-bit word boundary */
    while (Buffer_Level & 3)
      ld->Rdbfr[Buffer_Level++] = 0;

	/* pad the buffer with sequence end codes */
    while (Buffer_Level < 2048)
    {
      ld->Rdbfr[Buffer_Level++] = SEQUENCE_END_CODE>>24;
      ld->Rdbfr[Buffer_Level++] = SEQUENCE_END_CODE>>16;
      ld->Rdbfr[Buffer_Level++] = SEQUENCE_END_CODE>>8;
      ld->Rdbfr[Buffer_Level++] = SEQUENCE_END_CODE&0xff;
    }
  }
}

/* advance by n bits */
void Flush_Buffer(int N)
{
  int Incnt;

  ld->Bfr <<= N;

  Incnt = ld->Incnt -= N;

  if (Incnt <= 24)
  {
    if (System_Stream_Flag && (ld->Rdptr >= ld->Rdmax-4))
    {
      do
      {
        if (ld->Rdptr >= ld->Rdmax)
          Next_Packet();
        ld->Bfr |= Get_Byte() << (24 - Incnt);
        Incnt += 8;
      }
      while (Incnt <= 24);
    }
    else if (ld->Rdptr < ld->Rdbfr+2044)
    {
      do
      {
        ld->Bfr |= *ld->Rdptr++ << (24 - Incnt);
        Incnt += 8;
      }
      while (Incnt <= 24);
    }
    else
    {
      do
      {
        if (ld->Rdptr >= ld->Rdbfr+2048)
          Fill_Buffer();
        ld->Bfr |= *ld->Rdptr++ << (24 - Incnt);
        Incnt += 8;
      }
      while (Incnt <= 24);
    }
    ld->Incnt = Incnt;
  }

#ifdef VERIFY 
  ld->Bitcnt += N;
#endif /* VERIFY */

}

/* return next n bits (right adjusted) without advancing */
unsigned int Show_Bits(int N)
{
  return ld->Bfr >> (32-N);
}

/* return next n bits (right adjusted) */
unsigned int Get_Bits(int N)
{
  unsigned int Val;

  Val = Show_Bits(N);
  Flush_Buffer(N);

  return Val;
}

void Initialize_Buffer()
{
  ld = calloc (1, sizeof (struct layer_data));
  ld->Incnt = 0;
  ld->Rdptr = ld->Rdbfr + 2048;
  ld->Rdmax = ld->Rdptr;
  ld->Bfr = 0;
  ld->Infile = fopen (INFILE, "rb");
  Flush_Buffer (0); /* fills valid data into bfr */
}



/* MPEG-1 system layer demultiplexer */

int Get_Byte()
{
  while(ld->Rdptr >= ld->Rdbfr+2048)
  {
    fread(ld->Rdbfr,1,2048,ld->Infile);
    ld->Rdptr -= 2048;
    ld->Rdmax -= 2048;
  }
  return *ld->Rdptr++;
}

/* extract a 16-bit word from the bitstream buffer */
int Get_Word()
{
  int Val;

  Val = Get_Byte();
  return (Val<<8) | Get_Byte();
}

int Get_Long()
{
  int i;

  i = Get_Word();
  return (i<<16) | Get_Word();
}

/* return next bit (could be made faster than Get_Bits(1)) */

unsigned int Get_Bits1()
{
  return Get_Bits(1);
}

/* align to start of next next_start_code */

void
next_start_code (void)
{
  /* byte align */
  Flush_Buffer(ld->Incnt&7);
  while (Show_Bits(24)!=0x01L)
    {
      Flush_Buffer(8);
    }
}

 Next_Packet()
{
  unsigned int code;
  int l;

  for(;;)
  {
    code = Get_Long();

    /* remove system layer byte stuffing */
    while ((code & 0xffffff00) != 0x100)
      code = (code<<8) | Get_Byte();

    switch(code)
    {
    case PACK_START_CODE: /* pack header */
      /* skip pack header (system_clock_reference and mux_rate) */
      ld->Rdptr += 8;
      break;
    case VIDEO_ELEMENTARY_STREAM:   
      code = Get_Word();             /* packet_length */
      ld->Rdmax = ld->Rdptr + code;

      code = Get_Byte();

      if((code>>6)==0x02)
      {
        ld->Rdptr++;
        code=Get_Byte();  /* parse PES_header_data_length */
        ld->Rdptr+=code;    /* advance pointer by PES_header_data_length */
        printf("MPEG-2 PES packet\n");
        return;
      }
      else if(code==0xff)
      {
        /* parse MPEG-1 packet header */
        while((code=Get_Byte())== 0xFF);
      }
       
      /* stuffing bytes */
      if(code>=0x40)
      {
        if(code>=0x80)
        {
          fprintf(stderr,"Error in packet header\n");
          exit(1);
        }
        /* skip STD_buffer_scale */
        ld->Rdptr++;
        code = Get_Byte();
      }

      if(code>=0x30)
      {
        if(code>=0x40)
        {
          fprintf(stderr,"Error in packet header\n");
          exit(1);
        }
        /* skip presentation and decoding time stamps */
        ld->Rdptr += 9;
      }
      else if(code>=0x20)
      {
        /* skip presentation time stamps */
        ld->Rdptr += 4;
      }
      else if(code!=0x0f)
      {
        fprintf(stderr,"Error in packet header\n");
        exit(1);
      }
      return;
    case ISO_END_CODE: /* end */
      /* simulate a buffer full of sequence end codes */
      l = 0;
      while (l<2048)
      {
        ld->Rdbfr[l++] = SEQUENCE_END_CODE>>24;
        ld->Rdbfr[l++] = SEQUENCE_END_CODE>>16;
        ld->Rdbfr[l++] = SEQUENCE_END_CODE>>8;
        ld->Rdbfr[l++] = SEQUENCE_END_CODE&0xff;
      }
      ld->Rdptr = ld->Rdbfr;
      ld->Rdmax = ld->Rdbfr + 2048;
      return;
    default:
      if(code>=SYSTEM_START_CODE)
      {
        /* skip system headers and non-video packets*/
        code = Get_Word();
        ld->Rdptr += code;
      }
      else
      {
        fprintf(stderr,"Unexpected startcode %08x in system layer\n",code);
        exit(1);
      }
      break;
    }
  }
}

int
main (int argc, char *arv[])
{
  int c;
  Initialize_Buffer(); 
  
  c = Show_Bits(8);
  printf ("%d\n", c);
  next_start_code();
  c = Show_Bits(32);
  printf ("%d\n", c);
  
  return c;
}
