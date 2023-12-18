#!/usr/bin/python3

# SPDX-FileCopyrightText: 2023 Mete Balci
#
# SPDX-License-Identifier: Apache-2.0

import os
import sys

def main():
    msg = None
    md = None
    fname = sys.argv[1]
    fbasename = os.path.basename(fname)
    hname = '%s.h' % fbasename
    macro = hname.replace('.', '_').upper()
    vname = fbasename.replace('.', '_').lower()
    print('#ifndef __%s__' % macro)
    print('#define __%s__' % macro)
    print()
    print('static const char* %s[] = {' % vname)
    with open(fname, 'r') as fp:
        while True:
            line = fp.readline()
            if len(line) == 0:
                break
            line = line.strip()
            if len(line) == 0:
                continue
            elif line.startswith('#'):
                continue
            elif line.startswith('['):
                continue
            else:
                parts = line.split('=')
                if parts[0].strip() == 'Len':
                    length = int(parts[1].strip())
                if parts[0].strip() == 'Msg':
                    msg = parts[1].strip()
                elif parts[0].strip() == 'MD':
                    md = parts[1].strip()
                    if ((length is not None) and
                        (msg is not None) and
                        (md is not None)):
                        msg = msg[0:4*length]
                        print('\t"%s",' % msg)
                        print('\t"%s",' % md)
                        length = None
                        msg = None
                        md = None
    print('\tNULL');
    print('};')
    print('#endif // __%s__' % macro)

if __name__ == '__main__':
    main()
