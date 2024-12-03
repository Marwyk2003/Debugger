echo "recurence second"
echo "calling rec1:"
$DEBUG ./test/rec1.sh
echo "now calling simple test:"
$DEBUG ./test/simple.sh
echo "ending"
>&2 echo "errorringg"