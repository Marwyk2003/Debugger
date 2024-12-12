echo "recurence second"
echo "calling rec1:"
$DEBUG ./test/simple_tests/rec1.sh
echo "now calling simple test:"
$DEBUG ./test/simple_tests/simple.sh
echo "ending"
>&2 echo "errorringg"
