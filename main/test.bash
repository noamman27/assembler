rm -r outputs/1
mkdir outputs/1
rm -r outputs/2
mkdir outputs/2
rm -r outputs/3
mkdir outputs/3
rm -r outputs/4
mkdir outputs/4
rm -r outputs/5
mkdir outputs/5
rm -r outputs/6
mkdir outputs/6

./assembler tests/test1.as
./assembler tests/test2.as
./assembler tests/test3.af 2> outputs/3/output3.txt
./assembler tests/test4.as 2> outputs/4/output4.txt
./assembler tests/test5.as 2> outputs/5/output5.txt
./assembler tests/test6.as 2> outputs/6/output6.txt

mv tests/test1.am outputs/1
mv tests/test1.ent outputs/1
mv tests/test1.ob outputs/1
mv tests/test1.ext outputs/1

mv tests/test2.am outputs/2
mv tests/test2.ent outputs/2
mv tests/test2.ob outputs/2

rm tests/test4.am
rm tests/test5.am
rm tests/test6.am