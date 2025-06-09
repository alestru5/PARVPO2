cd ./hg
# Modify even files: add line "// changed"
for idx in $(seq -w 2 2 10000); do
  echo "// changed $(date +%s)" >> "source${idx}.dat"
done
