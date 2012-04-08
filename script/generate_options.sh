#!/bin/sh

root=`dirname $0`

OPT_LONG=0
OPT_OBJECTPOINT=10000
OPT_OFF_T=30000

INFO_STRING=0x100000
INFO_LONG=0x200000
INFO_DOUBLE=0x300000
INFO_SLIST=0x400000

generate() {
  name=$1
  pattern=$2
  prefix=$3
  echo "generate $name"
  (
    echo "module.exports.$name = {"
    cat /usr/include/curl/curl.h|perl -ne "/$pattern/i && print \"  '\$1': $3 + \$2,\n\""
    echo '}'
  ) >> $root/curl_options.js
}

echo -n "" > $root/curl_options.js

generate offTOptions 'CINIT\((\w+).*OFF_T, (\d+)' ${OPT_OFF_T}
generate integerOptions 'CINIT\((\w+).*LONG, (\d+)' ${OPT_LONG}
generate stringOptions  'CINIT\((\w+).*OBJECTPOINT, (\d+)' ${OPT_OBJECTPOINT}

generate integerInfos 'CURLINFO_(\w+).*LONG.*\+ (\d+)' ${INFO_LONG}
generate stringInfos 'CURLINFO_(\w+).*STRING.*\+ (\d+)' ${INFO_STRING}
generate doubleInfos 'CURLINFO_(\w+).*DOUBLE.*\+ (\d+)' ${INFO_DOUBLE}
generate listInfos 'CURLINFO_(\w+).*SLIST.*\+ (\d+)' ${INFO_SLIST}
