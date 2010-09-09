#!/usr/bin/env bash

# Goto sources directory
cd .. >/dev/null

# Get svn information
svn info > svnInfo.txt

# Don´t abort on errors
set +e >/dev/null

# Run FEM benchmarks
cd build_fem
ctest -R 'NB|MCGB|FS|AKS|YW|UJG|MDL|NW|WW' -E 'FILE|EXCEED|thm_decov' -j 32 > ../benchOut.txt
ctest -R FILECOMPARE -E 'CB|HS|JOD|PCH|EXCEED|thm_decov' >> ../benchOut.txt
cd .. >/dev/null

# Run FEM_BRNS benchmarks
cd build_brns
ctest -R FC -E "FILE|EXCEED" -j 32 >> ../benchOut.txt
ctest -R FILECOMPARE -E "EXCEED" >> ../benchOut.txt
cd .. >/dev/null

# Print results
cat benchOut.txt

# Send emails on errors
python26 ~/bin/sendmail/sendmail.py

set -e >/dev/null
