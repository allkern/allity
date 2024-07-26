switch ((cpu->opcode & 0xfc000000) >> 26) {
    case 0x08000000ul >> 26: ppc_tdi(cpu); break;
    case 0x0c000000ul >> 26: ppc_twi(cpu); break;
    case 0x1c000000ul >> 26: ppc_mulli(cpu); break;
    case 0x20000000ul >> 26: ppc_subfic(cpu); break;
    case 0x28000000ul >> 26: ppc_cmpli(cpu); break;
    case 0x2c000000ul >> 26: ppc_cmpi(cpu); break;
    case 0x30000000ul >> 26: ppc_addic(cpu); break;
    case 0x34000000ul >> 26: ppc_addic_u(cpu); break;
    case 0x38000000ul >> 26: ppc_addi(cpu); break;
    case 0x3c000000ul >> 26: ppc_addis(cpu); break;
    case 0x40000000ul >> 26: ppc_bc(cpu); break;
    case 0x44000000ul >> 26: ppc_sc(cpu); break;
    case 0x48000000ul >> 26: ppc_b(cpu); break;
    case 0x4c000000ul >> 26: {
        switch ((cpu->opcode & 0x000007fe) >> 1) {
            case 0x00000000ul >> 1: ppc_mcrf(cpu); break;
            case 0x00000020ul >> 1: ppc_bclr(cpu); break;
            case 0x00000024ul >> 1: ppc_rfid(cpu); break;
            case 0x00000042ul >> 1: ppc_crnor(cpu); break;
            case 0x00000102ul >> 1: ppc_crandc(cpu); break;
            case 0x0000012cul >> 1: ppc_isync(cpu); break;
            case 0x00000182ul >> 1: ppc_crxor(cpu); break;
            case 0x000001c2ul >> 1: ppc_crnand(cpu); break;
            case 0x00000202ul >> 1: ppc_crand(cpu); break;
            case 0x00000224ul >> 1: ppc_hrfid(cpu); break;
            case 0x00000242ul >> 1: ppc_creqv(cpu); break;
            case 0x00000342ul >> 1: ppc_crorc(cpu); break;
            case 0x00000382ul >> 1: ppc_cror(cpu); break;
            case 0x00000420ul >> 1: ppc_bcctr(cpu); break;
        }
    } break;
    case 0x50000000ul >> 26: ppc_rlwimi(cpu); break;
    case 0x54000000ul >> 26: ppc_rlwinm(cpu); break;
    case 0x5c000000ul >> 26: ppc_rlwnm(cpu); break;
    case 0x60000000ul >> 26: ppc_ori(cpu); break;
    case 0x64000000ul >> 26: ppc_oris(cpu); break;
    case 0x68000000ul >> 26: ppc_xori(cpu); break;
    case 0x6c000000ul >> 26: ppc_xoris(cpu); break;
    case 0x70000000ul >> 26: ppc_andi_u(cpu); break;
    case 0x74000000ul >> 26: ppc_andis_u(cpu); break;
    case 0x78000000ul >> 26: {
        switch ((cpu->opcode & 0x0000001e) >> 1) {
            case 0x00000010ul >> 1: ppc_rldcl(cpu); break;
            case 0x00000012ul >> 1: ppc_rldcr(cpu); break;
        }
        switch ((cpu->opcode & 0x0000001c) >> 2) {
            case 0x00000000ul >> 2: ppc_rldicl(cpu); break;
            case 0x00000004ul >> 2: ppc_rldicr(cpu); break;
            case 0x00000008ul >> 2: ppc_rldic(cpu); break;
            case 0x0000000cul >> 2: ppc_rldimi(cpu); break;
        }
    } break;
    case 0x7c000000ul >> 26: {
        switch ((cpu->opcode & 0x000007fe) >> 1) {
            case 0x0000032eul >> 1: ppc_sthx(cpu); break;
            case 0x00000436ul >> 1: ppc_srd(cpu); break;
            case 0x00000430ul >> 1: ppc_srw(cpu); break;
            case 0x0000042eul >> 1: ppc_lfsx(cpu); break;
            case 0x0000042cul >> 1: ppc_lwbrx(cpu); break;
            case 0x0000042aul >> 1: ppc_lswx(cpu); break;
            case 0x00000400ul >> 1: ppc_mcrxr(cpu); break;
            case 0x000003e4ul >> 1: ppc_slbia(cpu); break;
            case 0x000003b8ul >> 1: ppc_nand(cpu); break;
            case 0x000003a6ul >> 1: ppc_mtspr(cpu); break;
            case 0x00000378ul >> 1: ppc_or(cpu); break;
            case 0x0000036eul >> 1: ppc_sthux(cpu); break;
            case 0x0000036cul >> 1: ppc_ecowx(cpu); break;
            case 0x00000364ul >> 1: ppc_slbie(cpu); break;
            case 0x00000338ul >> 1: ppc_orc(cpu); break;
            case 0x0000046cul >> 1: ppc_tlbsync(cpu); break;
            case 0x00000324ul >> 1: ppc_slbmte(cpu); break;
            case 0x000002eeul >> 1: ppc_lhaux(cpu); break;
            case 0x000002eaul >> 1: ppc_lwaux(cpu); break;
            case 0x000002e6ul >> 1: ppc_mftb(cpu); break;
            case 0x000002e4ul >> 1: ppc_tlbia(cpu); break;
            case 0x000002aeul >> 1: ppc_lhax(cpu); break;
            case 0x000002aaul >> 1: ppc_lwax(cpu); break;
            case 0x000002a6ul >> 1: ppc_mfspr(cpu); break;
            case 0x00000278ul >> 1: ppc_xor(cpu); break;
            case 0x0000026eul >> 1: ppc_lhzux(cpu); break;
            case 0x0000026cul >> 1: ppc_eciwx(cpu); break;
            case 0x00000000ul >> 1: ppc_cmp(cpu); break;
            case 0x00000238ul >> 1: ppc_eqv(cpu); break;
            case 0x000005eeul >> 1: ppc_stfdux(cpu); break;
            case 0x000007ecul >> 1: ppc_dcbz(cpu); break;
            case 0x000007b4ul >> 1: ppc_extsw(cpu); break;
            case 0x000007aeul >> 1: ppc_stfiwx(cpu); break;
            case 0x000007acul >> 1: ppc_icbi(cpu); break;
            case 0x00000774ul >> 1: ppc_extsb(cpu); break;
            case 0x00000734ul >> 1: ppc_extsh(cpu); break;
            case 0x0000072cul >> 1: ppc_sthbrx(cpu); break;
            case 0x00000726ul >> 1: ppc_slbmfee(cpu); break;
            case 0x000006acul >> 1: ppc_eieio(cpu); break;
            case 0x000006a6ul >> 1: ppc_slbmfev(cpu); break;
            case 0x00000670ul >> 1: ppc_srawi(cpu); break;
            case 0x00000634ul >> 1: ppc_srad(cpu); break;
            case 0x00000630ul >> 1: ppc_sraw(cpu); break;
            case 0x0000062cul >> 1: ppc_lhbrx(cpu); break;
            case 0x00000264ul >> 1: ppc_tlbie(cpu); break;
            case 0x000005aeul >> 1: ppc_stfdx(cpu); break;
            case 0x000005aaul >> 1: ppc_stswi(cpu); break;
            case 0x0000056eul >> 1: ppc_stfsux(cpu); break;
            case 0x0000052eul >> 1: ppc_stfsx(cpu); break;
            case 0x0000052cul >> 1: ppc_stwbrx(cpu); break;
            case 0x0000052aul >> 1: ppc_stswx(cpu); break;
            case 0x00000526ul >> 1: ppc_mfsrin(cpu); break;
            case 0x000004eeul >> 1: ppc_lfdux(cpu); break;
            case 0x000004aeul >> 1: ppc_lfdx(cpu); break;
            case 0x000004acul >> 1: ppc_sync(cpu); break;
            case 0x000004aaul >> 1: ppc_lswi(cpu); break;
            case 0x000004a6ul >> 1: ppc_mfsr(cpu); break;
            case 0x0000046eul >> 1: ppc_lfsux(cpu); break;
            case 0x0000006cul >> 1: ppc_dcbst(cpu); break;
            case 0x00000120ul >> 1: ppc_mtocrf(cpu); break;
            case 0x00000120ul >> 1: ppc_mtcrf(cpu); break;
            case 0x0000022eul >> 1: ppc_lhzx(cpu); break;
            case 0x000000f4ul >> 1: ppc_popcntb(cpu); break;
            case 0x000000eeul >> 1: ppc_lbzux(cpu); break;
            case 0x000000aeul >> 1: ppc_lbzx(cpu); break;
            case 0x000000acul >> 1: ppc_dcbf(cpu); break;
            case 0x000000a8ul >> 1: ppc_ldarx(cpu); break;
            case 0x000000a6ul >> 1: ppc_mfmsr(cpu); break;
            case 0x00000088ul >> 1: ppc_td(cpu); break;
            case 0x00000078ul >> 1: ppc_andc(cpu); break;
            case 0x00000074ul >> 1: ppc_cntlzd(cpu); break;
            case 0x0000006eul >> 1: ppc_lwzux(cpu); break;
            case 0x000000f8ul >> 1: ppc_nor(cpu); break;
            case 0x0000006aul >> 1: ppc_ldux(cpu); break;
            case 0x00000040ul >> 1: ppc_cmpl(cpu); break;
            case 0x00000038ul >> 1: ppc_and(cpu); break;
            case 0x00000036ul >> 1: ppc_sld(cpu); break;
            case 0x00000034ul >> 1: ppc_cntlzw(cpu); break;
            case 0x00000030ul >> 1: ppc_slw(cpu); break;
            case 0x0000002eul >> 1: ppc_lwzx(cpu); break;
            case 0x0000002aul >> 1: ppc_ldx(cpu); break;
            case 0x00000028ul >> 1: ppc_lwarx(cpu); break;
            case 0x00000026ul >> 1: ppc_mfocrf(cpu); break;
            case 0x00000026ul >> 1: ppc_mfcr(cpu); break;
            case 0x00000008ul >> 1: ppc_tw(cpu); break;
            case 0x0000022cul >> 1: ppc_dcbt(cpu); break;
            case 0x000001aeul >> 1: ppc_stbx(cpu); break;
            case 0x000001acul >> 1: ppc_stdcx_u(cpu); break;
            case 0x000001a4ul >> 1: ppc_mtsr(cpu); break;
            case 0x000001e4ul >> 1: ppc_mtsrin(cpu); break;
            case 0x0000016eul >> 1: ppc_stwux(cpu); break;
            case 0x0000016aul >> 1: ppc_stdux(cpu); break;
            case 0x000001ecul >> 1: ppc_dcbtst(cpu); break;
            case 0x000001eeul >> 1: ppc_stbux(cpu); break;
            case 0x00000164ul >> 1: ppc_mtmsrd(cpu); break;
            case 0x0000012eul >> 1: ppc_stwx(cpu); break;
            case 0x0000012cul >> 1: ppc_stwcx_u(cpu); break;
            case 0x0000012aul >> 1: ppc_stdx(cpu); break;
            case 0x00000124ul >> 1: ppc_mtmsr(cpu); break;
        }
        switch ((cpu->opcode & 0x000007fc) >> 2) {
            case 0x00000674ul >> 2: ppc_sradi(cpu); break;
        }
        switch ((cpu->opcode & 0x000003fe) >> 1) {
            case 0x00000214ul >> 1: ppc_add(cpu); break;
            case 0x00000016ul >> 1: ppc_mulhwu(cpu); break;
            case 0x00000014ul >> 1: ppc_addc(cpu); break;
            case 0x000001d6ul >> 1: ppc_mullw(cpu); break;
            case 0x00000012ul >> 1: ppc_mulhdu(cpu); break;
            case 0x000001d4ul >> 1: ppc_addme(cpu); break;
            case 0x00000010ul >> 1: ppc_subfc(cpu); break;
            case 0x00000194ul >> 1: ppc_addze(cpu); break;
            case 0x000001d2ul >> 1: ppc_mulld(cpu); break;
            case 0x000001d0ul >> 1: ppc_subfme(cpu); break;
            case 0x00000050ul >> 1: ppc_subf(cpu); break;
            case 0x000003d6ul >> 1: ppc_divw(cpu); break;
            case 0x00000190ul >> 1: ppc_subfze(cpu); break;
            case 0x00000092ul >> 1: ppc_mulhd(cpu); break;
            case 0x00000096ul >> 1: ppc_mulhw(cpu); break;
            case 0x00000392ul >> 1: ppc_divdu(cpu); break;
            case 0x000000d0ul >> 1: ppc_neg(cpu); break;
            case 0x00000396ul >> 1: ppc_divwu(cpu); break;
            case 0x00000110ul >> 1: ppc_subfe(cpu); break;
            case 0x00000114ul >> 1: ppc_adde(cpu); break;
            case 0x000003d2ul >> 1: ppc_divd(cpu); break;
        }
    } break;
    case 0x80000000ul >> 26: ppc_lwz(cpu); break;
    case 0x84000000ul >> 26: ppc_lwzu(cpu); break;
    case 0x88000000ul >> 26: ppc_lbz(cpu); break;
    case 0x8c000000ul >> 26: ppc_lbzu(cpu); break;
    case 0x90000000ul >> 26: ppc_stw(cpu); break;
    case 0x94000000ul >> 26: ppc_stwu(cpu); break;
    case 0x98000000ul >> 26: ppc_stb(cpu); break;
    case 0x9c000000ul >> 26: ppc_stbu(cpu); break;
    case 0xa0000000ul >> 26: ppc_lhz(cpu); break;
    case 0xa4000000ul >> 26: ppc_lhzu(cpu); break;
    case 0xa8000000ul >> 26: ppc_lha(cpu); break;
    case 0xac000000ul >> 26: ppc_lhau(cpu); break;
    case 0xb0000000ul >> 26: ppc_sth(cpu); break;
    case 0xb4000000ul >> 26: ppc_sthu(cpu); break;
    case 0xb8000000ul >> 26: ppc_lmw(cpu); break;
    case 0xbc000000ul >> 26: ppc_stmw(cpu); break;
    case 0xc0000000ul >> 26: ppc_lfs(cpu); break;
    case 0xc4000000ul >> 26: ppc_lfsu(cpu); break;
    case 0xc8000000ul >> 26: ppc_lfd(cpu); break;
    case 0xcc000000ul >> 26: ppc_lfdu(cpu); break;
    case 0xd0000000ul >> 26: ppc_stfs(cpu); break;
    case 0xd4000000ul >> 26: ppc_stfsu(cpu); break;
    case 0xd8000000ul >> 26: ppc_stfd(cpu); break;
    case 0xdc000000ul >> 26: ppc_stfdu(cpu); break;
    case 0xe8000000ul >> 26: {
        switch ((cpu->opcode & 0x00000003) >> 0) {
            case 0x00000000ul >> 0: ppc_ld(cpu); break;
            case 0x00000001ul >> 0: ppc_ldu(cpu); break;
            case 0x00000002ul >> 0: ppc_lwa(cpu); break;
        }
    } break;
    case 0xec000000ul >> 26: {
        switch ((cpu->opcode & 0x0000003e) >> 1) {
            case 0x00000024ul >> 1: ppc_fdivs(cpu); break;
            case 0x00000028ul >> 1: ppc_fsubs(cpu); break;
            case 0x0000002aul >> 1: ppc_fadds(cpu); break;
            case 0x0000002cul >> 1: ppc_fsqrts(cpu); break;
            case 0x00000030ul >> 1: ppc_fres(cpu); break;
            case 0x00000032ul >> 1: ppc_fmuls(cpu); break;
            case 0x00000034ul >> 1: ppc_frsqrtes(cpu); break;
            case 0x00000038ul >> 1: ppc_fmsubs(cpu); break;
            case 0x0000003aul >> 1: ppc_fmadds(cpu); break;
            case 0x0000003cul >> 1: ppc_fnmsubs(cpu); break;
            case 0x0000003eul >> 1: ppc_fnmadds(cpu); break;
        }
    } break;
    case 0xf8000000ul >> 26: {
        switch ((cpu->opcode & 0x00000003) >> 0) {
            case 0x00000000ul >> 0: ppc_std(cpu); break;
            case 0x00000001ul >> 0: ppc_stdu(cpu); break;
        }
    } break;
    case 0xfc000000ul >> 26: {
        switch ((cpu->opcode & 0x000007fe) >> 1) {
            case 0x00000018ul >> 1: ppc_frsp(cpu); break;
            case 0x0000069cul >> 1: ppc_fcfid(cpu); break;
            case 0x0000065eul >> 1: ppc_fctidz(cpu); break;
            case 0x0000065cul >> 1: ppc_fctid(cpu); break;
            case 0x0000058eul >> 1: ppc_mtfsf(cpu); break;
            case 0x0000048eul >> 1: ppc_mffs(cpu); break;
            case 0x00000210ul >> 1: ppc_fabs(cpu); break;
            case 0x00000110ul >> 1: ppc_fnabs(cpu); break;
            case 0x0000010cul >> 1: ppc_mtfsfi(cpu); break;
            case 0x00000090ul >> 1: ppc_fmr(cpu); break;
            case 0x0000008cul >> 1: ppc_mtfsb0(cpu); break;
            case 0x00000080ul >> 1: ppc_mcrfs(cpu); break;
            case 0x00000050ul >> 1: ppc_fneg(cpu); break;
            case 0x0000004cul >> 1: ppc_mtfsb1(cpu); break;
            case 0x00000040ul >> 1: ppc_fcmpo(cpu); break;
            case 0x0000001eul >> 1: ppc_fctiwz(cpu); break;
            case 0x0000001cul >> 1: ppc_fctiw(cpu); break;
            case 0x00000000ul >> 1: ppc_fcmpu(cpu); break;
        }
        switch ((cpu->opcode & 0x0000003e) >> 1) {
            case 0x0000003eul >> 1: ppc_fnmadd(cpu); break;
            case 0x0000003cul >> 1: ppc_fnmsub(cpu); break;
            case 0x0000003aul >> 1: ppc_fmadd(cpu); break;
            case 0x00000038ul >> 1: ppc_fmsub(cpu); break;
            case 0x00000034ul >> 1: ppc_frsqrte(cpu); break;
            case 0x00000032ul >> 1: ppc_fmul(cpu); break;
            case 0x00000030ul >> 1: ppc_fre(cpu); break;
            case 0x0000002eul >> 1: ppc_fsel(cpu); break;
            case 0x0000002cul >> 1: ppc_fsqrt(cpu); break;
            case 0x0000002aul >> 1: ppc_fadd(cpu); break;
            case 0x00000028ul >> 1: ppc_fsub(cpu); break;
            case 0x00000024ul >> 1: ppc_fdiv(cpu); break;
        }
    } break;
}