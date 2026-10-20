#ifndef CP_COMPONENTS_FPS_ASM_HPP
#define CP_COMPONENTS_FPS_ASM_HPP

#define CP_FPS_ASM_ADD(a,b) "add " #a ", " #b "\n\t"
#define CP_FPS_ASM_AND(a,b) "and " #a ", " #b "\n\t"
#define CP_FPS_ASM_CMP(a,b) "cmp " #a ", " #b "\n\t"
#define CP_FPS_ASM_INC(a) "inc " #a "\n\t"
#define CP_FPS_ASM_JNE(a) "jne " #a "\n\t"
#define CP_FPS_ASM_LEA(a,b) "lea " #a ", " #b "\n\t"
#define CP_FPS_ASM_MOV(a,b) "mov " #a ", " #b "\n\t"
#define CP_FPS_ASM_VBROADCASTSS(a,b) "vbroadcastss " #a ", " #b "\n\t"
#define CP_FPS_ASM_VMOVDQA(a,b) "vmovdqa " #a ", " #b "\n\t"
#define CP_FPS_ASM_VMOVDQU(a,b) "vmovdqu " #a ", " #b "\n\t"
#define CP_FPS_ASM_VPADDD(a,b,c) "vpaddd " #a ", " #b ", " #c "\n\t"
#define CP_FPS_ASM_VPADDQ(a,b,c) "vpaddq " #a ", " #b ", " #c "\n\t"
#define CP_FPS_ASM_VPBLENDD(a,b,c,d) "vpblendd " #a ", " #b ", " #c ", " #d "\n\t"
#define CP_FPS_ASM_VPBROADCASTD(a,b) "vpbroadcastd " #a ", " #b "\n\t"
#define CP_FPS_ASM_VPMINUD(a,b,c) "vpminud " #a ", " #b ", " #c "\n\t"
#define CP_FPS_ASM_VPMULLD(a,b,c) "vpmulld " #a ", " #b ", " #c "\n\t"
#define CP_FPS_ASM_VPMULUDQ(a,b,c) "vpmuludq " #a ", " #b ", " #c "\n\t"
#define CP_FPS_ASM_VPSRLQ(a,b,c) "vpsrlq " #a ", " #b ", " #c "\n\t"
#define CP_FPS_ASM_VPSUBD(a,b,c) "vpsubd " #a ", " #b ", " #c "\n\t"

#endif
