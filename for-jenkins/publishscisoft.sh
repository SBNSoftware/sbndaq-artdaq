#!/bin/bash
scratch_dir=${HOME}/tmp

function publishscisoft(){
  local ts=$(date -d "today" +"%Y%m%d%H%M%S")
  local tmp_packages="${scratch_dir}/publish-${ts}"

  mkdir -p ${tmp_packages} || exit 1
  cd  ${tmp_packages}

  curl -O https://scisoft.fnal.gov/scisoft/bundles/tools/copyFromJenkins
  chmod a+x copyFromJenkins

  curl -O https://scisoft.fnal.gov/scisoft/bundles/tools/copyToSciSoft
  chmod a+x copyToSciSoft

 ./copyFromJenkins \
-m SLF7 \
-q s97-e19 \
-q s97-py2-e19 \
 sbndaq_artdaq-build


find ./ -name "*bz2" |xargs -n 1 -I {} ./copyToSciSoft --node scisoftportal.fnal.gov {}
}

publishscisoft
