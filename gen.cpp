#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdint>

enum : int {
    PPC_F_I,
    PPC_F_B,
    PPC_F_SC,
    PPC_F_D,
    PPC_F_DS,
    PPC_F_X,
    PPC_F_XL,
    PPC_F_XFX,
    PPC_F_XFL,
    PPC_F_XS,
    PPC_F_XO,
    PPC_F_A,
    PPC_F_M,
    PPC_F_MD,
    PPC_F_MDS
};

#define PPC_XOM_NONE 0x00000000
#define PPC_XOM_DS   0x00000003
#define PPC_XOM_X    0x000007fe
#define PPC_XOM_XL   0x000007fe
#define PPC_XOM_XFX  0x000007fe
#define PPC_XOM_XFL  0x000007fe
#define PPC_XOM_XS   0x000007fc
#define PPC_XOM_XO   0x000003fe
#define PPC_XOM_A    0x0000003e
#define PPC_XOM_MD   0x0000001c
#define PPC_XOM_MDS  0x0000001e

static const uint32_t ppc_xom[] = {
    PPC_XOM_NONE,
    PPC_XOM_NONE,
    PPC_XOM_NONE,
    PPC_XOM_NONE,
    PPC_XOM_DS,
    PPC_XOM_X,
    PPC_XOM_XL,
    PPC_XOM_XFX,
    PPC_XOM_XFL,
    PPC_XOM_XS,
    PPC_XOM_XO,
    PPC_XOM_A,
    PPC_XOM_NONE,
    PPC_XOM_MD,
    PPC_XOM_MDS
};

static const uint32_t ppc_xop[] = {
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 2,
    1, 1, 0, 2, 1
};

static std::unordered_map <std::string, int> ppc_format_map = {
    { "I"  , PPC_F_I   },
    { "B"  , PPC_F_B   },
    { "SC" , PPC_F_SC  },
    { "D"  , PPC_F_D   },
    { "DS" , PPC_F_DS  },
    { "X"  , PPC_F_X   },
    { "XL" , PPC_F_XL  },
    { "XFX", PPC_F_XFX },
    { "XFL", PPC_F_XFL },
    { "XS" , PPC_F_XS  },
    { "XO" , PPC_F_XO  },
    { "A"  , PPC_F_A   },
    { "M"  , PPC_F_M   },
    { "MD" , PPC_F_MD  },
    { "MDS", PPC_F_MDS }
};

std::vector <uint32_t> get_contiguous_masks(uint32_t m) {
    std::vector <uint32_t> masks;

    uint32_t mask = 0;

    for (int i = 0; i < 32;) {
        int b = m & (1 << (31 - i));

        if (b) {
            while (b && (i < 32)) {
                mask |= b;

                b = m & (1 << (31 - ++i));
            }

            masks.push_back(mask);

            mask = 0;
        } else {
            ++i;
        }
    }

    return masks;
}

struct ppc_instruction {
    std::string fmt;
    uint32_t primary;
    uint32_t extend;
    std::string mode;
    std::string mnemonic;
    std::string description;
    std::string page;
};

struct ppc_decode_info {
    uint32_t primary;
    uint32_t xom;
    uint32_t xop;
    std::string name;
};

int main() {
    std::ifstream file("ppc.txt");
    std::vector <std::string> parts;
    std::vector <ppc_instruction> instructions;

    char c;

    while (!file.eof()) {
        ppc_instruction i;
        std::string line;
        std::string buf;
        std::getline(file, line);
        std::istringstream ss(line);

        while (std::getline(ss, buf, ' ')) {
            parts.push_back(buf);

            if (islower(buf[0]))
                break;

            buf.clear();
        }

        std::getline(ss, i.description);

        i.fmt = parts[0];
        i.primary = std::stoul(parts[1]);
        i.mnemonic = parts.back();
        i.page = parts.at(parts.size() - 2);

        // Get rid of form, primary, page/book and mnemonic
        parts.erase(std::begin(parts), std::begin(parts) + 2);
        parts.erase(std::end(parts) - 2, std::end(parts));

        switch (parts.size()) {
            case 0:
                i.extend = -1;
            break;

            case 1:
                if (isdigit(parts[0][0])) {
                    i.extend = std::stoul(parts[0]);
                } else {
                    i.extend = -1;
                    i.mode = parts[0];
                }
            break;

            case 2:
                i.extend = std::stoul(parts[0]);
                i.mode = parts[1];
            break;
        }

        // printf("%-3s %-2d %-4d %-2s %-12s %s\n",
        //     i.fmt.c_str(),
        //     i.primary,
        //     i.extend,
        //     i.mode.size() ? i.mode.c_str() : "-",
        //     i.mnemonic.c_str(),
        //     i.description.c_str()
        // );

        instructions.push_back(i);

        parts.clear();
    }

    std::vector <ppc_decode_info> decode_table[64];

    for (ppc_instruction& i : instructions) {
        uint32_t mask = 0xfc000000;
        uint32_t match = i.primary << 26;
        int form = ppc_format_map[i.fmt];

        // if ((int)i.extend != -1) {
        //     mask |= ppc_xom[form];
        //     match |= i.extend << ppc_xop[form];

        //     if ((i.extend << ppc_xop[form]) & ~ppc_xom[form]) {
        //         printf("ERROR! XO ov fmt=%s code=%08x (%d) mask=%08x pos=%d\n",
        //             i.fmt.c_str(),
        //             i.extend << ppc_xop[form], i.extend << ppc_xop[form],
        //             ppc_xom[form],
        //             ppc_xop[form]
        //         );
        //     }
        // }

        std::string name = i.mnemonic;

        name = name.substr(0, name.find('['));

        auto dot = name.find('.');

        if (dot != std::string::npos)
            name = name.replace(dot, 1, "_u");

        decode_table[i.primary].push_back({ i.primary << 26, ppc_xom[form], i.extend << ppc_xop[form], name });
    }

    printf("switch ((cpu->opcode & 0xfc000000) >> 26) {\n");

    for (auto& v : decode_table) {
        if (!v.size())
            continue;

        if (v.size() == 1) {
            printf("    case 0x%08xul >> 26: ppc_%s(cpu); break;\n", v[0].primary, v[0].name.c_str());
        } else {
            printf("    case 0x%08xul >> 26: {\n", v[0].primary);

            std::sort(v.begin(), v.end(), [](ppc_decode_info a, const ppc_decode_info& b) -> bool {
                return a.xom > b.xom;
            });

            uint32_t xom = v[0].xom;
            int ffs = __builtin_ffs(xom) - 1;

            printf("        switch ((cpu->opcode & 0x%08x) >> %d) {\n", xom, ffs);

            for (const ppc_decode_info& d : v) {
                if (d.xom != xom) {
                    xom = d.xom;
                    ffs = __builtin_ffs(xom) - 1;

                    printf("        }\n        switch ((cpu->opcode & 0x%08x) >> %d) {\n", xom, ffs);
                }

                printf("            case 0x%08xul >> %d: ppc_%s(cpu); break;\n",
                    d.xop, ffs, d.name.c_str()
                );
            }

            printf("        }\n    } break;\n");
        }
    }

    printf("}\n");
}