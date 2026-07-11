#!/bin/bash
# Full analysis: NEW/DRIFT tagging + present-elsewhere classification. LC_ALL=C everywhere.
set -e
export LC_ALL=C
SP=/private/tmp/claude-501/-Volumes-ExData-game-jx3/4aecc4f7-3d4e-4ee1-ac60-0bdf28a2a840/scratchpad
cd "$SP"

comm -23 v25_members.c.txt ours_members.c.txt > missing_members.txt
comm -23 v25_classes.c.txt  ours_classes.c.txt  > absent_classes.txt

# present-elsewhere classification (fast: use grep -Ff against index files)
# PRESENT_EXACT: full Class::method in source
grep -Fxf src_qualified.txt missing_members.txt | sort -u > pe_exact.txt || true
# remaining
comm -23 missing_members.txt pe_exact.txt > rem1.txt
# NAME_ELSEWHERE: bare name in source barenames
awk -F'::' '{print $NF"\t"$0}' rem1.txt | sort > rem1_bare.tsv
join -t$'\t' -1 1 -2 1 <(sort src_barenames.txt | awk '{print $1"\tX"}') rem1_bare.tsv 2>/dev/null | cut -f1,3 > /dev/null || true
# simpler: line-by-line but vectorized via grep
: > pe_name.txt; : > truly.txt
while IFS= read -r m; do
  bare="${m##*::}"
  if grep -qxF "$bare" src_barenames.txt; then echo "$m" >> pe_name.txt; else echo "$m" >> truly.txt; fi
done < rem1.txt

echo "PRESENT_EXACT   = $(wc -l < pe_exact.txt)"
echo "NAME_ELSEWHERE  = $(wc -l < pe_name.txt)"
echo "TRULY_MISSING   = $(wc -l < truly.txt)"

# Build master TSV: category  tag(NEW/DRIFT)  class  member
build() { # $1=file $2=category
  awk -F'\t' -v cat="$2" 'NR==FNR{new[$1]=1;next}{m=$0;cls=m;sub(/::.*/,"",cls);tag=(cls in new)?"NEW":"DRIFT";print cat"\t"tag"\t"cls"\t"m}' absent_classes.txt "$1"
}
{ build pe_exact.txt PRESENT_EXACT; build pe_name.txt NAME_ELSEWHERE; build truly.txt TRULY_MISSING; } > missing_full.tsv
echo "=== master rows: $(wc -l < missing_full.tsv) ==="
