echo "recurence second"
echo "calling rec1:"
./bin/debugger ./test/rec1.sh
echo "now calling simple test:"
./bin/debugger ./test/simple.sh
echo "ending"
>&2 echo "errorringg"