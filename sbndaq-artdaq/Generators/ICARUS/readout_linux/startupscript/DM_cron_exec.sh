#!/bin/bash -l

#DC ONLINE
export DCONLINE_PATH="/nfs/dconline/myOnline"
export PATH=${PATH}:${DCONLINE_PATH}/bin
export DOGS_PATH=/local/home/dconline/myDOGS
#
export USER=dconline

if [ -d "$DCONLINE_PATH/DCDataMigro/scripts" ]
then
date
#
# Transfer BIN
$DCONLINE_PATH/DCDataMigro/scripts/runTransferOV &> /dev/null
# Delete BIN
$DCONLINE_PATH/DCDataMigro/scripts/runDeleteOV &> /dev/null
else
  exit 0
fi

exit 0
