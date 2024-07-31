#include <stdlib.h>
#include <stdio.h>

#include "ppc.h"

#define SE64(v, s) (((int64_t)((uint64_t)v << (63 - s))) >> (63 - s))
#define ROTL64(v, s) ((v << s) | (v >> (64 - s)))

#define PPC_XFX_SPR (((cpu->opcode >> 16) & 0x1f) | ((cpu->opcode >> 6) & 0x3e0))
#define PPC_XFX_RT ((cpu->opcode >> 21) & 0x1f)
#define PPC_DS_RS ((cpu->opcode >> 21) & 0x1f)
#define PPC_DS_RA ((cpu->opcode >> 16) & 0x1f)
#define PPC_DS_DS ((cpu->opcode >> 2) & 0x3fff)
#define PPC_X_RS ((cpu->opcode >> 21) & 0x1f)
#define PPC_X_RA ((cpu->opcode >> 16) & 0x1f)
#define PPC_X_RB ((cpu->opcode >> 11) & 0x1f)
#define PPC_X_RC (cpu->opcode & 1)
#define PPC_I_LI ((cpu->opcode & 0x3fffffc) >> 2)
#define PPC_I_AA ((cpu->opcode & 2) >> 1)
#define PPC_I_LK (cpu->opcode & 1)
#define PPC_D_RT ((cpu->opcode >> 21) & 0x1f)
#define PPC_D_RS ((cpu->opcode >> 21) & 0x1f)
#define PPC_D_RA ((cpu->opcode >> 16) & 0x1f)
#define PPC_D_D (cpu->opcode & 0xffff)
#define PPC_D_SI (cpu->opcode & 0xffff)
#define PPC_D_UI (cpu->opcode & 0xffff)
#define PPC_D_L (cpu->opcode & 0x200000)
#define PPC_D_BF ((cpu->opcode >> 23) & 7)
#define PPC_B_BO ((cpu->opcode >> 21) & 0x1f)
#define PPC_B_BI ((cpu->opcode >> 16) & 0x1f)
#define PPC_B_BD ((cpu->opcode & 0xfffc) >> 2)
#define PPC_B_AA ((cpu->opcode & 2) >> 1)
#define PPC_B_LK (cpu->opcode & 1)
#define PPC_MD_RS ((cpu->opcode >> 21) & 0x1f)
#define PPC_MD_RA ((cpu->opcode >> 16) & 0x1f)
#define PPC_MD_SH (((cpu->opcode >> 11) & 0x1f) | ((cpu->opcode & 2) << 4))
#define PPC_MD_MB (((cpu->opcode >> 6) & 0x1f) | ((cpu->opcode & 0x20)))
#define PPC_MD_RC (cpu->opcode & 1)

static inline void ppc_update_cr0(struct ppc_state* cpu, uint64_t v) {
    cpu->cr0 =
        ((((int64_t)v) < 0) ? 4 : 0) |
        ((((int64_t)v) > 0) ? 2 : 0) |
        ((((int64_t)v) == 0) ? 1 : 0) |
        ((cpu->xer & 0x80000000) ? 8 : 0);
}

static inline int ppc_test_branch(struct ppc_state* cpu) {
    /*
    BO      Description
    0000z   Decrement the CTR, then branch if the decremented CTR M:63:0 and CRBI=0
    0001z   Decrement the CTR, then branch if the decremented CTR M:63=0 and CR BI=0
    001at   Branch if CRBI =0
    0100z   Decrement the CTR, then branch if the decremented CTR M:63:0 and CRBI=1
    0101z   Decrement the CTR, then branch if the decremented CTR M:63=0 and CR BI=1
    011at   Branch if CR BI =1
    1a00t   Decrement the CTR, then branch if the decremented CTR M:63:0
    1a01t   Decrement the CTR, then branch if the decremented CTR M:63=0
    1z1zz   Branch always
    */

    switch (PPC_B_BO >> 1) {
        case 0: --cpu->ctr; return ( cpu->ctr) && !(cpu->cr & (1 << PPC_B_BI));
        case 1: --cpu->ctr; return (!cpu->ctr) && !(cpu->cr & (1 << PPC_B_BI));
        case 2: case 3: return !(cpu->cr & (1 << PPC_B_BI));
        case 4: --cpu->ctr; return ( cpu->ctr) && (cpu->cr & (1 << PPC_B_BI));
        case 5: --cpu->ctr; return (!cpu->ctr) && (cpu->cr & (1 << PPC_B_BI));
        case 6: case 7: return !!(cpu->cr & (1 << PPC_B_BI));
        case 8: case 12: --cpu->ctr; return !!cpu->ctr;
        case 9: case 13: --cpu->ctr; return !cpu->ctr;
        default: return 1;
    }

    return 1;
}
void ppc_adde(struct ppc_state* cpu) { puts("ppc: adde unimplemented"); exit(1); }
void ppc_add(struct ppc_state* cpu) { puts("ppc: add unimplemented"); exit(1); }
void ppc_and(struct ppc_state* cpu) { puts("ppc: and unimplemented"); exit(1); }
void ppc_addc(struct ppc_state* cpu) { puts("ppc: addc unimplemented"); exit(1); }
void ppc_andc(struct ppc_state* cpu) { puts("ppc: andc unimplemented"); exit(1); }
void ppc_addme(struct ppc_state* cpu) { puts("ppc: addme unimplemented"); exit(1); }
void ppc_addis(struct ppc_state* cpu) {
    int t = PPC_D_RT;
    int a = PPC_D_RA;

    cpu->r[t] = SE64(PPC_D_SI << 16, 31);

    if (a) cpu->r[t] += cpu->r[a];
}
void ppc_addi(struct ppc_state* cpu) {
    int t = PPC_D_RT;
    int a = PPC_D_RA;

    cpu->r[t] = SE64(PPC_D_SI, 15);

    if (a) cpu->r[t] += cpu->r[a];
}
void ppc_addic_u(struct ppc_state* cpu) { puts("ppc: addic_u unimplemented"); exit(1); }
void ppc_addic(struct ppc_state* cpu) { puts("ppc: addic unimplemented"); exit(1); }
void ppc_andi_u(struct ppc_state* cpu) { puts("ppc: andi_u unimplemented"); exit(1); }
void ppc_addze(struct ppc_state* cpu) { puts("ppc: addze unimplemented"); exit(1); }
void ppc_andis_u(struct ppc_state* cpu) { puts("ppc: andis_u unimplemented"); exit(1); }
void ppc_bc(struct ppc_state* cpu) {
    if (PPC_B_LK) cpu->lr = cpu->pc;

    if (!ppc_test_branch(cpu))
        return;

    cpu->pc = (PPC_B_AA ? 0 : (cpu->pc - 4)) + SE64(PPC_B_BD << 2, 15);
}
void ppc_b(struct ppc_state* cpu) {
    if (PPC_I_LK) cpu->lr = cpu->pc;

    cpu->pc = (PPC_I_AA ? 0 : (cpu->pc - 4)) + SE64(PPC_I_LI << 2, 25);
}
void ppc_bclr(struct ppc_state* cpu) {
    uint64_t old_pc = cpu->pc;

    if (!ppc_test_branch(cpu))
        return;

    cpu->pc = cpu->lr;

    if (PPC_B_LK) cpu->lr = old_pc;
}
void ppc_bcctr(struct ppc_state* cpu) { puts("ppc: bcctr unimplemented"); exit(1); }
void ppc_cmp(struct ppc_state* cpu) { puts("ppc: cmp unimplemented"); exit(1); }
void ppc_cntlzw(struct ppc_state* cpu) { puts("ppc: cntlzw unimplemented"); exit(1); }
void ppc_cmpl(struct ppc_state* cpu) { puts("ppc: cmpl unimplemented"); exit(1); }
void ppc_crxor(struct ppc_state* cpu) { puts("ppc: crxor unimplemented"); exit(1); }
void ppc_cror(struct ppc_state* cpu) { puts("ppc: cror unimplemented"); exit(1); }
void ppc_crorc(struct ppc_state* cpu) { puts("ppc: crorc unimplemented"); exit(1); }
void ppc_creqv(struct ppc_state* cpu) { puts("ppc: creqv unimplemented"); exit(1); }
void ppc_crand(struct ppc_state* cpu) { puts("ppc: crand unimplemented"); exit(1); }
void ppc_crnand(struct ppc_state* cpu) { puts("ppc: crnand unimplemented"); exit(1); }
void ppc_crandc(struct ppc_state* cpu) { puts("ppc: crandc unimplemented"); exit(1); }
void ppc_crnor(struct ppc_state* cpu) { puts("ppc: crnor unimplemented"); exit(1); }
void ppc_cntlzd(struct ppc_state* cpu) { puts("ppc: cntlzd unimplemented"); exit(1); }
void ppc_cmpi(struct ppc_state* cpu) {
    int64_t a = cpu->r[PPC_D_RA];
    int64_t si = SE64(PPC_D_SI, 15);

    if (PPC_D_L) a = SE64(a, 31);

    uint64_t cr =
        ((a < si) ? 4 : 0) |
        ((a > si) ? 2 : 0) |
        ((a == si) ? 1 : 0) |
        ((cpu->xer & 0x80000000) ? 8 : 0);

    cpu->cr &= ~(0xful << (PPC_D_BF << 2));
    cpu->cr |= cr << (PPC_D_BF << 2);
}
void ppc_cmpli(struct ppc_state* cpu) { puts("ppc: cmpli unimplemented"); exit(1); }
void ppc_dcbst(struct ppc_state* cpu) { puts("ppc: dcbst unimplemented"); exit(1); }
void ppc_divd(struct ppc_state* cpu) { puts("ppc: divd unimplemented"); exit(1); }
void ppc_dcbtst(struct ppc_state* cpu) { puts("ppc: dcbtst unimplemented"); exit(1); }
void ppc_dcbt(struct ppc_state* cpu) { puts("ppc: dcbt unimplemented"); exit(1); }
void ppc_dcbf(struct ppc_state* cpu) { puts("ppc: dcbf unimplemented"); exit(1); }
void ppc_divdu(struct ppc_state* cpu) { puts("ppc: divdu unimplemented"); exit(1); }
void ppc_divwu(struct ppc_state* cpu) { puts("ppc: divwu unimplemented"); exit(1); }
void ppc_divw(struct ppc_state* cpu) { puts("ppc: divw unimplemented"); exit(1); }
void ppc_dcbz(struct ppc_state* cpu) { puts("ppc: dcbz unimplemented"); exit(1); }
void ppc_ecowx(struct ppc_state* cpu) { puts("ppc: ecowx unimplemented"); exit(1); }
void ppc_eieio(struct ppc_state* cpu) { puts("ppc: eieio unimplemented"); exit(1); }
void ppc_extsh(struct ppc_state* cpu) { puts("ppc: extsh unimplemented"); exit(1); }
void ppc_extsw(struct ppc_state* cpu) { puts("ppc: extsw unimplemented"); exit(1); }
void ppc_extsb(struct ppc_state* cpu) { puts("ppc: extsb unimplemented"); exit(1); }
void ppc_eqv(struct ppc_state* cpu) { puts("ppc: eqv unimplemented"); exit(1); }
void ppc_eciwx(struct ppc_state* cpu) { puts("ppc: eciwx unimplemented"); exit(1); }
void ppc_frsqrte(struct ppc_state* cpu) { puts("ppc: frsqrte unimplemented"); exit(1); }
void ppc_fadds(struct ppc_state* cpu) { puts("ppc: fadds unimplemented"); exit(1); }
void ppc_fsubs(struct ppc_state* cpu) { puts("ppc: fsubs unimplemented"); exit(1); }
void ppc_fdivs(struct ppc_state* cpu) { puts("ppc: fdivs unimplemented"); exit(1); }
void ppc_fre(struct ppc_state* cpu) { puts("ppc: fre unimplemented"); exit(1); }
void ppc_fmul(struct ppc_state* cpu) { puts("ppc: fmul unimplemented"); exit(1); }
void ppc_fsqrts(struct ppc_state* cpu) { puts("ppc: fsqrts unimplemented"); exit(1); }
void ppc_fmsub(struct ppc_state* cpu) { puts("ppc: fmsub unimplemented"); exit(1); }
void ppc_fmadd(struct ppc_state* cpu) { puts("ppc: fmadd unimplemented"); exit(1); }
void ppc_fnmsub(struct ppc_state* cpu) { puts("ppc: fnmsub unimplemented"); exit(1); }
void ppc_fnmadd(struct ppc_state* cpu) { puts("ppc: fnmadd unimplemented"); exit(1); }
void ppc_fcmpo(struct ppc_state* cpu) { puts("ppc: fcmpo unimplemented"); exit(1); }
void ppc_fcfid(struct ppc_state* cpu) { puts("ppc: fcfid unimplemented"); exit(1); }
void ppc_fsel(struct ppc_state* cpu) { puts("ppc: fsel unimplemented"); exit(1); }
void ppc_fnmadds(struct ppc_state* cpu) { puts("ppc: fnmadds unimplemented"); exit(1); }
void ppc_fres(struct ppc_state* cpu) { puts("ppc: fres unimplemented"); exit(1); }
void ppc_fmuls(struct ppc_state* cpu) { puts("ppc: fmuls unimplemented"); exit(1); }
void ppc_frsqrtes(struct ppc_state* cpu) { puts("ppc: frsqrtes unimplemented"); exit(1); }
void ppc_fmsubs(struct ppc_state* cpu) { puts("ppc: fmsubs unimplemented"); exit(1); }
void ppc_fsqrt(struct ppc_state* cpu) { puts("ppc: fsqrt unimplemented"); exit(1); }
void ppc_fadd(struct ppc_state* cpu) { puts("ppc: fadd unimplemented"); exit(1); }
void ppc_fsub(struct ppc_state* cpu) { puts("ppc: fsub unimplemented"); exit(1); }
void ppc_fdiv(struct ppc_state* cpu) { puts("ppc: fdiv unimplemented"); exit(1); }
void ppc_fctiwz(struct ppc_state* cpu) { puts("ppc: fctiwz unimplemented"); exit(1); }
void ppc_fmadds(struct ppc_state* cpu) { puts("ppc: fmadds unimplemented"); exit(1); }
void ppc_fctiw(struct ppc_state* cpu) { puts("ppc: fctiw unimplemented"); exit(1); }
void ppc_frsp(struct ppc_state* cpu) { puts("ppc: frsp unimplemented"); exit(1); }
void ppc_fnmsubs(struct ppc_state* cpu) { puts("ppc: fnmsubs unimplemented"); exit(1); }
void ppc_fcmpu(struct ppc_state* cpu) { puts("ppc: fcmpu unimplemented"); exit(1); }
void ppc_fneg(struct ppc_state* cpu) { puts("ppc: fneg unimplemented"); exit(1); }
void ppc_fctid(struct ppc_state* cpu) { puts("ppc: fctid unimplemented"); exit(1); }
void ppc_fmr(struct ppc_state* cpu) { puts("ppc: fmr unimplemented"); exit(1); }
void ppc_fctidz(struct ppc_state* cpu) { puts("ppc: fctidz unimplemented"); exit(1); }
void ppc_fabs(struct ppc_state* cpu) { puts("ppc: fabs unimplemented"); exit(1); }
void ppc_fnabs(struct ppc_state* cpu) { puts("ppc: fnabs unimplemented"); exit(1); }
void ppc_hrfid(struct ppc_state* cpu) { puts("ppc: hrfid unimplemented"); exit(1); }
void ppc_isync(struct ppc_state* cpu) { puts("ppc: isync unimplemented"); exit(1); }
void ppc_icbi(struct ppc_state* cpu) { puts("ppc: icbi unimplemented"); exit(1); }
void ppc_ldx(struct ppc_state* cpu) { puts("ppc: ldx unimplemented"); exit(1); }
void ppc_lswx(struct ppc_state* cpu) { puts("ppc: lswx unimplemented"); exit(1); }
void ppc_lhzux(struct ppc_state* cpu) { puts("ppc: lhzux unimplemented"); exit(1); }
void ppc_lwarx(struct ppc_state* cpu) { puts("ppc: lwarx unimplemented"); exit(1); }
void ppc_lwax(struct ppc_state* cpu) { puts("ppc: lwax unimplemented"); exit(1); }
void ppc_lhax(struct ppc_state* cpu) { puts("ppc: lhax unimplemented"); exit(1); }
void ppc_ld(struct ppc_state* cpu) {
    uint64_t a = PPC_DS_RA;
    uint64_t ds = PPC_DS_DS;
    uint64_t ea = (!!a) * cpu->r[a];

    cpu->r[PPC_DS_RS] = ppc_read64(cpu, ea + (SE64(PPC_DS_DS, 13) << 2));
}
void ppc_ldu(struct ppc_state* cpu) { puts("ppc: ldu unimplemented"); exit(1); }
void ppc_lwaux(struct ppc_state* cpu) { puts("ppc: lwaux unimplemented"); exit(1); }
void ppc_lhaux(struct ppc_state* cpu) { puts("ppc: lhaux unimplemented"); exit(1); }
void ppc_lwa(struct ppc_state* cpu) { puts("ppc: lwa unimplemented"); exit(1); }
void ppc_lhbrx(struct ppc_state* cpu) { puts("ppc: lhbrx unimplemented"); exit(1); }
void ppc_lfdux(struct ppc_state* cpu) { puts("ppc: lfdux unimplemented"); exit(1); }
void ppc_lfdx(struct ppc_state* cpu) { puts("ppc: lfdx unimplemented"); exit(1); }
void ppc_lswi(struct ppc_state* cpu) { puts("ppc: lswi unimplemented"); exit(1); }
void ppc_lfsux(struct ppc_state* cpu) { puts("ppc: lfsux unimplemented"); exit(1); }
void ppc_lfsx(struct ppc_state* cpu) { puts("ppc: lfsx unimplemented"); exit(1); }
void ppc_lwbrx(struct ppc_state* cpu) { puts("ppc: lwbrx unimplemented"); exit(1); }
void ppc_lhzx(struct ppc_state* cpu) { puts("ppc: lhzx unimplemented"); exit(1); }
void ppc_lmw(struct ppc_state* cpu) { puts("ppc: lmw unimplemented"); exit(1); }
void ppc_lfs(struct ppc_state* cpu) { puts("ppc: lfs unimplemented"); exit(1); }
void ppc_lhau(struct ppc_state* cpu) { puts("ppc: lhau unimplemented"); exit(1); }
void ppc_lha(struct ppc_state* cpu) { puts("ppc: lha unimplemented"); exit(1); }
void ppc_lhzu(struct ppc_state* cpu) { puts("ppc: lhzu unimplemented"); exit(1); }
void ppc_lhz(struct ppc_state* cpu) { puts("ppc: lhz unimplemented"); exit(1); }
void ppc_lbzux(struct ppc_state* cpu) { puts("ppc: lbzux unimplemented"); exit(1); }
void ppc_lfsu(struct ppc_state* cpu) { puts("ppc: lfsu unimplemented"); exit(1); }
void ppc_lbzx(struct ppc_state* cpu) { puts("ppc: lbzx unimplemented"); exit(1); }
void ppc_ldarx(struct ppc_state* cpu) { puts("ppc: ldarx unimplemented"); exit(1); }
void ppc_lfd(struct ppc_state* cpu) { puts("ppc: lfd unimplemented"); exit(1); }
void ppc_lfdu(struct ppc_state* cpu) { puts("ppc: lfdu unimplemented"); exit(1); }
void ppc_lbzu(struct ppc_state* cpu) { puts("ppc: lbzu unimplemented"); exit(1); }
void ppc_lbz(struct ppc_state* cpu) {
    uint64_t a = PPC_D_RA;
    uint64_t ea = (!!a) * cpu->r[a];

    cpu->r[PPC_D_RT] = ppc_read8(cpu, ea + SE64(PPC_D_D, 15));
}
void ppc_lwzu(struct ppc_state* cpu) { puts("ppc: lwzu unimplemented"); exit(1); }
void ppc_lwz(struct ppc_state* cpu) { puts("ppc: lwz unimplemented"); exit(1); }
void ppc_lwzux(struct ppc_state* cpu) { puts("ppc: lwzux unimplemented"); exit(1); }
void ppc_ldux(struct ppc_state* cpu) { puts("ppc: ldux unimplemented"); exit(1); }
void ppc_lwzx(struct ppc_state* cpu) { puts("ppc: lwzx unimplemented"); exit(1); }
void ppc_mfsrin(struct ppc_state* cpu) { puts("ppc: mfsrin unimplemented"); exit(1); }
void ppc_mulli(struct ppc_state* cpu) { puts("ppc: mulli unimplemented"); exit(1); }
void ppc_mtfsf(struct ppc_state* cpu) { puts("ppc: mtfsf unimplemented"); exit(1); }
void ppc_mfsr(struct ppc_state* cpu) { puts("ppc: mfsr unimplemented"); exit(1); }
void ppc_mcrf(struct ppc_state* cpu) { puts("ppc: mcrf unimplemented"); exit(1); }
void ppc_mffs(struct ppc_state* cpu) { puts("ppc: mffs unimplemented"); exit(1); }
void ppc_mtcrf(struct ppc_state* cpu) { puts("ppc: mtcrf unimplemented"); exit(1); }
void ppc_mulld(struct ppc_state* cpu) { puts("ppc: mulld unimplemented"); exit(1); }
void ppc_mfcr(struct ppc_state* cpu) { puts("ppc: mfcr unimplemented"); exit(1); }
void ppc_mftb(struct ppc_state* cpu) { puts("ppc: mftb unimplemented"); exit(1); }
void ppc_mfocrf(struct ppc_state* cpu) { puts("ppc: mfocrf unimplemented"); exit(1); }
void ppc_mcrxr(struct ppc_state* cpu) { puts("ppc: mcrxr unimplemented"); exit(1); }
void ppc_mtfsfi(struct ppc_state* cpu) { puts("ppc: mtfsfi unimplemented"); exit(1); }
void ppc_mtfsb0(struct ppc_state* cpu) { puts("ppc: mtfsb0 unimplemented"); exit(1); }
void ppc_mtsrin(struct ppc_state* cpu) { puts("ppc: mtsrin unimplemented"); exit(1); }
void ppc_mullw(struct ppc_state* cpu) { puts("ppc: mullw unimplemented"); exit(1); }
void ppc_mcrfs(struct ppc_state* cpu) { puts("ppc: mcrfs unimplemented"); exit(1); }
void ppc_mfspr(struct ppc_state* cpu) {
    uint64_t spr = PPC_XFX_SPR;

    switch (spr) {
        case 1: spr = cpu->xer; break;
        case 8: spr = cpu->lr; break;
        case 9: spr = cpu->ctr; break;
    }

    cpu->r[PPC_XFX_RT] = spr;
}
void ppc_mulhd(struct ppc_state* cpu) { puts("ppc: mulhd unimplemented"); exit(1); }
void ppc_mulhw(struct ppc_state* cpu) { puts("ppc: mulhw unimplemented"); exit(1); }
void ppc_mfmsr(struct ppc_state* cpu) { puts("ppc: mfmsr unimplemented"); exit(1); }
void ppc_mtsr(struct ppc_state* cpu) { puts("ppc: mtsr unimplemented"); exit(1); }
void ppc_mtmsrd(struct ppc_state* cpu) { puts("ppc: mtmsrd unimplemented"); exit(1); }
void ppc_mtfsb1(struct ppc_state* cpu) { puts("ppc: mtfsb1 unimplemented"); exit(1); }
void ppc_mtmsr(struct ppc_state* cpu) { puts("ppc: mtmsr unimplemented"); exit(1); }
void ppc_mtocrf(struct ppc_state* cpu) { puts("ppc: mtocrf unimplemented"); exit(1); }
void ppc_mulhwu(struct ppc_state* cpu) { puts("ppc: mulhwu unimplemented"); exit(1); }
void ppc_mulhdu(struct ppc_state* cpu) { puts("ppc: mulhdu unimplemented"); exit(1); }
void ppc_mtspr(struct ppc_state* cpu) { puts("ppc: mtspr unimplemented"); exit(1); }
void ppc_nor(struct ppc_state* cpu) { puts("ppc: nor unimplemented"); exit(1); }
void ppc_neg(struct ppc_state* cpu) { puts("ppc: neg unimplemented"); exit(1); }
void ppc_nand(struct ppc_state* cpu) { puts("ppc: nand unimplemented"); exit(1); }
void ppc_orc(struct ppc_state* cpu) { puts("ppc: orc unimplemented"); exit(1); }
void ppc_ori(struct ppc_state* cpu) {
    cpu->r[PPC_D_RA] = cpu->r[PPC_D_RS] | PPC_D_UI;
}
void ppc_oris(struct ppc_state* cpu) { puts("ppc: oris unimplemented"); exit(1); }
void ppc_or(struct ppc_state* cpu) {
    int a = PPC_X_RA;

    cpu->r[a] = cpu->r[PPC_X_RS] | cpu->r[PPC_X_RB];

    if (PPC_X_RC) {
        ppc_update_cr0(cpu, cpu->r[a]);
    }
}
void ppc_popcntb(struct ppc_state* cpu) { puts("ppc: popcntb unimplemented"); exit(1); }
void ppc_rfid(struct ppc_state* cpu) { puts("ppc: rfid unimplemented"); exit(1); }
void ppc_rldicl(struct ppc_state* cpu) {
    uint64_t m = 0xffffffffffffffff >> PPC_MD_MB;
    int a = PPC_MD_RA;

    cpu->r[a] = ROTL64(cpu->r[PPC_MD_RS], PPC_MD_SH) & m;

    if (PPC_MD_RC)
        ppc_update_cr0(cpu, cpu->r[a]);
}
void ppc_rldicr(struct ppc_state* cpu) { puts("ppc: rldicr unimplemented"); exit(1); }
void ppc_rldic(struct ppc_state* cpu) { puts("ppc: rldic unimplemented"); exit(1); }
void ppc_rldimi(struct ppc_state* cpu) { puts("ppc: rldimi unimplemented"); exit(1); }
void ppc_rldcl(struct ppc_state* cpu) { puts("ppc: rldcl unimplemented"); exit(1); }
void ppc_rldcr(struct ppc_state* cpu) { puts("ppc: rldcr unimplemented"); exit(1); }
void ppc_rlwnm(struct ppc_state* cpu) { puts("ppc: rlwnm unimplemented"); exit(1); }
void ppc_rlwinm(struct ppc_state* cpu) { puts("ppc: rlwinm unimplemented"); exit(1); }
void ppc_rlwimi(struct ppc_state* cpu) { puts("ppc: rlwimi unimplemented"); exit(1); }
void ppc_sradi(struct ppc_state* cpu) { puts("ppc: sradi unimplemented"); exit(1); }
void ppc_stfs(struct ppc_state* cpu) { puts("ppc: stfs unimplemented"); exit(1); }
void ppc_stfsu(struct ppc_state* cpu) { puts("ppc: stfsu unimplemented"); exit(1); }
void ppc_stfd(struct ppc_state* cpu) { puts("ppc: stfd unimplemented"); exit(1); }
void ppc_stfdu(struct ppc_state* cpu) { puts("ppc: stfdu unimplemented"); exit(1); }
void ppc_std(struct ppc_state* cpu) {
    uint64_t s = PPC_DS_RS;
    uint64_t a = PPC_DS_RA;
    uint64_t ds = PPC_DS_DS;
    uint64_t ea = (!!a) * cpu->r[a];

    ppc_write64(cpu, ea + (SE64(PPC_DS_DS, 13) << 2), cpu->r[s]);
}
void ppc_stmw(struct ppc_state* cpu) { puts("ppc: stmw unimplemented"); exit(1); }
void ppc_stdu(struct ppc_state* cpu) {
    uint64_t s = PPC_DS_RS;
    uint64_t a = PPC_DS_RA;
    uint64_t ds = SE64(PPC_DS_DS, 13) << 2;
    uint64_t ea = cpu->r[a] + (SE64(PPC_DS_DS, 13) << 2);

    ppc_write64(cpu, ea + (SE64(PPC_DS_DS, 13) << 2), cpu->r[s]);

    cpu->r[a] = ea;
}
void ppc_stdcx_u(struct ppc_state* cpu) { puts("ppc: stdcx_u unimplemented"); exit(1); }
void ppc_slbia(struct ppc_state* cpu) { puts("ppc: slbia unimplemented"); exit(1); }
void ppc_sthux(struct ppc_state* cpu) { puts("ppc: sthux unimplemented"); exit(1); }
void ppc_slbie(struct ppc_state* cpu) { puts("ppc: slbie unimplemented"); exit(1); }
void ppc_subfic(struct ppc_state* cpu) { puts("ppc: subfic unimplemented"); exit(1); }
void ppc_sthx(struct ppc_state* cpu) { puts("ppc: sthx unimplemented"); exit(1); }
void ppc_slbmte(struct ppc_state* cpu) { puts("ppc: slbmte unimplemented"); exit(1); }
void ppc_sc(struct ppc_state* cpu) { puts("ppc: sc unimplemented"); exit(1); }
void ppc_subfc(struct ppc_state* cpu) { puts("ppc: subfc unimplemented"); exit(1); }
void ppc_stbux(struct ppc_state* cpu) { puts("ppc: stbux unimplemented"); exit(1); }
void ppc_subfme(struct ppc_state* cpu) { puts("ppc: subfme unimplemented"); exit(1); }
void ppc_stbx(struct ppc_state* cpu) { puts("ppc: stbx unimplemented"); exit(1); }
void ppc_srw(struct ppc_state* cpu) { puts("ppc: srw unimplemented"); exit(1); }
void ppc_subfze(struct ppc_state* cpu) { puts("ppc: subfze unimplemented"); exit(1); }
void ppc_stwux(struct ppc_state* cpu) { puts("ppc: stwux unimplemented"); exit(1); }
void ppc_stdux(struct ppc_state* cpu) { puts("ppc: stdux unimplemented"); exit(1); }
void ppc_stwx(struct ppc_state* cpu) { puts("ppc: stwx unimplemented"); exit(1); }
void ppc_stwcx_u(struct ppc_state* cpu) { puts("ppc: stwcx_u unimplemented"); exit(1); }
void ppc_stdx(struct ppc_state* cpu) { puts("ppc: stdx unimplemented"); exit(1); }
void ppc_subfe(struct ppc_state* cpu) { puts("ppc: subfe unimplemented"); exit(1); }
void ppc_slw(struct ppc_state* cpu) { puts("ppc: slw unimplemented"); exit(1); }
void ppc_subf(struct ppc_state* cpu) { puts("ppc: subf unimplemented"); exit(1); }
void ppc_sld(struct ppc_state* cpu) { puts("ppc: sld unimplemented"); exit(1); }
void ppc_sraw(struct ppc_state* cpu) { puts("ppc: sraw unimplemented"); exit(1); }
void ppc_sth(struct ppc_state* cpu) { puts("ppc: sth unimplemented"); exit(1); }
void ppc_stbu(struct ppc_state* cpu) { puts("ppc: stbu unimplemented"); exit(1); }
void ppc_stb(struct ppc_state* cpu) {
    uint64_t a = PPC_D_RA;
    uint64_t ea = (!!a) * cpu->r[a];

    ppc_write8(cpu, ea + (SE64(PPC_D_D, 15)), cpu->r[PPC_D_RS]);
}
void ppc_stwu(struct ppc_state* cpu) { puts("ppc: stwu unimplemented"); exit(1); }
void ppc_stw(struct ppc_state* cpu) { puts("ppc: stw unimplemented"); exit(1); }
void ppc_stfiwx(struct ppc_state* cpu) { puts("ppc: stfiwx unimplemented"); exit(1); }
void ppc_sthbrx(struct ppc_state* cpu) { puts("ppc: sthbrx unimplemented"); exit(1); }
void ppc_slbmfee(struct ppc_state* cpu) { puts("ppc: slbmfee unimplemented"); exit(1); }
void ppc_slbmfev(struct ppc_state* cpu) { puts("ppc: slbmfev unimplemented"); exit(1); }
void ppc_srawi(struct ppc_state* cpu) { puts("ppc: srawi unimplemented"); exit(1); }
void ppc_srad(struct ppc_state* cpu) { puts("ppc: srad unimplemented"); exit(1); }
void ppc_stfdux(struct ppc_state* cpu) { puts("ppc: stfdux unimplemented"); exit(1); }
void ppc_stfdx(struct ppc_state* cpu) { puts("ppc: stfdx unimplemented"); exit(1); }
void ppc_stswi(struct ppc_state* cpu) { puts("ppc: stswi unimplemented"); exit(1); }
void ppc_stfsux(struct ppc_state* cpu) { puts("ppc: stfsux unimplemented"); exit(1); }
void ppc_stfsx(struct ppc_state* cpu) { puts("ppc: stfsx unimplemented"); exit(1); }
void ppc_stwbrx(struct ppc_state* cpu) { puts("ppc: stwbrx unimplemented"); exit(1); }
void ppc_stswx(struct ppc_state* cpu) { puts("ppc: stswx unimplemented"); exit(1); }
void ppc_sync(struct ppc_state* cpu) { puts("ppc: sync unimplemented"); exit(1); }
void ppc_sthu(struct ppc_state* cpu) { puts("ppc: sthu unimplemented"); exit(1); }
void ppc_srd(struct ppc_state* cpu) { puts("ppc: srd unimplemented"); exit(1); }
void ppc_twi(struct ppc_state* cpu) { puts("ppc: twi unimplemented"); exit(1); }
void ppc_tw(struct ppc_state* cpu) { puts("ppc: tw unimplemented"); exit(1); }
void ppc_td(struct ppc_state* cpu) { puts("ppc: td unimplemented"); exit(1); }
void ppc_tlbie(struct ppc_state* cpu) { puts("ppc: tlbie unimplemented"); exit(1); }
void ppc_tlbia(struct ppc_state* cpu) { puts("ppc: tlbia unimplemented"); exit(1); }
void ppc_tdi(struct ppc_state* cpu) { puts("ppc: tdi unimplemented"); exit(1); }
void ppc_tlbsync(struct ppc_state* cpu) { puts("ppc: tlbsync unimplemented"); exit(1); }
void ppc_xor(struct ppc_state* cpu) { puts("ppc: xor unimplemented"); exit(1); }
void ppc_xoris(struct ppc_state* cpu) { puts("ppc: xoris unimplemented"); exit(1); }
void ppc_xori(struct ppc_state* cpu) { puts("ppc: xori unimplemented"); exit(1); }