//TODO: github
//TODO: basic IRQ handling
//TODO: verify that MPC10x writes (and initial reg writes) are the same as observed in VF3
//TODO: implement support for reading controls
//TODO: implement a simple menu system to conduct experiments
//TODO: which system regs (and in which order) does VF3 access at startup?
//TODO: before running on board, test to make sure no unaligned accesses occur

#include "model3/jtag.h"
#include "model3/tilegen.h"
#include "model3/ppc.h"

static void setup_pci_devices()
{
  uint32_t pci_config = 0xf0800cf8;
  uint32_t pci_data = 0xf0c00cfc;
  // Real3D
  ppc_stwbrx(pci_config, 0x80006804);
  ppc_stwbrx(pci_data, 0xffff0042);
  ppc_stwbrx(pci_config, 0x80006810);
  ppc_stwbrx(pci_data, 0x80000000);
}

static void setup_unknown_regs()
{
  // Real3D?
  volatile uint32_t *regs_9c = (uint32_t *) 0x9c000000;
  regs_9c[0] = 0xfc7f0000;
  regs_9c[1] = 0x04000000;
  regs_9c[2] = 0x00000200;
}

static void trigger_real3d()
{
  ppc_stwbrx(0x88000000, 0xdeaddead);
}

static void print_bat(uint32_t batu, uint32_t batl)
{
  uint32_t bepi = batu >> (31 - 14);
  uint32_t bl = (batu >> (31 - 29)) & 0x7ff;
  int vs = !!(batu & 2);
  int vp = !!(batu & 1);
  uint32_t brpn = batl >> (31 - 14);
  uint32_t wimg = (batl >> (31 - 28)) & 0xf;
  uint32_t pp = batl & 3;
  uint32_t size = (bl + 1) * 128 * 1024;
  uint32_t ea_base = bepi << (31 - 14);
  uint32_t ea_limit = ea_base + size - 1;
  uint32_t pa_base = brpn << (31 - 14);
  uint32_t pa_limit = pa_base + size - 1;
  tilegen_printf("%08X-%08X %08X-%08X ", ea_base, ea_limit, pa_base, pa_limit);
  tilegen_printf("%c%c%c%c ", (wimg&8)?'W':'-', (wimg&4)?'I':'-', (wimg&2)?'M':'-', (wimg&1)?'G':'-');
  tilegen_printf("PP=");
  if (pp == 0)
    tilegen_printf("NA");
  else if (pp == 2)
    tilegen_printf("RW");
  else
    tilegen_printf("RO");
  tilegen_printf(" Vs=%d Vp=%d\n", vs, vp);
}

int main(void)
{
  jtag_init();
  tilegen_init();

  tilegen_printf("\n*** MODEL 3 TEST PROGRAM ***\n");
  tilegen_printf("\tby Bart Trzynadlowski\n\n\n\n");

  tilegen_printf("\tPVR   =%08X\n", ppc_get_pvr());
  tilegen_printf("\tHID0  =%08X\n", ppc_get_hid0());
  tilegen_printf("\tHID1  =%08X\n", ppc_get_hid1());
  tilegen_printf("\tMSR   =%08X\tSDR1=%08X\n", ppc_get_msr(), ppc_get_sdr1());
  tilegen_printf("\tDBAT0U=%08X\tSR0 =%08X\n", ppc_get_dbatu(0), ppc_get_sr(0));
  tilegen_printf("\tDBAT0L=%08X\tSR1 =%08X\n", ppc_get_dbatl(0), ppc_get_sr(1));
  tilegen_printf("\tDBAT1U=%08X\tSR2 =%08X\n", ppc_get_dbatu(1), ppc_get_sr(2));
  tilegen_printf("\tDBAT1L=%08X\tSR3 =%08X\n", ppc_get_dbatl(1), ppc_get_sr(3));
  tilegen_printf("\tDBAT2U=%08X\tSR4 =%08X\n", ppc_get_dbatu(2), ppc_get_sr(4));
  tilegen_printf("\tDBAT2L=%08X\tSR5 =%08X\n", ppc_get_dbatl(2), ppc_get_sr(5));
  tilegen_printf("\tDBAT3U=%08X\tSR6 =%08X\n", ppc_get_dbatu(3), ppc_get_sr(6));
  tilegen_printf("\tDBAT3L=%08X\tSR7 =%08X\n", ppc_get_dbatl(3), ppc_get_sr(7));
  tilegen_printf("\tIBAT0U=%08X\tSR8 =%08X\n", ppc_get_ibatu(0), ppc_get_sr(8));
  tilegen_printf("\tIBAT0L=%08X\tSR9 =%08X\n", ppc_get_ibatl(0), ppc_get_sr(9));
  tilegen_printf("\tIBAT1U=%08X\tSR10=%08X\n", ppc_get_ibatu(1), ppc_get_sr(10));
  tilegen_printf("\tIBAT1L=%08X\tSR11=%08X\n", ppc_get_ibatl(1), ppc_get_sr(11));
  tilegen_printf("\tIBAT2U=%08X\tSR12=%08X\n", ppc_get_ibatu(2), ppc_get_sr(12));
  tilegen_printf("\tIBAT2L=%08X\tSR13=%08X\n", ppc_get_ibatl(2), ppc_get_sr(13));
  tilegen_printf("\tIBAT3U=%08X\tSR14=%08X\n", ppc_get_ibatu(3), ppc_get_sr(14));
  tilegen_printf("\tIBAT3L=%08X\tSR15=%08X\n", ppc_get_ibatl(3), ppc_get_sr(15));
  tilegen_printf("\n\n\n");

  tilegen_printf("DB0: "); print_bat(ppc_get_dbatu(0), ppc_get_dbatl(0));
  tilegen_printf("DB1: "); print_bat(ppc_get_dbatu(1), ppc_get_dbatl(1));
  tilegen_printf("DB2: "); print_bat(ppc_get_dbatu(2), ppc_get_dbatl(2));
  tilegen_printf("DB3: "); print_bat(ppc_get_dbatu(3), ppc_get_dbatl(3));
  tilegen_printf("IB0: "); print_bat(ppc_get_ibatu(0), ppc_get_ibatl(0));
  tilegen_printf("IB1: "); print_bat(ppc_get_ibatu(1), ppc_get_ibatl(1));
  tilegen_printf("IB2: "); print_bat(ppc_get_ibatu(2), ppc_get_ibatl(2));
  tilegen_printf("IB3: "); print_bat(ppc_get_ibatu(3), ppc_get_ibatl(3));

  return 0;
}