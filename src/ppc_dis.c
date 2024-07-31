#include <stdint.h>

#include "ppc_dis.h"

#include <stdlib.h>
#include <stdio.h>

#include "ppc.h"

#define SE64(v, s) (((int64_t)((uint64_t)v << (63 - s))) >> (63 - s))
#define PPC_XFX_SPR (((opcode >> 16) & 0x1f) | ((opcode >> 6) & 0x3e0))
#define PPC_XFX_RT ((opcode >> 21) & 0x1f)
#define PPC_DS_RS ((opcode >> 21) & 0x1f)
#define PPC_DS_RA ((opcode >> 16) & 0x1f)
#define PPC_DS_DS ((opcode >> 2) & 0x3fff)
#define PPC_X_RS ((opcode >> 21) & 0x1f)
#define PPC_X_RA ((opcode >> 16) & 0x1f)
#define PPC_X_RB ((opcode >> 11) & 0x1f)
#define PPC_X_RC (opcode & 1)
#define PPC_I_LI ((opcode & 0x3fffffc) >> 2)
#define PPC_I_AA ((opcode & 2) >> 1)
#define PPC_I_LK (opcode & 1)
#define PPC_D_RT ((opcode >> 21) & 0x1f)
#define PPC_D_RS ((opcode >> 21) & 0x1f)
#define PPC_D_RA ((opcode >> 16) & 0x1f)
#define PPC_D_D (opcode & 0xffff)
#define PPC_D_SI (opcode & 0xffff)
#define PPC_D_UI (opcode & 0xffff)
#define PPC_D_L (opcode & 0x200000)
#define PPC_D_BF ((opcode >> 23) & 7)
#define PPC_B_BO ((opcode >> 21) & 0x1f)
#define PPC_B_BI ((opcode >> 16) & 0x1f)
#define PPC_B_BD ((opcode & 0xfffc) >> 2)
#define PPC_B_AA ((opcode & 2) >> 1)
#define PPC_B_LK (opcode & 1)
#define PPC_MD_RS ((opcode >> 21) & 0x1f)
#define PPC_MD_RA ((opcode >> 16) & 0x1f)
#define PPC_MD_SH (((opcode >> 11) & 0x1f) | ((opcode & 2) << 4))
#define PPC_MD_MB (((opcode >> 6) & 0x1f) | ((opcode & 0x20)))
#define PPC_MD_RC (opcode & 1)

uint32_t pc;

static const char* ppc_b_table[] = {
    "b", "bl", "ba", "bla"
};

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

static inline const char* ppc_get_spr(int i) {
    switch (i) {
        case 1: return "xer";
        case 8: return "lr";
        case 9: return "ctr";
        case 18: return "dsisr";
        case 19: return "dar";
        case 22: return "dec";
        case 25: return "sdr1";
        case 26: return "srr0";
        case 27: return "srr1";
        case 29: return "accr";
        case 152: return "ctrl";
        case 272: return "sprg0";
        case 273: return "sprg1";
        case 274: return "sprg2";
        case 275: return "sprg3";
        case 282: return "ear";
        case 284: return "tbl";
        case 285: return "tbu";
        case 304: return "hsprg0";
        case 305: return "hsprg1";
        case 309: return "purr";
        case 310: return "hdec";
        case 312: return "rmor";
        case 313: return "hrmor";
        case 314: return "hsrr0";
        case 315: return "hsrr1";
        case 318: return "lpcr";
        case 319: return "lpidr";
    }
    // 784-799 11000 1xxxx perf_mon3 yes
    // 1012 11111 10101 DABR 4,5 hypv
    // 1015 11111 10111 DABRX 4,5 hypv
}

static inline char* ppc_d_adde(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "adde"); return buf; }
static inline char* ppc_d_add(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "add"); return buf; }
static inline char* ppc_d_and(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "and"); return buf; }
static inline char* ppc_d_addc(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "addc"); return buf; }
static inline char* ppc_d_andc(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "andc"); return buf; }
static inline char* ppc_d_addme(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "addme"); return buf; }
static inline char* ppc_d_addis(char* buf, uint32_t opcode) {
    if (!PPC_D_RA)
        sprintf(buf, "%-10s r%d, %d", "lis", PPC_D_RT, SE64(PPC_D_SI, 15));
    else
        sprintf(buf, "%-10s r%d, r%d, %d", "addis", PPC_D_RT, PPC_D_RA, SE64(PPC_D_SI, 15));

    return buf;
}
static inline char* ppc_d_addi(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s r%d, r%d, %d", "addi", PPC_D_RT, PPC_D_RA, SE64(PPC_D_SI, 15));

    return buf;
}
static inline char* ppc_d_addic_u(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "addic_u"); return buf; }
static inline char* ppc_d_addic(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "addic"); return buf; }
static inline char* ppc_d_andi_u(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "andi_u"); return buf; }
static inline char* ppc_d_addze(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "addze"); return buf; }
static inline char* ppc_d_andis_u(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "andis_u"); return buf; }
static inline char* ppc_d_bc(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s %d, cr%d, %x", "bc", PPC_B_BO, PPC_B_BI >> 2, (PPC_B_AA ? 0 : pc) + SE64(PPC_B_BD << 2, 15));

    return buf;
}
static inline char* ppc_d_b(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s %x", ppc_b_table[opcode & 3], (PPC_I_AA ? 0 : pc) + SE64(PPC_I_LI << 2, 25));

    return buf;
}
static inline char* ppc_d_bclr(char* buf, uint32_t opcode) {
    if ((PPC_B_BO & 0x14) == 0x14)
        sprintf(buf, "%s", "blr");
    else
        sprintf(buf, "%-10s %d, cr%d", "bclr", PPC_B_BO, PPC_B_BI >> 2);

    return buf;
}
static inline char* ppc_d_bcctr(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "bcctr"); return buf; }
static inline char* ppc_d_cmp(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "cmp"); return buf; }
static inline char* ppc_d_cntlzw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "cntlzw"); return buf; }
static inline char* ppc_d_cmpl(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "cmpl"); return buf; }
static inline char* ppc_d_crxor(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "crxor"); return buf; }
static inline char* ppc_d_cror(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "cror"); return buf; }
static inline char* ppc_d_crorc(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "crorc"); return buf; }
static inline char* ppc_d_creqv(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "creqv"); return buf; }
static inline char* ppc_d_crand(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "crand"); return buf; }
static inline char* ppc_d_crnand(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "crnand"); return buf; }
static inline char* ppc_d_crandc(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "crandc"); return buf; }
static inline char* ppc_d_crnor(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "crnor"); return buf; }
static inline char* ppc_d_cntlzd(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "cntlzd"); return buf; }
static inline char* ppc_d_cmpi(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s cr%d, r%d, %d", PPC_D_L ? "cmpdi" : "cmpwi", PPC_D_BF, PPC_D_RA, SE64(PPC_D_D, 15));

    return buf;
}
static inline char* ppc_d_cmpli(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "cmpli"); return buf; }
static inline char* ppc_d_dcbst(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "dcbst"); return buf; }
static inline char* ppc_d_divd(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "divd"); return buf; }
static inline char* ppc_d_dcbtst(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "dcbtst"); return buf; }
static inline char* ppc_d_dcbt(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "dcbt"); return buf; }
static inline char* ppc_d_dcbf(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "dcbf"); return buf; }
static inline char* ppc_d_divdu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "divdu"); return buf; }
static inline char* ppc_d_divwu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "divwu"); return buf; }
static inline char* ppc_d_divw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "divw"); return buf; }
static inline char* ppc_d_dcbz(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "dcbz"); return buf; }
static inline char* ppc_d_ecowx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "ecowx"); return buf; }
static inline char* ppc_d_eieio(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "eieio"); return buf; }
static inline char* ppc_d_extsh(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "extsh"); return buf; }
static inline char* ppc_d_extsw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "extsw"); return buf; }
static inline char* ppc_d_extsb(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "extsb"); return buf; }
static inline char* ppc_d_eqv(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "eqv"); return buf; }
static inline char* ppc_d_eciwx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "eciwx"); return buf; }
static inline char* ppc_d_frsqrte(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "frsqrte"); return buf; }
static inline char* ppc_d_fadds(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fadds"); return buf; }
static inline char* ppc_d_fsubs(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fsubs"); return buf; }
static inline char* ppc_d_fdivs(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fdivs"); return buf; }
static inline char* ppc_d_fre(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fre"); return buf; }
static inline char* ppc_d_fmul(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fmul"); return buf; }
static inline char* ppc_d_fsqrts(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fsqrts"); return buf; }
static inline char* ppc_d_fmsub(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fmsub"); return buf; }
static inline char* ppc_d_fmadd(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fmadd"); return buf; }
static inline char* ppc_d_fnmsub(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fnmsub"); return buf; }
static inline char* ppc_d_fnmadd(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fnmadd"); return buf; }
static inline char* ppc_d_fcmpo(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fcmpo"); return buf; }
static inline char* ppc_d_fcfid(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fcfid"); return buf; }
static inline char* ppc_d_fsel(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fsel"); return buf; }
static inline char* ppc_d_fnmadds(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fnmadds"); return buf; }
static inline char* ppc_d_fres(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fres"); return buf; }
static inline char* ppc_d_fmuls(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fmuls"); return buf; }
static inline char* ppc_d_frsqrtes(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "frsqrtes"); return buf; }
static inline char* ppc_d_fmsubs(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fmsubs"); return buf; }
static inline char* ppc_d_fsqrt(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fsqrt"); return buf; }
static inline char* ppc_d_fadd(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fadd"); return buf; }
static inline char* ppc_d_fsub(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fsub"); return buf; }
static inline char* ppc_d_fdiv(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fdiv"); return buf; }
static inline char* ppc_d_fctiwz(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fctiwz"); return buf; }
static inline char* ppc_d_fmadds(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fmadds"); return buf; }
static inline char* ppc_d_fctiw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fctiw"); return buf; }
static inline char* ppc_d_frsp(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "frsp"); return buf; }
static inline char* ppc_d_fnmsubs(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fnmsubs"); return buf; }
static inline char* ppc_d_fcmpu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fcmpu"); return buf; }
static inline char* ppc_d_fneg(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fneg"); return buf; }
static inline char* ppc_d_fctid(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fctid"); return buf; }
static inline char* ppc_d_fmr(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fmr"); return buf; }
static inline char* ppc_d_fctidz(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fctidz"); return buf; }
static inline char* ppc_d_fabs(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fabs"); return buf; }
static inline char* ppc_d_fnabs(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "fnabs"); return buf; }
static inline char* ppc_d_hrfid(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "hrfid"); return buf; }
static inline char* ppc_d_isync(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "isync"); return buf; }
static inline char* ppc_d_icbi(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "icbi"); return buf; }
static inline char* ppc_d_ldx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "ldx"); return buf; }
static inline char* ppc_d_lswx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lswx"); return buf; }
static inline char* ppc_d_lhzux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lhzux"); return buf; }
static inline char* ppc_d_lwarx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lwarx"); return buf; }
static inline char* ppc_d_lwax(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lwax"); return buf; }
static inline char* ppc_d_lhax(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lhax"); return buf; }
static inline char* ppc_d_ld(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s r%d, %d(r%d)", "ld", PPC_DS_RS, SE64(PPC_DS_DS, 13) << 2, PPC_DS_RA);

    return buf;
}
static inline char* ppc_d_ldu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "ldu"); return buf; }
static inline char* ppc_d_lwaux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lwaux"); return buf; }
static inline char* ppc_d_lhaux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lhaux"); return buf; }
static inline char* ppc_d_lwa(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lwa"); return buf; }
static inline char* ppc_d_lhbrx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lhbrx"); return buf; }
static inline char* ppc_d_lfdux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lfdux"); return buf; }
static inline char* ppc_d_lfdx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lfdx"); return buf; }
static inline char* ppc_d_lswi(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lswi"); return buf; }
static inline char* ppc_d_lfsux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lfsux"); return buf; }
static inline char* ppc_d_lfsx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lfsx"); return buf; }
static inline char* ppc_d_lwbrx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lwbrx"); return buf; }
static inline char* ppc_d_lhzx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lhzx"); return buf; }
static inline char* ppc_d_lmw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lmw"); return buf; }
static inline char* ppc_d_lfs(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lfs"); return buf; }
static inline char* ppc_d_lhau(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lhau"); return buf; }
static inline char* ppc_d_lha(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lha"); return buf; }
static inline char* ppc_d_lhzu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lhzu"); return buf; }
static inline char* ppc_d_lhz(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lhz"); return buf; }
static inline char* ppc_d_lbzux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lbzux"); return buf; }
static inline char* ppc_d_lfsu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lfsu"); return buf; }
static inline char* ppc_d_lbzx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lbzx"); return buf; }
static inline char* ppc_d_ldarx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "ldarx"); return buf; }
static inline char* ppc_d_lfd(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lfd"); return buf; }
static inline char* ppc_d_lfdu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lfdu"); return buf; }
static inline char* ppc_d_lbzu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lbzu"); return buf; }
static inline char* ppc_d_lbz(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s r%d, %d(r%d)", "lbz", PPC_D_RT, SE64(PPC_D_D, 15), PPC_D_RA);

    return buf;
}
static inline char* ppc_d_lwzu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lwzu"); return buf; }
static inline char* ppc_d_lwz(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lwz"); return buf; }
static inline char* ppc_d_lwzux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lwzux"); return buf; }
static inline char* ppc_d_ldux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "ldux"); return buf; }
static inline char* ppc_d_lwzx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "lwzx"); return buf; }
static inline char* ppc_d_mfsrin(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mfsrin"); return buf; }
static inline char* ppc_d_mulli(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mulli"); return buf; }
static inline char* ppc_d_mtfsf(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mtfsf"); return buf; }
static inline char* ppc_d_mfsr(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mfsr"); return buf; }
static inline char* ppc_d_mcrf(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mcrf"); return buf; }
static inline char* ppc_d_mffs(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mffs"); return buf; }
static inline char* ppc_d_mtcrf(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mtcrf"); return buf; }
static inline char* ppc_d_mulld(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mulld"); return buf; }
static inline char* ppc_d_mfcr(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mfcr"); return buf; }
static inline char* ppc_d_mftb(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mftb"); return buf; }
static inline char* ppc_d_mfocrf(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mfocrf"); return buf; }
static inline char* ppc_d_mcrxr(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mcrxr"); return buf; }
static inline char* ppc_d_mtfsfi(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mtfsfi"); return buf; }
static inline char* ppc_d_mtfsb0(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mtfsb0"); return buf; }
static inline char* ppc_d_mtsrin(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mtsrin"); return buf; }
static inline char* ppc_d_mullw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mullw"); return buf; }
static inline char* ppc_d_mcrfs(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mcrfs"); return buf; }
static inline char* ppc_d_mfspr(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s r%d, %s", "mfspr", PPC_XFX_RT, ppc_get_spr(PPC_XFX_SPR));

    return buf;
}
static inline char* ppc_d_mulhd(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mulhd"); return buf; }
static inline char* ppc_d_mulhw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mulhw"); return buf; }
static inline char* ppc_d_mfmsr(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mfmsr"); return buf; }
static inline char* ppc_d_mtsr(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mtsr"); return buf; }
static inline char* ppc_d_mtmsrd(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mtmsrd"); return buf; }
static inline char* ppc_d_mtfsb1(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mtfsb1"); return buf; }
static inline char* ppc_d_mtmsr(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mtmsr"); return buf; }
static inline char* ppc_d_mtocrf(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mtocrf"); return buf; }
static inline char* ppc_d_mulhwu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mulhwu"); return buf; }
static inline char* ppc_d_mulhdu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mulhdu"); return buf; }
static inline char* ppc_d_mtspr(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "mtspr"); return buf; }
static inline char* ppc_d_nor(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "nor"); return buf; }
static inline char* ppc_d_neg(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "neg"); return buf; }
static inline char* ppc_d_nand(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "nand"); return buf; }
static inline char* ppc_d_orc(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "orc"); return buf; }
static inline char* ppc_d_ori(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s r%d, r%d, %d", "ori", PPC_D_RA, PPC_D_RS, PPC_D_UI);

    return buf;
}
static inline char* ppc_d_oris(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "oris"); return buf; }
static inline char* ppc_d_or(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s r%d, r%d, r%d", PPC_X_RC ? "or." : "or", PPC_X_RA, PPC_X_RS, PPC_X_RB);

    return buf;
}
static inline char* ppc_d_popcntb(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "popcntb"); return buf; }
static inline char* ppc_d_rfid(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "rfid"); return buf; }
static inline char* ppc_d_rldicl(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s r%d, r%d, %d, %d", PPC_MD_RC ? "rldicl." : "rldicl", PPC_MD_RA, PPC_MD_RS, PPC_MD_SH, PPC_MD_MB);

    return buf;
}
static inline char* ppc_d_rldicr(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "rldicr"); return buf; }
static inline char* ppc_d_rldic(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "rldic"); return buf; }
static inline char* ppc_d_rldimi(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "rldimi"); return buf; }
static inline char* ppc_d_rldcl(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "rldcl"); return buf; }
static inline char* ppc_d_rldcr(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "rldcr"); return buf; }
static inline char* ppc_d_rlwnm(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "rlwnm"); return buf; }
static inline char* ppc_d_rlwinm(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "rlwinm"); return buf; }
static inline char* ppc_d_rlwimi(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "rlwimi"); return buf; }
static inline char* ppc_d_sradi(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "sradi"); return buf; }
static inline char* ppc_d_stfs(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stfs"); return buf; }
static inline char* ppc_d_stfsu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stfsu"); return buf; }
static inline char* ppc_d_stfd(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stfd"); return buf; }
static inline char* ppc_d_stfdu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stfdu"); return buf; }
static inline char* ppc_d_std(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s r%d, %d(r%d)", "std", PPC_DS_RS, SE64(PPC_DS_DS, 13) << 2, PPC_DS_RA);

    return buf;
}
static inline char* ppc_d_stmw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stmw"); return buf; }
static inline char* ppc_d_stdu(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s r%d, %d(r%d)", "stdu", PPC_DS_RS, SE64(PPC_DS_DS, 13) << 2, PPC_DS_RA);

    return buf;
}
static inline char* ppc_d_stdcx_u(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stdcx_u"); return buf; }
static inline char* ppc_d_slbia(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "slbia"); return buf; }
static inline char* ppc_d_sthux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "sthux"); return buf; }
static inline char* ppc_d_slbie(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "slbie"); return buf; }
static inline char* ppc_d_subfic(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "subfic"); return buf; }
static inline char* ppc_d_sthx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "sthx"); return buf; }
static inline char* ppc_d_slbmte(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "slbmte"); return buf; }
static inline char* ppc_d_sc(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "sc"); return buf; }
static inline char* ppc_d_subfc(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "subfc"); return buf; }
static inline char* ppc_d_stbux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stbux"); return buf; }
static inline char* ppc_d_subfme(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "subfme"); return buf; }
static inline char* ppc_d_stbx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stbx"); return buf; }
static inline char* ppc_d_srw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "srw"); return buf; }
static inline char* ppc_d_subfze(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "subfze"); return buf; }
static inline char* ppc_d_stwux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stwux"); return buf; }
static inline char* ppc_d_stdux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stdux"); return buf; }
static inline char* ppc_d_stwx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stwx"); return buf; }
static inline char* ppc_d_stwcx_u(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stwcx_u"); return buf; }
static inline char* ppc_d_stdx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stdx"); return buf; }
static inline char* ppc_d_subfe(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "subfe"); return buf; }
static inline char* ppc_d_slw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "slw"); return buf; }
static inline char* ppc_d_subf(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "subf"); return buf; }
static inline char* ppc_d_sld(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "sld"); return buf; }
static inline char* ppc_d_sraw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "sraw"); return buf; }
static inline char* ppc_d_sth(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "sth"); return buf; }
static inline char* ppc_d_stbu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stbu"); return buf; }
static inline char* ppc_d_stb(char* buf, uint32_t opcode) {
    sprintf(buf, "%-10s r%d, %d(r%d)", "stb", PPC_D_RS, SE64(PPC_D_D, 15), PPC_D_RA);

    return buf;
}
static inline char* ppc_d_stwu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stwu"); return buf; }
static inline char* ppc_d_stw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stw"); return buf; }
static inline char* ppc_d_stfiwx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stfiwx"); return buf; }
static inline char* ppc_d_sthbrx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "sthbrx"); return buf; }
static inline char* ppc_d_slbmfee(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "slbmfee"); return buf; }
static inline char* ppc_d_slbmfev(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "slbmfev"); return buf; }
static inline char* ppc_d_srawi(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "srawi"); return buf; }
static inline char* ppc_d_srad(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "srad"); return buf; }
static inline char* ppc_d_stfdux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stfdux"); return buf; }
static inline char* ppc_d_stfdx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stfdx"); return buf; }
static inline char* ppc_d_stswi(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stswi"); return buf; }
static inline char* ppc_d_stfsux(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stfsux"); return buf; }
static inline char* ppc_d_stfsx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stfsx"); return buf; }
static inline char* ppc_d_stwbrx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stwbrx"); return buf; }
static inline char* ppc_d_stswx(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "stswx"); return buf; }
static inline char* ppc_d_sync(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "sync"); return buf; }
static inline char* ppc_d_sthu(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "sthu"); return buf; }
static inline char* ppc_d_srd(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "srd"); return buf; }
static inline char* ppc_d_twi(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "twi"); return buf; }
static inline char* ppc_d_tw(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "tw"); return buf; }
static inline char* ppc_d_td(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "td"); return buf; }
static inline char* ppc_d_tlbie(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "tlbie"); return buf; }
static inline char* ppc_d_tlbia(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "tlbia"); return buf; }
static inline char* ppc_d_tdi(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "tdi"); return buf; }
static inline char* ppc_d_tlbsync(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "tlbsync"); return buf; }
static inline char* ppc_d_xor(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "xor"); return buf; }
static inline char* ppc_d_xoris(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "xoris"); return buf; }
static inline char* ppc_d_xori(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "xori"); return buf; }
static inline char* ppc_d_invalid(char* buf, uint32_t opcode) { sprintf(buf, "%-10s", "<invalid>"); return buf; }

char* ppc_disassemble(struct ppc_dis_state* s, char* buf, uint32_t opcode) {
    char* ptr = buf;

    pc = s->pc;

    if (s) if (s->print_address)
        ptr += sprintf(ptr, "%08x: ", s->pc);

    if (s) if (s->print_opcode)
        ptr += sprintf(ptr, "%08x ", opcode);

    switch ((opcode & 0xfc000000) >> 26) {
        case 0x08000000ul >> 26: ppc_d_tdi(ptr, opcode); return buf;
        case 0x0c000000ul >> 26: ppc_d_twi(ptr, opcode); return buf;
        case 0x1c000000ul >> 26: ppc_d_mulli(ptr, opcode); return buf;
        case 0x20000000ul >> 26: ppc_d_subfic(ptr, opcode); return buf;
        case 0x28000000ul >> 26: ppc_d_cmpli(ptr, opcode); return buf;
        case 0x2c000000ul >> 26: ppc_d_cmpi(ptr, opcode); return buf;
        case 0x30000000ul >> 26: ppc_d_addic(ptr, opcode); return buf;
        case 0x34000000ul >> 26: ppc_d_addic_u(ptr, opcode); return buf;
        case 0x38000000ul >> 26: ppc_d_addi(ptr, opcode); return buf;
        case 0x3c000000ul >> 26: ppc_d_addis(ptr, opcode); return buf;
        case 0x40000000ul >> 26: ppc_d_bc(ptr, opcode); return buf;
        case 0x44000000ul >> 26: ppc_d_sc(ptr, opcode); return buf;
        case 0x48000000ul >> 26: ppc_d_b(ptr, opcode); return buf;
        case 0x4c000000ul >> 26: {
            switch ((opcode & 0x000007fe) >> 1) {
                case 0x00000000ul >> 1: ppc_d_mcrf(ptr, opcode); return buf;
                case 0x00000020ul >> 1: ppc_d_bclr(ptr, opcode); return buf;
                case 0x00000024ul >> 1: ppc_d_rfid(ptr, opcode); return buf;
                case 0x00000042ul >> 1: ppc_d_crnor(ptr, opcode); return buf;
                case 0x00000102ul >> 1: ppc_d_crandc(ptr, opcode); return buf;
                case 0x0000012cul >> 1: ppc_d_isync(ptr, opcode); return buf;
                case 0x00000182ul >> 1: ppc_d_crxor(ptr, opcode); return buf;
                case 0x000001c2ul >> 1: ppc_d_crnand(ptr, opcode); return buf;
                case 0x00000202ul >> 1: ppc_d_crand(ptr, opcode); return buf;
                case 0x00000224ul >> 1: ppc_d_hrfid(ptr, opcode); return buf;
                case 0x00000242ul >> 1: ppc_d_creqv(ptr, opcode); return buf;
                case 0x00000342ul >> 1: ppc_d_crorc(ptr, opcode); return buf;
                case 0x00000382ul >> 1: ppc_d_cror(ptr, opcode); return buf;
                case 0x00000420ul >> 1: ppc_d_bcctr(ptr, opcode); return buf;
            }
        } break;
        case 0x50000000ul >> 26: ppc_d_rlwimi(ptr, opcode); return buf;
        case 0x54000000ul >> 26: ppc_d_rlwinm(ptr, opcode); return buf;
        case 0x5c000000ul >> 26: ppc_d_rlwnm(ptr, opcode); return buf;
        case 0x60000000ul >> 26: ppc_d_ori(ptr, opcode); return buf;
        case 0x64000000ul >> 26: ppc_d_oris(ptr, opcode); return buf;
        case 0x68000000ul >> 26: ppc_d_xori(ptr, opcode); return buf;
        case 0x6c000000ul >> 26: ppc_d_xoris(ptr, opcode); return buf;
        case 0x70000000ul >> 26: ppc_d_andi_u(ptr, opcode); return buf;
        case 0x74000000ul >> 26: ppc_d_andis_u(ptr, opcode); return buf;
        case 0x78000000ul >> 26: {
            switch ((opcode & 0x0000001e) >> 1) {
                case 0x00000010ul >> 1: ppc_d_rldcl(ptr, opcode); return buf;
                case 0x00000012ul >> 1: ppc_d_rldcr(ptr, opcode); return buf;
            }
            switch ((opcode & 0x0000001c) >> 2) {
                case 0x00000000ul >> 2: ppc_d_rldicl(ptr, opcode); return buf;
                case 0x00000004ul >> 2: ppc_d_rldicr(ptr, opcode); return buf;
                case 0x00000008ul >> 2: ppc_d_rldic(ptr, opcode); return buf;
                case 0x0000000cul >> 2: ppc_d_rldimi(ptr, opcode); return buf;
            }
        } break;
        case 0x7c000000ul >> 26: {
            switch ((opcode & 0x000007fe) >> 1) {
                case 0x0000032eul >> 1: ppc_d_sthx(ptr, opcode); return buf;
                case 0x00000436ul >> 1: ppc_d_srd(ptr, opcode); return buf;
                case 0x00000430ul >> 1: ppc_d_srw(ptr, opcode); return buf;
                case 0x0000042eul >> 1: ppc_d_lfsx(ptr, opcode); return buf;
                case 0x0000042cul >> 1: ppc_d_lwbrx(ptr, opcode); return buf;
                case 0x0000042aul >> 1: ppc_d_lswx(ptr, opcode); return buf;
                case 0x00000400ul >> 1: ppc_d_mcrxr(ptr, opcode); return buf;
                case 0x000003e4ul >> 1: ppc_d_slbia(ptr, opcode); return buf;
                case 0x000003b8ul >> 1: ppc_d_nand(ptr, opcode); return buf;
                case 0x000003a6ul >> 1: ppc_d_mtspr(ptr, opcode); return buf;
                case 0x00000378ul >> 1: ppc_d_or(ptr, opcode); return buf;
                case 0x0000036eul >> 1: ppc_d_sthux(ptr, opcode); return buf;
                case 0x0000036cul >> 1: ppc_d_ecowx(ptr, opcode); return buf;
                case 0x00000364ul >> 1: ppc_d_slbie(ptr, opcode); return buf;
                case 0x00000338ul >> 1: ppc_d_orc(ptr, opcode); return buf;
                case 0x0000046cul >> 1: ppc_d_tlbsync(ptr, opcode); return buf;
                case 0x00000324ul >> 1: ppc_d_slbmte(ptr, opcode); return buf;
                case 0x000002eeul >> 1: ppc_d_lhaux(ptr, opcode); return buf;
                case 0x000002eaul >> 1: ppc_d_lwaux(ptr, opcode); return buf;
                case 0x000002e6ul >> 1: ppc_d_mftb(ptr, opcode); return buf;
                case 0x000002e4ul >> 1: ppc_d_tlbia(ptr, opcode); return buf;
                case 0x000002aeul >> 1: ppc_d_lhax(ptr, opcode); return buf;
                case 0x000002aaul >> 1: ppc_d_lwax(ptr, opcode); return buf;
                case 0x000002a6ul >> 1: ppc_d_mfspr(ptr, opcode); return buf;
                case 0x00000278ul >> 1: ppc_d_xor(ptr, opcode); return buf;
                case 0x0000026eul >> 1: ppc_d_lhzux(ptr, opcode); return buf;
                case 0x0000026cul >> 1: ppc_d_eciwx(ptr, opcode); return buf;
                case 0x00000000ul >> 1: ppc_d_cmp(ptr, opcode); return buf;
                case 0x00000238ul >> 1: ppc_d_eqv(ptr, opcode); return buf;
                case 0x000005eeul >> 1: ppc_d_stfdux(ptr, opcode); return buf;
                case 0x000007ecul >> 1: ppc_d_dcbz(ptr, opcode); return buf;
                case 0x000007b4ul >> 1: ppc_d_extsw(ptr, opcode); return buf;
                case 0x000007aeul >> 1: ppc_d_stfiwx(ptr, opcode); return buf;
                case 0x000007acul >> 1: ppc_d_icbi(ptr, opcode); return buf;
                case 0x00000774ul >> 1: ppc_d_extsb(ptr, opcode); return buf;
                case 0x00000734ul >> 1: ppc_d_extsh(ptr, opcode); return buf;
                case 0x0000072cul >> 1: ppc_d_sthbrx(ptr, opcode); return buf;
                case 0x00000726ul >> 1: ppc_d_slbmfee(ptr, opcode); return buf;
                case 0x000006acul >> 1: ppc_d_eieio(ptr, opcode); return buf;
                case 0x000006a6ul >> 1: ppc_d_slbmfev(ptr, opcode); return buf;
                case 0x00000670ul >> 1: ppc_d_srawi(ptr, opcode); return buf;
                case 0x00000634ul >> 1: ppc_d_srad(ptr, opcode); return buf;
                case 0x00000630ul >> 1: ppc_d_sraw(ptr, opcode); return buf;
                case 0x0000062cul >> 1: ppc_d_lhbrx(ptr, opcode); return buf;
                case 0x00000264ul >> 1: ppc_d_tlbie(ptr, opcode); return buf;
                case 0x000005aeul >> 1: ppc_d_stfdx(ptr, opcode); return buf;
                case 0x000005aaul >> 1: ppc_d_stswi(ptr, opcode); return buf;
                case 0x0000056eul >> 1: ppc_d_stfsux(ptr, opcode); return buf;
                case 0x0000052eul >> 1: ppc_d_stfsx(ptr, opcode); return buf;
                case 0x0000052cul >> 1: ppc_d_stwbrx(ptr, opcode); return buf;
                case 0x0000052aul >> 1: ppc_d_stswx(ptr, opcode); return buf;
                case 0x00000526ul >> 1: ppc_d_mfsrin(ptr, opcode); return buf;
                case 0x000004eeul >> 1: ppc_d_lfdux(ptr, opcode); return buf;
                case 0x000004aeul >> 1: ppc_d_lfdx(ptr, opcode); return buf;
                case 0x000004acul >> 1: ppc_d_sync(ptr, opcode); return buf;
                case 0x000004aaul >> 1: ppc_d_lswi(ptr, opcode); return buf;
                case 0x000004a6ul >> 1: ppc_d_mfsr(ptr, opcode); return buf;
                case 0x0000046eul >> 1: ppc_d_lfsux(ptr, opcode); return buf;
                case 0x0000006cul >> 1: ppc_d_dcbst(ptr, opcode); return buf;
                // case 0x00000120ul >> 1: ppc_d_mtocrf(ptr, opcode); return buf;
                case 0x00000120ul >> 1: ppc_d_mtcrf(ptr, opcode); return buf;
                case 0x0000022eul >> 1: ppc_d_lhzx(ptr, opcode); return buf;
                case 0x000000f4ul >> 1: ppc_d_popcntb(ptr, opcode); return buf;
                case 0x000000eeul >> 1: ppc_d_lbzux(ptr, opcode); return buf;
                case 0x000000aeul >> 1: ppc_d_lbzx(ptr, opcode); return buf;
                case 0x000000acul >> 1: ppc_d_dcbf(ptr, opcode); return buf;
                case 0x000000a8ul >> 1: ppc_d_ldarx(ptr, opcode); return buf;
                case 0x000000a6ul >> 1: ppc_d_mfmsr(ptr, opcode); return buf;
                case 0x00000088ul >> 1: ppc_d_td(ptr, opcode); return buf;
                case 0x00000078ul >> 1: ppc_d_andc(ptr, opcode); return buf;
                case 0x00000074ul >> 1: ppc_d_cntlzd(ptr, opcode); return buf;
                case 0x0000006eul >> 1: ppc_d_lwzux(ptr, opcode); return buf;
                case 0x000000f8ul >> 1: ppc_d_nor(ptr, opcode); return buf;
                case 0x0000006aul >> 1: ppc_d_ldux(ptr, opcode); return buf;
                case 0x00000040ul >> 1: ppc_d_cmpl(ptr, opcode); return buf;
                case 0x00000038ul >> 1: ppc_d_and(ptr, opcode); return buf;
                case 0x00000036ul >> 1: ppc_d_sld(ptr, opcode); return buf;
                case 0x00000034ul >> 1: ppc_d_cntlzw(ptr, opcode); return buf;
                case 0x00000030ul >> 1: ppc_d_slw(ptr, opcode); return buf;
                case 0x0000002eul >> 1: ppc_d_lwzx(ptr, opcode); return buf;
                case 0x0000002aul >> 1: ppc_d_ldx(ptr, opcode); return buf;
                case 0x00000028ul >> 1: ppc_d_lwarx(ptr, opcode); return buf;
                // case 0x00000026ul >> 1: ppc_d_mfocrf(ptr, opcode); return buf;
                case 0x00000026ul >> 1: ppc_d_mfcr(ptr, opcode); return buf;
                case 0x00000008ul >> 1: ppc_d_tw(ptr, opcode); return buf;
                case 0x0000022cul >> 1: ppc_d_dcbt(ptr, opcode); return buf;
                case 0x000001aeul >> 1: ppc_d_stbx(ptr, opcode); return buf;
                case 0x000001acul >> 1: ppc_d_stdcx_u(ptr, opcode); return buf;
                case 0x000001a4ul >> 1: ppc_d_mtsr(ptr, opcode); return buf;
                case 0x000001e4ul >> 1: ppc_d_mtsrin(ptr, opcode); return buf;
                case 0x0000016eul >> 1: ppc_d_stwux(ptr, opcode); return buf;
                case 0x0000016aul >> 1: ppc_d_stdux(ptr, opcode); return buf;
                case 0x000001ecul >> 1: ppc_d_dcbtst(ptr, opcode); return buf;
                case 0x000001eeul >> 1: ppc_d_stbux(ptr, opcode); return buf;
                case 0x00000164ul >> 1: ppc_d_mtmsrd(ptr, opcode); return buf;
                case 0x0000012eul >> 1: ppc_d_stwx(ptr, opcode); return buf;
                case 0x0000012cul >> 1: ppc_d_stwcx_u(ptr, opcode); return buf;
                case 0x0000012aul >> 1: ppc_d_stdx(ptr, opcode); return buf;
                case 0x00000124ul >> 1: ppc_d_mtmsr(ptr, opcode); return buf;
            }
            switch ((opcode & 0x000007fc) >> 2) {
                case 0x00000674ul >> 2: ppc_d_sradi(ptr, opcode); return buf;
            }
            switch ((opcode & 0x000003fe) >> 1) {
                case 0x00000214ul >> 1: ppc_d_add(ptr, opcode); return buf;
                case 0x00000016ul >> 1: ppc_d_mulhwu(ptr, opcode); return buf;
                case 0x00000014ul >> 1: ppc_d_addc(ptr, opcode); return buf;
                case 0x000001d6ul >> 1: ppc_d_mullw(ptr, opcode); return buf;
                case 0x00000012ul >> 1: ppc_d_mulhdu(ptr, opcode); return buf;
                case 0x000001d4ul >> 1: ppc_d_addme(ptr, opcode); return buf;
                case 0x00000010ul >> 1: ppc_d_subfc(ptr, opcode); return buf;
                case 0x00000194ul >> 1: ppc_d_addze(ptr, opcode); return buf;
                case 0x000001d2ul >> 1: ppc_d_mulld(ptr, opcode); return buf;
                case 0x000001d0ul >> 1: ppc_d_subfme(ptr, opcode); return buf;
                case 0x00000050ul >> 1: ppc_d_subf(ptr, opcode); return buf;
                case 0x000003d6ul >> 1: ppc_d_divw(ptr, opcode); return buf;
                case 0x00000190ul >> 1: ppc_d_subfze(ptr, opcode); return buf;
                case 0x00000092ul >> 1: ppc_d_mulhd(ptr, opcode); return buf;
                case 0x00000096ul >> 1: ppc_d_mulhw(ptr, opcode); return buf;
                case 0x00000392ul >> 1: ppc_d_divdu(ptr, opcode); return buf;
                case 0x000000d0ul >> 1: ppc_d_neg(ptr, opcode); return buf;
                case 0x00000396ul >> 1: ppc_d_divwu(ptr, opcode); return buf;
                case 0x00000110ul >> 1: ppc_d_subfe(ptr, opcode); return buf;
                case 0x00000114ul >> 1: ppc_d_adde(ptr, opcode); return buf;
                case 0x000003d2ul >> 1: ppc_d_divd(ptr, opcode); return buf;
            }
        } break;
        case 0x80000000ul >> 26: ppc_d_lwz(ptr, opcode); return buf;
        case 0x84000000ul >> 26: ppc_d_lwzu(ptr, opcode); return buf;
        case 0x88000000ul >> 26: ppc_d_lbz(ptr, opcode); return buf;
        case 0x8c000000ul >> 26: ppc_d_lbzu(ptr, opcode); return buf;
        case 0x90000000ul >> 26: ppc_d_stw(ptr, opcode); return buf;
        case 0x94000000ul >> 26: ppc_d_stwu(ptr, opcode); return buf;
        case 0x98000000ul >> 26: ppc_d_stb(ptr, opcode); return buf;
        case 0x9c000000ul >> 26: ppc_d_stbu(ptr, opcode); return buf;
        case 0xa0000000ul >> 26: ppc_d_lhz(ptr, opcode); return buf;
        case 0xa4000000ul >> 26: ppc_d_lhzu(ptr, opcode); return buf;
        case 0xa8000000ul >> 26: ppc_d_lha(ptr, opcode); return buf;
        case 0xac000000ul >> 26: ppc_d_lhau(ptr, opcode); return buf;
        case 0xb0000000ul >> 26: ppc_d_sth(ptr, opcode); return buf;
        case 0xb4000000ul >> 26: ppc_d_sthu(ptr, opcode); return buf;
        case 0xb8000000ul >> 26: ppc_d_lmw(ptr, opcode); return buf;
        case 0xbc000000ul >> 26: ppc_d_stmw(ptr, opcode); return buf;
        case 0xc0000000ul >> 26: ppc_d_lfs(ptr, opcode); return buf;
        case 0xc4000000ul >> 26: ppc_d_lfsu(ptr, opcode); return buf;
        case 0xc8000000ul >> 26: ppc_d_lfd(ptr, opcode); return buf;
        case 0xcc000000ul >> 26: ppc_d_lfdu(ptr, opcode); return buf;
        case 0xd0000000ul >> 26: ppc_d_stfs(ptr, opcode); return buf;
        case 0xd4000000ul >> 26: ppc_d_stfsu(ptr, opcode); return buf;
        case 0xd8000000ul >> 26: ppc_d_stfd(ptr, opcode); return buf;
        case 0xdc000000ul >> 26: ppc_d_stfdu(ptr, opcode); return buf;
        case 0xe8000000ul >> 26: {
            switch ((opcode & 0x00000003) >> 0) {
                case 0x00000000ul >> 0: ppc_d_ld(ptr, opcode); return buf;
                case 0x00000001ul >> 0: ppc_d_ldu(ptr, opcode); return buf;
                case 0x00000002ul >> 0: ppc_d_lwa(ptr, opcode); return buf;
            }
        } break;
        case 0xec000000ul >> 26: {
            switch ((opcode & 0x0000003e) >> 1) {
                case 0x00000024ul >> 1: ppc_d_fdivs(ptr, opcode); return buf;
                case 0x00000028ul >> 1: ppc_d_fsubs(ptr, opcode); return buf;
                case 0x0000002aul >> 1: ppc_d_fadds(ptr, opcode); return buf;
                case 0x0000002cul >> 1: ppc_d_fsqrts(ptr, opcode); return buf;
                case 0x00000030ul >> 1: ppc_d_fres(ptr, opcode); return buf;
                case 0x00000032ul >> 1: ppc_d_fmuls(ptr, opcode); return buf;
                case 0x00000034ul >> 1: ppc_d_frsqrtes(ptr, opcode); return buf;
                case 0x00000038ul >> 1: ppc_d_fmsubs(ptr, opcode); return buf;
                case 0x0000003aul >> 1: ppc_d_fmadds(ptr, opcode); return buf;
                case 0x0000003cul >> 1: ppc_d_fnmsubs(ptr, opcode); return buf;
                case 0x0000003eul >> 1: ppc_d_fnmadds(ptr, opcode); return buf;
            }
        } break;
        case 0xf8000000ul >> 26: {
            switch ((opcode & 0x00000003) >> 0) {
                case 0x00000000ul >> 0: ppc_d_std(ptr, opcode); return buf;
                case 0x00000001ul >> 0: ppc_d_stdu(ptr, opcode); return buf;
            }
        } break;
        case 0xfc000000ul >> 26: {
            switch ((opcode & 0x000007fe) >> 1) {
                case 0x00000018ul >> 1: ppc_d_frsp(ptr, opcode); return buf;
                case 0x0000069cul >> 1: ppc_d_fcfid(ptr, opcode); return buf;
                case 0x0000065eul >> 1: ppc_d_fctidz(ptr, opcode); return buf;
                case 0x0000065cul >> 1: ppc_d_fctid(ptr, opcode); return buf;
                case 0x0000058eul >> 1: ppc_d_mtfsf(ptr, opcode); return buf;
                case 0x0000048eul >> 1: ppc_d_mffs(ptr, opcode); return buf;
                case 0x00000210ul >> 1: ppc_d_fabs(ptr, opcode); return buf;
                case 0x00000110ul >> 1: ppc_d_fnabs(ptr, opcode); return buf;
                case 0x0000010cul >> 1: ppc_d_mtfsfi(ptr, opcode); return buf;
                case 0x00000090ul >> 1: ppc_d_fmr(ptr, opcode); return buf;
                case 0x0000008cul >> 1: ppc_d_mtfsb0(ptr, opcode); return buf;
                case 0x00000080ul >> 1: ppc_d_mcrfs(ptr, opcode); return buf;
                case 0x00000050ul >> 1: ppc_d_fneg(ptr, opcode); return buf;
                case 0x0000004cul >> 1: ppc_d_mtfsb1(ptr, opcode); return buf;
                case 0x00000040ul >> 1: ppc_d_fcmpo(ptr, opcode); return buf;
                case 0x0000001eul >> 1: ppc_d_fctiwz(ptr, opcode); return buf;
                case 0x0000001cul >> 1: ppc_d_fctiw(ptr, opcode); return buf;
                case 0x00000000ul >> 1: ppc_d_fcmpu(ptr, opcode); return buf;
            }
            switch ((opcode & 0x0000003e) >> 1) {
                case 0x0000003eul >> 1: ppc_d_fnmadd(ptr, opcode); return buf;
                case 0x0000003cul >> 1: ppc_d_fnmsub(ptr, opcode); return buf;
                case 0x0000003aul >> 1: ppc_d_fmadd(ptr, opcode); return buf;
                case 0x00000038ul >> 1: ppc_d_fmsub(ptr, opcode); return buf;
                case 0x00000034ul >> 1: ppc_d_frsqrte(ptr, opcode); return buf;
                case 0x00000032ul >> 1: ppc_d_fmul(ptr, opcode); return buf;
                case 0x00000030ul >> 1: ppc_d_fre(ptr, opcode); return buf;
                case 0x0000002eul >> 1: ppc_d_fsel(ptr, opcode); return buf;
                case 0x0000002cul >> 1: ppc_d_fsqrt(ptr, opcode); return buf;
                case 0x0000002aul >> 1: ppc_d_fadd(ptr, opcode); return buf;
                case 0x00000028ul >> 1: ppc_d_fsub(ptr, opcode); return buf;
                case 0x00000024ul >> 1: ppc_d_fdiv(ptr, opcode); return buf;
            }
        } break;
    }

    ppc_d_invalid(ptr, opcode); return buf;
}