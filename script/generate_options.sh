#!/bin/sh

root=`dirname $0`

LONG=0
OBJECTPOINT=10000
FUNCTIONPOINT=20000
OFF_T=30000

generate() {
  name=$1
  pattern=$2
  prefix=$3
  echo "generate $root/$name.js"
  (
    echo "var $name = {"
    cat /usr/include/curl/curl.h|perl -ne "/$pattern/i && print \"  '\$1': ${OBJECTPOINT} + \$2,\n\""
    echo '}'
  ) > $root/$name.js
}
#generate integer_options 'CINIT\((\w+).*LONG' OPT
generate string_options  'CINIT\((\w+).*OBJECTPOINT, (\d+)' OPT

#generate integer_infos 'CURLINFO_(\w+).*LONG' INFO
#generate string_infos 'CURLINFO_(\w+).*STRING' INFO
#generate double_infos 'CURLINFO_(\w+).*DOUBLE' INFO
