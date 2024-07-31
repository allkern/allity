#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ppc.h"
#include "ppc_dis.h"

struct ppc_state* ppc_create(struct ppc_bus bus) {
    struct ppc_state* cpu = malloc(sizeof(struct ppc_state));

    ppc_init(cpu, bus);

    return cpu;
}

void ppc_init(struct ppc_state* cpu, struct ppc_bus bus) {
    memset(cpu, 0, sizeof(struct ppc_bus));

    cpu->bus = bus;
}

inline uint8_t ppc_read8(struct ppc_state* cpu, uint32_t addr) {
    return cpu->bus.read8(cpu->bus.udata, addr);
}

inline uint16_t ppc_read16(struct ppc_state* cpu, uint32_t addr) {
    return cpu->bus.read16(cpu->bus.udata, addr);
}

inline uint32_t ppc_read32(struct ppc_state* cpu, uint32_t addr) {
    return cpu->bus.read32(cpu->bus.udata, addr);
}

inline uint64_t ppc_read64(struct ppc_state* cpu, uint32_t addr) {
    return cpu->bus.read64(cpu->bus.udata, addr);
}

inline void ppc_write8(struct ppc_state* cpu, uint32_t addr, uint8_t data) {
    cpu->bus.write8(cpu->bus.udata, addr, data);
}

inline void ppc_write16(struct ppc_state* cpu, uint32_t addr, uint16_t data) {
    cpu->bus.write16(cpu->bus.udata, addr, data);
}

inline void ppc_write32(struct ppc_state* cpu, uint32_t addr, uint32_t data) {
    cpu->bus.write32(cpu->bus.udata, addr, data);
}

inline void ppc_write64(struct ppc_state* cpu, uint32_t addr, uint64_t data) {
    cpu->bus.write64(cpu->bus.udata, addr, data);
}

static struct ppc_dis_state dis_state;
static char buf[128];

int ppc_execute(struct ppc_state* cpu) {
    cpu->opcode = ppc_read32(cpu, cpu->pc);

    // dis_state.pc = cpu->pc;
    // dis_state.print_address = 1;
    // dis_state.print_opcode = 1;

    // puts(ppc_disassemble(&dis_state, buf, cpu->opcode));

    cpu->pc += 4;

    switch ((cpu->opcode & 0xfc000000) >> 26) {
        case 0x08000000ul >> 26: ppc_tdi(cpu); return 1;
        case 0x0c000000ul >> 26: ppc_twi(cpu); return 1;
        case 0x1c000000ul >> 26: ppc_mulli(cpu); return 1;
        case 0x20000000ul >> 26: ppc_subfic(cpu); return 1;
        case 0x28000000ul >> 26: ppc_cmpli(cpu); return 1;
        case 0x2c000000ul >> 26: ppc_cmpi(cpu); return 1;
        case 0x30000000ul >> 26: ppc_addic(cpu); return 1;
        case 0x34000000ul >> 26: ppc_addic_u(cpu); return 1;
        case 0x38000000ul >> 26: ppc_addi(cpu); return 1;
        case 0x3c000000ul >> 26: ppc_addis(cpu); return 1;
        case 0x40000000ul >> 26: ppc_bc(cpu); return 1;
        case 0x44000000ul >> 26: ppc_sc(cpu); return 1;
        case 0x48000000ul >> 26: ppc_b(cpu); return 1;
        case 0x4c000000ul >> 26: {
            switch ((cpu->opcode & 0x000007fe) >> 1) {
                case 0x00000000ul >> 1: ppc_mcrf(cpu); return 1;
                case 0x00000020ul >> 1: ppc_bclr(cpu); return 1;
                case 0x00000024ul >> 1: ppc_rfid(cpu); return 1;
                case 0x00000042ul >> 1: ppc_crnor(cpu); return 1;
                case 0x00000102ul >> 1: ppc_crandc(cpu); return 1;
                case 0x0000012cul >> 1: ppc_isync(cpu); return 1;
                case 0x00000182ul >> 1: ppc_crxor(cpu); return 1;
                case 0x000001c2ul >> 1: ppc_crnand(cpu); return 1;
                case 0x00000202ul >> 1: ppc_crand(cpu); return 1;
                case 0x00000224ul >> 1: ppc_hrfid(cpu); return 1;
                case 0x00000242ul >> 1: ppc_creqv(cpu); return 1;
                case 0x00000342ul >> 1: ppc_crorc(cpu); return 1;
                case 0x00000382ul >> 1: ppc_cror(cpu); return 1;
                case 0x00000420ul >> 1: ppc_bcctr(cpu); return 1;
            }
        } break;
        case 0x50000000ul >> 26: ppc_rlwimi(cpu); return 1;
        case 0x54000000ul >> 26: ppc_rlwinm(cpu); return 1;
        case 0x5c000000ul >> 26: ppc_rlwnm(cpu); return 1;
        case 0x60000000ul >> 26: ppc_ori(cpu); return 1;
        case 0x64000000ul >> 26: ppc_oris(cpu); return 1;
        case 0x68000000ul >> 26: ppc_xori(cpu); return 1;
        case 0x6c000000ul >> 26: ppc_xoris(cpu); return 1;
        case 0x70000000ul >> 26: ppc_andi_u(cpu); return 1;
        case 0x74000000ul >> 26: ppc_andis_u(cpu); return 1;
        case 0x78000000ul >> 26: {
            switch ((cpu->opcode & 0x0000001e) >> 1) {
                case 0x00000010ul >> 1: ppc_rldcl(cpu); return 1;
                case 0x00000012ul >> 1: ppc_rldcr(cpu); return 1;
            }
            switch ((cpu->opcode & 0x0000001c) >> 2) {
                case 0x00000000ul >> 2: ppc_rldicl(cpu); return 1;
                case 0x00000004ul >> 2: ppc_rldicr(cpu); return 1;
                case 0x00000008ul >> 2: ppc_rldic(cpu); return 1;
                case 0x0000000cul >> 2: ppc_rldimi(cpu); return 1;
            }
        } break;
        case 0x7c000000ul >> 26: {
            switch ((cpu->opcode & 0x000007fe) >> 1) {
                case 0x0000032eul >> 1: ppc_sthx(cpu); return 1;
                case 0x00000436ul >> 1: ppc_srd(cpu); return 1;
                case 0x00000430ul >> 1: ppc_srw(cpu); return 1;
                case 0x0000042eul >> 1: ppc_lfsx(cpu); return 1;
                case 0x0000042cul >> 1: ppc_lwbrx(cpu); return 1;
                case 0x0000042aul >> 1: ppc_lswx(cpu); return 1;
                case 0x00000400ul >> 1: ppc_mcrxr(cpu); return 1;
                case 0x000003e4ul >> 1: ppc_slbia(cpu); return 1;
                case 0x000003b8ul >> 1: ppc_nand(cpu); return 1;
                case 0x000003a6ul >> 1: ppc_mtspr(cpu); return 1;
                case 0x00000378ul >> 1: ppc_or(cpu); return 1;
                case 0x0000036eul >> 1: ppc_sthux(cpu); return 1;
                case 0x0000036cul >> 1: ppc_ecowx(cpu); return 1;
                case 0x00000364ul >> 1: ppc_slbie(cpu); return 1;
                case 0x00000338ul >> 1: ppc_orc(cpu); return 1;
                case 0x0000046cul >> 1: ppc_tlbsync(cpu); return 1;
                case 0x00000324ul >> 1: ppc_slbmte(cpu); return 1;
                case 0x000002eeul >> 1: ppc_lhaux(cpu); return 1;
                case 0x000002eaul >> 1: ppc_lwaux(cpu); return 1;
                case 0x000002e6ul >> 1: ppc_mftb(cpu); return 1;
                case 0x000002e4ul >> 1: ppc_tlbia(cpu); return 1;
                case 0x000002aeul >> 1: ppc_lhax(cpu); return 1;
                case 0x000002aaul >> 1: ppc_lwax(cpu); return 1;
                case 0x000002a6ul >> 1: ppc_mfspr(cpu); return 1;
                case 0x00000278ul >> 1: ppc_xor(cpu); return 1;
                case 0x0000026eul >> 1: ppc_lhzux(cpu); return 1;
                case 0x0000026cul >> 1: ppc_eciwx(cpu); return 1;
                case 0x00000000ul >> 1: ppc_cmp(cpu); return 1;
                case 0x00000238ul >> 1: ppc_eqv(cpu); return 1;
                case 0x000005eeul >> 1: ppc_stfdux(cpu); return 1;
                case 0x000007ecul >> 1: ppc_dcbz(cpu); return 1;
                case 0x000007b4ul >> 1: ppc_extsw(cpu); return 1;
                case 0x000007aeul >> 1: ppc_stfiwx(cpu); return 1;
                case 0x000007acul >> 1: ppc_icbi(cpu); return 1;
                case 0x00000774ul >> 1: ppc_extsb(cpu); return 1;
                case 0x00000734ul >> 1: ppc_extsh(cpu); return 1;
                case 0x0000072cul >> 1: ppc_sthbrx(cpu); return 1;
                case 0x00000726ul >> 1: ppc_slbmfee(cpu); return 1;
                case 0x000006acul >> 1: ppc_eieio(cpu); return 1;
                case 0x000006a6ul >> 1: ppc_slbmfev(cpu); return 1;
                case 0x00000670ul >> 1: ppc_srawi(cpu); return 1;
                case 0x00000634ul >> 1: ppc_srad(cpu); return 1;
                case 0x00000630ul >> 1: ppc_sraw(cpu); return 1;
                case 0x0000062cul >> 1: ppc_lhbrx(cpu); return 1;
                case 0x00000264ul >> 1: ppc_tlbie(cpu); return 1;
                case 0x000005aeul >> 1: ppc_stfdx(cpu); return 1;
                case 0x000005aaul >> 1: ppc_stswi(cpu); return 1;
                case 0x0000056eul >> 1: ppc_stfsux(cpu); return 1;
                case 0x0000052eul >> 1: ppc_stfsx(cpu); return 1;
                case 0x0000052cul >> 1: ppc_stwbrx(cpu); return 1;
                case 0x0000052aul >> 1: ppc_stswx(cpu); return 1;
                case 0x00000526ul >> 1: ppc_mfsrin(cpu); return 1;
                case 0x000004eeul >> 1: ppc_lfdux(cpu); return 1;
                case 0x000004aeul >> 1: ppc_lfdx(cpu); return 1;
                case 0x000004acul >> 1: ppc_sync(cpu); return 1;
                case 0x000004aaul >> 1: ppc_lswi(cpu); return 1;
                case 0x000004a6ul >> 1: ppc_mfsr(cpu); return 1;
                case 0x0000046eul >> 1: ppc_lfsux(cpu); return 1;
                case 0x0000006cul >> 1: ppc_dcbst(cpu); return 1;
                // case 0x00000120ul >> 1: ppc_mtocrf(cpu); return 1;
                case 0x00000120ul >> 1: ppc_mtcrf(cpu); return 1;
                case 0x0000022eul >> 1: ppc_lhzx(cpu); return 1;
                case 0x000000f4ul >> 1: ppc_popcntb(cpu); return 1;
                case 0x000000eeul >> 1: ppc_lbzux(cpu); return 1;
                case 0x000000aeul >> 1: ppc_lbzx(cpu); return 1;
                case 0x000000acul >> 1: ppc_dcbf(cpu); return 1;
                case 0x000000a8ul >> 1: ppc_ldarx(cpu); return 1;
                case 0x000000a6ul >> 1: ppc_mfmsr(cpu); return 1;
                case 0x00000088ul >> 1: ppc_td(cpu); return 1;
                case 0x00000078ul >> 1: ppc_andc(cpu); return 1;
                case 0x00000074ul >> 1: ppc_cntlzd(cpu); return 1;
                case 0x0000006eul >> 1: ppc_lwzux(cpu); return 1;
                case 0x000000f8ul >> 1: ppc_nor(cpu); return 1;
                case 0x0000006aul >> 1: ppc_ldux(cpu); return 1;
                case 0x00000040ul >> 1: ppc_cmpl(cpu); return 1;
                case 0x00000038ul >> 1: ppc_and(cpu); return 1;
                case 0x00000036ul >> 1: ppc_sld(cpu); return 1;
                case 0x00000034ul >> 1: ppc_cntlzw(cpu); return 1;
                case 0x00000030ul >> 1: ppc_slw(cpu); return 1;
                case 0x0000002eul >> 1: ppc_lwzx(cpu); return 1;
                case 0x0000002aul >> 1: ppc_ldx(cpu); return 1;
                case 0x00000028ul >> 1: ppc_lwarx(cpu); return 1;
                // case 0x00000026ul >> 1: ppc_mfocrf(cpu); return 1;
                case 0x00000026ul >> 1: ppc_mfcr(cpu); return 1;
                case 0x00000008ul >> 1: ppc_tw(cpu); return 1;
                case 0x0000022cul >> 1: ppc_dcbt(cpu); return 1;
                case 0x000001aeul >> 1: ppc_stbx(cpu); return 1;
                case 0x000001acul >> 1: ppc_stdcx_u(cpu); return 1;
                case 0x000001a4ul >> 1: ppc_mtsr(cpu); return 1;
                case 0x000001e4ul >> 1: ppc_mtsrin(cpu); return 1;
                case 0x0000016eul >> 1: ppc_stwux(cpu); return 1;
                case 0x0000016aul >> 1: ppc_stdux(cpu); return 1;
                case 0x000001ecul >> 1: ppc_dcbtst(cpu); return 1;
                case 0x000001eeul >> 1: ppc_stbux(cpu); return 1;
                case 0x00000164ul >> 1: ppc_mtmsrd(cpu); return 1;
                case 0x0000012eul >> 1: ppc_stwx(cpu); return 1;
                case 0x0000012cul >> 1: ppc_stwcx_u(cpu); return 1;
                case 0x0000012aul >> 1: ppc_stdx(cpu); return 1;
                case 0x00000124ul >> 1: ppc_mtmsr(cpu); return 1;
            }
            switch ((cpu->opcode & 0x000007fc) >> 2) {
                case 0x00000674ul >> 2: ppc_sradi(cpu); return 1;
            }
            switch ((cpu->opcode & 0x000003fe) >> 1) {
                case 0x00000214ul >> 1: ppc_add(cpu); return 1;
                case 0x00000016ul >> 1: ppc_mulhwu(cpu); return 1;
                case 0x00000014ul >> 1: ppc_addc(cpu); return 1;
                case 0x000001d6ul >> 1: ppc_mullw(cpu); return 1;
                case 0x00000012ul >> 1: ppc_mulhdu(cpu); return 1;
                case 0x000001d4ul >> 1: ppc_addme(cpu); return 1;
                case 0x00000010ul >> 1: ppc_subfc(cpu); return 1;
                case 0x00000194ul >> 1: ppc_addze(cpu); return 1;
                case 0x000001d2ul >> 1: ppc_mulld(cpu); return 1;
                case 0x000001d0ul >> 1: ppc_subfme(cpu); return 1;
                case 0x00000050ul >> 1: ppc_subf(cpu); return 1;
                case 0x000003d6ul >> 1: ppc_divw(cpu); return 1;
                case 0x00000190ul >> 1: ppc_subfze(cpu); return 1;
                case 0x00000092ul >> 1: ppc_mulhd(cpu); return 1;
                case 0x00000096ul >> 1: ppc_mulhw(cpu); return 1;
                case 0x00000392ul >> 1: ppc_divdu(cpu); return 1;
                case 0x000000d0ul >> 1: ppc_neg(cpu); return 1;
                case 0x00000396ul >> 1: ppc_divwu(cpu); return 1;
                case 0x00000110ul >> 1: ppc_subfe(cpu); return 1;
                case 0x00000114ul >> 1: ppc_adde(cpu); return 1;
                case 0x000003d2ul >> 1: ppc_divd(cpu); return 1;
            }
        } break;
        case 0x80000000ul >> 26: ppc_lwz(cpu); return 1;
        case 0x84000000ul >> 26: ppc_lwzu(cpu); return 1;
        case 0x88000000ul >> 26: ppc_lbz(cpu); return 1;
        case 0x8c000000ul >> 26: ppc_lbzu(cpu); return 1;
        case 0x90000000ul >> 26: ppc_stw(cpu); return 1;
        case 0x94000000ul >> 26: ppc_stwu(cpu); return 1;
        case 0x98000000ul >> 26: ppc_stb(cpu); return 1;
        case 0x9c000000ul >> 26: ppc_stbu(cpu); return 1;
        case 0xa0000000ul >> 26: ppc_lhz(cpu); return 1;
        case 0xa4000000ul >> 26: ppc_lhzu(cpu); return 1;
        case 0xa8000000ul >> 26: ppc_lha(cpu); return 1;
        case 0xac000000ul >> 26: ppc_lhau(cpu); return 1;
        case 0xb0000000ul >> 26: ppc_sth(cpu); return 1;
        case 0xb4000000ul >> 26: ppc_sthu(cpu); return 1;
        case 0xb8000000ul >> 26: ppc_lmw(cpu); return 1;
        case 0xbc000000ul >> 26: ppc_stmw(cpu); return 1;
        case 0xc0000000ul >> 26: ppc_lfs(cpu); return 1;
        case 0xc4000000ul >> 26: ppc_lfsu(cpu); return 1;
        case 0xc8000000ul >> 26: ppc_lfd(cpu); return 1;
        case 0xcc000000ul >> 26: ppc_lfdu(cpu); return 1;
        case 0xd0000000ul >> 26: ppc_stfs(cpu); return 1;
        case 0xd4000000ul >> 26: ppc_stfsu(cpu); return 1;
        case 0xd8000000ul >> 26: ppc_stfd(cpu); return 1;
        case 0xdc000000ul >> 26: ppc_stfdu(cpu); return 1;
        case 0xe8000000ul >> 26: {
            switch ((cpu->opcode & 0x00000003) >> 0) {
                case 0x00000000ul >> 0: ppc_ld(cpu); return 1;
                case 0x00000001ul >> 0: ppc_ldu(cpu); return 1;
                case 0x00000002ul >> 0: ppc_lwa(cpu); return 1;
            }
        } break;
        case 0xec000000ul >> 26: {
            switch ((cpu->opcode & 0x0000003e) >> 1) {
                case 0x00000024ul >> 1: ppc_fdivs(cpu); return 1;
                case 0x00000028ul >> 1: ppc_fsubs(cpu); return 1;
                case 0x0000002aul >> 1: ppc_fadds(cpu); return 1;
                case 0x0000002cul >> 1: ppc_fsqrts(cpu); return 1;
                case 0x00000030ul >> 1: ppc_fres(cpu); return 1;
                case 0x00000032ul >> 1: ppc_fmuls(cpu); return 1;
                case 0x00000034ul >> 1: ppc_frsqrtes(cpu); return 1;
                case 0x00000038ul >> 1: ppc_fmsubs(cpu); return 1;
                case 0x0000003aul >> 1: ppc_fmadds(cpu); return 1;
                case 0x0000003cul >> 1: ppc_fnmsubs(cpu); return 1;
                case 0x0000003eul >> 1: ppc_fnmadds(cpu); return 1;
            }
        } break;
        case 0xf8000000ul >> 26: {
            switch ((cpu->opcode & 0x00000003) >> 0) {
                case 0x00000000ul >> 0: ppc_std(cpu); return 1;
                case 0x00000001ul >> 0: ppc_stdu(cpu); return 1;
            }
        } break;
        case 0xfc000000ul >> 26: {
            switch ((cpu->opcode & 0x000007fe) >> 1) {
                case 0x00000018ul >> 1: ppc_frsp(cpu); return 1;
                case 0x0000069cul >> 1: ppc_fcfid(cpu); return 1;
                case 0x0000065eul >> 1: ppc_fctidz(cpu); return 1;
                case 0x0000065cul >> 1: ppc_fctid(cpu); return 1;
                case 0x0000058eul >> 1: ppc_mtfsf(cpu); return 1;
                case 0x0000048eul >> 1: ppc_mffs(cpu); return 1;
                case 0x00000210ul >> 1: ppc_fabs(cpu); return 1;
                case 0x00000110ul >> 1: ppc_fnabs(cpu); return 1;
                case 0x0000010cul >> 1: ppc_mtfsfi(cpu); return 1;
                case 0x00000090ul >> 1: ppc_fmr(cpu); return 1;
                case 0x0000008cul >> 1: ppc_mtfsb0(cpu); return 1;
                case 0x00000080ul >> 1: ppc_mcrfs(cpu); return 1;
                case 0x00000050ul >> 1: ppc_fneg(cpu); return 1;
                case 0x0000004cul >> 1: ppc_mtfsb1(cpu); return 1;
                case 0x00000040ul >> 1: ppc_fcmpo(cpu); return 1;
                case 0x0000001eul >> 1: ppc_fctiwz(cpu); return 1;
                case 0x0000001cul >> 1: ppc_fctiw(cpu); return 1;
                case 0x00000000ul >> 1: ppc_fcmpu(cpu); return 1;
            }
            switch ((cpu->opcode & 0x0000003e) >> 1) {
                case 0x0000003eul >> 1: ppc_fnmadd(cpu); return 1;
                case 0x0000003cul >> 1: ppc_fnmsub(cpu); return 1;
                case 0x0000003aul >> 1: ppc_fmadd(cpu); return 1;
                case 0x00000038ul >> 1: ppc_fmsub(cpu); return 1;
                case 0x00000034ul >> 1: ppc_frsqrte(cpu); return 1;
                case 0x00000032ul >> 1: ppc_fmul(cpu); return 1;
                case 0x00000030ul >> 1: ppc_fre(cpu); return 1;
                case 0x0000002eul >> 1: ppc_fsel(cpu); return 1;
                case 0x0000002cul >> 1: ppc_fsqrt(cpu); return 1;
                case 0x0000002aul >> 1: ppc_fadd(cpu); return 1;
                case 0x00000028ul >> 1: ppc_fsub(cpu); return 1;
                case 0x00000024ul >> 1: ppc_fdiv(cpu); return 1;
            }
        } break;
    }

    return 0;
}

void ppc_destroy(struct ppc_state* cpu) {
    free(cpu);
}