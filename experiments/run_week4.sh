# AI generated script

set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
[[ -x ./simulator ]] || make

OUT="$ROOT/experiments/results.txt"
rm -f "$OUT"

run() {
  echo "" >> "$OUT"
  echo "######## $1 ########" >> "$OUT"
  shift
  ./simulator "$@" >> "$OUT" 2>&1
}

S=(--seed 42 --duration 800 --cost-rate 0.001 --servers 2)

for bal in rr lc; do
  run "low constant $bal"        "${S[@]}" --balancer "$bal" --arrival-mode constant --arrival-rate 0.8
  run "high constant $bal"       "${S[@]}" --balancer "$bal" --arrival-mode constant --arrival-rate 4.5
  run "burst $bal"               "${S[@]}" --balancer "$bal" --arrival-mode burst --arrival-rate 2.5
  run "sine $bal"                "${S[@]}" --balancer "$bal" --arrival-mode sine --arrival-rate 3 --arrival-period 150
done

for bal in rr lc; do
  run "scale aggressive $bal"    "${S[@]}" --balancer "$bal" --arrival-mode constant --arrival-rate 4.5 \
    --scale-up 3 --scale-down 1 --cooldown 15
  run "scale conservative $bal"  "${S[@]}" --balancer "$bal" --arrival-mode constant --arrival-rate 4.5 \
    --scale-up 10 --scale-down 2 --cooldown 30
  run "scale fast cooldown $bal" "${S[@]}" --balancer "$bal" --arrival-mode constant --arrival-rate 4.5 \
    --scale-up 5 --scale-down 1 --cooldown 8
done

echo "Wrote $OUT"
