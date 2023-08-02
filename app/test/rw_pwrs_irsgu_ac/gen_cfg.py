#!/usr/bin/python3

num_of_cu = 4

fin = open("template.cfg", "rt")
fout = open("kernel.cfg", "wt")

for line in fin:
    test_nk =line.replace('nk=', '!nk=')
    if(test_nk != line):
        fout.write(line.replace(':*', ':' + str(num_of_cu)))
    else :
        test_sc = line.replace('stream_connect=', '!stream_connect=')
        if (test_sc != line):
            for i in range(num_of_cu):
                fout.write(line.replace('_1.', '_' + str(i + 1) + '.'))
        else :
            test_slr = line.replace('slr=', '!slr=')
            if (test_slr != line):
                for i in range(num_of_cu):
                    replace_slr =  line.replace('SLR0', 'SLR' + str(i))
                    new_slr = replace_slr.replace('_1:', '_' + str(i + 1) + ':')
                    fout.write(new_slr);

            else :
                test_sp = line.replace('sp=', '!sp=')
                if (test_sp != line):
                    for i in range(num_of_cu):
                        replace_ddr =  line.replace('DDR[0]', 'DDR[' + str(i) + ']')
                        new_ddr = replace_ddr.replace('_1.', '_' + str(i + 1) + '.')
                        fout.write(new_ddr);
                else :
                    fout.write(line);



fin.close()

fout.close()