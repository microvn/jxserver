#!/usr/bin/env bash
# Extract normalized project symbols from a non-stripped ELF binary.
# Output: two files given as $2 (members: Class::method) and $3 (classes)
# Method: text-defined _ZN (nested-name mangled = has class scope) -> demangle
#         -> strip args (cut at first '(') -> drop noise -> sort -u
set -euo pipefail
if [ "$#" -ne 3 ]; then
  echo "usage: extract_symbols.sh <ELF> <members-out> <classes-out>" >&2
  exit 2
fi
B="$1"; OUT_MEMBERS="$2"; OUT_CLASSES="$3"
# LLVM c++filt (macOS) strips leading underscore by default -> need -n; GNU (Linux) does not.
if c++filt --help 2>&1 | grep -q 'LLVM'; then CXXFILT="c++filt -n -p"; else CXXFILT="c++filt -p"; fi
nm "$B" 2>/dev/null \
  | awk '($2=="T"||$2=="t") && $3 ~ /^_ZN/ {print $3}' \
  | $CXXFILT \
  | grep -v -E '^std::|^__gnu|^__cxx|^_|^operator|<|\(anonymous|\.cold|\.part|\.constprop|\.isra' \
  | grep '::' \
  | sort -u > "$OUT_MEMBERS"
# top-level class = token before first ::
sed 's/::.*//' "$OUT_MEMBERS" | grep -v -E '^std|^__' | sort -u > "$OUT_CLASSES"
echo "members=$(wc -l < "$OUT_MEMBERS") classes=$(wc -l < "$OUT_CLASSES")"
