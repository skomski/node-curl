#!/bin/sh

root=`dirname $0`

OPT_LONG=0
OPT_OBJECTPOINT=10000
OPT_OFF_T=30000

INFO_STRING=0x100000
INFO_LONG=0x200000
INFO_DOUBLE=0x300000

generate() {
  name=$1
  pattern=$2
  prefix=$3
  echo "generate $root/$name.js"
  (
    echo "var $name = {"
    cat /usr/include/curl/curl.h|perl -ne "/$pattern/i && print \"  '\$1': $3 + \$2,\n\""
    echo '}'
  ) > $root/$name.js
}
generate integer_options 'CINIT\((\w+).*LONG, (\d+)' ${OPT_LONG}
generate string_options  'CINIT\((\w+).*OBJECTPOINT, (\d+)' ${OPT_OBJECTPOINT}

generate integer_infos 'CURLINFO_(\w+).*LONG \+ (\d+)' ${INFO_LONG}
generate string_infos 'CURLINFO_(\w+).*STRING \+ (\d+)' ${INFO_STRING}
generate double_infos 'CURLINFO_(\w+).*DOUBLE \+ (\d+)' ${INFO_DOUBLE}
